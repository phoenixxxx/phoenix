#pragma once

float4 PointSample(ByteAddressBuffer texelData, ImageBlock image, uint32_t mip, int32_t x, int32_t y)
{
	//clamp coords
    Size res = image.mMips[mip].mResolution;//get mip res
    uint32_t byteOffset = image.mImageOffset + image.mMips[mip].mTexelsOffset;

	x = clamp(x, 0, (int)(res.mWidth - 1));
	y = clamp(y, 0, (int)(res.mHeight - 1));
	uint32_t index = (x + y * res.mWidth);

	float4 texSample;
	switch (image.mFormat)
	{
	case Format::eFloat3:
	{
        texSample = float4(texelData.Load<float3>(byteOffset + index * sizeof(float3)), 1.0f);
		break;
	}
	case Format::eFloat4:
	{
		texSample = texelData.Load<float4>(byteOffset + index * sizeof(float4));
		break;
	}
	// case Format::eUbyte3:
	// {
	// 	index *= 3;//3 bytes per comp
	// 	texSample.x = data[index] / 255.0f;
	// 	texSample.y = data[index + 1] / 255.0f;
	// 	texSample.z = data[index + 2] / 255.0f;
	// 	texSample.w = 1.0f;
	// 	break;
	// }
	case Format::eUbyte4:
	{
        uint32_t value = texelData.Load(byteOffset + index * 4);
        texSample.x = (value & 0xFF) / 255.0f;
        texSample.y = ((value & 0xFF00) >> 8) / 255.0f;
        texSample.z = ((value & 0xFF0000) >> 16) / 255.0f;
        texSample.w = ((value & 0xFF000000) >> 24) / 255.0f;
		break;
	}
    case Format::eUbyte3:
    case Format::eUnknown:
    {
        texSample = float4(0,0,0,1);
        break;
    }
	}
	return texSample;
}