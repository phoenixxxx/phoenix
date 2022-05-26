#include "Visualize.h"
#include <Window/Window.h>
#include <Utils/Math.h>
#include <Utils/Console.h>
#include <Utils/Utils.h>
#include <Utils/VectorMath.h>

#include <Window/Window.h>
#include <D3D11/D3D11Manager.h>
#include <D3D11LineRenderer/D3D11LineRenderer.h>
#include <Utils/D3D11imgui.h>
#include <Image/Image.h>

#include <Material/Material.h>
#include <Utils/CSVDocument.h>
#include <Utils/CPUTimer.h>
#include <Utils/BlockIO.h>
#include <map>
#include <filesystem>
#include <D3D11MeshRenderer/D3D11MeshRenderer.h>

#include <ThirdParty/tinyxml2/tinyxml2.h>

namespace Phoenix
{
	static std::filesystem::path GetFile(cstr_t filter, bool load=true)
	{
		stdstr_t fileName;

		OPENFILENAME ofn = { 0 };
		TCHAR szFile[260] = { 0 };
		// Initialize remaining fields of OPENFILENAME structure
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = 0;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;// "All\0*.*\0obj\0*.OBJ\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		if (load)
		{
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn) == TRUE)
			{
				fileName = ofn.lpstrFile;
			}
		}
		else
		{
			if (GetSaveFileName(&ofn) == TRUE)
			{
				fileName = ofn.lpstrFile;
			}
		}
		return fileName;
	}

	Visualize::Visualize()
	{
		mCamera.LookAt(float3(10, 10, 10), float3(0, 0, 0), float3(0, 0, 1));
		mMouseClickX = -1;
		mMouseClickY = -1;

		mHDRI.mAlbedo = 0.5f;
		mHDRI.mTurbidity = 10.0f;
		mHDRI.mElevation = PiOver4;
		mHDRI.mResolution = 512;

		mShowBVHLighting = false;

		mOptics.mFStop = 0;
		mOptics.mFocalLength = 16;
		mOptics.mSensorWidth = 36;//35mm (36x34)

		mSurface.mHit = false;
		mSurface.mRay.mDirection = float3(0, 0, 0);
	}

	cstr_t Visualize::Name()
	{
		return "Visualize";
	}

	Visualize::~Visualize()
	{
	}

