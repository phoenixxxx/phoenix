#include "BVHLighting.h"

#define RGB_TO_Y(v)(0.299f*v.x + 0.587f*v.y + 0.114f*v.z)

namespace Phoenix
{
	static float ComputeMOmega(float angle)
	{
		float thetaw = std::min(angle + PiOver2, Pi);//PiOver2 because we assume all emittors are triangles, and have a Pi/2 angle of emission
		float Momega = 2 * Pi * (1 - cosf(angle)) + PiOver2 * (2 * thetaw * sinf(angle) - cosf(angle - 2 * thetaw) - 2 * angle * sinf(angle) + cosf(angle));
		return Momega;
	}
	BVHNodeIndex BVHLighting::AppendVolumes(AABB& rootBox, std::vector<ConstructionItem>& items)
	{
		mLengthMax = float3(-1, -1, -1);
		BVHNodeIndex index = AppendVolumesSAOH(0, uint32_t(items.size()), items);
		rootBox = AABB(mNodes[0].mBoxMin, mNodes[0].mBoxMax);
		return index;
	}

	BVHNodeIndex BVHLighting::AppendVolumesSAOH(uint32_t primStart, uint32_t primEnd, std::vector<ConstructionItem>& emissives, BVHNodeIndex parent)
	{
		assert(primStart < primEnd);
		assert(primEnd <= emissives.size());
		assert(emissives.size() != 0);

		uint32_t count = primEnd - primStart;
		assert(count != 0);

		AABB centroidBounds, volumesBounds;
		float3 coneDirection(0, 0, 0);
		float3 flux(0, 0, 0);
		float coneAngle(0);//parent theta0
		for (size_t i = primStart; i < primEnd; ++i)
		{
			const AABB& v = emissives[i].mBox;
			const float3& c = v.Centroid();
			volumesBounds += v;
			centroidBounds += c;

			flux += emissives[i].mFlux;
			coneDirection += emissives[i].mEmissionDirection;
		}

		/*In the case of a perfect sphere (or a cube) the average direction is the Zero vector
		* in that case the emittor becomes a perfect spherical emittor
		*	Axis: Any unit vector (1,0,0) by convention
		*	theta0=Pi
		*   thetaE=Pi
		*/
		if (coneDirection.isZero())
		{
			coneDirection = float3(1, 0, 0);
			coneAngle = Pi;
		}
		else
		{
			//Compute the cone from all the directions
			coneDirection = float3::normalize(coneDirection);
			for (size_t i = primStart; i < primEnd; ++i)
			{
				float angle = std::acosf(float3::dot(emissives[i].mEmissionDirection, coneDirection));
				if (angle > coneAngle)
					coneAngle = angle;
			}
		}

		//Doesn't work when you get one emittor at a time
		////store lengthmax
		//const float3& volumesBoundsDim = volumesBounds.Dimensions();
		//if (mLengthMax.x == -1)
		//{
		//	mLengthMax = volumesBoundsDim;
		//}

		//Allocate the node
		BVHNodeIndex nodeRef = static_cast<BVHNodeIndex>(mNodes.size()); mNodes.push_back(GPUBVHLightingNode());
		GPUBVHLightingNode& node = mNodes[nodeRef];
		//We cannot create a constructor to GPUBVHNode because GPU shared
		//Set the data here:
		node.mLeafData = INVALID_INDEX;
		node.mLeftChild = INVALID_INDEX;
		node.mRightChild = INVALID_INDEX;

		//node->mBox = volumesBounds;
		node.mBoxMin = float4(volumesBounds.mMin, 1);
		node.mBoxMax = float4(volumesBounds.mMax, 1);

		//cone direction
		node.mConeInfo = float4(coneDirection, coneAngle);

		//flux
		node.mFlux = float4(flux, 0);

		//if we reached the desired leaf node size, store it and bail
		if (count == 1)
		{
			node.mLeafData = emissives[primStart].mLeafData;//store leaf data pointer
			return nodeRef;
		}

		//compute M_omega
		//float thetaw = std::min(coneAngle + PiOver2, Pi);
		//float Momega = 2 * Pi * (1 - cosf(coneAngle)) + PiOver2 * (2 * thetaw * sinf(coneAngle) - cosf(coneAngle - 2 * thetaw) - 2 * coneAngle * sinf(coneAngle) + cosf(coneAngle));
		float Momega = ComputeMOmega(coneAngle);

		uint32_t mid = 0;
		const float3& centroidBoundsDim = centroidBounds.Dimensions();

		Axis dimensions[] = { eX, eY, eZ };
		float parentSurfaceArea = std::max(volumesBounds.SurfaceArea(), FLT_MIN);

		if (((centroidBoundsDim.x == 0) && (centroidBoundsDim.y == 0) && (centroidBoundsDim.z == 0)))//<--- Second check is for the case when all tris share the same AABB
		{
			mid = primStart + std::max(1u, count / 2);
		}
		else
		{
			size_t minCostSplitBucket = size_t(-1);
			float saohMin = FLT_MAX;
			Axis splitDim = eX;
			for (uint32_t iDim = 0; iDim < 3; ++iDim)
			{
				if (centroidBoundsDim.m[iDim] == 0)
					continue;//we cannot split across an axis that is zero!

				std::vector<SAOHBucket> buckets(SAOH_BUCKET_COUNT);
				ComputeBuckets(primStart, primEnd, buckets, emissives, centroidBounds, dimensions[iDim]);
				for (size_t iIter = 0; iIter < buckets.size() - 1; ++iIter)
				{
					AABB b0, b1;
					//first half of the split
					float3 flux0(0, 0, 0), flux1(0, 0, 0);
					float Momega0(0), Momega1(0);
					for (uint32_t iB0 = 0; iB0 <= iIter; ++iB0)
					{
						if (buckets[iB0].mPrimitiveCount > 0)//some slabs might have NO centroids, in that case, skip them
						{
							b0 += buckets[iB0].mBounds;
							flux0 += buckets[iB0].mFlux;

							//float theta0 = buckets[iB0].mConeAngle;
							//float thetaw = std::min(theta0 + PiOver2, Pi);
							//Momega0 = 2 * Pi * (1 - cosf(theta0)) + PiOver2 * (2 * thetaw * sinf(theta0) - cosf(theta0 - 2 * thetaw) - 2 * theta0 * sinf(theta0) + cosf(theta0));
							Momega0 = ComputeMOmega(buckets[iB0].mConeAngle);
						}
					}
					//second half of the split
					for (size_t iB1 = iIter + 1; iB1 < buckets.size(); ++iB1)
					{
						if (buckets[iB1].mPrimitiveCount > 0)//some slabs might have NO centroids, in that case, skip them
						{
							b1 += buckets[iB1].mBounds;
							flux1 += buckets[iB1].mFlux;

							//float theta0 = buckets[iB1].mConeAngle;
							//float thetaw = std::min(theta0 + PiOver2, Pi);
							//Momega1 = 2 * Pi * (1 - cosf(theta0)) + PiOver2 * (2 * thetaw * sinf(theta0) - cosf(theta0 - 2 * thetaw) - 2 * theta0 * sinf(theta0) + cosf(theta0));
							Momega1 = ComputeMOmega(buckets[iB1].mConeAngle);
						}
					}

					//For the common case of uniform emitters (Thetae = Pi/2), the measure MOmega varies smoothly between Pi(for a flat emitter) to 4Pi for a complete sphere of directions.
					static const float MomegaEpsilon = 0.00001f;
					assert(Momega >= Pi); assert(Momega <= (4 * Pi) + MomegaEpsilon);
					assert(Momega0 >= Pi); assert(Momega0 <= (4 * Pi + MomegaEpsilon));
					assert(Momega1 >= Pi); assert(Momega1 <= (4 * Pi) + MomegaEpsilon);

					//compute SAOH
					float Ki = 1;// Fix that-->> mLengthMax.m[iDim] / volumesBoundsDim.m[iDim];
					float saoh = Ki * ((RGB_TO_Y(flux0) * b0.SurfaceArea() * Momega0) + (RGB_TO_Y(flux1) * b1.SurfaceArea() * Momega1)) / (parentSurfaceArea * Momega);

					if (saohMin > saoh)
					{
						//found a better split
						saohMin = saoh;
						minCostSplitBucket = iIter;//for this dimension, we have the bucket index with min SAH
						splitDim = dimensions[iDim];
					}
				}

				assert(minCostSplitBucket != size_t(-1));
				assert(minCostSplitBucket < SAOH_BUCKET_COUNT);

				//partition the volumes, this only rearranges nodes that need moving
				auto pmid = std::partition(&emissives[primStart], (&emissives[primEnd - 1]) + 1, [=](const ConstructionItem& item) //ptr + 1 to emulate v.end()
					{
						const float3& c = item.mCentroid; //get the centroid

						float offset = centroidBounds.Offset(c).m[splitDim];//GetOffsetAABB(centroidBounds, c).m[splitDim];
						uint32_t b = static_cast<uint32_t>(SAOH_BUCKET_COUNT * offset); //tells us the index of the bucket in which this centroid falls;
						if (b == SAOH_BUCKET_COUNT)
							b = SAOH_BUCKET_COUNT - 1;

						return b <= minCostSplitBucket;
					});

				//Iterator to the first element of the second group.
				//that gives us the # of elements in the first volume
				mid = uint32_t(pmid - &emissives[0]);
			}
		}

		assert(mid > primStart);
		assert(mid < primEnd);

		//recurse down left child
		BVHNodeIndex leftChild = AppendVolumesSAOH(primStart, mid, emissives);
		//recurse down right child
		BVHNodeIndex rightChild = AppendVolumesSAOH(mid, primEnd, emissives);

		//grab the node again (vector data has changed)
		mNodes[nodeRef].mLeftChild = leftChild;
		mNodes[nodeRef].mRightChild = rightChild;

		return nodeRef;
	}

