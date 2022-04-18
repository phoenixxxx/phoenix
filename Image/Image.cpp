#include "Image.h"
#define TINYEXR_IMPLEMENTATION
#include <ThirdParty/tinyexr/tinyexr.h>
#define STB_IMAGE_IMPLEMENTATION
#include <ThirdParty/stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <ThirdParty/stb_image/stb_image_write.h>
#include <Utils/Math.h>
#include <vector>

namespace Phoenix
{
	Image::Image()
	{
		mLog2LargestDim = 0;
		mAllocationSize = 0;
		mFormat = Format::eUnknown;
		std::memset(mMips, 0, sizeof(mMips));
	}

	Image::~Image()
	{
		FreeMemory();
	}

	void Image::FreeMemory()
	{
		if (mMips[0].mTexels != nullptr)
		{
			free(mMips[0].mTexels);//all mip chain is in alloc 0
			mMips[0].mTexels = nullptr;
		}
	}

	uint32_t Image::GetTexelSize(Format format)
	{
		switch (format)
		{
		case Format::eFloat3:
		case Format::eFloat4:
			return GetTexelDimension(format) * 4;
		case Format::eUbyte3:
		case Format::eUbyte4:
			return GetTexelDimension(format);
		default:
			return 0;
		}
	}

	uint32_t Image::GetTexelDimension(Format format)
	{
		switch (format)
		{
		case Format::eFloat3:
		case Format::eUbyte3:
			return 3;
		case Format::eFloat4:
		case Format::eUbyte4:
			return 4;
		default:
			return 0;
		}
	}

	void Image::Write(const std::filesystem::path& filePath, uint32_t mip)
	{
		WriteToFile(filePath, mMips[mip].mTexels, mMips[mip].mResolution, mFormat);
	}

	bool Image::WriteToFile(const std::filesystem::path& filePath, byte_t* data, const Size& geom, Format format)
	{
		uint32_t fmtDim = GetTexelDimension(format);
		if ((format == Format::eFloat3) || (format == Format::eFloat4))
		{
			if ((filePath.extension() == ".hdr") || (filePath.extension() == ".HDR"))
			{
				//save HDR
				stbi_write_hdr(filePath.string().c_str(), geom.mWidth, geom.mHeight, fmtDim, reinterpret_cast<const float*>(data));
			}
			else if ((filePath.extension() == ".exr") || (filePath.extension() == ".EXR"))
			{
				//save EXR
				EXRHeader header;
				InitEXRHeader(&header);

				EXRImage image;
				InitEXRImage(&image);

				std::vector<float> images[3];
				images[0].resize(geom.mWidth * geom.mHeight);
				images[1].resize(geom.mWidth * geom.mHeight);
				images[2].resize(geom.mWidth * geom.mHeight);

				// Split RGBRGBRGB... into R, G and B layer
				const float* rgb = reinterpret_cast<const float*>(&data[0]);
				for (uint32_t i = 0; i < geom.mWidth * geom.mHeight; i++)
				{
					images[0][i] = rgb[fmtDim * i + 0];
					images[1][i] = rgb[fmtDim * i + 1];
					images[2][i] = rgb[fmtDim * i + 2];
				}

				float* image_ptr[3];
				image_ptr[0] = &(images[2][0]); // B
				image_ptr[1] = &(images[1][0]); // G
				image_ptr[2] = &(images[0][0]); // R

				image.images = reinterpret_cast<unsigned char**>(image_ptr);
				image.width = geom.mWidth;
				image.height = geom.mHeight;

				header.num_channels = 3;
				header.channels = reinterpret_cast<EXRChannelInfo*>(malloc(header.num_channels * sizeof(EXRChannelInfo)));//, EXRChannelInfo);
#ifdef _MSC_VER
				strncpy_s(header.channels[0].name, "B", 255);// header.channels[0].name[strlen("B")] = '\0';
				strncpy_s(header.channels[1].name, "G", 255);// header.channels[1].name[strlen("G")] = '\0';
				strncpy_s(header.channels[2].name, "R", 255);// header.channels[2].name[strlen("R")] = '\0';
#else
				strncpy(header.channels[0].name, "B", 255);// header.channels[0].name[strlen("B")] = '\0';
				strncpy(header.channels[1].name, "G", 255);// header.channels[1].name[strlen("G")] = '\0';
				strncpy(header.channels[2].name, "R", 255);// header.channels[2].name[strlen("R")] = '\0';
#endif

				header.pixel_types = reinterpret_cast<int*>(malloc(header.num_channels * sizeof(int)));
				header.requested_pixel_types = reinterpret_cast<int*>(malloc(header.num_channels *sizeof(int)));
				for (int i = 0; i < header.num_channels; i++)
				{
					header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
					header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
				}

				const char* err = nullptr;
				int ret = SaveEXRImageToFile(&image, &header, filePath.string().c_str(), &err);
				if (ret != TINYEXR_SUCCESS)
				{
					//Core::Logger::Instance().Log("Save EXR err: %s\n", err);
					FreeEXRErrorMessage(err); // free's buffer for an error message 
					return false;
				}

				free(header.channels);
				free(header.pixel_types);
				free(header.requested_pixel_types);
			}
		}
		else if ((filePath.extension() == ".png") || (filePath.extension() == ".PNG"))
		{
			//png, jpg
			stbi_write_png(filePath.string().c_str(), geom.mWidth, geom.mHeight, fmtDim, data, 0);
		}
		return true;
	}