#pragma region From https://github.com/tinyobjloader/tinyobjloader/blob/master/examples/viewer/viewer.cc
	bool hasSmoothingGroup(const tinyobj::shape_t& shape)
	{
		for (size_t i = 0; i < shape.mesh.smoothing_group_ids.size(); i++) {
			if (shape.mesh.smoothing_group_ids[i] > 0) {
				return true;
			}
		}
		return false;
	}
	static void CalcNormal(float N[3], float v0[3], float v1[3], float v2[3]) {
		float v10[3];
		v10[0] = v1[0] - v0[0];
		v10[1] = v1[1] - v0[1];
		v10[2] = v1[2] - v0[2];

		float v20[3];
		v20[0] = v2[0] - v0[0];
		v20[1] = v2[1] - v0[1];
		v20[2] = v2[2] - v0[2];

		N[0] = v10[1] * v20[2] - v10[2] * v20[1];
		N[1] = v10[2] * v20[0] - v10[0] * v20[2];
		N[2] = v10[0] * v20[1] - v10[1] * v20[0];

		float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
		if (len2 > 0.0f) {
			float len = sqrtf(len2);

			N[0] /= len;
			N[1] /= len;
			N[2] /= len;
		}
	}
	void computeSmoothingNormals(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape,
		std::map<int, float3>& smoothVertexNormals) {
		smoothVertexNormals.clear();
		std::map<int, float3>::iterator iter;

		for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
			// Get the three indexes of the face (all faces are triangular)
			tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
			tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
			tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];

			// Get the three vertex indexes and coordinates
			int vi[3];      // indexes
			float v[3][3];  // coordinates

			for (int k = 0; k < 3; k++) {
				vi[0] = idx0.vertex_index;
				vi[1] = idx1.vertex_index;
				vi[2] = idx2.vertex_index;
				assert(vi[0] >= 0);
				assert(vi[1] >= 0);
				assert(vi[2] >= 0);

				v[0][k] = attrib.vertices[3 * vi[0] + k];
				v[1][k] = attrib.vertices[3 * vi[1] + k];
				v[2][k] = attrib.vertices[3 * vi[2] + k];
			}

			// Compute the normal of the face
			float normal[3];
			CalcNormal(normal, v[0], v[1], v[2]);

			// Add the normal to the three vertexes
			for (size_t i = 0; i < 3; ++i) {
				iter = smoothVertexNormals.find(vi[i]);
				if (iter != smoothVertexNormals.end()) {
					// add
					iter->second.m[0] += normal[0];
					iter->second.m[1] += normal[1];
					iter->second.m[2] += normal[2];
				}
				else {
					smoothVertexNormals[vi[i]].m[0] = normal[0];
					smoothVertexNormals[vi[i]].m[1] = normal[1];
					smoothVertexNormals[vi[i]].m[2] = normal[2];
				}
			}
		}
	}
	static void computeSmoothingShape(tinyobj::attrib_t& inattrib, tinyobj::shape_t& inshape,
		std::vector<std::pair<unsigned int, unsigned int>>& sortedids,
		unsigned int idbegin, unsigned int idend,
		std::vector<tinyobj::shape_t>& outshapes,
		tinyobj::attrib_t& outattrib) {
		unsigned int sgroupid = sortedids[idbegin].first;
		bool hasmaterials = inshape.mesh.material_ids.size();
		// Make a new shape from the set of faces in the range [idbegin, idend).
		outshapes.emplace_back();
		tinyobj::shape_t& outshape = outshapes.back();
		outshape.name = inshape.name;
		// Skip lines and points.

		std::unordered_map<unsigned int, unsigned int> remap;
		for (unsigned int id = idbegin; id < idend; ++id) {
			unsigned int face = sortedids[id].second;

			outshape.mesh.num_face_vertices.push_back(3); // always triangles
			if (hasmaterials)
				outshape.mesh.material_ids.push_back(inshape.mesh.material_ids[face]);
			outshape.mesh.smoothing_group_ids.push_back(sgroupid);
			// Skip tags.

			for (unsigned int v = 0; v < 3; ++v) {
				tinyobj::index_t inidx = inshape.mesh.indices[3 * face + v], outidx;
				assert(inidx.vertex_index != -1);
				auto iter = remap.find(inidx.vertex_index);
				// Smooth group 0 disables smoothing so no shared vertices in that case.
				if (sgroupid && iter != remap.end()) {
					outidx.vertex_index = (*iter).second;
					outidx.normal_index = outidx.vertex_index;
					outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : outidx.vertex_index;
				}
				else {
					assert(outattrib.vertices.size() % 3 == 0);
					unsigned int offset = static_cast<unsigned int>(outattrib.vertices.size() / 3);
					outidx.vertex_index = outidx.normal_index = offset;
					outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : offset;
					outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index]);
					outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index + 1]);
					outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index + 2]);
					outattrib.normals.push_back(0.0f);
					outattrib.normals.push_back(0.0f);
					outattrib.normals.push_back(0.0f);
					if (inidx.texcoord_index != -1) {
						outattrib.texcoords.push_back(inattrib.texcoords[2 * inidx.texcoord_index]);
						outattrib.texcoords.push_back(inattrib.texcoords[2 * inidx.texcoord_index + 1]);
					}
					remap[inidx.vertex_index] = offset;
				}
				outshape.mesh.indices.push_back(outidx);
			}
		}
	}
	static void computeAllSmoothingNormals(tinyobj::attrib_t& attrib,
		std::vector<tinyobj::shape_t>& shapes) {
		float3 p[3];
		for (size_t s = 0, slen = shapes.size(); s < slen; ++s) {
			const tinyobj::shape_t& shape(shapes[s]);
			size_t facecount = shape.mesh.num_face_vertices.size();
			assert(shape.mesh.smoothing_group_ids.size());

			for (size_t f = 0, flen = facecount; f < flen; ++f) {
				for (unsigned int v = 0; v < 3; ++v) {
					tinyobj::index_t idx = shape.mesh.indices[3 * f + v];
					assert(idx.vertex_index != -1);
					p[v].m[0] = attrib.vertices[3 * idx.vertex_index];
					p[v].m[1] = attrib.vertices[3 * idx.vertex_index + 1];
					p[v].m[2] = attrib.vertices[3 * idx.vertex_index + 2];
				}

				// cross(p[1] - p[0], p[2] - p[0])
				float nx = (p[1].m[1] - p[0].m[1]) * (p[2].m[2] - p[0].m[2]) -
					(p[1].m[2] - p[0].m[2]) * (p[2].m[1] - p[0].m[1]);
				float ny = (p[1].m[2] - p[0].m[2]) * (p[2].m[0] - p[0].m[0]) -
					(p[1].m[0] - p[0].m[0]) * (p[2].m[2] - p[0].m[2]);
				float nz = (p[1].m[0] - p[0].m[0]) * (p[2].m[1] - p[0].m[1]) -
					(p[1].m[1] - p[0].m[1]) * (p[2].m[0] - p[0].m[0]);

				// Don't normalize here.
				for (unsigned int v = 0; v < 3; ++v) {
					tinyobj::index_t idx = shape.mesh.indices[3 * f + v];
					attrib.normals[3 * idx.normal_index] += nx;
					attrib.normals[3 * idx.normal_index + 1] += ny;
					attrib.normals[3 * idx.normal_index + 2] += nz;
				}
			}
		}

		assert(attrib.normals.size() % 3 == 0);
		for (size_t i = 0, nlen = attrib.normals.size() / 3; i < nlen; ++i) {
			tinyobj::real_t& nx = attrib.normals[3 * i];
			tinyobj::real_t& ny = attrib.normals[3 * i + 1];
			tinyobj::real_t& nz = attrib.normals[3 * i + 2];
			tinyobj::real_t len = sqrtf(nx * nx + ny * ny + nz * nz);
			tinyobj::real_t scale = len == 0 ? 0 : 1 / len;
			nx *= scale;
			ny *= scale;
			nz *= scale;
		}
	}
	static void computeSmoothingShapes(tinyobj::attrib_t& inattrib,
		std::vector<tinyobj::shape_t>& inshapes,
		std::vector<tinyobj::shape_t>& outshapes,
		tinyobj::attrib_t& outattrib) {
		for (size_t s = 0, slen = inshapes.size(); s < slen; ++s) {
			tinyobj::shape_t& inshape = inshapes[s];

			unsigned int numfaces = static_cast<unsigned int>(inshape.mesh.smoothing_group_ids.size());
			assert(numfaces);
			std::vector<std::pair<unsigned int, unsigned int>> sortedids(numfaces);
			for (unsigned int i = 0; i < numfaces; ++i)
				sortedids[i] = std::make_pair(inshape.mesh.smoothing_group_ids[i], i);
			sort(sortedids.begin(), sortedids.end());

			unsigned int activeid = sortedids[0].first;
			unsigned int id = activeid, idbegin = 0, idend = 0;
			// Faces are now bundled by smoothing group id, create shapes from these.
			while (idbegin < numfaces) {
				while (activeid == id && ++idend < numfaces)
					id = sortedids[idend].first;
				computeSmoothingShape(inattrib, inshape, sortedids, idbegin, idend,
					outshapes, outattrib);
				activeid = id;
				idbegin = idend;
			}
		}
	}
