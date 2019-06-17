#include <optix.h>
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include <OptiX/_Define.h>
#include "Define.h"
using namespace optix;

rtDeclareVariable(uint2, index, rtLaunchIndex, );
rtBuffer<float4, 2>result;
rtBuffer<float3>vertexBuffer;
rtBuffer<float3>normalBuffer;
rtBuffer<uint3>indexBuffer;
rtDeclareVariable(unsigned int, frame, , );
rtDeclareVariable(unsigned int, texid, , );
//rtDeclareVariable(float3, background, , );
rtDeclareVariable(float3, materialColor, , );
rtDeclareVariable(Define::Trans, trans, , );
rtDeclareVariable(float, offset, , );
rtDeclareVariable(unsigned int, depthMax, , );
rtDeclareVariable(rtObject, group, , );
rtDeclareVariable(Ray, ray, rtCurrentRay, );
rtDeclareVariable(Define::RayData, rayData, rtPayload, );
rtDeclareVariable(float, l, rtIntersectionDistance, );
rtDeclareVariable(float3, normal, attribute normal, );
rtDeclareVariable(float2, texcoord, attribute texcoord, );

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
	float3 answer = make_float3(0);
	if (rayData.depth < depthMax)
	{
		float3 r = make_float3(1);
		float3 t = make_float3(1);
		float n = 1.5f;
		float cosi1 = dot(ray.direction, normal);
		if (cosi1 > 0) n = 1 / n;
		float sini1 = sqrtf(1 - cosi1 * cosi1);
		float sini2 = sini1 / n;
		Ray rayNow;
		rayNow.origin = ray.origin + l * ray.direction;
		Define::RayData rayDataNow;
		rayDataNow.color = make_float3(0);
		rayDataNow.depth = rayData.depth + 1;
		if (sini2 < 1)
		{
			float cosi2 = sqrtf(1 - sini2 * sini2);
			if (sini2 <= 0.02)
			{
				float ahh = 4 * n / ((n + 1) * (n + 1));
				t *= ahh;
				r *= 1 - ahh;
			}
			else
			{
				float a1 = n * fabsf(cosi1) + cosi2;
				float a2 = fabsf(cosi1) + n * cosi2;
				r *= (pow((n * cosi2 - fabsf(cosi1)) / a2, 2) + pow((cosi2 - n * fabsf(cosi1)) / a1, 2)) / 2;
				t *= 2 * cosi2 * (1 / pow(a1, 2) + 1 / pow(a2, 2)) * n * fabsf(cosi1);
			}
			rayDataNow.weight = rayData.weight * t;
			if (rayDataNow.weight.x + rayDataNow.weight.y + rayDataNow.weight.z > 0.01)
			{
				rayNow.direction = (ray.direction + (n * copysignf(cosi2, cosi1) - cosi1) * normal) / n;
				rayNow.tmin = offset;
				rayNow.tmax = RT_DEFAULT_MAX;
				rtTrace(group, rayNow, rayDataNow);
				rayNow.origin = ray.origin + l * ray.direction;
				answer += rayDataNow.color * t;
			}
		}
		else
			r = make_float3(1);
		rayDataNow.weight = rayData.weight * t;
		if (rayDataNow.weight.x + rayDataNow.weight.y + rayDataNow.weight.z > 0.01)
		{
			rayNow.direction = ray.direction - 2 * cosi1 * normal;
			rayNow.tmin = offset;
			rayNow.tmax = RT_DEFAULT_MAX;
			rayDataNow.color = make_float3(0);
			rtTrace(group, rayNow, rayDataNow);
			answer += rayDataNow.color * r;
		}
	}
	rayData.color = answer;
}
RT_PROGRAM void miss()
{
	rayData.color = make_float3(rtTexCubemap<float4>(texid, ray.direction.x, ray.direction.y, ray.direction.z));
}
RT_PROGRAM void exception()
{
	result[index] = make_float4(1.0f, 0.0f, 0.0f, 1.0f);
}
RT_PROGRAM void attrib()
{
	uint3 id = indexBuffer[rtGetPrimitiveIndex()];
	//float3 p0 = vertexBuffer[3 * id];
	//float3 p1 = vertexBuffer[3 * id + 1];
	//float3 p2 = vertexBuffer[3 * id + 2];
	//float3 d1 = p1 - p0;
	//float3 d2 = p2 - p0;
	texcoord = rtGetTriangleBarycentrics();
	normal = normalize(
		texcoord.x * normalBuffer[id.y] +
		texcoord.y * normalBuffer[id.z] +
		(1 - texcoord.x - texcoord.y) * normalBuffer[id.x]);
}