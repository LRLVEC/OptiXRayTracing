#include <cstdio>
#include <cstdlib>
#define __OptiX__
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <GL/_Texture.h>
#include <OptiX/_OptiX.h>
#include <OptiX/_Define.h>
#include "Define.h"
#include <_Math.h>
#include <_Time.h>
#include <_Array.h>
#include <_STL.h>

namespace OpenGL
{
	namespace OptiX
	{
		using namespace Define;
		struct MonteCarlo :RayTracer
		{
			struct Metal :Material
			{
				Variable<RTmaterial>color;
				Program closeHitProgram;
				Program anyHitProgram;
				CloseHit closeHit;
				AnyHit anyHit;
				Metal(RTcontext* _context, PTXManager& _pm)
					:
					Material(_context),
					color(&material, "materialColor"),
					closeHitProgram(_context, _pm, "metalCloseHit"),
					anyHitProgram(_context, _pm, "metalAnyHit"),
					closeHit(&material, closeHitProgram, CloseRay),
					anyHit(&material, anyHitProgram, AnyRay)
				{
					closeHit.setProgram();
					anyHit.setProgram();
				}
			};
			struct Glass :Material
			{
				Variable<RTmaterial>color;
				Program closeHitProgram;
				Program anyHitProgram;
				CloseHit closeHit;
				AnyHit anyHit;
				Glass(RTcontext* _context, PTXManager& _pm)
					:
					Material(_context),
					color(&material, "materialColor"),
					closeHitProgram(_context, _pm, "glassCloseHit"),
					anyHitProgram(_context, _pm, "glassAnyHit"),
					closeHit(&material, closeHitProgram, CloseRay),
					anyHit(&material, anyHitProgram, AnyRay)
				{
					closeHit.setProgram();
					anyHit.setProgram();
				}
			};
			struct Diffuse :Material
			{
				Variable<RTmaterial>color;
				Program closeHitProgram;
				Program anyHitProgram;
				CloseHit closeHit;
				AnyHit anyHit;
				Diffuse(RTcontext* _context, PTXManager& _pm)
					:
					Material(_context),
					color(&material, "materialColor"),
					closeHitProgram(_context, _pm, "diffuseCloseHIt"),
					anyHitProgram(_context, _pm, "diffuseAnyHit"),
					closeHit(&material, closeHitProgram, CloseRay),
					anyHit(&material, anyHitProgram, AnyRay)
				{
					closeHit.setProgram();
					anyHit.setProgram();
				}
			};
			struct Scatter :Material
			{
				Variable<RTmaterial>color;
				Program closeHitProgram;
				Program anyHitProgram;
				CloseHit closeHit;
				AnyHit anyHit;
				Scatter(RTcontext* _context, PTXManager& _pm)
					:
					Material(_context),
					color(&material, "materialColor"),
					closeHitProgram(_context, _pm, "scatterCloseHit"),
					anyHitProgram(_context, _pm, "scatterAnyHit"),
					closeHit(&material, closeHitProgram, CloseRay),
					anyHit(&material, anyHitProgram, AnyRay)
				{
					closeHit.setProgram();
					anyHit.setProgram();
				}
			};
			DefautRenderer renderer;
			PTXManager pm;
			Context context;
			Trans trans;
			Program exception;
			Buffer resultBuffer;
			Buffer texBuffer;
			Metal metal;
			Glass glass;
			Diffuse diffuse;
			Scatter scatter;
			GeometryTriangles triangles;
			GeometryTriangles trianglesIndexed;
			RTtexturesampler sampler;
			Variable<RTcontext> result;
			Variable<RTcontext> texTest;
			Variable<RTcontext> frame;
			Variable<RTcontext> texid;
			Variable<RTcontext> offset;
			Variable<RTcontext> depthMax;
			Variable<RTcontext> russian;
			GeometryInstance instanceMetal;
			GeometryInstance instanceGlass;
			GeometryInstance instanceDiffuse;
			GeometryInstance instanceScatter;
			GeometryGroup geoGroupMetal;
			GeometryGroup geoGroupGlass;
			GeometryGroup geoGroupDiffuse;
			GeometryGroup geoGroupScatter;
			Transform transGlass;
			Transform transDiffuse;
			Transform transScatter;
			Group group;
			STL ahh;
			BMP testBMP;
			BMPCube testCube;
			unsigned int frameNum;
			MonteCarlo(::OpenGL::SourceManager* _sm, FrameScale const& _size)
				:
				renderer(_sm, _size),
				pm(&_sm->folder),
				context(pm, { {0,"rayAllocator"} }, { {CloseRay,"miss"} }, 2, 30),
				trans({ context, {70.0},{0.001,0.9,0.0005},{0.03},{0,0,0},700.0 }),
				exception(context, pm, "exception"),
				resultBuffer(context, RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, renderer),
				texBuffer(context, RT_BUFFER_INPUT | RT_BUFFER_CUBEMAP, RT_FORMAT_UNSIGNED_BYTE4),
				metal(context, pm),
				glass(context, pm),
				diffuse(context, pm),
				scatter(context, pm),
				triangles(context, pm, "attrib", 1, RT_GEOMETRY_BUILD_FLAG_NONE,
					{
						{"vertexBuffer",RT_BUFFER_INPUT, RT_FORMAT_FLOAT3}
					}),
				trianglesIndexed(context, pm, "attribIndexed", 1, RT_GEOMETRY_BUILD_FLAG_NONE,
					{
						{"vertexBufferIndexed",RT_BUFFER_INPUT, RT_FORMAT_FLOAT3},
						{"normalBuffer",RT_BUFFER_INPUT, RT_FORMAT_FLOAT3},
						{"indexBuffer",RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT3}
					}),
				result(context, "result"),
				texTest(context, "ahh"),
				frame(context, "frame"),
				texid(context, "texid"),
				offset(context, "offset"),
				depthMax(context, "depthMax"),
				russian(context, "russian"),
				instanceMetal(context),
				instanceGlass(context),
				instanceDiffuse(context),
				instanceScatter(context),
				geoGroupMetal(context, Acceleration::Trbvh),
				geoGroupGlass(context, Acceleration::Trbvh),
				geoGroupDiffuse(context, Acceleration::Trbvh),
				geoGroupScatter(context, Acceleration::Trbvh),
				transGlass(context),
				transDiffuse(context),
				transScatter(context),
				group(context, "group", Acceleration::Trbvh),
				ahh(pm.folder->find("resources/Stanford_bunny.stl").readSTL()),
				testBMP("resources/lightSource.bmp"),
				testCube("resources/room/"),
				frameNum(0)
			{
				renderer.prepare();
				context.pringStackSize();
				trans.init(_size);
				resultBuffer.setSize(_size.w, _size.h);
				rtContextSetExceptionProgram(context, 0, exception);
				//RTtexturesampler sampler;
				//rtContextSetPrintEnabled(context, 1);
				//rtContextSetPrintBufferSize(context, 4096);
				texBuffer.readCube(testCube);
				texBuffer.readBMP(testBMP);
				rtTextureSamplerCreate(context, &sampler);
				rtTextureSamplerSetWrapMode(sampler, 0, RT_WRAP_CLAMP_TO_EDGE);
				rtTextureSamplerSetWrapMode(sampler, 1, RT_WRAP_CLAMP_TO_EDGE);
				rtTextureSamplerSetFilteringModes(sampler, RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
				rtTextureSamplerSetIndexingMode(sampler, RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
				rtTextureSamplerSetReadMode(sampler, RT_TEXTURE_READ_NORMALIZED_FLOAT);
				rtTextureSamplerSetMaxAnisotropy(sampler, 1.0f);
				rtTextureSamplerSetBuffer(sampler, 0, 0, texBuffer);
				int tex_id;
				rtTextureSamplerGetId(sampler, &tex_id);
				texid.set1u(tex_id);

				triangles.addSTL("vertexBuffer", ahh, ahh.triangles.length);
				trianglesIndexed.addSTL("vertexBufferIndexed", "normalBuffer", "indexBuffer", ahh);
				instanceMetal.setTriangles(trianglesIndexed);
				instanceGlass.setTriangles(trianglesIndexed);
				instanceDiffuse.setTriangles(trianglesIndexed);
				instanceScatter.setTriangles(triangles);
				instanceMetal.setMaterial({ &metal });
				instanceGlass.setMaterial({ &glass });
				instanceDiffuse.setMaterial({ &diffuse });
				instanceScatter.setMaterial({ &scatter });
				geoGroupMetal.setInstance({ &instanceMetal });
				geoGroupGlass.setInstance({ &instanceGlass });
				geoGroupDiffuse.setInstance({ &instanceDiffuse });
				geoGroupScatter.setInstance({ &instanceScatter });

				transGlass.setMat({
					{1, 0, 0, 5},
					{0, 1, 0, 0},
					{0, 0, 1, 0},
					{0, 0, 0, 1}
					});
				transDiffuse.setMat({
					{1, 0, 0, 0},
					{0, 1, 0, 5},
					{0, 0, 1, 0},
					{0, 0, 0, 1}
					});
				transScatter.setMat({
					{1, 0, 0, 5},
					{0, 1, 0, 5},
					{0, 0, 1, 0},
					{0, 0, 0, 1}
					});
				transGlass.setChild(geoGroupGlass);
				transDiffuse.setChild(geoGroupDiffuse);
				transScatter.setChild(geoGroupScatter);

				group.setGeoGroup({ geoGroupMetal, transGlass,transDiffuse,transScatter });
				result.setObject(resultBuffer);
				metal.color.set3f(1.0f, 1.0f, 1.0f);
				glass.color.set3f(1.0f, 1.0f, 1.0f);
				diffuse.color.set3f(0.7, 0.7, 0.7);
				scatter.color.set3f(1, 1, 1);
				offset.set1f(1e-5f);
				depthMax.set1u(context.maxDepth - 1);
				russian.set1u(10);
				context.validate();
			}
			virtual void run()override
			{
				trans.operate();
				if (trans.updated)
				{
					frameNum = 0;
					trans.updated = false;
				}
				else ++frameNum;
				frame.set1u(frameNum);
				FrameScale size(renderer.size());
				context.launch(0, size.w, size.h);
				renderer.updated = true;
				renderer.use();
				renderer.run();
			}
			virtual void resize(FrameScale const& _size)override
			{
				trans.resize(_size.w, _size.h);
				resultBuffer.unreg();
				renderer.resize(_size);
				resultBuffer.setSize(_size.w, _size.h);
				resultBuffer.reg();
			}
			virtual void terminate()override
			{
				trianglesIndexed.destory();
				resultBuffer.destory();
				instanceGlass.destory();
				geoGroupGlass.destory();
				geoGroupMetal.destory();
				group.destory();
				context.destory();
			}
		};
	}
	struct RayTracing :OpenGL
	{
		SourceManager sm;
		OptiX::MonteCarlo monteCarlo;
		RayTracing(FrameScale const& _size)
			:
			sm(),
			monteCarlo(&sm, _size)
		{
		}
		virtual void init(FrameScale const& _size) override
		{
			monteCarlo.resize(_size);
		}
		virtual void run() override
		{
			monteCarlo.run();
		}
		void terminate()
		{
			monteCarlo.terminate();
		}
		virtual void frameSize(int _w, int _h)override
		{
			monteCarlo.resize({ _w,_h });
		}
		virtual void framePos(int, int) override {}
		virtual void frameFocus(int) override {}
		virtual void mouseButton(int _button, int _action, int _mods)override
		{
			switch (_button)
			{
				case GLFW_MOUSE_BUTTON_LEFT:monteCarlo.trans.mouse.refreshButton(0, _action); break;
				case GLFW_MOUSE_BUTTON_MIDDLE:monteCarlo.trans.mouse.refreshButton(1, _action); break;
				case GLFW_MOUSE_BUTTON_RIGHT:monteCarlo.trans.mouse.refreshButton(2, _action); break;
			}
		}
		virtual void mousePos(double _x, double _y)override
		{
			monteCarlo.trans.mouse.refreshPos(_x, _y);
		}
		virtual void mouseScroll(double _x, double _y)override
		{
			if (_y != 0.0)
				monteCarlo.trans.scroll.refresh(_y);
		}
		virtual void key(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) override
		{
			{
				switch (_key)
				{
					case GLFW_KEY_ESCAPE:
						if (_action == GLFW_PRESS)
							glfwSetWindowShouldClose(_window, true);
						break;
					case GLFW_KEY_A:monteCarlo.trans.key.refresh(0, _action); break;
					case GLFW_KEY_D:monteCarlo.trans.key.refresh(1, _action); break;
					case GLFW_KEY_W:monteCarlo.trans.key.refresh(2, _action); break;
					case GLFW_KEY_S:monteCarlo.trans.key.refresh(3, _action); break;
				}
			}
		}
	};
}


int main()
{
	OpenGL::OpenGLInit init(4, 5);
	Window::Window::Data winParameters
	{
		"Scatter",
		{
			{1920,1080},
			true,false
		}
	};
	Window::WindowManager wm(winParameters);
	OpenGL::RayTracing monteCarlo({ 1920,1080 });
	wm.init(0, &monteCarlo);
	//init.printRenderer();
	glfwSwapInterval(0);
	FPS fps;
	fps.refresh();
	while (!wm.close())
	{
		wm.pullEvents();
		wm.render();
		wm.swapBuffers();
		//fps.refresh();
		//fps.printFPS(1);
	}
	monteCarlo.terminate();
	return 0;
}

