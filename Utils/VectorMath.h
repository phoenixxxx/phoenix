#pragma once
#define NOMINMAX
#include "Types.h"

struct float4
{
	float4(float x = 0, float y = 0, float z = 0, float w = 1);
	float4(const struct float3&xyz, float w = 1);
	float4(const struct float2& xy, float z = 0, float w = 1);
	float4(const struct float2& xy, const struct float2& zw);

	float4 operator+(const float4& right)const;
	float4 operator-(const float4& right)const;
	float4 operator*(const float4& right)const;
	float4 operator*(float factor)const;
	float4 operator/(float factor)const;
	float4 friend operator*(float factor, const float4& right);

	float4& operator+=(const float4& right);
	float& operator[](int index) { return m[index]; }
	const float& operator[](int index)const { return m[index]; }

	//data
	union
	{
		float m[4];
		struct {
			float x, y, z, w;
		};
	};
};

struct uint2
{
	uint2(uint32_t x = 0, uint32_t y = 0);
	uint32_t x, y;
};
struct int2
{
	int2(int32_t x = 0, int32_t y = 0);
	int x, y;
};

struct float2
{
	float2(float x = 0, float y = 1);

	float2 operator+(const float2& right)const;
	float2 operator-(const float2& right)const;

	float2 operator*(const float2& rhs)const;
	float2 operator*(float factor);
	float2 friend operator*(float factor, const float2& right);

	float x, y;
};
float2 MakeFloat2(float x, float y);

struct float3
{
	float3():x(0), y(0), z(0){}
	float3(const struct float4& xyzw);
	float3(float x, float y, float z);
	float3 operator+(const float3& right)const;
	float3 operator-(const float3& right)const;
	float3 operator/(float rhs)const;
	float3& operator+=(const float3& right);

	float3 operator*(float factor)const;
	friend float3  operator*(float factor, const float3& right);
	friend float3 operator-(const float3& right);
	static float3 min(const float3& a, const float3& b);
	static float3 max(const float3& a, const float3& b);

	static float  dot(const float3& left, const float3& right);
	//Cross product is an R^3 operation
	static float3 cross(const float3& left, const float3& right);
	static bool any(const float3& v) { return ((v.x != 0) || (v.y != 0) || (v.z != 0)); }

	float& operator[](int index) { return m[index]; }
	const float& operator[](int index)const { return m[index]; }

	static float3 orthogonal(const float3 n)
	{
		float3 u;
		if (n.x)
		{
			u = float3(n.z, 0, -n.x);
		}
		else if (n.y)
		{
			u = float3(n.y, -n.x, 0);
		}
		else
		{
			u = float3(0, n.z, -n.y);
		}
		return u;
	}

	float  lengthSqrd()const;
	float  length()const;
	static float  length(const float3& v);
	static float3 normalize(const float3& v);
	bool   isNormalized()const;
	bool   isZero()const;

	union
	{
		float m[3];
		struct {
			float x, y, z;
		};
	};
};
float3 MakeFloat3(float x = 0, float y = 0, float z = 1);

struct int3
{
	int3():x(0), y(0), z(0) {}
	int3(int x, int y, int z);
	int3 operator+(const int3& right)const;
	int3 operator-(const int3& right)const;
	int3 operator/(const int3& right)const;
	int3 operator/(int right)const;

	int3& operator*=(const int3& right);
	int3& operator-=(const int3& right);
	int3& operator+=(const int3& right);

	int3& operator/=(const int3& right);
	int3 operator*(const int3& right)const;
	int3 operator*(int factor)const;
	int3 friend operator*(int factor, const int3& right);

	bool operator==(const int3& right)const;

	union
	{
		int m[3];
		struct {
			int x, y, z;
		};
	};
};

struct int4
{
	int4() :x(0), y(0), z(0), w(0) {}
	int4(int x, int y, int z, int w);

	int x, y, z, w;
};
struct uint4
{
	uint4(uint32_t x, uint32_t y, uint32_t z, uint32_t w);
	uint32_t x, y, z, w;
};

class float4x4
{
public:
	float4x4()
	{

	}
	float4x4(const float4x4& rhs)
	{
		std::memcpy(m, rhs.m, sizeof(float4x4));
	}
	float4x4(float4x4&& rhs)noexcept
	{
		std::memcpy(m, rhs.m, sizeof(float4x4));
	}
	float4x4& operator=(const float4x4& rhs)
	{
		std::memcpy(m, rhs.m, sizeof(float4x4));
		return *this;
	}

