#pragma once
#include <chrono>

namespace Phoenix
{
	class CPUTimer
	{
	public:
		CPUTimer();
		~CPUTimer();
		void Start();
		void Stop();
		double GetMilliseconds();
		double GetMicroseconds();
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> mStart, mStop;
	};
}