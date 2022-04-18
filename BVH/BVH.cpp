#include "BVH.h"
#include <Utils/Console.h>

namespace Phoenix
{
	const float BVH::INTERSECT_COST = 1.0f;//effectively not used (no need to multiply by 1). Kept here for informational use only.
	const float BVH::TRAVERSAL_COST = 1 / 8.0f;

	BVHNodeIndex BVH::CreateVolumes(AABB& rootBox, std::vector<ConstructionItem*> items, BVH::BuildPolicy policy)
	{
		BVHNodeIndex rootRef = 0;
		switch (policy)
		{
		case eSurfaceAreaHuristic:
		{
			rootRef = CreateVolumesSAH(0, uint32_t(items.size()), items);
		}
		break;
		case eSpaceFillingCurve:
		{
		}
		break;
		}
		GPUBVHNode& node = mNodes[rootRef];
		rootBox = AABB(node.mBoxMin, node.mBoxMax);
		return rootRef;
	}

	BVHNodeIndex BVH::CreateVolumesSAH(uint32_t primStart, uint32_t primEnd, std::vector<ConstructionItem*>& volumes)
	{
		assert(primStart < primEnd);
		assert(primEnd <= volumes.size());
		assert(volumes.size() != 0);

		uint32_t count = primEnd - primStart;
		assert(count != 0);

		AABB centroidBounds, volumesBounds;
		for (size_t i = primStart; i < primEnd; ++i)
		{
			const AABB& v = volumes[i]->mBox;
			const float3& c = v.Centroid();
			volumesBounds += v;//compute the volume encompassing all AABBs
			centroidBounds += c;//compute the volume encompassing all centroids
		}
		//assert(GetSurfaceAreaAABB(centroidBounds) > 0);
		//assert(GetSurfaceAreaAABB(volumesBounds) > 0);

		//Allocate the node
		BVHNodeIndex nodeRef = static_cast<BVHNodeIndex>(mNodes.size()); mNodes.push_back(GPUBVHNode());
		GPUBVHNode& node = mNodes[nodeRef];
		//We cannot create a constructor to GPUBVHNode because GPU shared
		//Set the data here:
		node.mLeafData = INVALID_INDEX;
		node.mLeftChild = INVALID_INDEX;
		node.mRightChild = INVALID_INDEX;
		node.mParent = INVALID_INDEX;
		node.mBoxMin = float4(volumesBounds.mMin, 1);
		node.mBoxMax = float4(volumesBounds.mMax, 1);
		//if we reached the desired leaf node size, store it and bail
		if (count == 1)
		{
			node.mLeafData = volumes[primStart]->mLeafData;//store leaf data pointer
			//auto type = BVHReference(node->mLeafData).GetType();
			return nodeRef;
		}

		uint32_t mid = 0;

		const float3& centroidBoundsDim = centroidBounds.Dimensions();
		//with small enough triangle count, SAH is unstable, any split is ok at that point
		if (count <= SAH_MIN_SPLIT_COUNT)
		{
			mid = primStart + std::max(1u, count / 2);
		}
		//if all tris have the same centroid, the centroidBoundsDim is zero
		else if ((centroidBoundsDim.x <= Epsilon) && (centroidBoundsDim.y <= Epsilon) && (centroidBoundsDim.z <= Epsilon))
		{
			mid = primStart + std::max(1u, count / 2);
		}
		else
		{
			size_t minCostSplitBucket = size_t(-1);
			//iterate over the SAH
			Axis dimensions[] = { eX, eY, eZ };
			float parentSurfaceArea = volumesBounds.SurfaceArea();

			float sahMin = FLT_MAX;
			Axis splitDim = eX;
			assert((centroidBoundsDim.x > 0) || (centroidBoundsDim.y > 0) || (centroidBoundsDim.z > 0));
			for (uint32_t iDim = 0; iDim < 3; ++iDim)
			{
				if (centroidBoundsDim.m[iDim] <= Epsilon)
					continue;//we cannot plit across an axis that is zero!

				std::vector<SAHBucket> buckets(SAH_BUCKET_COUNT);
				ComputeBuckets(primStart, primEnd, buckets, volumes, centroidBounds, dimensions[iDim]);
				for (size_t iIter = 0; iIter < buckets.size() - 1; ++iIter)
				{
					AABB b0, b1;
					uint32_t count0(0), count1(0);
					//first half of the split
					for (uint32_t iB0 = 0; iB0 <= iIter; ++iB0)
					{
						if (buckets[iB0].mPrimitiveCount > 0)//some slabs might have NO centroids, in that case, skip them
						{
							b0 += buckets[iB0].mBounds;
							count0 += buckets[iB0].mPrimitiveCount;
						}
					}
					//second half of the split
					for (size_t iB1 = iIter + 1; iB1 < buckets.size(); ++iB1)
					{
						if (buckets[iB1].mPrimitiveCount > 0)//some slabs might have NO centroids, in that case, skip them
						{
							b1 += buckets[iB1].mBounds;
							count1 += buckets[iB1].mPrimitiveCount;
						}
					}

					//compute SAH
					float sah = TRAVERSAL_COST + (b0.SurfaceArea() * count0 + b1.SurfaceArea() * count1) / std::max(1.0f, parentSurfaceArea);//we technically do process AABBs with a volume of 0 in the case when one or more dim is 0
					//if(sah != sah)
					//	printf("parentSurfaceArea= %f, sah:%f\n", parentSurfaceArea, sah);

					if (sahMin > sah)
					{
						//found a better split
						sahMin = sah;
						minCostSplitBucket = iIter;//for this dimension, we have the bucket index with min SAH
						splitDim = dimensions[iDim];
					}
				}
			}

			assert(minCostSplitBucket != size_t(-1));
			assert(minCostSplitBucket < SAH_BUCKET_COUNT);

			//partition the volumes, this only rearranges nodes that need moving
			auto pmid = std::partition(&volumes[primStart], (&volumes[primEnd - 1]) + 1, [=](const ConstructionItem* item) //ptr + 1 to emulate v.end()
				{
					const float3& c = item->mCentroid; //get the centroid

					float offset = centroidBounds.Offset(c).m[splitDim];
					uint32_t b = static_cast<uint32_t>(SAH_BUCKET_COUNT * offset); //tells us the index of the bucket in which this centroid falls;
					if (b == SAH_BUCKET_COUNT)
						b = SAH_BUCKET_COUNT - 1;

					return b <= minCostSplitBucket;
				});

			//Iterator to the first element of the second group.
			//that gives us the # of elements in the first volume
			mid = uint32_t(pmid - &volumes[0]);
		}

		assert(mid > primStart);
		assert(mid < primEnd);

		//recurse down left child
		BVHNodeIndex leftChild = CreateVolumesSAH(primStart, mid, volumes);
		//recurse down right child
		BVHNodeIndex rightChild = CreateVolumesSAH(mid, primEnd, volumes);

		//grab the node again (vector data has changed)
		mNodes[nodeRef].mLeftChild = leftChild;
		mNodes[nodeRef].mRightChild = rightChild;

		return nodeRef;
	}