#pragma endregion

	void Visualize::RebuildLighting()
	{
#if 0
		CPUTimer tm;
		tm.Start();
		{
			mBVHLighting.Clear();
			std::vector<BVHLighting::ConstructionItem> items;
			uint32_t faceIndex = 0;

			for (auto& mesh : mMeshes)
			{
				const auto& material = materials[mesh.mMaterialIndex];
				float matEmissiveIntensity = materialsEmissionIntensity[mesh.mMaterialIndex];
				bool emissive = matEmissiveIntensity > 0;
				emissive &= (material.emission[0] || material.emission[1] || material.emission[2]);

				if (emissive)
				{
					for (auto& face : mesh.mIndexBuffer)
					{
						BVHLighting::ConstructionItem item;

						auto& v0 = mesh.mVertexBuffer[face.x];
						auto& v1 = mesh.mVertexBuffer[face.y];
						auto& v2 = mesh.mVertexBuffer[face.z];

						float3 p0 = v0.mPosition;
						float3 p1 = v1.mPosition;
						float3 p2 = v2.mPosition;

						item.mBox += p0;
						item.mBox += p1;
						item.mBox += p2;

						float3 u = p1 - p0;
						float3 v = p2 - p0;
						float3 n = float3::cross(u, v);

						//check that normal outward pointing
						float3 tempN(mesh.mVertexBuffer[face.x].mNormal.x, mesh.mVertexBuffer[face.x].mNormal.y, mesh.mVertexBuffer[face.x].mNormal.z);
						if (float3::dot(n, tempN) < 0)
						{
							n = -n;//flip direction
						}
						n = float3::normalize(n);

						//compute area
						float a = 0.5f * n.length();

						item.mLeafData = faceIndex;
						item.mCentroid = item.mBox.Centroid();

						item.mEmissionDirection = n;
						item.mFlux = Pi * a * float3(material.emission[0], material.emission[1], material.emission[2]) * matEmissiveIntensity;

						const float3& itemBounds = item.mBox.Dimensions();
						assert((itemBounds.x > 0) || (itemBounds.y > 0) || (itemBounds.z > 0));

						faceIndex++;
						items.push_back(item);
					}
				}
			}

			if (items.size() != 0)
			{
				mBVHLighting.AppendVolumes(mLightingAABB, items);
			}
		}
		tm.Stop();
		Console::Instance()->Log(Console::LogType::eInfo, "Lighting BVH building time: %d [ms]\n", (int)tm.GetMilliseconds());
#endif
	}

	void Visualize::RebuildBVHs()
	{
		CPUTimer tm;
		tm.Start();
		mBottomLevels.clear();
		mTopLevel.clear();
		mInstances.clear();
		mInstanceVolumes.clear();

		std::vector<BVH::ConstructionItem> itemsMemory;
		std::vector<BVH::ConstructionItem*> items;
		BVH bvhBuilder;
		uint64_t bvhOffset = 0;

		std::vector<AABB> topLvlVolumes;
		for (auto& mesh : mMeshes)
		{
			itemsMemory.resize(mesh.mFaceCount);// mesh.mIndexBuffer.size());
			items.resize(mesh.mFaceCount);// mesh.mIndexBuffer.size());
			uint32_t faceIndex = 0;

			//get the subpart of the index and vertex buffers for this particular mesh
			const int3* meshIndexBuffer = &(mIndexBuffer[mesh.mEntry.mIndexOffset]);
			const Vertex* meshVertexBuffer = &(mVertexBuffer[mesh.mEntry.mVertexOffset]);
			for (uint32_t iFace = 0; iFace < mesh.mFaceCount; ++iFace)
			{
				const int3& face = meshIndexBuffer[iFace];

				auto& v0 = meshVertexBuffer[face.x];
				auto& v1 = meshVertexBuffer[face.y];
				auto& v2 = meshVertexBuffer[face.z];

				float3 p0 = v0.mPosition;
				float3 p1 = v1.mPosition;
				float3 p2 = v2.mPosition;

				items[faceIndex] = &itemsMemory[faceIndex];//saves us the small allocs

				items[faceIndex]->mBox += p0;
				items[faceIndex]->mBox += p1;
				items[faceIndex]->mBox += p2;
				items[faceIndex]->mCentroid = items[faceIndex]->mBox.Centroid();
				items[faceIndex]->mLeafData = faceIndex;

				faceIndex++;
			}
			AABB rootVolume;
			bvhBuilder.Clear();
			mesh.mEntry.mRootBVH = bvhOffset;
			bvhBuilder.CreateVolumes(rootVolume, items, BVH::eSurfaceAreaHeuristic);
			mesh.mAABBCount = bvhBuilder.GetNodes().size();
			mBottomLevels.insert(mBottomLevels.end(), bvhBuilder.GetNodes().begin(), bvhBuilder.GetNodes().end());
			topLvlVolumes.push_back(rootVolume);

			bvhOffset += mesh.mAABBCount;
		}
		tm.Stop();
		Console::Instance()->Log(Console::LogType::eInfo, "Bottom level BVH building time: %d [ms]\n", (int)tm.GetMilliseconds());

		tm.Start();
		itemsMemory.resize(topLvlVolumes.size());
		items.resize(topLvlVolumes.size());
		uint32_t instanceIndex = 0;

		for (auto &aabb : topLvlVolumes)
		{
			//for right now, just ID
			float4x4 identity;
			float4x4::MakeIdentity(identity);

			const float3& a = identity.TransformPoint(aabb.mMin);
			const float3& b = identity.TransformPoint(aabb.mMax);
			AABB transformedVolume;
			transformedVolume += a;
			transformedVolume += b;

			items[instanceIndex] = &itemsMemory[instanceIndex];//saves us the small allocs

			items[instanceIndex]->mBox = transformedVolume;
			items[instanceIndex]->mCentroid = items[instanceIndex]->mBox.Centroid();
			items[instanceIndex]->mLeafData = instanceIndex;

			mInstances.push_back({ identity, identity, identity, instanceIndex });
			mInstanceVolumes.push_back(transformedVolume);

			instanceIndex++;
		}
		AABB rootVolume;
		bvhBuilder.Clear();
		BVHNodeIndex rootIndex = bvhBuilder.CreateVolumes(rootVolume, items, BVH::eSurfaceAreaHeuristic);
		mTopLevel = bvhBuilder.GetNodes();
		tm.Stop();
		Console::Instance()->Log(Console::LogType::eInfo, "Top level BVH building time: %d [ms]\n", (int)tm.GetMilliseconds());
	}

	void Visualize::SceneExportHander()
	{
		const auto& filename = GetFile(".bin", false);
		if (!filename.empty())
		{
			
		}
	}

	void Visualize::FileLoadHandler()
	{
		const auto& filename = GetFile("obj\0*.OBJ\0mtlx\0*.MTLX");
		if (!filename.empty())
		{
			if ((filename.extension() == ".mtlx") || (filename.extension() == ".MTLX"))
			{
				tinyxml2::XMLDocument doc;
				doc.LoadFile(filename.string().c_str());
				auto err = doc.ErrorID();

				tinyxml2::XMLElement* titleElement = doc.FirstChildElement("materialx");
			}
			if ((filename.extension() == ".OBJ") || (filename.extension() == ".obj"))
			{
				std::string warn;
				std::string err;

				tinyobj::attrib_t inattrib;
				std::vector<tinyobj::shape_t> inshapes;
				std::vector<tinyobj::material_t> materials;

				CPUTimer tm;
				tm.Start();
				bool ret = tinyobj::LoadObj(&inattrib, &inshapes, &materials, &warn, &err, filename.string().c_str(), nullptr, true);
				bool regen_all_normals = inattrib.normals.size() == 0;
				tinyobj::attrib_t outattrib;
				std::vector<tinyobj::shape_t> outshapes;
				if (regen_all_normals) {
					computeSmoothingShapes(inattrib, inshapes, outshapes, outattrib);
					computeAllSmoothingNormals(outattrib, outshapes);
				}
				std::vector<tinyobj::shape_t>& shapes = regen_all_normals ? outshapes : inshapes;
				tinyobj::attrib_t& attrib = regen_all_normals ? outattrib : inattrib;

				tm.Stop();
				Console::Instance()->Log(Console::LogType::eInfo, "%s Parsing time: %d [ms]\n", filename.c_str(), (int)tm.GetMilliseconds());

				Console::Instance()->Log(Console::LogType::eInfo, "vertices  = %d\n", (int)(inattrib.vertices.size()) / 3);
				Console::Instance()->Log(Console::LogType::eInfo, "normals   = %d\n", (int)(inattrib.normals.size()) / 3);
				Console::Instance()->Log(Console::LogType::eInfo, "texcoords = %d\n", (int)(inattrib.texcoords.size()) / 2);
				Console::Instance()->Log(Console::LogType::eInfo, "materials = %d\n", (int)materials.size());
				Console::Instance()->Log(Console::LogType::eInfo, "shapes    = %d\n", (int)inshapes.size());

				if (!warn.empty())
					Console::Instance()->Log(Console::LogType::eWarning, "%s\n", warn.c_str());
				if (!err.empty())
					Console::Instance()->Log(Console::LogType::eError, "%s\n", err.c_str());

				std::map<stdstr_t, uint32_t> nameMap;

				uint32_t vertexOffset = 0;
				uint32_t indexOffset = 0;
				//iterate over the shapes
				float3 bmin, bmax;
				bmin[0] = bmin[1] = bmin[2] = std::numeric_limits<float>::max();
				bmax[0] = bmax[1] = bmax[2] = -std::numeric_limits<float>::max();
				for (size_t s = 0; s < shapes.size(); s++)
				{
					Mesh mesh;

					// Check for smoothing group and compute smoothing normals
					std::map<int, float3> smoothVertexNormals;
					if (!regen_all_normals && (hasSmoothingGroup(shapes[s]) > 0))
					{
						Console::Instance()->Log(Console::LogType::eInfo, "Compute smoothingNormal for shape [%d]", s);
						computeSmoothingNormals(attrib, shapes[s], smoothVertexNormals);
					}

					//iterate over indices
					const auto& nameIter = nameMap.find(shapes[s].name);
					if (nameIter == nameMap.end())
					{
						nameMap[shapes[s].name] = 2;
						mesh.mName = shapes[s].name;
					}
					else
					{
						char str[1024];
						sprintf(str, "%s.%d", shapes[s].name.c_str(), nameIter->second);
						mesh.mName = str;
						nameMap[shapes[s].name]++;
					}

					auto cmp = [](const tinyobj::index_t& lhs, const tinyobj::index_t& rhs)->bool
					{
						if (lhs.vertex_index == rhs.vertex_index)
						{
							if (lhs.normal_index == rhs.normal_index)
							{
								return (lhs.texcoord_index < rhs.texcoord_index);
							}
							else
								return (lhs.normal_index < rhs.normal_index);
						}
						else
						{
							return (lhs.vertex_index < rhs.vertex_index);
						}
					};
					std::map<tinyobj::index_t, int, decltype(cmp)> remap(cmp);


					for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++)
					{
						tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
						tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
						tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];

						int current_material_id = shapes[s].mesh.material_ids[f];
						if ((current_material_id < 0) ||
							(current_material_id >= static_cast<int>(materials.size())))
						{
							// Invalid material ID. Use default material.
							current_material_id =
								materials.size() -
								1;  // Default material is added to the last item in `materials`.
						}
						mesh.mMaterialIndex = current_material_id;
#pragma region TexCoords
						float4 tc[3] = { {0} };
						if (attrib.texcoords.size() > 0)
						{
							if ((idx0.texcoord_index < 0) || (idx1.texcoord_index < 0) ||
								(idx2.texcoord_index < 0))
							{
								// face does not contain valid uv index.
								tc[0][0] = 0.0f;
								tc[0][1] = 0.0f;
								tc[1][0] = 0.0f;
								tc[1][1] = 0.0f;
								tc[2][0] = 0.0f;
								tc[2][1] = 0.0f;
							}
							else
							{
								assert(attrib.texcoords.size() >
									size_t(2 * idx0.texcoord_index + 1));
								assert(attrib.texcoords.size() >
									size_t(2 * idx1.texcoord_index + 1));
								assert(attrib.texcoords.size() >
									size_t(2 * idx2.texcoord_index + 1));

								// Flip Y coord.
								tc[0][0] = attrib.texcoords[2 * idx0.texcoord_index];
								tc[0][1] = 1.0f - attrib.texcoords[2 * idx0.texcoord_index + 1];
								tc[1][0] = attrib.texcoords[2 * idx1.texcoord_index];
								tc[1][1] = 1.0f - attrib.texcoords[2 * idx1.texcoord_index + 1];
								tc[2][0] = attrib.texcoords[2 * idx2.texcoord_index];
								tc[2][1] = 1.0f - attrib.texcoords[2 * idx2.texcoord_index + 1];
							}
						}
						else
						{
							tc[0][0] = 0.0f;
							tc[0][1] = 0.0f;
							tc[1][0] = 0.0f;
							tc[1][1] = 0.0f;
							tc[2][0] = 0.0f;
							tc[2][1] = 0.0f;
						}
#pragma endregion
#pragma region Positions
						float4 v[3];
						for (int k = 0; k < 3; k++)
						{
							int f0 = idx0.vertex_index;
							int f1 = idx1.vertex_index;
							int f2 = idx2.vertex_index;
							assert(f0 >= 0);
							assert(f1 >= 0);
							assert(f2 >= 0);

							v[0][k] = attrib.vertices[3 * f0 + k];
							v[1][k] = attrib.vertices[3 * f1 + k];
							v[2][k] = attrib.vertices[3 * f2 + k];
							bmin[k] = std::min(v[0][k], bmin[k]);
							bmin[k] = std::min(v[1][k], bmin[k]);
							bmin[k] = std::min(v[2][k], bmin[k]);
							bmax[k] = std::max(v[0][k], bmax[k]);
							bmax[k] = std::max(v[1][k], bmax[k]);
							bmax[k] = std::max(v[2][k], bmax[k]);
						}
#pragma endregion
#pragma region Normals
						float4 n[3] = { {0,0,0,0}, {0,0,0,0} , {0,0,0,0} };
						{
							bool invalid_normal_index = false;
							if (attrib.normals.size() > 0)
							{
								int nf0 = idx0.normal_index;
								int nf1 = idx1.normal_index;
								int nf2 = idx2.normal_index;

								if ((nf0 < 0) || (nf1 < 0) || (nf2 < 0))
								{
									// normal index is missing from this face.
									invalid_normal_index = true;
								}
								else
								{
									for (int k = 0; k < 3; k++)
									{
										assert(size_t(3 * nf0 + k) < attrib.normals.size());
										assert(size_t(3 * nf1 + k) < attrib.normals.size());
										assert(size_t(3 * nf2 + k) < attrib.normals.size());
										n[0][k] = attrib.normals[3 * nf0 + k];
										n[1][k] = attrib.normals[3 * nf1 + k];
										n[2][k] = attrib.normals[3 * nf2 + k];
									}
								}
							}
							else
							{
								invalid_normal_index = true;
							}

							if (invalid_normal_index && !smoothVertexNormals.empty())
							{
								// Use smoothing normals
								int f0 = idx0.vertex_index;
								int f1 = idx1.vertex_index;
								int f2 = idx2.vertex_index;

								if (f0 >= 0 && f1 >= 0 && f2 >= 0)
								{
									n[0][0] = smoothVertexNormals[f0].m[0];
									n[0][1] = smoothVertexNormals[f0].m[1];
									n[0][2] = smoothVertexNormals[f0].m[2];

									n[1][0] = smoothVertexNormals[f1].m[0];
									n[1][1] = smoothVertexNormals[f1].m[1];
									n[1][2] = smoothVertexNormals[f1].m[2];

									n[2][0] = smoothVertexNormals[f2].m[0];
									n[2][1] = smoothVertexNormals[f2].m[1];
									n[2][2] = smoothVertexNormals[f2].m[2];

									invalid_normal_index = false;
								}
							}

							if (invalid_normal_index)
							{
								// compute geometric normal
								CalcNormal(n[0].m, v[0].m, v[1].m, v[2].m);
								n[1][0] = n[0][0];
								n[1][1] = n[0][1];
								n[1][2] = n[0][2];
								n[2][0] = n[0][0];
								n[2][1] = n[0][1];
								n[2][2] = n[0][2];
							}
						}
#pragma endregion

						Vertex vertex0 = { v[0], n[0], tc[0] };
						Vertex vertex1 = { v[1], n[1], tc[1] };
						Vertex vertex2 = { v[2], n[2], tc[2] };

						int3 face;
						const auto& foundI0 = remap.find(idx0);
						if (foundI0 == remap.end())//not found
						{
							face.x = mesh.mVertexCount++;
							mVertexBuffer.push_back(vertex0);
							remap[idx0] = face.x;//store the remap for next time
						}
						else
							face.x = foundI0->second;

						const auto& foundI1 = remap.find(idx1);
						if (foundI1 == remap.end())//not found
						{
							face.y = mesh.mVertexCount++;
							mVertexBuffer.push_back(vertex1);
							remap[idx1] = face.y;//store the remap for next time
						}
						else
							face.y = foundI1->second;

						const auto& foundI2 = remap.find(idx2);
						if (foundI2 == remap.end())//not found
						{
							face.z = mesh.mVertexCount++;
							mVertexBuffer.push_back(vertex2);
							remap[idx2] = face.z;//store the remap for next time
						}
						else
							face.z = foundI2->second;

						//store the new face
						mIndexBuffer.push_back(face);
						mesh.mFaceCount++;
					}

					//store mesh
					if (mesh.mFaceCount != 0 && mesh.mVertexCount != 0)
					{
						mesh.mEntry.mIndexOffset = indexOffset;
						mesh.mEntry.mVertexOffset = vertexOffset;
						mMeshes.push_back(mesh);// std::move(mesh));

						indexOffset += mesh.mFaceCount;
						vertexOffset += mesh.mVertexCount;
					}
				}
			}
		}
	}

	//CSVDocument<2> doc("test.csv", { "Theta", "Phi" });
	bool Visualize::Initialize()
	{
		//Console::Instance()->RegisterCommand("open", Console::CommandEvent_t::Callback(&Visualize::FileLoadHandler, this));

		//Sky s(0.5f, 512, 10.f, PiOver4);
		//s.RenderToFile("sky.exr");
		return true;
	}

	void Visualize::Run()
	{
		DrawUI();

		D3D11LineRenderer::Instance()->DrawLine(float3(0, 0, 0), float3(1, 0, 0), float3(1, 0, 0));
		D3D11LineRenderer::Instance()->DrawLine(float3(0, 0, 0), float3(0, 1, 0), float3(0, 1, 0));
		D3D11LineRenderer::Instance()->DrawLine(float3(0, 0, 0), float3(0, 0, 1), float3(0, 0, 1));

		const float3& O = mCamera.GetEyePosition();
		const float3& X = mCamera.GetRightVector();
		const float3& Y = mCamera.GetUpVector();
		const float3& Z = mCamera.GetLookVector();

		float halfDx = mCamera.GetViewPlaneWidth() / 2;
		float halfDy = mCamera.GetViewPlaneHeight() / 2;
		float3 offset(-halfDx + halfDx / 10, -halfDy + halfDx / 10, 1);
		float3 Ow = O + offset.x * X + offset.y * Y + offset.z * Z;
		D3D11LineRenderer::Instance()->DrawLine(Ow, Ow + float3(0.05f, 0, 0), float3(1, 0, 0));
		D3D11LineRenderer::Instance()->DrawLine(Ow, Ow + float3(0, 0.05f, 0), float3(0, 1, 0));
		D3D11LineRenderer::Instance()->DrawLine(Ow, Ow + float3(0, 0, 0.05f), float3(0, 0, 1));

		float4x4 identity;
		float4x4::MakeIdentity(identity);
		uint32_t iMesh = 0;
		for (auto& mesh : mMeshes)
		{
			float4 color(.5f, .5f, .5f, 1);
			if (mesh.mVisible)
			{
				//get the subpart of the index and vertex buffers for this particular mesh
				int3* meshIndexBuffer = &(mIndexBuffer[mesh.mEntry.mIndexOffset]);
				Vertex* meshVertexBuffer = &(mVertexBuffer[mesh.mEntry.mVertexOffset]);
				D3D11MeshRenderer::Instance()->DrawMesh(meshVertexBuffer, mesh.mVertexCount, meshIndexBuffer, mesh.mFaceCount, identity, identity, color);
			}
			if (mesh.mBVHVisible)
			{
				//for (uint32_t iBox = 0; iBox < mesh.mAABBCount; ++iBox)
				//{
				//	const auto& node = mBottomLevel[mesh.mEntry.mRootBVH + iBox];
				//	if (mesh.mBVHShowOnlyLeaves)
				//	{
				//		if (node.mLeafData != INVALID_INDEX)
				//			D3D11LineRenderer::Instance()->DrawBox(node.mBoxMin, node.mBoxMax, float3(1, 0, 0));
				//	}
				//	else
				//	{
				//		D3D11LineRenderer::Instance()->DrawBox(node.mBoxMin, node.mBoxMax, float3(1, 0, 0));
				//	}
				//}
			}
			iMesh++;
		}

		if (mShowBVHLighting)
		{
			D3D11LineRenderer::Instance()->DrawBox(mLightingAABB.mMin, mLightingAABB.mMax, float3(1, 0, 0));
		}

		if (mSurface.mHit)
		{
			D3D11LineRenderer::Instance()->DrawPlane(mSurface.mPoint, mSurface.mNormal, float3(0, 0, 1), 1);
			auto& box = mInstanceVolumes[mSurface.mHitInstanceIndex];
			D3D11LineRenderer::Instance()->DrawBox(box.mMin, box.mMax, float3(1, 1, 0));
		}
	}

	void Visualize::DrawUI()
	{
		////draw the UI
		ImGui::Begin(Name());
		if (ImGui::Button("Open"))
		{
			D3D11MeshRenderer::Instance()->FlushCache();
			mVertexBuffer.clear();
			mIndexBuffer.clear();
			mMeshes.clear();

			FileLoadHandler();
			RebuildBVHs();
			RebuildLighting();
		}
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			//save the scene
			SceneExportHander();
		}
		ImGui::SameLine();
		if (ImGui::Button("Flip Indices"))
		{
			D3D11MeshRenderer::Instance()->FlushCache();
			for (auto& mesh : mMeshes)
			{
				for (auto& face : mIndexBuffer)
				{
					std::swap(face.y, face.z);
				}
			}
		}
		bool rotate = false;
		float4x4 mtx;
		ImGui::SameLine();
		if (ImGui::Button("RotateX"))
		{
			float4x4::MakeRotationX(mtx, PiOver2);
			rotate = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("RotateY"))
		{
			float4x4::MakeRotationY(mtx, PiOver2);
			rotate = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("RotateZ"))
		{
			float4x4::MakeRotationZ(mtx, PiOver2);
			rotate = true;
		}

		//Camera 
		ImGui::Separator();
		ImGui::Text("Camera");
		if (ImGui::Button("Set"))
		{
		}
		ImGui::SliderFloat("F-Stop", &mOptics.mFStop, 0, 11, "%.1f");
		ImGui::SliderInt("Focal Length", &mOptics.mFocalLength, 16, 100);


		if (rotate)
		{
			D3D11MeshRenderer::Instance()->FlushCache();
			//for (auto& mesh : mMeshes)
			//{
				for (auto& idx : mVertexBuffer)
				{
					idx.mPosition = mtx.Transform(idx.mPosition);
					idx.mNormal = mtx.Transform(idx.mNormal);
				}
			//}
			RebuildBVHs();
			RebuildLighting();
		}

		ImGui::Separator();
		static bool useSkyLighting = false;
		ImGui::Text("HDRI");
		ImGui::Checkbox("Procedural Sky", &useSkyLighting);
		if (useSkyLighting)
		{
			//Sky s(0.5f, 512, 10.f, PiOver4);
			//s.RenderToFile("sky.exr");
			//albedo, uint32_t resolution, float turbidity, float elevation
			/*
			int mResolution;
			float mAlbedo, mTurbidity, mElevation;
			*/
			ImGui::SliderFloat("Albedo",    &mHDRI.mAlbedo, 0, 1.0f);
			ImGui::SliderFloat("Turbidity", &mHDRI.mTurbidity, 0, 20.0f);
			ImGui::SliderAngle("Elevation", &mHDRI.mElevation, 0, 180.0f);
			ImGui::SliderInt("Resolution",  &mHDRI.mResolution, 512, 1024);

			mHDRI.mSky.Initialize(0.5f, 512, 10.f, PiOver4);
		}
		else
		{
			if (ImGui::SmallButton("HDRI Map"))
			{
				const auto& filename = GetFile("hdr\0*.HDR\0*.exr\0*.EXR");
				if (!filename.empty())
				{
					mHDRI.mPath = filename.string().c_str();
					mHDRI.mSky.Initialize(mHDRI.mPath);
				}
			}

			ImGui::SameLine(); ImGui::Text(mHDRI.mPath.c_str());
		}
		if (!mBVHLighting.Empty())
		{
			ImGui::Checkbox("Show Lighting BVH", &mShowBVHLighting);
		}

		static int pixelShader = 0;
		ImGui::Separator();
		ImGui::RadioButton("Lighting", &pixelShader, 0); ImGui::SameLine();
		ImGui::RadioButton("Normal", &pixelShader, 1); ImGui::SameLine();
		ImGui::RadioButton("TexCoords", &pixelShader, 2);
		switch (pixelShader)
		{
		case 0:
			D3D11MeshRenderer::Instance()->SetPixelShaderType(D3D11MeshRenderer::eLIGHTING);
			break;
		case 1:
			D3D11MeshRenderer::Instance()->SetPixelShaderType(D3D11MeshRenderer::eNORMAL);
			break;
		case 2:
			D3D11MeshRenderer::Instance()->SetPixelShaderType(D3D11MeshRenderer::eUV);
			break;
		}

		static bool wireframe = false;
		ImGui::Checkbox("Wireframe", &wireframe);
		D3D11Manager::Instance()->SetWireframe(wireframe);
		ImGui::Separator();

		bool lightingNeedsRecompute = false;

		if (lightingNeedsRecompute)
			RebuildLighting();

		if (mMeshes.size() && ImGui::CollapsingHeader("Meshes"))
		{
			for (uint32_t iMesh = 0; iMesh < mMeshes.size(); ++iMesh)
			{
				if (ImGui::TreeNode(mMeshes[iMesh].mName.c_str()))
				{
					ImGui::BulletText("%d faces", mMeshes[iMesh].mFaceCount);
					ImGui::BulletText("%d vertices", mMeshes[iMesh].mVertexCount);
					ImGui::Checkbox("Show BVH", &mMeshes[iMesh].mBVHVisible);
					ImGui::Checkbox("BVH Leaves", &mMeshes[iMesh].mBVHShowOnlyLeaves);
					ImGui::Checkbox("Visible", &mMeshes[iMesh].mVisible);

					ImGui::TreePop();
				}
			}
		}

		ImGui::End();
	}

	void Visualize::MouseMove(const MouseInfo& info)
	{
	}

	//float RayBVHIntersectRec(const Ray& ray, const BVH::NodeList_t& nodes, BVHNodeIndex nodeIndex, std::vector<Vertex>& vertexBuffer, std::vector<int3>& indexBuffer, float3& p, float3& n)
	//{
	//	float hitTime = FLT_MAX;
	//	const GPUBVHNode& node = nodes[nodeIndex];

	//	AABB box(node.mBoxMin, node.mBoxMax);
	//	if (box.Intersect(ray))
	//	{
	//		if (node.mLeafData != INVALID_INDEX)
	//		{
	//			int3 face = indexBuffer[node.mLeafData];

	//			auto& v0 = vertexBuffer[face.x];
	//			auto& v1 = vertexBuffer[face.y];
	//			auto& v2 = vertexBuffer[face.z];

	//			float3 p0 = v0.mPosition;
	//			float3 p1 = v1.mPosition;
	//			float3 p2 = v2.mPosition;

	//			float3 n0 = v0.mNormal;
	//			float3 n1 = v1.mNormal;
	//			float3 n2 = v2.mNormal;

	//			float t, u, v;
	//			if (IntersectionRayTriangle(ray, p0, p1, p2, t, u, v))
	//			{
	//				hitTime = t;
	//				//write out the SHADING normal
	//				float one_minusu_minusv = (1 - u - v);
	//				n = float3::normalize((one_minusu_minusv * n0) + (u * n1) + (v * n2));
	//				p = ray.mOrigin + t * ray.mDirection;
	//			}
	//		}
	//		else
	//		{
	//			float3 pL, pR, nL, nR;
	//			float tL = RayBVHIntersectRec(ray, nodes, node.mLeftChild, vertexBuffer, indexBuffer, pL, nL);
	//			float tR = RayBVHIntersectRec(ray, nodes, node.mRightChild, vertexBuffer, indexBuffer, pR, nR);
	//			if (tL < tR)
	//			{
	//				n = nL;
	//				p = pL;
	//				hitTime = tL;
	//			}
	//			else
	//			{
	//				n = nR;
	//				p = pR;
	//				hitTime = tR;
	//			}
	//		}
	//	}
	//	return hitTime;
	//}

	void Visualize::MouseClick(const MouseInfo& info)
	{
		PrimaryRay(mSurface.mRay, uint2(info.x, info.y));
		float tMin = FLT_MAX;
		float3 hitPos, hitNml;

		uint64_t stack[256];
		int stackIndex = 0;
		//push on the stack
		stack[stackIndex] = 0;

		do
		{
			uint64_t currentNodeIndex = stack[stackIndex];
			const auto& node = mTopLevel[currentNodeIndex];
			AABB box(node.mBoxMin, node.mBoxMax);
			if (box.Intersect(mSurface.mRay))
			{
				if (node.mLeafData != INVALID_INDEX)
				{

					//hit instance
					float3 p, n;
					float t = RayBVHIntersect(mSurface.mRay, node.mLeafData, p, n);
					if (t < tMin)
					{
						tMin = t;
						hitPos = p;
						hitNml = n;
						mSurface.mHitInstanceIndex = node.mLeafData;
					}

					//pop stack
					stackIndex--;
				}
				else
				{
					//recurse
					stack[stackIndex] = node.mLeftChild; //Note we are overriding the stackIndex (squash the node we just traversed)
					++stackIndex;
					stack[stackIndex] = node.mRightChild;
				}
			}
			else
			{
				//pop stack
				stackIndex--;
			}

		} while (stackIndex >= 0);

		if (tMin < FLT_MAX)
		{
			mSurface.mHit = true;
			mSurface.mPoint = hitPos;
			mSurface.mNormal = hitNml;
		}
	}

	void Visualize::MouseDrag(const MouseInfo& info)
	{
		if (info.mButton == MouseButton::eMBCenter)
		{
			if (info.mModifier & SystemKeyBits::eSKShift)
				mCamera.Pan(mCameraSpeed * -info.dx, mCameraSpeed * info.dy);
			else
			{
				mCamera.Rotate(mCameraSpeed * -info.dx, mCameraSpeed * info.dy);
				//Console::Instance()->Log(Console::LogType::eInfo, "Spherical %f, %f", mCamera.GetSpherical().x, mCamera.GetSpherical().y);
				//doc.Write(mCamera.GetSpherical().x, mCamera.GetSpherical().y);
			}
		}
	}

	void Visualize::MouseWheel(const MouseInfo& info)
	{
		if (info.dz != 0)
			mCamera.Zoom(mCameraSpeed * info.dz);
	}

	void Visualize::Resize(uint32_t width, uint32_t height)
	{
		mOptics.mResolution.x = width;
		mOptics.mResolution.y = height;
		mOptics.mAspect = width / float(height);
		mCamera.SetPerspective(45, float(width) / height, 0.016f, 10000);
	}

