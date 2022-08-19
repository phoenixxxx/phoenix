#include "VKManager.h"
#include <ThirdParty/spirv_reflect/spirv_reflect.h>

#define VK_CHECK(x) { VkResult status = x; if(status != VK_SUCCESS) {printf("%s:%d Vulkan Error: %s\n", __FILE__, __LINE__, VKErrorToString(status)); assert(0);}}
#define VK_CHECK_ERR() { if (error != CL_SUCCESS) {printf("%s:%d Vulkan Error: %s\n", __FILE__, __LINE__, VKErrorToString(error)); assert(0);}}

namespace Phoenix
{
	struct Device_T
	{
		VkPhysicalDevice mPhyDevice;
		VkDevice mDevice;
		VkQueue mComputeQueue;

		VkFence mFence;
		VkCommandPool   mCommandPool;
		VkCommandBuffer mCommandBuffer;

		VkQueryPool mQueryPool;
	};

	struct Buffer_T
	{
		Buffer_T():mBuffer(nullptr), mMemory(nullptr), mSize(0), mAlignedSize(0), mCPUAccess(false){}
		VkBuffer mBuffer;
		VkDeviceMemory mMemory;
		size_t mSize;
		size_t mAlignedSize;
		bool mCPUAccess;
	};

	struct Program_T
	{
		VkDevice mDevice;

		VkShaderModule mModule;
		VkPipeline mComputePipeline;
		VkPipelineLayout mComputePipelineLayout;
		VkDescriptorSetLayout mDescriptorSetLayout;
		VkDescriptorPool mDescPool;

		std::map<stdstr_t, VkDescriptorSetLayoutBinding> mInputs;
	};

	static inline VkDescriptorType ConvertDescriptorType(SpvReflectDescriptorType type)
	{
		switch (type)
		{
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		}

	}

	static inline VkShaderStageFlagBits ConvertDescriptorType(SpvReflectShaderStageFlagBits stageBits)
	{
		switch (stageBits)
		{
		case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT: return VK_SHADER_STAGE_GEOMETRY_BIT;
		case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT: return VK_SHADER_STAGE_FRAGMENT_BIT;
		case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT: return VK_SHADER_STAGE_COMPUTE_BIT;
		case SPV_REFLECT_SHADER_STAGE_TASK_BIT_NV: return VK_SHADER_STAGE_TASK_BIT_NV;
		case SPV_REFLECT_SHADER_STAGE_MESH_BIT_NV: return VK_SHADER_STAGE_MESH_BIT_NV;
		case SPV_REFLECT_SHADER_STAGE_RAYGEN_BIT_KHR: return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		case SPV_REFLECT_SHADER_STAGE_ANY_HIT_BIT_KHR: return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		case SPV_REFLECT_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		case SPV_REFLECT_SHADER_STAGE_MISS_BIT_KHR: return VK_SHADER_STAGE_MISS_BIT_KHR;
		case SPV_REFLECT_SHADER_STAGE_INTERSECTION_BIT_KHR: return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
		case SPV_REFLECT_SHADER_STAGE_CALLABLE_BIT_KHR: return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
		}

	}

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
	static uint32_t FindSuitableMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
		uint32_t index = uint32_t(-1);
		uint64_t maxHeapSize = 0;
		for (uint32_t iMem = 0; iMem < memProperties.memoryTypeCount; ++iMem)
		{
			uint32_t heapIndex = memProperties.memoryTypes[iMem].heapIndex;

			if ((typeFilter & (1 << iMem)) && ((memProperties.memoryTypes[iMem].propertyFlags & properties) == properties))
			{
				if (memProperties.memoryHeaps[heapIndex].size > maxHeapSize)
				{
					maxHeapSize = memProperties.memoryHeaps[heapIndex].size;
					index = iMem;
				}
			}
		}
		assert(index != uint32_t(-1));
		return index;
	}

