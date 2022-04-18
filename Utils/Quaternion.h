#pragma once
#include <assert.h>
#include "Math.h"
#include "VectorMath.h"

class quaternion
{
public:
	quaternion(float _x = 0, float _y = 0, float _z = 0, float _w = 1) : x(_x), y(_y), z(_z), w(_w)
	{

	}
	quaternion(const float3 v) :x(v.x), y(v.y), z(v.z), w(0)
	{

	}
	void makeRotation(const float4& axis, float angle)
	{
		assert(!Phoenix::Math::IsNaN(axis));
		//assert(axis.isNormalized());
		float s = sinf(angle / 2);
		x = axis.x * s;
		y = axis.y * s;
		z = axis.z * s;
		w = cosf(angle / 2);
		assert(isNormalized());
	}
	void makeRotation(float thetaX, float thetaY, float thetaZ)
	{
		quaternion res;

		quaternion Qx, Qy, Qz;
		Qx.makeRotation(float4(1, 0, 0), thetaX);
		Qy.makeRotation(float4(0, 1, 0), thetaY);
		Qz.makeRotation(float4(0, 0, 1), thetaZ);

		//right to left to follow matrix mul	
		// <----| Qz * Qy * Qx;
		*this = Qz * Qy * Qx;
	}
	quaternion getConjugate() { return quaternion(-x, -y, -z, w); }
	float4x4 getMatrix()const
	{
		float4x4 output;
		float xSqr = x * x; float ySqr = y * y; float zSqr = z * z; float wSqr = w * w;

		output.m[0] = wSqr + xSqr - ySqr - zSqr;      output.m[4] = 2 * x * y - 2 * w * z;          output.m[8] = 2 * x * z + 2 * w * y;          output.m[12] = 0;
		output.m[1] = 2 * x * y + 2 * w * z;          output.m[5] = wSqr - xSqr + ySqr - zSqr;      output.m[9] = 2 * y * z - 2 * w * x;          output.m[13] = 0;
		output.m[2] = 2 * x * z - 2 * w * y;          output.m[6] = 2 * y * z + 2 * w * x;          output.m[10] = wSqr - xSqr - ySqr + zSqr;     output.m[14] = 0;
		output.m[3] = 0;                              output.m[7] = 0;                              output.m[11] = 0;                             output.m[15] = 1;
		return output;
	}
	float3 rotate(const float3& vec)
	{
		quaternion v = vec;
		quaternion rot = (*this) * v * getConjugate();
		return float3(rot.x, rot.y, rot.z);
	}
	//this->operator*(q2)
	//q1 * q2
	//first apply q2, then q1 (read left to right, execute right to left)
	quaternion operator*(const quaternion& q2)
	{
		const quaternion& q1 = *this;
		quaternion muled;
		muled.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
		muled.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
		muled.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
		muled.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
		return muled;
	}
	quaternion operator*(float factor)const
	{
		quaternion muled
		(
			x * factor,
			y * factor,
			z * factor,
			w * factor
		);
		return muled;
	}
	float  lengthSqrd()const
	{
		return (x * x + y * y + z * z + w * w);
	}
	float  length()const
	{
		return sqrtf(lengthSqrd());
	}
	static float  length(const quaternion& q)
	{
		return sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	}
	static quaternion normalize(const quaternion q)
	{
		return q * (1.0f / length(q));
	}
	bool   isNormalized()const
	{
		return Phoenix::Math::Equals(length(), 1.0f);
	}

public:
	//data
	float x, y, z, w;
};