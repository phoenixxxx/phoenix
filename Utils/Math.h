#pragma once
#define NOMINMAX

#include "Types.h"
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cfloat>
#include <assert.h>

#include "VectorMath.h"

extern const float E;
extern const float Log10E;
extern const float Log2E;
extern const float Pi;
extern const float OneDivPi;
extern const float OneDiv2Pi;
extern const float OneDiv4Pi;
extern const float PiOver2;
extern const float PiOver4;
extern const float TwoPi;
extern const float PiOver180;
extern const float InvPiOver180;
extern const float Infinity;
extern const float Epsilon;

namespace Phoenix
{
	enum Axis { eX = 0, eY = 1, eZ = 2 };

	namespace Math
	{
		inline static bool Equals(double f0, double f1)
		{
			double delta = f0 - f1;
			return (delta > -std::numeric_limits<double>::epsilon()) && (delta < std::numeric_limits<double>::epsilon());
		}
		inline static bool Equals(float f0, float f1)
		{
			float delta = f0 - f1;
			return (delta > -std::numeric_limits<float>::epsilon()) && (delta < std::numeric_limits<float>::epsilon());
		}
		inline static bool IsNaN(const float4& v)
		{
			return (std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z) || std::isnan(v.w));
		}
		inline static float ToRadians(float degrees)
		{
			return (PiOver180 * degrees);
		}
		inline static float ToDegrees(float radians)
		{
			return (InvPiOver180 * radians);
		}
		inline static float  Lerp(float t, float v1, float v2) { return (1 - t) * v1 + t * v2; }
		inline static float4 Lerp(float t, const float4& v1, const float4& v2) 
		{ 
			float4 delta = (v2 - v1);
			return v1 + t * delta;
		}

		static inline float atan0_to_2pi(float y, float x)
		{
			float angle = atan2(y, x);
			if (y >= 0)
				return angle;
			assert(angle < 0);
			return TwoPi + angle;
		}

		static inline float2 Spherical(const float3& cart)
		{
			return float2(
				atan0_to_2pi(cart.y, cart.x),// + Pi,
				acos(cart.z)
			);
		}

		static inline float3 Cartesian(const float2& sph)
		{
			//Phi = X; Theta = Y
			return float3(
				cos(sph.x) * sin(sph.y),
				sin(sph.x) * sin(sph.y),
				cos(sph.y)
			);
		}

		template <typename T>
		inline static T Clamp(const T& v, const T& lo, const T& hi){return std::max(std::min(v, hi), lo); }

		inline static int Nearest(float v) 
		{ 
			if(v < 0)
				return static_cast<int>(v - 0.5f);
			return static_cast<int>(v + 0.5f); 
		}

		inline static bool Polynomial(float a, float b, float c, float& x0, float& x1)
		{
			float discr = b * b - 4 * a * c;
			if (discr < 0) 
				return false;
			else if (discr == 0) 
				x0 = x1 = -0.5 * b / a;
			else 
			{
				float q = (b > 0) ?
					-0.5 * (b + sqrt(discr)) :
					-0.5 * (b - sqrt(discr));
				x0 = q / a;
				x1 = c / q;
			}
			if (x0 > x1) 
				std::swap(x0, x1);

			return true;
		}
	}
}

