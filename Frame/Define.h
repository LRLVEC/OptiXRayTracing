#pragma once
#include <optix.h>
#include <optix_host.h>
//#include <optix_device.h>
#define NOMINMAX
#include <optixu/optixu_math_namespace.h>

#define CloseRay 0
#define AnyRay 1


namespace Define
{
	struct Color
	{
		float x;
		float y;
		float z;
		__device__ operator optix::float3()
		{
			return optix::make_float3(x, y, z);
		}
	};
	struct Eye
	{
		optix::float3 r0;
		float z0;
	};
	struct RayData
	{
		optix::float3 color;
		int depth;
	};
	struct Trans
	{
		
	};
}