	float4 operator*(const float4& v)const
	{
		float4 vPrime;
		vPrime.x = (m[0] * v.x) + (m[4] * v.y) + (m[8] * v.z) + (m[12] * v.w);
		vPrime.y = (m[1] * v.x) + (m[5] * v.y) + (m[9] * v.z) + (m[13] * v.w);
		vPrime.z = (m[2] * v.x) + (m[6] * v.y) + (m[10] * v.z) + (m[14] * v.w);
		vPrime.w = (m[3] * v.x) + (m[7] * v.y) + (m[11] * v.z) + (m[15] * v.w);
		return vPrime;
	}

	float4x4 operator*(const float4x4& mtx0)const
	{
		float4x4 output;
		//xform first col
		output.m[0] = (m[0] * mtx0.m[0]) + (m[4] * mtx0.m[1]) + (m[8] * mtx0.m[2]) + (m[12] * mtx0.m[3]);
		output.m[1] = (m[1] * mtx0.m[0]) + (m[5] * mtx0.m[1]) + (m[9] * mtx0.m[2]) + (m[13] * mtx0.m[3]);
		output.m[2] = (m[2] * mtx0.m[0]) + (m[6] * mtx0.m[1]) + (m[10] * mtx0.m[2]) + (m[14] * mtx0.m[3]);
		output.m[3] = (m[3] * mtx0.m[0]) + (m[7] * mtx0.m[1]) + (m[11] * mtx0.m[2]) + (m[15] * mtx0.m[3]);

		//xform 2nd col
		output.m[4] = (m[0] * mtx0.m[4]) + (m[4] * mtx0.m[5]) + (m[8] * mtx0.m[6]) + (m[12] * mtx0.m[7]);
		output.m[5] = (m[1] * mtx0.m[4]) + (m[5] * mtx0.m[5]) + (m[9] * mtx0.m[6]) + (m[13] * mtx0.m[7]);
		output.m[6] = (m[2] * mtx0.m[4]) + (m[6] * mtx0.m[5]) + (m[10] * mtx0.m[6]) + (m[14] * mtx0.m[7]);
		output.m[7] = (m[3] * mtx0.m[4]) + (m[7] * mtx0.m[5]) + (m[11] * mtx0.m[6]) + (m[15] * mtx0.m[7]);

		//xform 3rd col
		output.m[8] = (m[0] * mtx0.m[8]) + (m[4] * mtx0.m[9]) + (m[8] * mtx0.m[10]) + (m[12] * mtx0.m[11]);
		output.m[9] = (m[1] * mtx0.m[8]) + (m[5] * mtx0.m[9]) + (m[9] * mtx0.m[10]) + (m[13] * mtx0.m[11]);
		output.m[10] = (m[2] * mtx0.m[8]) + (m[6] * mtx0.m[9]) + (m[10] * mtx0.m[10]) + (m[14] * mtx0.m[11]);
		output.m[11] = (m[3] * mtx0.m[8]) + (m[7] * mtx0.m[9]) + (m[11] * mtx0.m[10]) + (m[15] * mtx0.m[11]);

		//xform 4th col
		output.m[12] = (m[0] * mtx0.m[12]) + (m[4] * mtx0.m[13]) + (m[8] * mtx0.m[14]) + (m[12] * mtx0.m[15]);
		output.m[13] = (m[1] * mtx0.m[12]) + (m[5] * mtx0.m[13]) + (m[9] * mtx0.m[14]) + (m[13] * mtx0.m[15]);
		output.m[14] = (m[2] * mtx0.m[12]) + (m[6] * mtx0.m[13]) + (m[10] * mtx0.m[14]) + (m[14] * mtx0.m[15]);
		output.m[15] = (m[3] * mtx0.m[12]) + (m[7] * mtx0.m[13]) + (m[11] * mtx0.m[14]) + (m[15] * mtx0.m[15]);
		return output;
	}

