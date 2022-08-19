#ifndef __PRIMARY_RAY_SHARED_HLSL__
#define __PRIMARY_RAY_SHARED_HLSL__

#if defined CPU_ENVIRONMENT
namespace PrimaryRaysShared {
#endif
	struct GlobalInput
	{
		uint32_t mOffsetX;
		uint32_t mOffsetY;
	};

#if defined CPU_ENVIRONMENT
}//namespace PrimaryRaysShared 
#endif

#endif //__PRIMARY_RAY_SHARED_HLSL__