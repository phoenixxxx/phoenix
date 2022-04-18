#pragma once

#include <vector>
#include <Utils/Math.h>
#include <Utils/VectorMath.h>
#include <Utils/Types.h>
#include <Utils/AABB.h>
#include <SharedGPU_CPU/SharedHierarchy.h>

namespace Phoenix
{
	class BVHLighting
	{
	public:
		struct ConstructionItem 
		{
			AABB mBox;
			float3 mCentroid;
			float3 mEmissionDirection;
			float3 mFlux;

			uint64_t mLeafData;//the triangle
		};

	public:
		BVHNodeIndex AppendVolumes(AABB& rootBox, std::vector<ConstructionItem>& items);
		void Clear() {
			mNodes.clear();
		}
		bool Empty()const
		{
			return (mNodes.size() == 0);
		}
	private:
		static const uint32_t SAOH_BUCKET_COUNT = 5;
		struct SAOHBucket
		{
			SAOHBucket():mPrimitiveCount(0), mFlux(0, 0, 0), mConeDirection(0, 0, 0), mConeAngle(0) {}
			uint32_t mPrimitiveCount;
			AABB mBounds;
			float3 mFlux;
			float3 mConeDirection;//XYZ:direction and W:angle of emissive cone
			float mConeAngle;
		};
		void ComputeBuckets(uint32_t primStart, uint32_t primEnd, std::vector<SAOHBucket>& buckets, const std::vector<ConstructionItem>& volumes, AABB& centroidBounds, Axis dimension);
		BVHNodeIndex AppendVolumesSAOH(uint32_t primStart, uint32_t primEnd, std::vector<ConstructionItem>& volumes, BVHNodeIndex parent = INVALID_INDEX);

	private:
		std::vector<GPUBVHLightingNode> mNodes;
		float3 mLengthMax;//maximum length of the cluster’s 3D box. (for Ki comp)
	};
}
