#include "VKManager.h"

#define VK_CHECK(x) { VkResult status = x; if(status != VK_SUCCESS) {printf("%s:%d Vulkan Error: %s\n", __FILE__, __LINE__, VKErrorToString(status)); assert(0);}}
#define VK_CHECK_ERR() { if (error != CL_SUCCESS) {printf("%s:%d Vulkan Error: %s\n", __FILE__, __LINE__, VKErrorToString(error)); assert(0);}}

static inline const char* VKErrorToString(VkResult result)
{
	switch (result)
	{
	case VK_SUCCESS:
		return "VK_SUCCESS";
	case VK_NOT_READY:
		return "VK_NOT_READY";
	case VK_TIMEOUT:
		return "VK_TIMEOUT";
	case VK_EVENT_SET:
		return "VK_EVENT_SET";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL:
		return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "VK_ERROR_VALIDATION_FAILED_EXT";
	default:
		return "";
	}
}

namespace Phoenix
{
	bool VKManager::Initialize()
	{
		mCache.Initialize(DXC_PATH + stdstr_t("dxc.exe"), DXC_PATH + stdstr_t("ShaderCache"));

		const char* entryPoint = "Evaluate";
		uint32_t local = 32;
		std::vector<KeyValuePair> defines = {
			{"PRIMARY_RAY_LOCAL_SIZE", std::to_string(local)},
			{"MAX_STACK_DEPTH", "16"}
		};
		auto shaderPath = mCache.Compile((ASSETS_PATH + stdstr_t("Shaders/MaterialEvaluation.hlsl")).c_str(), 
			entryPoint,
			"cs_6_2",
			defines,
			SHARED_GPU_CPU);

		return true;
	}

	void VKManager::Shutdown()
	{

	}
}