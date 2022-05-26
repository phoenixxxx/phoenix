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
			Mesh():mBVHVisible(false), mVisible(true), mBVHShowOnlyLeaves(true), mVertexCount(0), mFaceCount(0)
			{
			}
			uint32_t mMaterialIndex;
			stdstr_t mName;

			MeshEntry mEntry;

			uint32_t mAABBCount;
			uint32_t mVertexCount;
			uint32_t mFaceCount;
			bool mBVHVisible;
			bool mBVHShowOnlyLeaves;
			bool mVisible;
		};
		float RayBVHIntersect(const Ray& ray, uint64_t instanceIndex, float3& p, float3& n);

		std::vector<Vertex> mVertexBuffer;
		std::vector<int3> mIndexBuffer;
		std::vector<GPUBVHNode> mBottomLevels;
		std::vector<GPUBVHNode> mTopLevel;

		std::vector<Mesh> mMeshes;

		std::vector<Instance> mInstances;


		BVHLighting mBVHLighting;
		bool mShowBVHLighting;
		AABB mLightingAABB;

		std::vector<AABB> mInstanceVolumes;

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
			uint64_t mHitInstanceIndex;
			Ray mRay;
			bool mHit;
			float3 mPoint;
			float3 mNormal;
		}mSurface;
	};
}