	byte_t* Image::ReadFromFile(const std::filesystem::path& filePath, Size& res, Format& format)
	{
		byte_t* data = nullptr;
		if ((filePath.extension() == ".hdr") || (filePath.extension() == ".HDR"))
		{
			int width, height, compCount;
			float* rgb = stbi_loadf(filePath.string().c_str(), &width, &height, &compCount, 0);
			if (rgb != nullptr)
			{
				format = Format::eFloat4;

				res.mWidth = (uint32_t)width;
				res.mHeight = (uint32_t)height;

				data = reinterpret_cast<byte_t*>(malloc(GetTexelSize(format) * res.mWidth * res.mHeight));
				float* rgba = reinterpret_cast<float*>(data);
				//convert to RGBA
				for (uint32_t iTxl = 0; iTxl < res.mWidth * res.mHeight; ++iTxl)
				{
					rgba[4 * iTxl + 0] = rgb[3 * iTxl + 0];
					rgba[4 * iTxl + 1] = rgb[3 * iTxl + 1];
					rgba[4 * iTxl + 2] = rgb[3 * iTxl + 2];
					rgba[4 * iTxl + 3] = 0;
				}
				stbi_image_free(rgb);//free this because we converted to RGBA_F32
			}
		}
		else if ((filePath.extension() == ".exr") || (filePath.extension() == ".EXR"))
		{
			float* out;
			int width;
			int height;
			const char* err = nullptr;
			int ret = LoadEXR(&out, &width, &height, filePath.string().c_str(), &err);

			if (ret != TINYEXR_SUCCESS)
			{
				if (err)
				{
					//Core::Logger::Instance().Log("ERR : %s\n", err);
					FreeEXRErrorMessage(err); // release memory of error message.
				}
			}
			else
			{
				res.mWidth = (uint32_t)width;
				res.mHeight = (uint32_t)height;

				format = Format::eFloat4;
			}
		}
		else
		{
			//png, jpeg
			int width, height, channels;
			data = stbi_load(filePath.string().c_str(), &width, &height, &channels, 0);
			if (data)
			{
				format = Format::eUbyte4;
				if (channels == 3)
					format = Format::eUbyte3;

				res.mWidth = (uint32_t)width;
				res.mHeight = (uint32_t)height;
			}
		}
		return data;
	}

