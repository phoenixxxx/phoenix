#include "CPUTimer.h"

namespace Phoenix
{
	CPUTimer::CPUTimer()
	{
	}
	CPUTimer::~CPUTimer()
	{
	}
	void CPUTimer::Start()
	{
		mStart = std::chrono::high_resolution_clock::now();
	}

	void CPUTimer::Stop()
	{
		mStop = std::chrono::high_resolution_clock::now();
	}

	double CPUTimer::GetMilliseconds()
	{
		std::chrono::duration<double> diff = mStop - mStart;
		double ms = diff.count() * 1000.0;
		return ms;
	}
	double CPUTimer::GetMicroseconds()
	{
		std::chrono::duration<double> diff = mStop - mStart;
		double ms = diff.count() * 1000000.0;
		return ms;
	}
}
