#include <optix.h>
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include <OptiX/_Define.h>
#include "Define.h"
using namespace optix;

rtDeclareVariable(uint2, index, rtLaunchIndex, );
rtBuffer<float4, 2>result;
rtBuffer<float3>vertexBuffer;
rtDeclareVariable(unsigned int, frame, , );
rtDeclareVariable(unsigned int, texid, , );
//rtDeclareVariable(float3, background, , );
rtDeclareVariable(float3, materialColor, , );
rtDeclareVariable(Define::Trans, trans, , );
rtDeclareVariable(float, offset, , );
rtDeclareVariable(rtObject, group, , );
rtDeclareVariable(Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t, rtIntersectionDistance, );
rtDeclareVariable(Define::RayData, rayData, rtPayload, );
rtDeclareVariable(float3, normal, attribute normal, );
rtDeclareVariable(float3, texcoord, attribute texcoord, );

//rtTextureSampler<uchar4, 3, cudaReadModeNormalizedFloat> ahh;

RT_PROGRAM void rayAllocator()
{
	size_t2 size(result.size());
	uint2 screen = make_uint2(size.x, size.y);
	float2 ahh = random(index, screen, frame) + make_float2(index) - make_float2(size) / 2.0f;
	float4 d = make_float4(ahh, trans.z0, 0);
	Ray rayOrigin(trans.r0, normalize(trans.trans * d), CloseRay, offset);
	Define::RayData rayDataOrigin;
	rayDataOrigin.depth = 0;
	rayDataOrigin.color = make_float3(0);
	rayDataOrigin.weight = make_float3(1);
	rtTrace(group, rayOrigin, rayDataOrigin);
	if (frame)
		result[index] += make_float4(rayDataOrigin.color, 1.0f);
	else
		result[index] = make_float4(rayDataOrigin.color, 1.0f);
}
RT_PROGRAM void anyHit()
{
	rtTerminateRay();
}
RT_PROGRAM void closeHit()
{
	if (rayData.depth < 3)
	{
		Ray rayRefl(
			ray.origin + t * ray.direction,
			ray.direction - 2 * dot(normal, ray.direction) * normal,
			CloseRay, offset);
		Define::RayData rayDataRefl = rayData;
		++rayDataRefl.depth;
		rtTrace(group, rayRefl, rayDataRefl);
		rayData.color += rayDataRefl.color * 0.9f;
	}
}
RT_PROGRAM void miss()
{
	rayData.color = make_float3(rtTexCubemap<float4>(texid, ray.direction.x, ray.direction.y, ray.direction.z));
}
RT_PROGRAM void exception()
{
	result[index] = make_float4(1.0f, 0.0f, 0.0f, 0.0f);
}
RT_PROGRAM void attrib()
{
	unsigned int id = rtGetPrimitiveIndex();
	float3 p0 = vertexBuffer[3 * id];
	float3 p1 = vertexBuffer[3 * id + 1];
	float3 p2 = vertexBuffer[3 * id + 2];
	float3 d1 = p1 - p0;
	float3 d2 = p2 - p0;
	normal = normalize(cross(d1, d2));
	texcoord = make_float3(rtGetTriangleBarycentrics());
}