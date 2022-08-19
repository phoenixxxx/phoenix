#pragma once
#include <vector>
#include <Utils/Singleton.h>
#include <Utils/Types.h>
#include <Utils/Utils.h>
#include "VKManager.h"
#include <vulkan/vulkan.h>
#include "HLSLShaderCache.h"

namespace Phoenix
{
	class VKManager
	{
	public:
		DEFINE_HANDLE(Device);
		DEFINE_HANDLE(Buffer);
		DEFINE_HANDLE(Program);

	public:
		static Buffer BufferAllocate(Device device, size_t size, bool allowTransfer, bool cpuAccess);
		static void   BufferRelease(Device device, Buffer buffer);
		static void*  BufferMap(Device device, Buffer buffer, size_t size = VK_WHOLE_SIZE, size_t offset = 0);
		static void   BufferUnMap(Device device, Buffer buffer, size_t size = VK_WHOLE_SIZE, size_t offset = 0);
		static void   BufferCopy(Device device, Buffer srcBuffer, Buffer dstBuffer, size_t size, size_t srcOffset, size_t dstOffset);
		static void   BufferFill(Device device, Buffer buffer, size_t size, size_t offset, uint32_t data);

		Program ProgramGetFromAssets(Device device, const char* file, const char* entryPoint);
		Program ProgramGetFromAssets(Device device, const char* file, const char* entryPoint, std::vector<KeyValuePair>& defines);
		Program ProgramGet(Device device, const char* file, const char* entryPoint, const char* headerDir, std::vector<KeyValuePair>& defines);

		static VkDescriptorSet ProgramAllocateDescriptorSet(Device device, Program program);
		static void   ProgramBindInput(Device device, VkDescriptorSet descriptorSet, Program program, uint32_t count, cstr_t names[], Buffer buffers[], VkDeviceSize offsets[]=nullptr, VkDeviceSize sizes[]=nullptr);
		static void   ProgramDispatch(Device device, VkDescriptorSet descriptorSet, Program program, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

	public:
		Device GetDevice(cstr_t vendor=nullptr);
		static void Begin(Device device);
		static void End(Device device);

	public:
		bool Initialize();
		void Shutdown();

		SINGLETON_METHODS(VKManager)

	private:
		HLSLShaderCache mCache;
		std::vector<Device> mDevices;
		std::map<stdstr_t, Program> mPrograms;
	};
}
