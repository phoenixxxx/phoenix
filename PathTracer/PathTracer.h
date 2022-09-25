#pragma once
#include <vector>
#include <memory>

#include <SharedGPU_CPU/SharedGeometry.h>
#include <SharedGPU_CPU/SharedScene.h>
#include <SharedGPU_CPU/SharedGeometry.h>

#include <BVH/BVH.h>
#include <Camera/Camera.h>
#include <VK/VKManager.h>

namespace Phoenix
{
	class PathTracer
	{
	public:
		PathTracer();
		~PathTracer();
		bool Initialize();
		void Render();

		void UpdateSceneData(std::vector<Vertex>& vertexBuffer, std::vector<int3>& IndexBuffer, std::vector<MeshEntry>& entries,
			BVH& topLevelAS, BVH& bottomLevelAS, 
			std::vector<Instance>& instances,
			std::vector<ProceduralGeometry>& procGeo);
		void UpdateCameraData(Camera& camera);

	private:
		VKManager::Device mDevice;

		VKManager::Buffer mCameraBuffer;
		VKManager::Buffer mIndexBuffer;
		VKManager::Buffer mVertexBuffer;
		VKManager::Buffer mMeshesBuffer;

		VKManager::Buffer mInstancesBuffer;
		VKManager::Buffer mTopLevelASBuffer;
		VKManager::Buffer mProceduralGeoBuffer;

		VKManager::Buffer mTopLevelAS;
		VKManager::Buffer mBottomLevelAS;

		VKManager::Program mPrimaryRays;
		enum eHitType {eStore=0, eNoStore=1, eCount};
		VKManager::Program mIntersection[eHitType::eCount];

		static const size_t mTilesInFlight = 8;
		VKManager::Buffer  mPrimaryRaysInputs[mTilesInFlight];
		VKManager::Buffer  mIntersectionsInputs[mTilesInFlight];

		struct {
			VKManager::Buffer  mPathsBuffer;
			VKManager::Buffer  mHitsBuffer;
			VKManager::Buffer  mSeeds;
			VkDescriptorSet    mPrimaryRaySet;
		}mTiles[mTilesInFlight];
		uint32_t mCurrentTileIndex = 0;

	private:
		static const uint32_t mLocalSize = 64;
		static const uint32_t mTileRes = 512;
		static const uint32_t mPathsPerTile = mTileRes * mTileRes;
		static_assert (mPathsPerTile% mLocalSize == 0);

		std::vector<uint2> mOffsets;
	};
}