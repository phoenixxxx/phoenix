
#include <SharedTypes.h>
#include <SharedScene.h>
#include <PrimaryRaysShared.h>
#include "sampling.hlsl"
#include "rng.hlsl"

StructuredBuffer<GlobalInput> gInputs;
RWStructuredBuffer<GPUPath> gPaths;
StructuredBuffer<GPUCamera> gCamera;
StructuredBuffer<float> gSeeds;

// #if 0
// RWStructuredBuffer<uint> gRGBABuffer;//temp
// struct Sphere
// {
//     float3 mCenter;
//     float mRadius;
// };
// static const uint gSphereCount = 5;
// static const Sphere spheres[] = {
//     {float3(-2,0,0), 0.5f},
//     {float3(0,0,0), 0.5f},
//     {float3(2,0,0), 0.5f},

//     {float3(0,-2,0), 0.5f},
//     {float3(0,2,0), 0.5f},

//     //{float3(0,0,-1000000), 1000000},
// };
// bool Polynomial(float a, float b, float c, out float x0, out float x1)
// {
//     float discr = b * b - 4 * a * c;
//     if (discr < 0) 
//         return false;
//     else if (discr == 0) 
//         x0 = x1 = -0.5 * b / a;
//     else 
//     {
//         float q = (b > 0) ?
//             -0.5 * (b + sqrt(discr)) :
//             -0.5 * (b - sqrt(discr));
//         x0 = q / a;
//         x1 = c / q;
//     }
//     if (x0 > x1)
//     {
//         float temp = x0;
//         x0 = x1;
//         x1 = temp;
//     }

//     return true;
// }

// float RayVsSphere(float4 rayO, float4 rayD, in Sphere sphere)
// { 
//     float t0, t1;
//     float3 L = rayO.xyz - sphere.mCenter; 
//     float a = dot(rayD.xyz, rayD.xyz); 
//     float b = 2 * dot(rayD.xyz, L);
//     float c = dot(L, L) - (sphere.mRadius * sphere.mRadius); 
//     if (!Polynomial(a, b, c, t0, t1)) return FLT_MAX; 

//     if (t0 < 0) 
//     { 
//         t0 = t1;
//         if (t0 < 0) 
//             return FLT_MAX;
//     } 

//     return t0; 
// }

// float Intersect(float4 rayO, float4 rayD)
// {
//     float tMin = FLT_MAX;
//     for(uint32_t iSphere = 0; iSphere < gSphereCount; ++iSphere)
//     {
//         float t = RayVsSphere(rayO, rayD, spheres[iSphere]);
//         if(t < tMin)
//         {
//             tMin = t;
//         }
//     }
//     return tMin;
// }
// #endif