	void BVHLighting::ComputeBuckets(uint32_t primStart, uint32_t primEnd, std::vector<SAOHBucket>& buckets, const std::vector<ConstructionItem>& emissives, AABB& centroidBounds, Axis dimension)
	{
		for (uint32_t iPrim = primStart; iPrim < primEnd; ++iPrim)
		{
			const AABB& v = emissives[iPrim].mBox;//get the volume
			const float3& c = emissives[iPrim].mCentroid;//get the centroid

			float offset = centroidBounds.Offset(c).m[dimension];//GetOffsetAABB(centroidBounds, c).m[dimension];
			assert(offset >= 0);
			uint32_t b = static_cast<uint32_t>(SAOH_BUCKET_COUNT * offset);//tells us the index of the bucket in which this centroid falls;
			if (b == SAOH_BUCKET_COUNT) b = SAOH_BUCKET_COUNT - 1;

			buckets[b].mPrimitiveCount++;//one more centroid within this slab
			buckets[b].mBounds += v;//grow this slab's bound

			buckets[b].mConeDirection += emissives[iPrim].mEmissionDirection;
			buckets[b].mFlux += emissives[iPrim].mFlux;
		}

		//normalize all the direction
		for (auto& bucket : buckets)
		{
			if (bucket.mConeDirection.isZero())
				bucket.mConeDirection = float3(1, 0, 0);
			else
				bucket.mConeDirection = float3::normalize(bucket.mConeDirection);
		}

		//compute the cone angle
		for (uint32_t iPrim = primStart; iPrim < primEnd; ++iPrim)
		{
			const float3& c = emissives[iPrim].mCentroid;//get the centroid
			float offset = centroidBounds.Offset(c).m[dimension];
			uint32_t b = static_cast<uint32_t>(SAOH_BUCKET_COUNT * offset);//tells us the index of the bucket in which this centroid falls;
			if (b == SAOH_BUCKET_COUNT) b = SAOH_BUCKET_COUNT - 1;

			float angle = std::acosf(float3::dot(buckets[b].mConeDirection, emissives[iPrim].mEmissionDirection));
			if (angle > buckets[b].mConeAngle)
				buckets[b].mConeAngle = angle;
		}
	}
}