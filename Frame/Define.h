#pragma once
#include <OptiX/_Define.h>
#define CloseRay 0
#define AnyRay 1
namespace Define
{
	struct RayData
	{
		float3 color;
		int depth;
	};

}