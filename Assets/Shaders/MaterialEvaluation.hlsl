
#include <MaterialEvaluationShared.h>
#include <SharedTypes.h>
#include <SharedImage.h>
#include "Image.hlsl"

//RWStructuredBuffer<GPUPath> gPaths;

ByteAddressBuffer texelData;
StructuredBuffer<ImageBlock> images;
RWByteAddressBuffer outputImage;
StructuredBuffer<GlobalInput> globals;

[numthreads(PRIMARY_RAY_LOCAL_SIZE, 1, 1)]
void Evaluate(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	int id = dispatchThreadID.x;
	uint32_t w = globals[0].mImageWidth;
	uint32_t h = globals[0].mImageHeight;
	unsigned int texelCount = w * h;
	if (id >= texelCount)
		return;

	uint32_t x = id % w;
	uint32_t y = id / w;

	float4 s = PointSample(texelData, images[globals[0].mImageIndex], globals[0].mImageMip,  x,  y);
}