	static void MakeIdentity(float4x4& mtx)
	{
		mtx.m[0] = 1; mtx.m[4] = 0; mtx.m[8] = 0;  mtx.m[12] = 0;
		mtx.m[1] = 0; mtx.m[5] = 1; mtx.m[9] = 0;  mtx.m[13] = 0;
		mtx.m[2] = 0; mtx.m[6] = 0; mtx.m[10] = 1; mtx.m[14] = 0;
		mtx.m[3] = 0; mtx.m[7] = 0; mtx.m[11] = 0; mtx.m[15] = 1;
	}
	static void MakeTranslation(float4x4& mtx, const float3& translate)
	{
		mtx.m[0] = 1; mtx.m[4] = 0; mtx.m[8] = 0;  mtx.m[12] = translate.x;
		mtx.m[1] = 0; mtx.m[5] = 1; mtx.m[9] = 0;  mtx.m[13] = translate.y;
		mtx.m[2] = 0; mtx.m[6] = 0; mtx.m[10] = 1;  mtx.m[14] = translate.z;
		mtx.m[3] = 0; mtx.m[7] = 0; mtx.m[11] = 0;  mtx.m[15] = 1;
	}

	static void MakeScale(float4x4& mtx, const float3& scale)
	{
		mtx.m[0] = scale.x; mtx.m[4] = 0;       mtx.m[8] = 0;        mtx.m[12] = 0;
		mtx.m[1] = 0;       mtx.m[5] = scale.y; mtx.m[9] = 0;        mtx.m[13] = 0;
		mtx.m[2] = 0;       mtx.m[6] = 0;       mtx.m[10] = scale.z; mtx.m[14] = 0;
		mtx.m[3] = 0;       mtx.m[7] = 0;       mtx.m[11] = 0;       mtx.m[15] = 1;
	}
	static void MakeRotationX(float4x4& mtx, float theta)
	{
		mtx.m[0] = 1;  mtx.m[4] = 0;           mtx.m[8] = 0;            mtx.m[12] = 0;
		mtx.m[1] = 0;  mtx.m[5] = cosf(theta);  mtx.m[9] = -sinf(theta);  mtx.m[13] = 0;
		mtx.m[2] = 0;  mtx.m[6] = sinf(theta);  mtx.m[10] = cosf(theta);  mtx.m[14] = 0;
		mtx.m[3] = 0;  mtx.m[7] = 0;           mtx.m[11] = 0;           mtx.m[15] = 1;
	}

	static void MakeRotationY(float4x4& mtx, float theta)
	{
		mtx.m[0] = cosf(theta);   mtx.m[4] = 0;   mtx.m[8] = sinf(theta);   mtx.m[12] = 0;
		mtx.m[1] = 0;            mtx.m[5] = 1;   mtx.m[9] = 0;            mtx.m[13] = 0;
		mtx.m[2] = -sinf(theta);  mtx.m[6] = 0;   mtx.m[10] = cosf(theta);  mtx.m[14] = 0;
		mtx.m[3] = 0;            mtx.m[7] = 0;   mtx.m[11] = 0;           mtx.m[15] = 1;
	}

	static void MakeRotationZ(float4x4& mtx, float theta)
	{
		mtx.m[0] = cosf(theta);   mtx.m[4] = -sinf(theta);  mtx.m[8] = 0;   mtx.m[12] = 0;
		mtx.m[1] = sinf(theta);   mtx.m[5] = cosf(theta);   mtx.m[9] = 0;   mtx.m[13] = 0;
		mtx.m[2] = 0;            mtx.m[6] = 0;            mtx.m[10] = 1;  mtx.m[14] = 0;
		mtx.m[3] = 0;            mtx.m[7] = 0;            mtx.m[11] = 0;  mtx.m[15] = 1;
	}

	static float4x4 MakeRotation(float4x4& mtx, const float3& theta)
	{
		float4x4 rX, rY, rZ;
		MakeRotationX(rX, theta.x);
		MakeRotationY(rY, theta.y);
		MakeRotationZ(rZ, theta.z);

		mtx = (rZ * (rY * rX));
		return mtx;
	}

	static void Transpose(float4x4& transposed, const float4x4& mtx)
	{
		transposed.m[0] = mtx.m[0];  transposed.m[4] = mtx.m[1];  transposed.m[8] = mtx.m[2];  transposed.m[12] = mtx.m[3];
		transposed.m[1] = mtx.m[4];  transposed.m[5] = mtx.m[5];  transposed.m[9] = mtx.m[6];  transposed.m[13] = mtx.m[7];
		transposed.m[2] = mtx.m[8];  transposed.m[6] = mtx.m[9];  transposed.m[10] = mtx.m[10]; transposed.m[14] = mtx.m[11];
		transposed.m[3] = mtx.m[12]; transposed.m[7] = mtx.m[13]; transposed.m[11] = mtx.m[14]; transposed.m[15] = mtx.m[15];
	}

