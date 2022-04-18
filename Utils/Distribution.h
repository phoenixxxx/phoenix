#pragma once

#include <vector>
#include <algorithm>

#include "Types.h"
#include "VectorMath.h"

/*
From Physically Based Rendering v3
Section 13.3.1
Section 13.6.7
Section 14.2.4
*/
namespace Phoenix
{
	class Distribution1D
	{
	public:
		template <typename Source>
		void Initialize(uint32_t n, const Source& data)
		{
			mCDF.resize(n + 1);
			mF.resize(n);

			mCDF[0] = 0;
			for (uint32_t iVal = 1; iVal < n + 1; ++iVal)
			{
				float f = data(iVal - 1);
				mF[iVal - 1] = f;
				mCDF[iVal] = mCDF[iVal - 1] + f / n;
			}

			mIntegral = mCDF[n];//last entry in cdf is sum of all f(x)

			//normalize to get CDF ([0:1])
			if (mIntegral == 0)
			{
				for (uint32_t i = 1; i < n + 1; ++i) mCDF[i] = float(i) / n;
			}
			else
			{
				for (uint32_t i = 1; i < n + 1; ++i) mCDF[i] /= mIntegral;
			}
		}

		float Sample(float& pdf, float uniformU, int* pOffset = nullptr)const
		{
			int cdfLen = (int)mCDF.size();
			int first = 0, len = cdfLen;
			while (len > 0)
			{
				int half = len >> 1, middle = first + half;
				// Bisect range based on value of _pred_ at _middle_
				if (mCDF[middle] <= uniformU)
				{
					first = middle + 1;
					len -= half + 1;
				}
				else
					len = half;
			}
			int offset = std::clamp(first - 1, 0, cdfLen - 2);
			if (pOffset != nullptr)
				*pOffset = offset;

			// Compute offset along CDF segment
			float du = uniformU - mCDF[offset];
			if ((mCDF[offset + 1] - mCDF[offset]) > 0) {
				du /= (mCDF[offset + 1] - mCDF[offset]);
			}

			// Compute PDF for sampled offset
			pdf = (mIntegral > 0) ? mF[offset] / (mIntegral * (int)mF.size()) : 0;

			return (offset + du) / ((int)mF.size());
		}

		//getters
		float GetIntegral()const { return mIntegral; }
		const std::vector<float>& GetCDF()const { return mCDF; }
		const std::vector<float>& GetF()const { return mF; }

	private:
		std::vector<float> mCDF;//cumulative distribution function
		std::vector<float> mF;//piecewise discreet function
		float mIntegral;
	};

	class Distribution2D
	{
	public:
		void Initialize(uint32_t width, uint32_t height, float* data)
		{
			//create the conditional sampling distribution
			mConDF.resize(height);
			for (uint32_t iHeight = 0; iHeight < height; ++iHeight)
			{
				float* pRow = data + (iHeight * width);//skip to the right row
				mConDF[iHeight].Initialize(width, [&](uint32_t index) {return pRow[index]; });
			}

			//create the marginal sampling distribution
			mMDF.Initialize(height, [&](uint32_t index) {return mConDF[index].GetIntegral(); });//each entry in the function for the MDS is the integral of the CDF
		}

		float2 Sample(float& pdf, const float2& uniformU) const
		{
			float pdfs[2];
			int v;
			float d1 = mMDF.Sample(pdfs[1], uniformU.y, &v);
			float d0 = mConDF[v].Sample(pdfs[0], uniformU.x);
			pdf = pdfs[0] * pdfs[1];
			return float2(d0, d1);
		}

		//getters
		const Distribution1D& GetMDF()const { return mMDF; }
		const std::vector<Distribution1D>& GetConDF()const { return mConDF; }
	private:
		// Distribution2D Private Data
		std::vector<Distribution1D> mConDF;//conditional density function
		Distribution1D mMDF;//marginal density function
	};
}

