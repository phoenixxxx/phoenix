#include "VectorMath.h"
#include "Math.h"

////////////////////////////Float4//////////////////////////////////////
float4::float4(float _x, float _y, float _z, float _w) :x(_x), y(_y), z(_z), w(_w)
{

}
float4::float4(const struct float3& xyz, float w):x(xyz.x), y(xyz.y), z(xyz.z), w(w)
{

}
float4::float4(const struct float2& xy, float z, float w) :x(xy.x), y(xy.y), z(z), w(w)
{

}
float4::float4(const struct float2& xy, const struct float2& zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y)
{

}
float4 float4::operator+(const float4& right)const
{
	return float4(x + right.x, y + right.y, z + right.z, w + right.w);
}
float4 float4::operator-(const float4& right)const
{
	return float4(x - right.x, y - right.y, z - right.z, w - right.w);
}
float4 float4::operator*(const float4& right)const
{
	return float4(x * right.x, y * right.y, z * right.z, w * right.w);
}
float4 float4::operator*(float factor)const
{
	return float4(x * factor, y * factor, z * factor, w * factor);
}
float4 float4::operator/(float factor)const
{
	return float4(x / factor, y / factor, z / factor, w / factor);
}
float4& float4::operator+=(const float4& right)
{
	x += right.x;
	y += right.y;
	z += right.z;
	w += right.w;
	return *this;
}
float4 operator*(float factor, const float4& right)
{
	return float4(right.x * factor, right.y * factor, right.z * factor, right.w * factor);
}
float4 MakeFloat4(float x , float y , float z , float w )
{
	return float4(x, y, z, w);
}
float4 MakeFloat4(const float3& xyz, float w)
{
	return float4(xyz.x, xyz.y, xyz.z, w);
}

/////////////////////////////End Float4//////////////////////////////////////

///////////////////////////////float2////////////////////////////////////////
float2::float2(float _x, float _y) :x(_x), y(_y)
{

}
float2 float2::operator+(const float2& right)const
{
	return float2(x + right.x, y + right.y);
}
float2 float2::operator-(const float2& right)const
{
	return float2(x - right.x, y - right.y);
}

float2 float2::operator*(const float2& rhs)const
{
	return float2(rhs.x * x, rhs.y * y);
}
float2 float2::operator*(float factor)
{
	return float2(factor * x, factor * y);
}
float2  operator*(float factor, const float2& right)
{
	return float2(factor * right.x, factor * right.y);
}
float2 MakeFloat2(float x, float y)
{
	return float2(x, y);
}
/////////////////////////////End float2//////////////////////////////////////

///////////////////////////////float3////////////////////////////////////////
float3::float3(const struct float4& xyzw) :x(xyzw.x), y(xyzw.y), z(xyzw.z)
{

}
float3::float3(float _x, float _y, float _z) :x(_x), y(_y), z(_z)
{

}

float3 float3::operator+(const float3& right)const
{
	return float3(x + right.x, y + right.y, z + right.z);
}
float3 float3::operator-(const float3& right)const
{
	return float3(x - right.x, y - right.y, z - right.z);
}
float3 float3::operator/(float rhs)const
{
	return float3(x / rhs, y / rhs, z / rhs);
}
float3& float3::operator+=(const float3& right)
{
	x += right.x;
	y += right.y;
	z += right.z;
	return *this;
}

float3 float3::operator*(float factor)const
{
	return float3(factor * x, factor * y, factor * z);
}
float3  operator*(float factor, const float3& right)
{
	return float3(factor * right.x, factor * right.y, factor * right.z);
}
float3 operator-(const float3& right)
{
	return float3(-right.x, -right.y, -right.z);
}

float3 float3::min(const float3& a, const float3& b)
{
	return float3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}
float3 float3::max(const float3& a, const float3& b)
{
	return float3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

float3 MakeFloat3(float x, float y, float z)
{
	return float3(x, y, z);
}
float  float3::dot(const float3& left, const float3& right)
{
	return (left.x * right.x + left.y * right.y + left.z * right.z);
}

float3 float3::cross(const float3& a, const float3& b)
{
	float3 t;
	t.x = a.y * b.z - a.z * b.y;
	t.y = a.z * b.x - a.x * b.z;
	t.z = a.x * b.y - a.y * b.x;
	return t;
}

float  float3::lengthSqrd()const
{
	return (x * x + y * y + z * z);
}
float  float3::length()const
{
	return ::sqrtf(lengthSqrd());
}
float  float3::length(const float3& v)
{
	return ::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}
float3 float3::normalize(const float3& v)
{
	return v * (1.0f / length(v));
}
bool   float3::isNormalized()const
{
	float length = lengthSqrd();
	return Phoenix::Math::Equals(length, 1.0f);
}
bool  float3::isZero()const
{
	return ((x == 0) && (y == 0) && (z == 0));
}
/////////////////////////////End float2//////////////////////////////////////

///////////////////////////////uint2////////////////////////////////////////
uint2::uint2(uint32_t _x, uint32_t _y) :x(_x), y(_y)
{

}
/////////////////////////////End uint2//////////////////////////////////////

///////////////////////////////int2////////////////////////////////////////
int2::int2(int32_t _x, int32_t _y) :x(_x), y(_y)
{

}
/////////////////////////////End int2//////////////////////////////////////


///////////////////////////////int3////////////////////////////////////////
int3::int3(int _x, int _y, int _z) :x(_x), y(_y), z(_z)
{
}
int3 int3::operator+(const int3& right)const
{
	return int3(x + right.x, y + right.y, z + right.z);
}
int3 int3::operator-(const int3& right)const
{
	return int3(x - right.x, y - right.y, z - right.z);
}
int3 int3::operator/(const int3& right)const
{
	return int3(x / right.x, y / right.y, z / right.z);
}
int3 int3::operator/(int right)const
{
	return int3(x / right, y / right, z / right);
}
int3 int3::operator*(int factor)const
{
	return int3(factor * x, factor * y, factor * z);
}
int3  operator*(int factor, const int3& right)
{
	return int3(factor * right.x, factor * right.y, factor * right.z);
}

bool int3::operator==(const int3& right)const
{
	return ((x == right.x) && (y == right.y) && (z == right.z));
}
int3& int3::operator/=(const int3& right)
{
	x /= right.x;
	y /= right.y;
	z /= right.z;
	return *this;
}
int3& int3::operator*=(const int3& right)
{
	x *= right.x;
	y *= right.y;
	z *= right.z;
	return *this;
}
int3& int3::operator-=(const int3& right)
{
	x -= right.x;
	y -= right.y;
	z -= right.z;
	return *this;
}
int3& int3::operator+=(const int3& right)
{
	x += right.x;
	y += right.y;
	z += right.z;
	return *this;
}
int3 int3::operator*(const int3& right)const
{
	return int3(x * right.x, y * right.y, z * right.z);
}
///////////////////////////////end of int3////////////////////////////////////////

///////////////////////////////int4////////////////////////////////////////
int4::int4(int _x, int _y, int _z, int _w) :x(_x), y(_y), z(_z), w(_w)
{
}
///////////////////////////////end of int4////////////////////////////////////////

///////////////////////////////uint4////////////////////////////////////////
uint4::uint4(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w) :x(_x), y(_y), z(_z), w(_w)
{
}
///////////////////////////////end of int4////////////////////////////////////////


