#pragma once
#include <Utils/Types.h>
#include <Utils/VectorMath.h>
#include <Utils/Utils.h>

#include <SharedGPU_CPU/SharedImage.h>

namespace Phoenix
{
	class Image
	{
	public:
		Image();
		~Image();

		bool Initialize(const Size& res, Format format, const byte_t* data = nullptr, bool mips = false);
		bool Initialize(const std::filesystem::path& filePath, bool mips = true);
		float4 Sample(float x, float y, Filter sampling = Filter::eLinear, uint32_t mip=0)const;
		float4 PointSample(int32_t x, int32_t y, uint32_t mip = 0)const;
		void   Write(float x, float y, const float4& texel, uint32_t mip = 0);
		uint32_t GetMipLevelCounts()const { return 1 + mLog2LargestDim; }

		const Size&   GetResolution(uint32_t mip=0)const { return mMips[mip].mResolution; }
		const byte_t* GetTexels(uint32_t mip=0)const { return mMips[mip].mTexels; }
		size_t GetAllocationSize() { return mAllocationSize; }
		byte_t* GetAllocation(){ return mMips[0].mTexels; }

		void WriteToFile(const std::filesystem::path& filePath, uint32_t mip = 0);
		void GetImageBlock(ImageBlock& block);//GPU version of the image structure
	public:
		static uint32_t GetTexelSize(Format format);
		static uint32_t GetTexelDimension(Format format);

	public:
		static bool    WriteToFile(const std::filesystem::path& filePath, byte_t* data, const Size& res, Format format);
		static byte_t* ReadFromFile(const std::filesystem::path& filePath, Size& res, Format& format);

	private:
		static void   Write(byte_t* data, const Size& res, Format format, int32_t x, int32_t y, const float4& texel);
		static float4 PointSample(const byte_t* data, const Size& res, Format format, int32_t x, int32_t y);
		static float4 Sample(const byte_t* data, const Size& res, Format format, float x, float y, Filter sampling);

	private:
		void FreeMemory();
	private:
		Size   mAllocationResolution;
		size_t mAllocationSize;
		uint32_t mLog2LargestDim;
		Format mFormat;
		struct
		{
			byte_t* mTexels;
			Size mResolution;
		}mMips[MAX_MIPS + 1];//
	};
}