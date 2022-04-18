#include <algorithm>
#include "PerfTracer.h"

namespace Phoenix
{

	static cstr_t perfFormatStart = "%s{\"name\": \"%s\", \"cat\" : \"PERF\", \"ph\" : \"B\", \"pid\" : 0, \"tid\" : 0, \"ts\" : %lld}";
	static cstr_t perfFormatStop = ",\n{\"name\": \"%s\", \"cat\" : \"PERF\", \"ph\" : \"E\", \"pid\" : 0, \"tid\" : 0, \"ts\" : %lld}";
	static cstr_t instantFormat = "%s{\"name\": \"%s\", \"ph\" : \"i\", \"pid\" : 0, \"tid\" : 0, \"ts\" : %lld, \"s\" : \"g\"}";
	static char tempBuffer[1024];

	void PerfTracer::Initialize(cstr_t outFilePath)
	{
		mOut.open(outFilePath, std::ios::out);
		mOut << "[";
		mFileEmpty = true;
	}

	void PerfTracer::Shutdown()
	{
		mOut << "]" << std::endl;
		mOut.close();
	}

	void PerfTracer::BeginEvent(cstr_t eventName)
	{
		uint64_t microsec;
		if (mFileEmpty)
		{
			mCPUTimer.Start();
			microsec = 0;
		}
		else
		{
			mCPUTimer.Stop();
			microsec = static_cast<uint64_t>(mCPUTimer.GetMicroseconds());
		}

		mEvents.push(eventName);
		snprintf(tempBuffer, sizeof(tempBuffer), perfFormatStart, mFileEmpty ? "" : ",\n", eventName, microsec);
		mFileEmpty = false;
		mOut << tempBuffer;
	}

	void PerfTracer::EndEvent()
	{
		uint64_t microsec;
		mCPUTimer.Stop();
		microsec = static_cast<uint64_t>(mCPUTimer.GetMicroseconds());

		snprintf(tempBuffer, sizeof(tempBuffer), perfFormatStop, mEvents.top().c_str(), microsec);
		mOut << tempBuffer;

		mEvents.pop();
	}

	void PerfTracer::InstantEvent(cstr_t eventName)
	{
		uint64_t microsec;
		mCPUTimer.Stop();
		microsec = static_cast<uint64_t>(mCPUTimer.GetMicroseconds());

		snprintf(tempBuffer, sizeof(tempBuffer), instantFormat, mFileEmpty ? "" : ",\n", eventName, microsec);
		mOut << tempBuffer;
	}

	void PerfTracer::StartGPUWork(cstr_t name)
	{
		uint64_t microsec;
		if (mFileEmpty)
		{
			mCPUTimer.Start();
			microsec = 0;
		}
		else
		{
			mCPUTimer.Stop();
			microsec = static_cast<uint64_t>(mCPUTimer.GetMicroseconds());
		}

		snprintf(tempBuffer, sizeof(tempBuffer), perfFormatStart, mFileEmpty ? "" : ",\n", name, microsec);
		mFileEmpty = false;
		mOut << tempBuffer;

		//record GPU work
		mGPUWorkload.mName = name;
		mGPUWorkload.mStart = microsec;

	}

	void PerfTracer::EndGPUWork()
	{
		uint64_t microsec;
		mCPUTimer.Stop();
		microsec = static_cast<uint64_t>(mCPUTimer.GetMicroseconds());

		snprintf(tempBuffer, sizeof(tempBuffer), perfFormatStop, mGPUWorkload.mName.c_str(), microsec);
		mOut << tempBuffer;
	}

	void PerfTracer::WriteGPUEvent(cstr_t eventName, uint64_t offset, uint64_t duration)
	{
		offset += mGPUWorkload.mStart;
		snprintf(tempBuffer, sizeof(tempBuffer), perfFormatStart, ",\n", eventName, offset);
		mOut << tempBuffer;
		snprintf(tempBuffer, sizeof(tempBuffer), perfFormatStop, eventName, offset + duration);
		mOut << tempBuffer;
	}

}
