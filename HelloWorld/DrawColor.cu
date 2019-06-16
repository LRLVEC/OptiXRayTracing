#include <optix.h>
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtBuffer<float4, 2>   result;

rtDeclareVariable(float3, color, , );

RT_PROGRAM void drawColor()
{
	result[launch_index] = make_float4(
		color.x + (float)launch_index.x / result.size().x,
		color.y + (float)launch_index.y / result.size().y,
		color.z + (float)launch_index.x / result.size().x, 0.f);
}