#pragma region Ray casting
	// Fast RayTriangle Intersection
	static bool IntersectionRayTriangle(const Ray& r, float3& A, float3& B, float3& C, float& out_tmin, float& uOut, float& vOut)
	{
		float3 E1 = B - A;
		float3 E2 = C - A;

		float3 P = float3::cross(r.mDirection, E2);
		float det = float3::dot(P, E1);

		// det is the same as the dot(r.direction, cross(E1, E2))
		// when det is < 0, r.direction is opposite direction then winding order
		// So if backfaceCull'ing and det < 0, then return false
		// If not backfaceCull'ing, then just check if det is close to zero
		if ((det < Epsilon) && (det > -Epsilon)) return false;

		float3 K = r.mOrigin - A;
		float3 Q = float3::cross(K, E1);
		float u = float3::dot(P, K) / det;

		if (u < 0.f || u > 1.f)
			return false;

		float v = float3::dot(Q, r.mDirection) / det;
		if (v < 0.f || (u + v) > 1.f)
			return false;

		// if need to compute v and u.
		//u /= det;
		//v /= det;
		float tmin = float3::dot(Q, E2) / det;
		if (tmin > 0)
		{
			out_tmin = tmin;

			//if (uOut && vOut)
			{
				uOut = u;
				vOut = v;
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	//float Visualize::RayBVHIntersect(const Ray& ray, const BVH& bvh, Mesh& mesh, float3& p, float3& n)
	//{
	//	return RayBVHIntersectRec(ray, bvh.GetNodes(), 0, mesh.mVertexBuffer, mesh.mIndexBuffer, p, n);
	//}

	float Visualize::RayBVHIntersect(const Ray& ray, uint64_t instanceIndex, float3& p, float3& n)
	{
		float hitTime = FLT_MAX;

		uint64_t stack[256];
		int stackIndex = 0;
		//push on the stack

		uint32_t meshIndex = mInstances[instanceIndex].mMeshEntry;
		BVHNodeIndex root = mMeshes[meshIndex].mEntry.mRootBVH;
		stack[stackIndex] = root;//push BVH root

		do
		{
			uint64_t currentNodeIndex = stack[stackIndex];
			const auto& node = mBottomLevels[currentNodeIndex];
			AABB box(node.mBoxMin, node.mBoxMax);
			if (box.Intersect(mSurface.mRay))
			{
				if (node.mLeafData != INVALID_INDEX)
				{
					//hit triangle AABB
					//Test triangle
					uint32_t faceOffset = mMeshes[meshIndex].mEntry.mIndexOffset;
					uint32_t vertexOffset = mMeshes[meshIndex].mEntry.mVertexOffset;

					const auto& face = mIndexBuffer[faceOffset + node.mLeafData];
					auto& v0 = mVertexBuffer[vertexOffset + face.x];
					auto& v1 = mVertexBuffer[vertexOffset + face.y];
					auto& v2 = mVertexBuffer[vertexOffset + face.z];

					float3 p0 = v0.mPosition;
					float3 p1 = v1.mPosition;
					float3 p2 = v2.mPosition;

					float3 n0 = v0.mNormal;
					float3 n1 = v1.mNormal;
					float3 n2 = v2.mNormal;

					float t, u, v;
					if (IntersectionRayTriangle(ray, p0, p1, p2, t, u, v))
					{
						if (t < hitTime)
						{
							hitTime = t;
							//write out the SHADING normal
							float one_minusu_minusv = (1 - u - v);
							n = float3::normalize((one_minusu_minusv * n0) + (u * n1) + (v * n2));
							p = ray.mOrigin + t * ray.mDirection;
						}
					}

					//pop stack
					stackIndex--;
				}
				else
				{
					//recurse
					stack[stackIndex] = root + node.mLeftChild; //Note we are overriding the stackIndex (squash the node we just traversed)
					++stackIndex;
					stack[stackIndex] = root + node.mRightChild;
				}
			}
			else
			{
				//pop stack
				stackIndex--;
			}

		} while (stackIndex >= 0);

		return hitTime;
	}

	void Visualize::PrimaryRay(Ray& ray, const uint2& coord)
	{
		float2 raster(coord.x, coord.y);
		float2 sensorDimension(mCamera.GetNearPlane() * mCamera.GetViewPlaneWidth(), mCamera.GetNearPlane() * mCamera.GetViewPlaneHeight());

		//compute sensor dimentions based on aspect
		//sensorDimension.x = mOptics.mSensorWidth;
		//sensorDimension.y = sensorDimension.x / mOptics.mAspect;

		//Raster to sensor
		float2 sensor;
		sensor.x = raster.x * (sensorDimension.x / mOptics.mResolution.x) - (sensorDimension.x / 2.0f);
		sensor.y = -raster.y * (sensorDimension.y / mOptics.mResolution.y) + (sensorDimension.y / 2.0f);

		float3 origin = float3(sensor.x, mCamera.GetNearPlane(), sensor.y);//all data in mm but world in m
		float3 direction = float3::normalize(origin);

		float4x4 view;
		mCamera.GetInverseViewMatrix(view);

		ray.mOrigin = view.TransformPoint(origin);
		ray.mDirection = view.Transform(direction);
	}
#pragma endregion
 
}
