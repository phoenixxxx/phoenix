#pragma once

#include <vector>
#include <Utils/Math.h>
#include <Utils/VectorMath.h>
#include <Utils/Types.h>
#include <SharedGPU_CPU/SharedHierarchy.h>
#include <Utils/AABB.h>

namespace Phoenix
{
	class BVH
	{
	public:
		enum BuildPolicy { eSurfaceAreaHuristic, eSpaceFillingCurve };
		
		class ConstructionItem
		{
		public:
			ConstructionItem() :mLeafData(INVALID_INDEX) {}
		public:
			AABB mBox;
			float3 mCentroid;
			uint64_t mLeafData;
		};
		BVH()
		{

		}
		BVH(BVH&& rhs)noexcept
		{
			mNodes = std::move(rhs.mNodes);
		}

	public:
		BVHNodeIndex CreateVolumes(AABB& rootBox, std::vector<ConstructionItem*> items, BuildPolicy policy);
		void Print(BVHNodeIndex root);
		uint32_t GetDepth(BVHNodeIndex root);

		typedef  std::vector<GPUBVHNode> NodeList_t;
		const NodeList_t& GetNodes()const { return mNodes; }

	private:
		enum Axis { eX = 0, eY = 1, eZ = 2 };
		static const float INTERSECT_COST;
		static const float TRAVERSAL_COST;
		static const uint32_t SAH_BUCKET_COUNT = 5;
		static const uint32_t SAH_MIN_SPLIT_COUNT = 4;

		struct SAHBucket
		{
			SAHBucket() :mPrimitiveCount(0) {}
			uint32_t mPrimitiveCount;
			AABB mBounds;
		};

	private:
		BVHNodeIndex CreateVolumesSAH(uint32_t primStart, uint32_t primEnd, std::vector<ConstructionItem*>& volumes);
		void ComputeBuckets(uint32_t primStart, uint32_t primEnd, std::vector<SAHBucket>& buckets, const std::vector<ConstructionItem*>& volumes, const AABB& centroidBounds, Axis dimension);
		void PrintRec(BVHNodeIndex nodeRef, std::vector<GPUBVHNode>& nodes, const stdstr_t& offset);
		uint32_t GetDepthRec(BVHNodeIndex root);
	private:
		std::vector<GPUBVHNode> mNodes;
	};
}