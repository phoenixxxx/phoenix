#pragma once

#include <Image/Image.h>
#include <Utils/VectorMath.h>
#include <Utils/Distribution.h>
#include <SharedGPU_CPU/SharedImage.h>

extern "C" {
#include <ThirdParty/Sky/HosekWilkie/ArHosekSkyModel.h>
}

namespace Phoenix
{
	class Sky
	{
	public:
		void Initialize(const std::filesystem::path& filePath);
		void Initialize(float albedo, uint32_t resolution, float turbidity, float elevation);
		void RenderToFile(const char* outfile);
		float4 Sample(float phi, float theta, Filter filter = Filter::eNearest);
		const Distribution2D& GetDistribution() { return mDistribution; }

	private:
		void ComputePDF();

	private:
		Image mSurface;
		Distribution2D mDistribution;
		int mTheta, mPhi;
	};
}
