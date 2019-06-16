#include <optix.h>
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include <OptiX/_Define.h>
using namespace optix;

rtDeclareVariable(uint2, index, rtLaunchIndex, );
rtBuffer<float4, 2>result;
rtBuffer<float3> vertices;
rtDeclareVariable(unsigned int, frame, , );
rtDeclareVariable(float3, background, , );
rtDeclareVariable(float3, materialColor, , );
rtDeclareVariable(Define::Trans, trans, , );
rtDeclareVariable(float, offset, , );
rtDeclareVariable(rtObject, group, , );
rtDeclareVariable(Define::RayData, rayData, rtPayload, );

RT_PROGRAM void rayAllocator()
{
	size_t2 size(result.size());
	uint2 screen = make_uint2(size.x, size.y);
	float2 ahh = random(index, screen, frame) + make_float2(index) - make_float2(size) / 2.0f;
	float4 d = make_float4(ahh, trans.z0, 0);
	Ray ray(trans.r0, normalize(trans.trans * d), CloseRay, offset);
	Define::RayData rayData;
	rayData.depth = 0;
	rtTrace(group, ray, rayData);
	if (frame)
		result[index] += make_float4(rayData.color, 1.0f);
	else
		result[index] = make_float4(rayData.color, 1.0f);
}
RT_PROGRAM void anyHit()
{
	rtTerminateRay();
}
RT_PROGRAM void closeHit()
{
	rayData.color = materialColor;
}
RT_PROGRAM void miss()
{
	rayData.color = background;
}
RT_PROGRAM void exception()
{
	result[index] = make_float4(1.0f, 0.0f, 0.0f, 0.0f);
}