	float4 Transform(const float4& v)const
	{
		float4 output;
		output.x = (m[0] * v.x) + (m[4] * v.y) + (m[8] * v.z) + (m[12] * v.w);
		output.y = (m[1] * v.x) + (m[5] * v.y) + (m[9] * v.z) + (m[13] * v.w);
		output.z = (m[2] * v.x) + (m[6] * v.y) + (m[10] * v.z) + (m[14] * v.w);
		output.w = (m[3] * v.x) + (m[7] * v.y) + (m[11] * v.z) + (m[15] * v.w);
		return output;
	}

	float3 Transform(const float3& v)const
	{
		float3 output;
		output.x = (m[0] * v.x) + (m[4] * v.y) + (m[8] * v.z);
		output.y = (m[1] * v.x) + (m[5] * v.y) + (m[9] * v.z);
		output.z = (m[2] * v.x) + (m[6] * v.y) + (m[10] * v.z);
		return output;
	}

	float3 TransformPoint(const float3& v)const
	{
		float3 output;
		output.x = (m[0] * v.x) + (m[4] * v.y) + (m[8] * v.z) + (m[12]);
		output.y = (m[1] * v.x) + (m[5] * v.y) + (m[9] * v.z) + (m[13]);
		output.z = (m[2] * v.x) + (m[6] * v.y) + (m[10] * v.z) + (m[14]);
		return output;
	}

	//Rasterization
	/*
	   Z
	   |   Y
	   |  /
	   | /
	   |/________X
	    
	*/
	static float4x4 MakeLookAt(const float3& eye, const float3& at, const float3& up)
	{
		float4x4 mat;

		float3 yaxis = float3::normalize(at - eye);
		float3 xaxis = float3::normalize(float3::cross(yaxis, up));
		float3 zaxis = float3::cross(xaxis, yaxis);

		mat.m[0] = xaxis.x; mat.m[4] = xaxis.y; mat.m[8]  = xaxis.z; mat.m[12] = -float3::dot(xaxis, eye);
		mat.m[1] = yaxis.x; mat.m[5] = yaxis.y; mat.m[9]  = yaxis.z; mat.m[13] = -float3::dot(yaxis, eye);
		mat.m[2] = zaxis.x; mat.m[6] = zaxis.y; mat.m[10] = zaxis.z; mat.m[14] = -float3::dot(zaxis, eye);
		mat.m[3] = 0;       mat.m[7] = 0;       mat.m[11] = 0;       mat.m[15] = 1;

		return mat;
	}

	static float4x4 MakePerspective(float fovVertical, float aspect, float Near, float Far)
	{
		float4x4 toD3d;
		toD3d.m[0] = 1; toD3d.m[4] = 0; toD3d.m[8] = 0;  toD3d.m[12] = 0;
		toD3d.m[1] = 0; toD3d.m[5] = 0; toD3d.m[9] = 1;  toD3d.m[13] = 0;
		toD3d.m[2] = 0; toD3d.m[6] = 1; toD3d.m[10] = 0; toD3d.m[14] = 0;
		toD3d.m[3] = 0; toD3d.m[7] = 0; toD3d.m[11] = 0; toD3d.m[15] = 1;

		float4x4 proj;
		float yScale = 1.0f / tanf(fovVertical * 0.5f);
		float xScale = yScale / aspect;
		float Q = Far / (Far - Near);

		proj.m[0] = xScale; proj.m[4] = 0;      proj.m[8]  = 0;  proj.m[12] = 0;
		proj.m[1] = 0;      proj.m[5] = yScale; proj.m[9]  = 0;  proj.m[13] = 0;
		proj.m[2] = 0;      proj.m[6] = 0;      proj.m[10] = Q;  proj.m[14] = -Q*Near;
		proj.m[3] = 0;      proj.m[7] = 0;      proj.m[11] = 1;  proj.m[15] = 0;

		return (proj * toD3d);
	}

public:
	float m[16];
};

