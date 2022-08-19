#include "PathTracer.h"
#include <SharedGPU_CPU/PrimaryRaysShared.h>
#include <SharedGPU_CPU/IntersectionShared.h>
#include <Image/Image.h>

namespace Phoenix
{
	PathTracer::PathTracer()
	{
	}

	PathTracer::~PathTracer()
	{
	}

	bool PathTracer::Initialize()
	{
		mDevice = VKManager::Instance()->GetDevice("NVIDIA");

		mCameraBuffer = VKManager::BufferAllocate(mDevice, sizeof(GPUCamera), true, true);
		std::vector<KeyValuePair> definesPrimary = {
			{"PRIMARY_RAY_LOCAL_SIZE", std::to_string(mLocalSize)},
			{"TILE_RESOLUTION", std::to_string(mTileRes)}
		};
		mPrimaryRays = VKManager::Instance()->ProgramGetFromAssets(mDevice, "PrimaryRays.hlsl", "Execute", definesPrimary);
		std::vector<KeyValuePair> definesIntersect = {
			{"INTERSECTION_LOCAL_SIZE", std::to_string(mLocalSize)},
			{"LOCAL_STACK_SIZE", std::to_string(32)}
		};
		mIntersection[eHitType::eNoStore] = VKManager::Instance()->ProgramGetFromAssets(mDevice, "Intersection.hlsl", "Execute", definesIntersect);
		definesIntersect.push_back({ "STORE_HIT_DATA", "1"});
		mIntersection[eHitType::eStore] = VKManager::Instance()->ProgramGetFromAssets(mDevice, "Intersection.hlsl", "Execute", definesIntersect);

		for (uint32_t iTile = 0; iTile < mTilesInFlight; ++iTile)
		{
			auto& tile = mTiles[iTile];
			tile.mPathsBuffer = VKManager::BufferAllocate(mDevice, sizeof(GPUPath) * mPathsPerTile, true, true);
			tile.mHitsBuffer = VKManager::BufferAllocate(mDevice, sizeof(GPUHit) * mPathsPerTile, true, true);
			tile.mSeeds = VKManager::BufferAllocate(mDevice, sizeof(float) * mPathsPerTile, true, true);
			tile.mPrimaryRaySet = VKManager::ProgramAllocateDescriptorSet(mDevice, mPrimaryRays);

			//allocate global input buffers
			mPrimaryRaysInputs[iTile] = VKManager::BufferAllocate(mDevice, sizeof(PrimaryRaysShared::GlobalInput), true, true);
			mIntersectionsInputs[iTile] = VKManager::BufferAllocate(mDevice, sizeof(IntersectionShared::GlobalInput), true, true);
		}

		return true;
	}

	void PathTracer::Render()
	{
		size_t iTileOffset = 0;
		do
		{
			size_t tilesToProcess = std::min(mTilesInFlight, mOffsets.size() - iTileOffset);

			//primary rays
			VKManager::Begin(mDevice);
			for (size_t iTileIndex = 0; iTileIndex < tilesToProcess; ++iTileIndex)
			{
				auto& iTile = mTiles[iTileIndex];
				VKManager::Buffer globalsBuffer = mPrimaryRaysInputs[iTileIndex];

				const auto& offset = mOffsets[iTileOffset + iTileIndex];
				PrimaryRaysShared::GlobalInput* pInput = (PrimaryRaysShared::GlobalInput*)VKManager::BufferMap(mDevice, globalsBuffer);
				pInput->mOffsetX = offset.x;
				pInput->mOffsetY = offset.y;
				VKManager::BufferUnMap(mDevice, globalsBuffer);

				float* pSeeds = (float*)VKManager::BufferMap(mDevice, iTile.mSeeds);
				for (uint32_t iSeed = 0; iSeed < mPathsPerTile; ++iSeed)
					pSeeds[iSeed] = rand() / float(RAND_MAX);
				VKManager::BufferUnMap(mDevice, iTile.mSeeds);

				cstr_t names[] = { "gInputs", "gPaths", "gCamera", "gSeeds" };
				VKManager::Buffer buffers[] = { globalsBuffer, iTile.mPathsBuffer, mCameraBuffer, iTile.mSeeds};
				VKManager::ProgramBindInput(mDevice, iTile.mPrimaryRaySet, mPrimaryRays, ARRAYCOUNT(names), names, buffers);
				VKManager::ProgramDispatch(mDevice, iTile.mPrimaryRaySet, mPrimaryRays, mPathsPerTile / mLocalSize, 1, 1);
			}
			VKManager::End(mDevice);
			
			//intersection
			for (size_t iTileIndex = 0; iTileIndex < tilesToProcess; ++iTileIndex)
			{
				auto& iTile = mTiles[iTileIndex];
				cstr_t names[] = { "gInputs", "gHits", "gPaths", "gInstances", "gTopLevelAS", "gProceduralGeo" };
				VKManager::Buffer buffers[] = { mIntersectionsInputs[iTileIndex], iTile.mHitsBuffer, iTile.mPathsBuffer, mInstancesBuffer, mTopLevelASBuffer, mProceduralGeoBuffer };
				VKManager::ProgramBindInput(mDevice, iTile.mPrimaryRaySet, mPrimaryRays, ARRAYCOUNT(names), names, buffers);
				VKManager::ProgramDispatch(mDevice, iTile.mPrimaryRaySet, mPrimaryRays, mPathsPerTile / mLocalSize, 1, 1);
			}


			iTileOffset += tilesToProcess;//<---- Has to be the last
		} while (iTileOffset < mOffsets.size());


		//Image rgba;
		//rgba.Initialize({ mCamera.GetSensorResolution().x,mCamera.GetSensorResolution().y }, Format::eUbyte4, (byte_t*)VKManager::BufferMap(mDevice, mRGBABuffer));
		//rgba.WriteToFile("C:\\Users\\sergemetral\\Desktop\\test.png");
	}