	void Image::Write(byte_t* data, const Size& res, Format format, int32_t x, int32_t y, const float4& texel)
	{
		//clamp coords
		x = Math::Clamp(x, 0, (int)(res.mWidth - 1));
		y = Math::Clamp(y, 0, (int)(res.mHeight - 1));

		uint32_t index = (x + y * res.mWidth);

		switch (format)
		{
		case Format::eFloat3:
		{
			reinterpret_cast<float3*>(data)[index] = float3(texel.x, texel.y, texel.z);
			break;
		}
		case Format::eFloat4:
		{
			reinterpret_cast<float4*>(data)[index] = texel;
			break;
		}
		case Format::eUbyte3:
		{
			index *= 3;//3 bytes per comp

			data[index] = Math::Nearest(texel.x * 255.0f);
			data[index + 1] = Math::Nearest(texel.y * 255.0f);
			data[index + 2] = Math::Nearest(texel.z * 255.0f);
			break;
		}
		case Format::eUbyte4:
		{
			index *= 4;//4 bytes per comp

			data[index] = Math::Nearest(texel.x * 255.0f);
			data[index + 1] = Math::Nearest(texel.y * 255.0f);
			data[index + 2] = Math::Nearest(texel.z * 255.0f);
			data[index + 3] = Math::Nearest(texel.w * 255.0f);
			break;
		}
		}
	}

	float4 Image::PointSample(const byte_t* data, const Size& res, Format format, int32_t x, int32_t y)
	{
		//clamp coords
		x = Math::Clamp(x, 0, (int)(res.mWidth - 1));
		y = Math::Clamp(y, 0, (int)(res.mHeight - 1));

		uint32_t index = (x + y * res.mWidth);

		float4 sample;
		switch (format)
		{
		case Format::eFloat3:
		{
			sample = reinterpret_cast<const float3*>(data)[index];
			break;
		}
		case Format::eFloat4:
		{
			sample = reinterpret_cast<const float4*>(data)[index];
			break;
		}
		case Format::eUbyte3:
		{
			index *= 3;//3 bytes per comp

			sample.x = data[index] / 255.0f;
			sample.y = data[index + 1] / 255.0f;
			sample.z = data[index + 2] / 255.0f;
			sample.w = 1.0f;
			break;
		}
		case Format::eUbyte4:
		{
			index *= 4;//4 bytes per comp

			sample.x = data[index] / 255.0f;
			sample.y = data[index + 1] / 255.0f;
			sample.z = data[index + 2] / 255.0f;
			sample.w = data[index + 3] / 255.0f;
			break;
		}
		}

		return sample;
	}

	static void GetSamplingInfo(float fractional, float& factor, int& offsetCoord)
	{
		assert(fractional < 1);
		assert(fractional >= 0);

		if (fractional < 0.5f)//fractional [0 : 0.5[ --> factor [0.5 : 1[
		{
			offsetCoord = -1;
			factor = 0.5f + fractional;
		}
		else//fractional [0.5 : 1[ --> factor [0.5 : 1[
		{
			offsetCoord = 1;
			factor = 1.5f - fractional;
		}
	}

	float4 Image::Sample(const byte_t* data, const Size& res, Format format, float xNorm, float yNorm, Filter sampling)
	{
		float x = xNorm * res.mWidth;
		float y = yNorm * res.mHeight;
		int xCast = static_cast<int>(x);
		int yCast = static_cast<int>(y);

		float4 sample;
		if (sampling == Filter::eNearest)
		{
			sample = PointSample(data, res, format, Math::Nearest(x), Math::Nearest(y));
		}
		else if(sampling == Filter::eLinear)
		{
			double integer;
			float xFrac = static_cast<float>(modf(x, &integer));
			float yFrac = static_cast<float>(modf(y, &integer));

			//sample along X
			float xFactor;
			int xCoordOffset;
			GetSamplingInfo(xFrac, xFactor, xCoordOffset);
			float4 X1 = xFactor * PointSample(data, res, format, xCast, yCast) +
				(1 - xFactor) * PointSample(data, res, format, xCast + xCoordOffset, yCast);

			//sample along X and Y
			float yFactor;
			int yCoordOffset;
			GetSamplingInfo(yFrac, yFactor, yCoordOffset);
			float4 X2 = xFactor * PointSample(data, res, format, xCast, yCast + yCoordOffset) +
				(1 - xFactor) * PointSample(data, res, format, xCast + xCoordOffset, yCast + yCoordOffset);

			//Y sample
			sample = yFactor * X1 + (1 - yFactor) * X2;
		}
		return sample;
	}