	void BVH::ComputeBuckets(uint32_t primStart, uint32_t primEnd, std::vector<SAHBucket>& buckets, const std::vector<ConstructionItem*>& volumes, const AABB& centroidBounds, Axis dimension)
	{
		for (uint32_t iVolume = primStart; iVolume < primEnd; ++iVolume)
		{
			const AABB& v = volumes[iVolume]->mBox;//get the volume
			const float3& c = volumes[iVolume]->mCentroid;//get the centroid

			float offset = centroidBounds.Offset(c).m[dimension];
			uint32_t b = static_cast<uint32_t>(SAH_BUCKET_COUNT * offset);//tells us the index of the bucket in which this centroid falls;
			if (b == SAH_BUCKET_COUNT) b = SAH_BUCKET_COUNT - 1;

			buckets[b].mPrimitiveCount++;//one more centroid within this slab
			buckets[b].mBounds += v;//grow this slab's bound
		}
	}

	void BVH::Print(BVHNodeIndex root)
	{
		PrintRec(root, mNodes, "");
	}
	uint32_t BVH::GetDepth(BVHNodeIndex root)
	{
		return GetDepthRec(root);
	}

	void BVH::PrintRec(BVHNodeIndex nodeRef, std::vector<GPUBVHNode>& nodes, const stdstr_t& offset)
	{
		GPUBVHNode& node = nodes[nodeRef];
		AABB nodeBox(node.mBoxMin, node.mBoxMax);
		const float3& dim = nodeBox.Dimensions();

		if (node.mLeafData != INVALID_INDEX)
		{
			Console::Instance()->Log(Console::LogType::eInfo, "%s[%d](%f,%f,%f) [%s index:%d]\n", offset.c_str(), nodeRef, dim.x, dim.y, dim.z, "Leaf", node.mLeafData);
		}
		else
		{
			Console::Instance()->Log(Console::LogType::eInfo, "%s[%d](%f,%f,%f)\n", offset.c_str(), nodeRef, dim.x, dim.y, dim.z);
			stdstr_t offset2 = offset + "\t";
			PrintRec(node.mLeftChild, nodes, offset2);
			PrintRec(node.mRightChild, nodes, offset2);
		}
	}
	uint32_t BVH::GetDepthRec(BVHNodeIndex root)
	{
		if (root == INVALID_INDEX)
			return 0;

		GPUBVHNode& node = mNodes[root];

		uint32_t leftDepth = GetDepthRec(node.mLeftChild);
		uint32_t rightDepth = GetDepthRec(node.mRightChild);

		return 1 + std::max(leftDepth, rightDepth);
	}
}
