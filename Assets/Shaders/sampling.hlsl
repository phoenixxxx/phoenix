#ifndef SAMPLING_HLSL
#define SAMPLING_HLSL

#include "core.hlsl"

inline float2 RasterToSensor(float2 raster, float2 sensorDimension, uint2 sensorResolution)
{
	float2 sensorPos;
	sensorPos.x = raster.x * (sensorDimension.x / sensorResolution.x) - (sensorDimension.x / 2.0f);
	sensorPos.y = -raster.y * (sensorDimension.y / sensorResolution.y) + (sensorDimension.y / 2.0f);
	return sensorPos;
}

inline float2 UniformSampleDisk(float2 u)
{
	float r = sqrt(u.x);
	float theta = 2 * Pi * u.y;//Epsilon1
	return float2(r * cos(theta), r * sin(theta));
}

inline float2 ConcentricSampleDisk(float2 u)
{
	float2 uOffset = 2.f * u - float2(1, 1);
	if ((uOffset.x == 0) && (uOffset.y == 0))
		return float2(0, 0);

	float theta, r;
	if (abs(uOffset.x) > abs(uOffset.y))
	{
		r = uOffset.x;
		theta = PiOver4 * (uOffset.y / uOffset.x);
	}
	else
	{
		r = uOffset.y;
		theta = PiOver2 - PiOver4 * (uOffset.x / uOffset.y);
	}
	return r * float2(cos(theta), sin(theta));
}

/*
	Page 779 of PBR
*/
inline float3 CosineSampleHemisphere(float2 u)
{
	float2 d = ConcentricSampleDisk(u);
	float z = sqrt(max(0.0f, 1 - d.x * d.x - d.y * d.y));
	return float3(d.x, d.y, z);
}

/*
Page 775 of Physically based rendering.
*/
inline float3 UniformSampleHemisphere(float2 u)
{
	float z = u.x;//Epsilon1
	float r = sqrt(max(0.0f, 1.0f - z * z));
	float phi = 2 * Pi * u.y;//Epsilon1
	return float3(r * cos(phi), r * sin(phi), z);
}

inline float UniformHemispherePdf()
{
	return OneDiv2Pi;
}

inline float UniformDiskPdf()
{
	return OneDivPi;
}

//recall that the PDF is wrt the sampling function NOT the shape you are creating
//were we are sampling the ConcentricSamplineDisk distribution, therefore the density is 1/Pi
//since it is weighted by cosTheta, the full PDF is as follows:
inline float CosineHemispherePdf(float cosTheta)
{
	return cosTheta * UniformDiskPdf();
}

inline float2 UniformSampleTriangle(float2 u)
{
	float su1 = sqrt(u.x);
	return float2(1.f - su1, u.y * su1);
}

//https://github.com/mmp/pbrt-v3
inline float BalanceHeuristic(int nf, float fPdf, int ng, float gPdf) 
{
	return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}

inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf) 
{
	float f = nf * fPdf, g = ng * gPdf;
	return (f * f) / (f * f + g * g);
}


#endif //SAMPLING_HLSL