#pragma region BUFFER
	VKManager::Buffer VKManager::BufferAllocate(Device device, size_t size, bool allowTransfer, bool cpuAccess)
	{
		Buffer buffer = new Buffer_T();
		buffer->mSize = size;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;

		bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | (allowTransfer ? VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT : 0);
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK(vkCreateBuffer(device->mDevice, &bufferInfo, nullptr, &buffer->mBuffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device->mDevice, buffer->mBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindSuitableMemoryType(device->mPhyDevice, memRequirements.memoryTypeBits, (cpuAccess ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

		VK_CHECK(vkAllocateMemory(device->mDevice, &allocInfo, nullptr, &buffer->mMemory));
		VK_CHECK(vkBindBufferMemory(device->mDevice, buffer->mBuffer, buffer->mMemory, 0));

		buffer->mAlignedSize = allocInfo.allocationSize;
		buffer->mCPUAccess = cpuAccess;

		return buffer;
	}

	void VKManager::BufferRelease(Device device, Buffer buffer)
	{
		vkDestroyBuffer(device->mDevice, buffer->mBuffer, nullptr);
		vkFreeMemory(device->mDevice, buffer->mMemory, nullptr);
		delete buffer;
	}

	void* VKManager::BufferMap(Device device, Buffer buffer, size_t size, size_t offset)
	{
		assert(buffer->mCPUAccess);
		void* data;
		VK_CHECK(vkMapMemory(device->mDevice, buffer->mMemory, offset, size, 0, &data));

		VkMappedMemoryRange range = {};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.pNext = NULL;
		range.memory = buffer->mMemory;
		range.offset = offset;
		range.size = size;
		//this ensure that cached data makes it to the mapped pages (in case reading)
		VK_CHECK(vkInvalidateMappedMemoryRanges(device->mDevice, 1, &range));

		return data;
	}

	void VKManager::BufferUnMap(Device device, Buffer buffer, size_t size, size_t offset)
	{
		//flush
		VkMappedMemoryRange range = {};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.pNext = NULL;
		range.memory = buffer->mMemory;
		range.offset = offset;
		range.size = size;
		//this ensures that cached data makes it to mapped pages (in case writing)
		VK_CHECK(vkFlushMappedMemoryRanges(device->mDevice, 1, &range));

		assert(buffer->mCPUAccess);
		vkUnmapMemory(device->mDevice, buffer->mMemory);
	}

	void VKManager::BufferCopy(Device device, Buffer srcBuffer, Buffer dstBuffer, size_t size, size_t srcOffset, size_t dstOffset)
	{
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = srcOffset;
		copyRegion.dstOffset = dstOffset;
		copyRegion.size = size;
		vkCmdCopyBuffer(device->mCommandBuffer, srcBuffer->mBuffer, dstBuffer->mBuffer, 1, &copyRegion);
	}

	void VKManager::BufferFill(Device device, Buffer buffer, size_t size, size_t offset, uint32_t data)
	{
		vkCmdFillBuffer(device->mCommandBuffer, buffer->mBuffer, offset, size, data);
	}

#pragma endregion 

#pragma region PROGRAM
	VKManager::Program VKManager::ProgramGetFromAssets(Device device, const char* file, const char* entryPoint)
	{
		std::vector<KeyValuePair> defines;
		return ProgramGetFromAssets(device, file, entryPoint, defines);
	}

	VKManager::Program VKManager::ProgramGetFromAssets(Device device, const char* file, const char* entryPoint, std::vector<KeyValuePair>& defines)
	{
		const auto& fullPath = (ASSETS_PATH + stdstr_t("Shaders/") + file);
		return ProgramGet(device, fullPath.c_str(), entryPoint, SHARED_GPU_CPU, defines);
	}

	VKManager::Program VKManager::ProgramGet(Device device, const char* file, const char* entryPoint, const char* headerDir, std::vector<KeyValuePair>& defines)
	{
		Program program = nullptr;

		const auto& shaderPath = mCache.Compile(file,
			entryPoint,
			"cs_6_2",
			defines,
			headerDir);
		assert(!shaderPath.empty());

		const auto& iter = mPrograms.find(shaderPath);
		if (iter == mPrograms.end())
		{
			program = new Program_T();
			program->mDevice = device->mDevice;

			//create shader module
			VkShaderStageFlagBits shaderStage;
			{
				std::vector<byte_t> source;
				source = std::move(LoadFile(shaderPath.c_str()));
				assert(!source.empty());

				VkShaderModuleCreateInfo moduleInfo = {};
				moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleInfo.pNext = nullptr;
				moduleInfo.codeSize = source.size();
				moduleInfo.pCode = reinterpret_cast<const uint32_t*>(&source[0]);
				VK_CHECK(vkCreateShaderModule(device->mDevice, &moduleInfo, nullptr, &program->mModule));

				SpvReflectShaderModule reflectModule;
				SpvReflectResult result = spvReflectCreateShaderModule(moduleInfo.codeSize, moduleInfo.pCode, &reflectModule);
				assert(result == SPV_REFLECT_RESULT_SUCCESS);

				// Get Entry point
				const SpvReflectEntryPoint* spvReflectEntry = spvReflectGetEntryPoint(&reflectModule, entryPoint);
				shaderStage = ConvertDescriptorType(spvReflectEntry->shader_stage);

				// Enumerate and extract shader's input variables
				uint32_t bindings_count = 0;
				result = spvReflectEnumerateDescriptorBindings(&reflectModule, &bindings_count, NULL);
				assert(result == SPV_REFLECT_RESULT_SUCCESS);
				assert(bindings_count != 0);//unused bindings optimized away
				std::vector<SpvReflectDescriptorBinding*> bindings(bindings_count);
				result = spvReflectEnumerateDescriptorBindings(&reflectModule, &bindings_count, &bindings[0]);
				assert(result == SPV_REFLECT_RESULT_SUCCESS);

				std::map<VkDescriptorType, uint32_t> descTypesUsed;
				std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding;
				for (const auto& binding : bindings)
				{
					VkDescriptorSetLayoutBinding vkBinding;
					vkBinding.binding = binding->binding;
					vkBinding.descriptorType = ConvertDescriptorType(binding->descriptor_type);
					vkBinding.descriptorCount = 1;
					vkBinding.stageFlags = shaderStage;
					vkBinding.pImmutableSamplers = nullptr;
					setLayoutBinding.push_back(vkBinding);
					program->mInputs[binding->name] = vkBinding;

					//store how many of each type is needed
					const auto& iter = descTypesUsed.find(vkBinding.descriptorType);
					if (iter == descTypesUsed.end())
						descTypesUsed[vkBinding.descriptorType] = 1;
					else
						iter->second++;
				}
				spvReflectDestroyShaderModule(&reflectModule);

				VkDescriptorSetLayoutCreateInfo setLayoutInfo = {};
				setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				setLayoutInfo.pNext = nullptr;
				setLayoutInfo.flags = 0;
				setLayoutInfo.bindingCount = bindings_count;
				setLayoutInfo.pBindings = &setLayoutBinding[0];
				VK_CHECK(vkCreateDescriptorSetLayout(device->mDevice, &setLayoutInfo, nullptr, &program->mDescriptorSetLayout));

				//create the descriptor pool for the program
				VkDescriptorPoolCreateInfo poolInfo = {};
				poolInfo.pNext = nullptr;
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;//vkAllocateDescriptorSets, vkFreeDescriptorSets, and vkResetDescriptorPool are allowed.
				poolInfo.maxSets = 10;//arbitrary (could be up to 10 frames in flight)

				std::vector<VkDescriptorPoolSize> sizes;
				for (const auto& type : descTypesUsed)
					sizes.push_back({ type.first, type.second });

				poolInfo.poolSizeCount = sizes.size();
				poolInfo.pPoolSizes = sizes.data();
				VK_CHECK(vkCreateDescriptorPool(device->mDevice, &poolInfo, nullptr, &program->mDescPool));
			}

			//{
			//	//Allocate the descriptor set
			//	VkDescriptorSetAllocateInfo dsAllocInfo = {};
			//	dsAllocInfo.pNext = nullptr;
			//	dsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			//	dsAllocInfo.descriptorPool = program->mDescPool;
			//	dsAllocInfo.descriptorSetCount = 1;
			//	dsAllocInfo.pSetLayouts = &program->mDescriptorSetLayout;
			//	VK_CHECK(vkAllocateDescriptorSets(device->mDevice, &dsAllocInfo, &program->mDescriptorSet));
			//}

			//Create layout
			{
				VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
				pipelineLayoutInfo.pNext = nullptr;
				pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.flags = 0;
				pipelineLayoutInfo.setLayoutCount = 1;
				pipelineLayoutInfo.pSetLayouts = &program->mDescriptorSetLayout;
				pipelineLayoutInfo.pushConstantRangeCount = 0;
				pipelineLayoutInfo.pPushConstantRanges = nullptr;
				VK_CHECK(vkCreatePipelineLayout(device->mDevice, &pipelineLayoutInfo, nullptr, &program->mComputePipelineLayout));

				VkPipelineShaderStageCreateInfo shaderStageInfo = {};
				shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageInfo.pNext = nullptr;
				shaderStageInfo.flags = 0;
				shaderStageInfo.stage = shaderStage;
				shaderStageInfo.module = program->mModule;
				shaderStageInfo.pName = entryPoint;
				shaderStageInfo.pSpecializationInfo = nullptr;

				VkComputePipelineCreateInfo pipelineInfo = {};
				pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
				pipelineInfo.pNext = nullptr;
				pipelineInfo.flags = 0;
				pipelineInfo.layout = program->mComputePipelineLayout;
				pipelineInfo.basePipelineHandle = nullptr;
				pipelineInfo.basePipelineIndex = 0;
				pipelineInfo.stage = shaderStageInfo;
				VK_CHECK(vkCreateComputePipelines(device->mDevice, nullptr, 1, &pipelineInfo, nullptr, &program->mComputePipeline));
			}

			//store the new program
			mPrograms[shaderPath] = program;
		}
		else
			program = iter->second;

		return program;
	}

	VkDescriptorSet VKManager::ProgramAllocateDescriptorSet(Device device, Program program)
	{
		VkDescriptorSet set;
		VkDescriptorSetAllocateInfo dsAllocInfo = {};
		dsAllocInfo.pNext = nullptr;
		dsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		dsAllocInfo.descriptorPool = program->mDescPool;
		dsAllocInfo.descriptorSetCount = 1;
		dsAllocInfo.pSetLayouts = &program->mDescriptorSetLayout;
		VK_CHECK(vkAllocateDescriptorSets(device->mDevice, &dsAllocInfo, &set));
		return set;
	}

	void VKManager::ProgramBindInput(Device device, VkDescriptorSet descriptorSet, Program program, uint32_t count, cstr_t names[], Buffer buffers[], VkDeviceSize offsets[], VkDeviceSize ranges[])
	{
		//VkWriteDescriptorSet descriptorWrite = {};
		//descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//descriptorWrite.dstSet = descriptorSet;
		//descriptorWrite.dstBinding = 0;
		//descriptorWrite.dstArrayElement = 0;
		//descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		//descriptorWrite.descriptorCount = 1;
		//descriptorWrite.pImageInfo = nullptr;
		//descriptorWrite.pTexelBufferView = nullptr;
		//VkDescriptorBufferInfo bufferWriteInfo;
		//bufferWriteInfo.buffer = buffers[0]->mBuffer;
		//bufferWriteInfo.offset = 0;
		//bufferWriteInfo.range = VK_WHOLE_SIZE;
		//descriptorWrite.pBufferInfo = &bufferWriteInfo;
		//vkUpdateDescriptorSets(device->mDevice, 1, &descriptorWrite, 0, nullptr);
		//return;


		//std::vector<VkWriteDescriptorSet> descriptorSets;
		for (uint32_t iBuffer = 0; iBuffer < count; ++iBuffer)
		{
			const auto& iter = program->mInputs.find(names[iBuffer]);
			assert(iter != program->mInputs.end());

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = iter->second.binding;

			switch (iter->second.descriptorType)
			{
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			{
				VkDescriptorBufferInfo bufferWriteInfo;
				bufferWriteInfo.buffer = buffers[iBuffer]->mBuffer;
				if (offsets != nullptr)
					bufferWriteInfo.offset = offsets[iBuffer];
				else
					bufferWriteInfo.offset = 0;
				if (ranges != nullptr)
					bufferWriteInfo.range = ranges[iBuffer];
				else
					bufferWriteInfo.range = VK_WHOLE_SIZE;

				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = iter->second.descriptorType;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pImageInfo = nullptr;
				descriptorWrite.pTexelBufferView = nullptr;
				descriptorWrite.pBufferInfo = &bufferWriteInfo;
			}break;
			}

			vkUpdateDescriptorSets(device->mDevice, 1, &descriptorWrite, 0, nullptr);
			//descriptorSets.push_back(descriptorWrite);
		}

		//vkUpdateDescriptorSets(device->mDevice, descriptorSets.size(), descriptorSets.data(), 0, nullptr);//not the fastest, but this allows for discontinuous ranges
	}

	void VKManager::ProgramDispatch(Device device, VkDescriptorSet descriptorSet, Program program, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
		vkCmdBindPipeline(device->mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, program->mComputePipeline);
		vkCmdBindDescriptorSets(device->mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, program->mComputePipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
		vkCmdDispatch(device->mCommandBuffer, groupCountX, groupCountY, groupCountZ);
	}

#pragma endregion 

	bool VKManager::Initialize()
	{
		uint32_t count;
		VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr)); //get number of extensions
		std::vector<VkExtensionProperties> extensions(count);
		VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));

		// initialize the VkApplicationInfo structure
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = NULL;
		app_info.pApplicationName = "Phoenix";
		app_info.applicationVersion = 1;
		app_info.pEngineName = "Candela";
		app_info.engineVersion = 1;
		app_info.apiVersion = VK_HEADER_VERSION_COMPLETE;

		// initialize the VkInstanceCreateInfo structure
		VkInstanceCreateInfo inst_info = {};
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = NULL;
		inst_info.flags = 0;
		inst_info.pApplicationInfo = &app_info;

		static const char* extNames[] = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
		inst_info.enabledExtensionCount = 1;
		inst_info.ppEnabledExtensionNames = extNames;

		static const char* layerNames[] = { "VK_LAYER_KHRONOS_validation" };
		inst_info.enabledLayerCount = 1;
		inst_info.ppEnabledLayerNames = layerNames;

		uint32_t layerCount;
		VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
		std::vector<VkLayerProperties> availableLayers(layerCount);
		VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

		VkInstance instance;
		VK_CHECK(vkCreateInstance(&inst_info, NULL, &instance));

		uint32_t physicalDeviceCount;
		VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
		std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
		VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, &devices[0]));
		for (uint32_t iDev = 0; iDev < physicalDeviceCount; ++iDev)
		{
			uint32_t queueFamilyPropertyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(devices[iDev], &queueFamilyPropertyCount, nullptr);
			std::vector<VkQueueFamilyProperties> qfProps(queueFamilyPropertyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(devices[iDev], &queueFamilyPropertyCount, &qfProps[0]);

			uint32_t computeQueueFamilyIndex = uint32_t(-1);
			for (uint32_t iQ = 0; iQ < qfProps.size(); ++iQ)
			{
				if (qfProps[iQ].queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					computeQueueFamilyIndex = iQ;
					break;//we don't care about any other queue at this point...
				}
			}

			if (computeQueueFamilyIndex != uint32_t(-1))
			{
				Device GPU = new Device_T();
				GPU->mPhyDevice = devices[iDev];

				//create virtual device
				{
					VkDeviceQueueCreateInfo queue_info = {};
					float queue_priorities[1] = { 0.0 };
					queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					queue_info.pNext = NULL;
					queue_info.queueCount = 1;
					queue_info.pQueuePriorities = queue_priorities;
					queue_info.queueFamilyIndex = computeQueueFamilyIndex;

					VkDeviceCreateInfo device_info = {};
					device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
					device_info.pNext = NULL;
					device_info.queueCreateInfoCount = 1;
					device_info.pQueueCreateInfos = &queue_info;
					device_info.enabledExtensionCount = 0;
					device_info.ppEnabledExtensionNames = NULL;
					device_info.enabledLayerCount = 0;
					device_info.ppEnabledLayerNames = NULL;

					VkPhysicalDeviceFeatures features;
					vkGetPhysicalDeviceFeatures(GPU->mPhyDevice, &features);
					//enable all the features exposed by the base physical device
					device_info.pEnabledFeatures = &features;

					VK_CHECK(vkCreateDevice(GPU->mPhyDevice, &device_info, nullptr, &GPU->mDevice));
				}

				//get the queue
				{
					//access queue 0 from the compute queue fam
					vkGetDeviceQueue(GPU->mDevice, computeQueueFamilyIndex, 0, &GPU->mComputeQueue);
				}

				//create cmd pool
				{
					VkCommandPoolCreateInfo poolInfo = {};
					poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
					poolInfo.pNext = nullptr;
					poolInfo.queueFamilyIndex = computeQueueFamilyIndex;
					poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;// transient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0;
					VK_CHECK(vkCreateCommandPool(GPU->mDevice, &poolInfo, nullptr, &GPU->mCommandPool));
				}

				//create cmd buffer
				{
					VkCommandBufferAllocateInfo allocInfo = {};
					allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
					allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
					allocInfo.commandPool = GPU->mCommandPool;
					allocInfo.commandBufferCount = 1;
					VK_CHECK(vkAllocateCommandBuffers(GPU->mDevice, &allocInfo, &GPU->mCommandBuffer));
				}

				//create the fence
				{
					VkFenceCreateInfo fenceInfo = {};
					fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
					fenceInfo.pNext = nullptr;
					fenceInfo.flags = 0;
					VK_CHECK(vkCreateFence(GPU->mDevice, &fenceInfo, nullptr, &GPU->mFence));
				}

				//query pool
				{
					uint32_t timeStampCount = 1024;
					VkQueryPoolCreateInfo queryPoolInfo = {};
					queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
					queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
					queryPoolInfo.queryCount = timeStampCount;
					VK_CHECK(vkCreateQueryPool(GPU->mDevice, &queryPoolInfo, nullptr, &GPU->mQueryPool));
				}

				mDevices.push_back(GPU);
			}
		}

		mCache.Initialize(DXC_PATH + stdstr_t("dxc.exe"), DXC_PATH + stdstr_t("ShaderCache"));

		return true;
	}

	void VKManager::Shutdown()
	{
		for (const auto& prog : mPrograms)
		{
			vkDestroyShaderModule(prog.second->mDevice, prog.second->mModule, nullptr);
			vkDestroyDescriptorPool(prog.second->mDevice, prog.second->mDescPool, nullptr);//destroys allocated sets
			vkDestroyDescriptorSetLayout(prog.second->mDevice, prog.second->mDescriptorSetLayout, nullptr);
			vkDestroyPipelineLayout(prog.second->mDevice, prog.second->mComputePipelineLayout, nullptr);
			vkDestroyPipeline(prog.second->mDevice, prog.second->mComputePipeline, nullptr);
			delete prog.second;
		}

		for (auto gpu : mDevices)
		{
			//an application is responsible for destroying/freeing any Vulkan 
			//objects that were created using that device as the first
			vkDestroyQueryPool(gpu->mDevice, gpu->mQueryPool, nullptr);
			vkDestroyFence(gpu->mDevice, gpu->mFence, nullptr);
			vkFreeCommandBuffers(gpu->mDevice, gpu->mCommandPool, 1, &gpu->mCommandBuffer);
			vkDestroyCommandPool(gpu->mDevice, gpu->mCommandPool, nullptr);
			vkDestroyDevice(gpu->mDevice, nullptr);

			delete gpu;
		}
	}

	static uint32_t GetVendorID(cstr_t vendor)
	{
		if (std::strcmp(vendor, "AMD") == 0)
			return 0x1002;
		else if (std::strcmp(vendor, "ImgTec") == 0)
			return 0x1010;
		else if (std::strcmp(vendor, "NVIDIA") == 0)
			return 0x10DE;
		else if (std::strcmp(vendor, "ARM") == 0)
			return 0x13B5;
		else if (std::strcmp(vendor, "Qualcomm") == 0)
			return 0x5143;
		else if (std::strcmp(vendor, "INTEL") == 0)
			return 0x8086;

		return 0;
	}

	VKManager::Device VKManager::GetDevice(cstr_t vendor)
	{
		Device device = nullptr;
		uint32_t vid = GetVendorID(vendor);
		VkPhysicalDeviceProperties props;
		for (auto iDev : mDevices)
		{
			vkGetPhysicalDeviceProperties(iDev->mPhyDevice, &props);
			if (props.vendorID == vid)
			{
				device = iDev;
				break;
			}
		}
		return device;
	}

	void VKManager::Begin(Device device)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK(vkBeginCommandBuffer(device->mCommandBuffer, &beginInfo));
		//vkCmdResetQueryPool(commandBuffer, queryPool, 0, 2);
	}

	void VKManager::End(Device device)
	{
		VK_CHECK(vkEndCommandBuffer(device->mCommandBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &device->mCommandBuffer;
		VK_CHECK(vkResetFences(device->mDevice, 1, &device->mFence));
		VK_CHECK(vkQueueSubmit(device->mComputeQueue, 1, &submitInfo, device->mFence));
		VK_CHECK(vkWaitForFences(device->mDevice, 1, &device->mFence, VK_TRUE, UINT64_MAX));
		//VK_CHECK(vkGetQueryPoolResults(device, queryPool, 0, timeStampCount, sizeof(cpuResults), cpuResults, sizeof(uint32_t), VK_QUERY_RESULT_WAIT_BIT));
	}
}