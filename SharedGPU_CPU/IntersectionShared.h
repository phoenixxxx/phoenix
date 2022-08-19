#ifndef __INTERSECTION_SHARED_HLSL__
#define __INTERSECTION_SHARED_HLSL__

#include "SharedTypes.h"

#if defined CPU_ENVIRONMENT
namespace IntersectionShared {
#endif
	struct GlobalInput
	{
		BVHNodeIndex mRootRef;
	};

#if defined CPU_ENVIRONMENT
}//namespace PrimaryRaysShared 
#endif

#endif //__INTERSECTION_SHARED_HLSL__