[numthreads(PRIMARY_RAY_LOCAL_SIZE, 1, 1)]
void Execute(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int id = dispatchThreadID.x;
    //get local X,Y res
    uint32_t xLocal = id % TILE_RESOLUTION;
	uint32_t yLocal = id / TILE_RESOLUTION;
    uint32_t xGlobal = xLocal + gInputs[0].mOffsetX;
	uint32_t yGlobal = yLocal + gInputs[0].mOffsetY;

    //check out of bounds
    if(xGlobal >= gCamera[0].mSensorResolution.x)
        return;
    if(yGlobal >= gCamera[0].mSensorResolution.y)
        return;

    //Initialize the RNG data
    PCG32RNG localRNG = gPaths[id].mRNG;
	rngSetSequence(localRNG, gSeeds[id], PCG32_DEFAULT_STATE);
    
	//covert thread ID to XY coords (using sensor resolution W)
	uint2 sensorResolution = gCamera[0].mSensorResolution;

	float2 rasterSample = float2(rngGetUniformFloat(localRNG), rngGetUniformFloat(localRNG));//raster sample
	float2 raster = float2(xGlobal, yGlobal) + rasterSample;

	//Convert raster position to sensor position
	float2 sensor = RasterToSensor(raster, gCamera[0].mSensorDimension, sensorResolution);

    float3 pos = float3(sensor.x, gCamera[0].mFocalLength, sensor.y);
	float3 dir = normalize(pos);

    //differential perfect rays
    float2 dp = RasterToSensor(raster + float2(1, 0), gCamera[0].mSensorDimension, sensorResolution);
    float3 rxOrigin = float3(dp.x, gCamera[0].mFocalLength, dp.y);
    float3 rxDirection = normalize(rxOrigin);

    dp = RasterToSensor(raster + float2(0, 1), gCamera[0].mSensorDimension, sensorResolution);
    float3 ryOrigin = float3(dp.x, gCamera[0].mFocalLength, dp.y);
    float3 ryDirection = normalize(ryOrigin);

    //sample lense
	if (gCamera[0].mLenseRadius > 0)
	{
		//sample point on lense
		float2 lenseSample = gCamera[0].mLenseRadius * UniformSampleDisk(float2(rngGetUniformFloat(localRNG), rngGetUniformFloat(localRNG)));
		pos = float3(lenseSample.x, gCamera[0].mFocalLength, lenseSample.y);

		/*
		we know that p = t*dir
		where p is the point on the focus plane when following direction dir
		we solve for t using the Z coordinate:
		p.y = t * dir.y
		t = p.y / dir.y
		p.y is the Y coordinate of the point p on the focus plane, which we know to be camera->mFocusDistance
		*/
		float t = gCamera[0].mFocusDistance / dir.y;
		float3 focusPoint = t * dir;

		//new direction
		dir = normalize(focusPoint - pos);

        //ray diff
        /*take another sample because recall we are trying to see how the ray function
        changes w.r.t. a small change in the x,y but in the case of thin lense, we have 
        to take a new sample because any raster sample can have come from any lense point
        */
        lenseSample = gCamera[0].mLenseRadius * UniformSampleDisk(float2(rngGetUniformFloat(localRNG), rngGetUniformFloat(localRNG)));
        rxOrigin = float3(lenseSample.x, gCamera[0].mFocalLength, lenseSample.y);
        t = gCamera[0].mFocusDistance / rxDirection.y;
        focusPoint = t * dir;
        rxDirection = normalize(focusPoint - rxOrigin);

        ryOrigin = rxOrigin;//could technically need a separate sample?
        t = gCamera[0].mFocusDistance / ryOrigin.y;
        focusPoint = t * dir;
        ryDirection = normalize(focusPoint - ryOrigin);
	}


    //write out the world space ray
	float4x4 transform    = gCamera[0].mTransform;//pull xform from global mem;
	gPaths[id].mOrigin    = mul(transform, float4(pos, 1));
	gPaths[id].mOffset    = float4(0, 0, 0, 0);//no offset for primary rays
	gPaths[id].mDirection = mul(transform, float4(dir, 0));

    //ray diff
    gPaths[id].mRxDirection = mul(transform, float4(rxDirection, 0));
    gPaths[id].mRxOrigin    = mul(transform, float4(rxOrigin, 1));
	gPaths[id].mRyDirection = mul(transform, float4(ryDirection, 0));
    gPaths[id].mRyOrigin    = mul(transform, float4(ryOrigin, 1));

    SetRayIsAlive(gPaths[id]);

    //update the RNG struct
    gPaths[id].mRNG = localRNG;


//     //HACK
// #if 0
//     float sphereT = Intersect(gPaths[id].mOrigin, gPaths[id].mDirection);
//     if(sphereT < FLT_MAX)
//     {
//         gRGBABuffer[xGlobal + yGlobal * gCamera[0].mSensorResolution.x] = ((255 << 24) | (0 << 16) | (0 << 8) | 255);
//     }
//     else
//     {
//         gRGBABuffer[xGlobal + yGlobal * gCamera[0].mSensorResolution.x] = 0xFFFFFFFF;
//     }
// #endif
}


