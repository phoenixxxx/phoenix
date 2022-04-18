#pragma once
#include <fstream>
#include <stack>
#include <map>
#include "Types.h"
#include "Singleton.h"
#include "CPUTimer.h"

namespace Phoenix
{
	class PerfTracer
	{
	public:
		void Initialize(cstr_t outFilePath);
		void Shutdown();
		void BeginEvent(cstr_t eventName);
		void EndEvent();
		void InstantEvent(cstr_t eventName);

		void StartGPUWork(cstr_t name);
		void EndGPUWork();
		void WriteGPUEvent(cstr_t eventName, uint64_t offset, uint64_t duration);

	private:
		std::stack<stdstr_t> mEvents;
		std::fstream mOut;
		CPUTimer     mCPUTimer;
		bool         mFileEmpty;

		struct
		{
			stdstr_t mName;
			uint64_t  mStart;
		}mGPUWorkload;

		SINGLETON_METHODS(PerfTracer)
	};
}
