#pragma once

#include <numeric>
#include <algorithm>
#include "Math.h"
#include "VectorMath.h"
#include "Ray.h"

namespace Phoenix
{
	class AABB
	{
	public:
		AABB() : mMin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()), mMax(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()) {
		}

		AABB(const float4& min, const float4& max) :mMin(min.x, min.y, min.z), mMax(max.x, max.y, max.z)
		{
		}

		AABB& operator+=(const float3& point)
		{
			//min
			mMin = float3::min(point, mMin);
			//max
			mMax = float3::max(point, mMax);
			return *this;
		}
		AABB& operator+=(const AABB& box)
		{
			//absor the box's min
			(*this) += (box.mMin);
			//absorb the box's max
			(*this) += (box.mMax);

			return *this;
		}

		float3 Centroid()const
		{
			return (mMin + mMax) * 0.5f;
		}

		float3 Dimensions()const
		{
			return (mMax - mMin);
		}

		float Volume()const
		{
			const float3& dimensions = Dimensions();
			return (dimensions.x * dimensions.y * dimensions.z);
		}

		float SurfaceArea()const
		{
			const float3& dimensions = Dimensions();
			float L = dimensions.x;
			float h = dimensions.y;
			float W = dimensions.z;

			return 2 * ((h * W) + (h * L) + (W * L));
		}

		/*
		float3 o = p - box.mMin;//box local space
		//normalization
		o.x /= (box.mMax.x - box.mMin.x);
		o.y /= (box.mMax.y - box.mMin.y);
		o.z /= (box.mMax.z - box.mMin.z);
		return o;
		*/
		//Turns an arbitrary point into a voxelized space where 
		//the unit voxel is this AABB
		//ie: P = aabb.pos + X * aabb.x + Y * aabb.y + Z * aabb.z
		//this function returns X,Y,Z
		float3 Offset(float3 p)const
		{
			const float3& dimensions = Dimensions();
			float3 o = p - mMin;//box local space
			//normalization
			o.x /= (dimensions.x);
			o.y /= (dimensions.y);
			o.z /= (dimensions.z);
			return o;
		}

		bool Intersect(const Ray& r)
		{
			float p[] = { r.mOrigin.x, r.mOrigin.y, r.mOrigin.z };
			float d[] = { r.mDirection.x, r.mDirection.y, r.mDirection.z };
			float aMin[] = { mMin.x, mMin.y, mMin.z };
			float aMax[] = { mMax.x, mMax.y, mMax.z };

			float tmin = -FLT_MAX;
			float tmax = FLT_MAX;
			float4 minNorm, maxNorm;

			// check vs. all three 'slabs' of the aabb
			for (int i = 0; i < 3; ++i)
			{
				if (fabs(d[i]) < Epsilon)
				{   // ray is parallel to slab, no hit if origin not within slab
					if (p[i] < aMin[i] || p[i] > aMax[i])
					{
						return false;
					}
				}
				else
				{
					// compute intersection t values of ray with near and far plane of slab
					float ood = 1.0f / d[i];
					float t1 = (aMin[i] - p[i]) * ood;
					float t2 = (aMax[i] - p[i]) * ood;

					tmin = std::max(tmin, std::min(t1, t2));
					tmax = std::min(tmax, std::max(t1, t2));

					// exit with no collision as soon as slab intersection becomes empty
					if (tmin > tmax)
					{
						return false;
					}
				}
			}

			if (tmax < 0.f)
			{
				// entire bounding box is behind us
				return false;
			}
			else if (tmin < 0.f)
			{
				// we are inside the bounding box
				//*out_tmin = 0.f;
				return true;
			}
			else
			{
				// ray intersects all 3 slabs. return point and normal of intersection
				//*out_tmin = tmin;
				return true;
			}
		}

	public:
		float3 mMin, mMax;
	};
}
