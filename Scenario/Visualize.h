#pragma once
#include <vector>
#include <memory>
#include "Scenario.h"
#include <SharedGPU_CPU/SharedGeometry.h>
#include <Utils/VectorMath.h>
#include <BVH/BVH.h>
#include <BVHLighting/BVHLighting.h>
#include <Image/Image.h>
#include <SkyIllumination/Sky.h>
#include <ThirdParty/tinyobjloader/tiny_obj_loader.h>

namespace Phoenix
{
	class Visualize : public Scenario
	{
	public:
		Visualize();
	public:
		virtual cstr_t Name();
		virtual ~Visualize();
		virtual bool Initialize();
		virtual void Run();

	public:
		virtual void MouseMove(const MouseInfo& info);
		virtual void MouseClick(const MouseInfo& info);
		virtual void MouseDrag(const MouseInfo& info);
		virtual void MouseWheel(const MouseInfo& info);

		virtual void Resize(uint32_t width, uint32_t height);

	private:
		const float mCameraSpeed = 1.0f / 100;
		void DrawUI();
	private:
		uint32_t mMouseClickX, mMouseClickY;

	private:
		void RebuildBVHs();
		void RebuildLighting();
		void FileLoadHandler();
		void SceneExportHander();
		void PrimaryRay(Ray& ray, const uint2& coord);

		struct Mesh
		{
			Mesh():mBVHVisible(false), mVisible(true), mBVHShowOnlyLeaves(true) {}
			Mesh( Mesh&& rhs) noexcept
			{ 
				mVertexBuffer = std::move(rhs.mVertexBuffer);
				mIndexBuffer = std::move(rhs.mIndexBuffer);
				mName = std::move(rhs.mName);

				mBVHVisible = rhs.mBVHVisible;
				mVisible = rhs.mVisible;
				mBVHShowOnlyLeaves = rhs.mBVHShowOnlyLeaves;
				mMaterialIndex = rhs.mMaterialIndex;
			}
			std::vector<Vertex> mVertexBuffer;
			std::vector<int3> mIndexBuffer;
			uint32_t mMaterialIndex;
			stdstr_t mName;
			bool mBVHVisible;
			bool mBVHShowOnlyLeaves;
			bool mVisible;
		};
		float RayBVHIntersect(const Ray& ray, const BVH& bvh, Mesh& mesh, float3& p, float3& n);

		std::vector<Mesh> mMeshes;
		std::vector<BVH>  mBVHs;
		BVHLighting mBVHLighting;
		bool mShowBVHLighting;
		AABB mLightingAABB;


		std::vector<tinyobj::material_t> materials;
		std::vector<uint32_t> materialsIsMetallic;
		std::vector<float> materialsEmissionIntensity;
		std::map<stdstr_t, Image*> mMaps;

		struct 
		{
			Sky mSky;
			stdstr_t mPath;
			int mResolution;
			float mAlbedo, mTurbidity, mElevation;
		}mHDRI;

		struct
		{
			uint32_t mSensorWidth;
			uint2 mResolution;
			float3 mFocusPoint;
			float mFStop;
			int mFocalLength;
			float mAspect;
		}mOptics;

		struct
		{
			Ray mRay;
			bool mHit;
			float3 mPoint;
			float3 mNormal;
		}mSurface;
	};
}