	void PathTracer::UpdateSceneData(std::vector<Vertex>& vertexBuffer, std::vector<int3>& indexBuffer, std::vector<MeshEntry>& entries,
		BVH& topLevelAS, BVH& bottomLevelAS, 
		std::vector<Instance>& instances,
		std::vector<ProceduralGeometry>& procGeo)
	{
		//copy instances
		{
			Instance* pInstances = (Instance*)VKManager::BufferMap(mDevice, mInstancesBuffer);
			std::memcpy(pInstances, instances.data(), sizeof(Instance) * instances.size());
			VKManager::BufferUnMap(mDevice, mInstancesBuffer);
		}

		//copy geometry
		{
			Vertex* pVerts = (Vertex*)VKManager::BufferMap(mDevice, mVertexBuffer);
			std::memcpy(pVerts, vertexBuffer.data(), sizeof(Vertex) * vertexBuffer.size());
			VKManager::BufferUnMap(mDevice, mVertexBuffer);

			int3* pFaces = (int3*)VKManager::BufferMap(mDevice, mIndexBuffer);
			std::memcpy(pFaces, indexBuffer.data(), sizeof(int3) * indexBuffer.size());
			VKManager::BufferUnMap(mDevice, mIndexBuffer);

			MeshEntry* pMeshes = (MeshEntry*)VKManager::BufferMap(mDevice, mMeshesBuffer);
			std::memcpy(pMeshes, entries.data(), sizeof(MeshEntry) * entries.size());
			VKManager::BufferUnMap(mDevice, mMeshesBuffer);
			
			if (procGeo.size() > 0)
			{
				ProceduralGeometry* pProcGeo = (ProceduralGeometry*)VKManager::BufferMap(mDevice, mProceduralGeoBuffer);
				std::memcpy(pProcGeo, procGeo.data(), sizeof(ProceduralGeometry) * procGeo.size());
				VKManager::BufferUnMap(mDevice, mProceduralGeoBuffer);
			}
		}

		//BVHs
		{
			BVH* pBottomL = (BVH*)VKManager::BufferMap(mDevice, mBottomLevelAS);
			std::memcpy(pBottomL, bottomLevelAS.GetNodes().data(), sizeof(BVH) * bottomLevelAS.GetNodes().size());
			VKManager::BufferUnMap(mDevice, mBottomLevelAS);

			BVH* pTopL = (BVH*)VKManager::BufferMap(mDevice, mTopLevelAS);
			std::memcpy(pTopL, topLevelAS.GetNodes().data(), sizeof(BVH) * topLevelAS.GetNodes().size());
			VKManager::BufferUnMap(mDevice, mTopLevelAS);
		}
	}

	void PathTracer::UpdateCameraData(Camera& camera)
	{
		const uint2& camRes = camera.GetSensorResolution();
		//mOffsets = TileRenderResolution(camRes.x, camRes.y);
		mOffsets.clear();
		{
			uint32_t x = 0;
			uint32_t y = 0;
			while (y < camRes.y)
			{
				while (x < camRes.x)
				{
					mOffsets.push_back({ x, y });
					x += mTileRes;
				}
				y += mTileRes;
				x = 0;
			}
		}

		//init camera
		GPUCamera* pCam = (GPUCamera*)VKManager::BufferMap(mDevice, mCameraBuffer);
		pCam->mFocalLength = camera.GetFocalLength() / 1000.0f;//to meter
		pCam->mFocusDistance = camera.GetFocusDistance() / 1000.0f;//to meter
		pCam->mLenseRadius = 0;// mCamera.GetLenseDiameter() / 2.0f / 1000.0f;//to meter
		camera.GetInverseViewMatrix(pCam->mTransform);
		pCam->mSensorDimension = { camera.GetSensorDimension().x / 1000.0f, camera.GetSensorDimension().y / 1000.0f };
		pCam->mSensorResolution = camera.GetSensorResolution();
		VKManager::BufferUnMap(mDevice, mCameraBuffer);
	}
}
