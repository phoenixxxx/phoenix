#pragma once
#include <vector>
#include <Utils/Singleton.h>
#include <Utils/Types.h>
#include "VKManager.h"
#include <vulkan/vulkan.h>
#include "HLSLShaderCache.h"

namespace Phoenix
{
	class VKManager
	{
	public:
		bool Initialize();
		void Shutdown();
		SINGLETON_METHODS(VKManager)

	private:
		HLSLShaderCache mCache;
	};
}