	bool Image::Initialize(const std::filesystem::path& filePath, bool mips)
	{
		Size res;
		Format format;
		byte_t* data = ReadFromFile(filePath, res, format);

		bool success = false;
		if (data != nullptr)
		{
			success = Initialize(res, format, data, mips);
			free(data);
		}
		return success;
	}

	bool Image::Initialize(const Size& res, Format format, const byte_t* data, bool mips)
	{
		FreeMemory();

		if ((res.mWidth > MAX_RESOLUTION_DIM) || (res.mHeight > MAX_RESOLUTION_DIM))
			return false;

		mFormat = format;
		mMips[0].mResolution = res;
		size_t bitmapSize = GetTexelSize(mFormat) * res.mWidth * res.mHeight;

		mAllocationResolution = { (mips ? res.mWidth + Math::Nearest(res.mWidth / 2.0f) : res.mWidth), res.mHeight };
		mAllocationSize = GetTexelSize(mFormat) * mAllocationResolution.mWidth * mAllocationResolution.mHeight;
		mMips[0].mTexels = reinterpret_cast<byte_t*>(malloc(mAllocationSize));// reinterpret_cast<byte_t*>(malloc(texelSize));

		assert(data != nullptr);
		{
			std::memcpy(mMips[0].mTexels, data, bitmapSize);
			if (mips)
			{
				uint32_t maxDim = std::max(res.mWidth, res.mHeight);
				mLog2LargestDim = static_cast<uint32_t>(Math::Nearest(log2f(maxDim)));
				for (uint32_t iMip = 0; iMip < mLog2LargestDim; ++iMip)
				{
					//compute mip resolutions
					Size mipRes;
					mipRes.mWidth  = Math::Nearest(mMips[iMip].mResolution.mWidth / 2.0f);
					mipRes.mHeight = Math::Nearest(mMips[iMip].mResolution.mHeight/ 2.0f);
					mMips[iMip + 1].mResolution = mipRes;

					//allocate size
					mMips[iMip + 1].mTexels = mMips[0].mTexels + bitmapSize;
					bitmapSize += GetTexelSize(mFormat) * mipRes.mWidth * mipRes.mHeight;
					
					//texelSize = GetTexelSize(mFormat) * mipRes.mWidth * mipRes.mHeight;
					//mMips[iMip + 1].mTexels = reinterpret_cast<byte_t*>(malloc(texelSize));

					//sample parent Mip
					float dx = 1.0f / mipRes.mWidth;
					float dy = 1.0f / mipRes.mHeight;
					for (uint32_t y = 0; y < mipRes.mHeight; ++y)
					{
						for (uint32_t x = 0; x < mipRes.mWidth; ++x)
						{
							//Sample parent mip
							float4 sample = Sample(mMips[iMip].mTexels, mMips[iMip].mResolution, mFormat, x * dx, y * dy, Filter::eLinear);
							//write to child mip
							Write(mMips[iMip + 1].mTexels, mMips[iMip + 1].mResolution, mFormat, x, y, sample);
						}
					}
				}
			}
		}

		return true;
	}

	float4 Image::PointSample(int32_t x, int32_t y, uint32_t mip)const
	{
		return PointSample(mMips[mip].mTexels, mMips[mip].mResolution, mFormat, x, y);
	}

	float4 Image::Sample(float x, float y, Filter sampling, uint32_t mip)const
	{
		return Sample(mMips[mip].mTexels, mMips[mip].mResolution, mFormat, x, y, sampling);
	}

	void Image::Write(float x, float y, const float4& texel, uint32_t mip)
	{
		Write(mMips[mip].mTexels, mMips[mip].mResolution, mFormat, x, y, texel);
	}
}
