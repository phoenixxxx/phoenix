#include "Sky.h"
#include <vector>
#include <Utils/Math.h>

namespace Phoenix
{
	static const uint32_t angleToSunActivation = 5;//5 deg before we use the solar disk rendering version
	void Sky::Initialize(float albedo, uint32_t resolution, float turbidity, float elevation)
	{
		constexpr int num_channels = 9;
		// Three wavelengths around red, three around green, and three around blue.
		double lambda[num_channels] = { 630, 680, 710, 500, 530, 560, 460, 480, 490 };

		ArHosekSkyModelState* skymodel_state[num_channels];
		for (int iChan = 0; iChan < num_channels; ++iChan)
		{
			skymodel_state[iChan] = arhosekskymodelstate_alloc_init(elevation, turbidity, albedo);
		}

		// Vector pointing at the sun. Note that elevation is measured from the
		// horizon--not the zenith.
		float3 sunDir(0., std::sin(elevation), std::cos(elevation));

		int nTheta = resolution, nPhi = 2 * nTheta;
		size_t sz = 4 * nTheta * nPhi * sizeof(float);
		std::vector<uint8_t> imgRaw;
		imgRaw.resize(sz);
		std::memset(&imgRaw[0], 0, sz);

		mTheta = nTheta;
		mPhi = nPhi;

		float* img = reinterpret_cast<float*>(&(imgRaw[0]));
		for (int t = 0; t < nTheta; ++t)
		{
			float theta = float(t + 0.5) / nTheta * Pi;
			for (int p = 0; p < nPhi; ++p)
			{
				float phi = float(p + 0.5) / nPhi * 2. * Pi;
				// Vector corresponding to the direction for this pixel.
				float3 v(std::cos(phi) * std::sin(theta), std::cos(theta),
					std::sin(phi) * std::sin(theta));
				// Compute the angle between the pixel's direction and the sun
				// direction.
				float gamma = std::acos(Math::Clamp(float3::dot(v, sunDir), -1.0f, 1.0f));
				assert(gamma >= 0 && gamma <= Pi);
				for (int c = 0; c < num_channels; ++c)
				{
					float val = 0;

					if (theta > PiOver2)//ground rendering
					{
						break;
					}
					else
					{
						if (gamma < Math::ToRadians(angleToSunActivation))
							val = arhosekskymodel_solar_radiance(skymodel_state[c], theta, gamma, lambda[c]);
						else
							val = arhosekskymodel_radiance(skymodel_state[c], theta, gamma, lambda[c]);
					}

					// For each of red, green, and blue, average the three
					// values for the three wavelengths for the color.
					// TODO: do a better spectral->RGB conversion.
					img[4 * (t * nPhi + p) + c / 3] += val / 3.f;
					img[4 * (t * nPhi + p) + 3] = 1.0f;//alpha
				}
			}
		}

		for (unsigned int i = 0; i < num_channels; i++)
			arhosekskymodelstate_free(skymodel_state[i]);

		mSurface.Initialize({ uint32_t(mPhi), uint32_t(mTheta) }, Image::Format::eFloat4, imgRaw.data());
		//mSurface.Initialize({ IMG_COMPONENT_TYPE_FLOAT32, 3 }, { uint32_t(mPhi), uint32_t(mTheta) }, img);

		ComputePDF();
	}

	void Sky::Initialize(const std::filesystem::path& filePath)
	{
		bool loaded = mSurface.Initialize(filePath, false);
		assert(loaded);
		ComputePDF();
	}

	//phi[0 : 2Pi]
	//theta[0(top) : Pi(bottom)]
	float4 Sky::Sample(float phi, float theta, Image::Filter filter)
	{
		float2 coords((phi / TwoPi), (theta / Pi));
		//int x = static_cast<int>((phi/TwoPi) * mPhi);
		//int y = static_cast<int>((theta/Pi) * mTheta);

		return mSurface.Sample(coords.x, coords.y, filter);
		//float3* img = reinterpret_cast<float3*>(&(mImgRaw[0]));
		//float3 pixel = img[x + y * mPhi];
		//return MakeFloat4(pixel.x, pixel.y, pixel.z, 1.0f);
	}

	void Sky::ComputePDF()
	{
		const auto& geom = mSurface.GetResolution();
		std::vector<float> surfacePDF;
		surfacePDF.resize(geom.mWidth * geom.mHeight);
		const auto& data = mSurface.GetTexels();
		const float4* pData = reinterpret_cast<const float4*>(&data[0]);

		//SphericalCoord maxLumCoord;
		float maxY = 0;
		uint2 maxYCoord;
		for (uint32_t y = 0; y < geom.mHeight; ++y)
		{
			for (uint32_t x = 0; x < geom.mWidth; ++x)
			{
				float2 spherical((float(x) / geom.mWidth) * TwoPi, (float(y) / geom.mHeight) * Pi);
				float4 sample = pData[x + y * geom.mWidth];
				float Y = 0.2126f * sample.x + 0.7152f * sample.y + 0.0722f * sample.z;
				if (maxY < Y)
				{
					//record this coord
					maxY = Y;
					//maxLumCoord.Set(spherical.x, spherical.y, 1);
					maxYCoord = { x,y };
				}
				surfacePDF[x + y * geom.mWidth] = Y * sin(spherical.y);
			}
		}
		//float3 carth = maxLumCoord.GetCarthesian();
		//Core::Logger::Instance().Log("Max Lum:(%d,%d) (%f, %f) (%f, %f, %f)\n", maxYCoord.x, maxYCoord.y, maxLumCoord.Azimuth(), maxLumCoord.Polar(), carth.x, carth.y, carth.z);

		//compute distribution
		mDistribution.Initialize(geom.mWidth, geom.mHeight, &surfacePDF[0]);
	}

	void Sky::RenderToFile(const char* outfile)
	{
		mSurface.Write(outfile);
	}
}
