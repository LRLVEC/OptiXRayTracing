#include <optix.h>
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include "Define.h"
using namespace optix;

rtDeclareVariable(uint2, index, rtLaunchIndex, );
rtBuffer<float4, 2>result;
rtBuffer<float3> vertices;
rtDeclareVariable(unsigned int, frame, , );
rtDeclareVariable(Define::Color, background, , );
rtDeclareVariable(float3, materialColor, , );
rtDeclareVariable(Define::Eye, eye, , );
rtDeclareVariable(float, offset, , );
rtDeclareVariable(rtObject, group, , );
rtDeclareVariable(Define::RayData, rayData, rtPayload, );

RT_PROGRAM void rayAllocator()
{
	size_t2 screen = result.size();
	float3 d = make_float3(
		float(index.x * 2) - float(screen.x),
		float(index.y * 2) - float(screen.y),
		eye.z0);
	optix::Ray ray(eye.r0, normalize(d), CloseRay, offset);
	Define::RayData rayData;
	rayData.depth = 0;
	rtTrace(group, ray, rayData);
	result[index] = make_float4(rayData.color, 1.0f);
}
RT_PROGRAM void anyHit()
{
	rtTerminateRay();
}
RT_PROGRAM void closeHit()
{
	rayData.color = make_float3(0.0f, 1.0f, 0.0f);
}
RT_PROGRAM void miss()
{
	rayData.color = background;
}
RT_PROGRAM void exception()
{
	result[index] = make_float4(1.0f, 0.0f, 0.0f, 0.0f);
}