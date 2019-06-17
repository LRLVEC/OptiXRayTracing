#include <cstdio>
#include <cstdlib>
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
		struct Whitted :RayTracer
		{
			struct SimpleMaterial :Material
			{
				Variable<RTmaterial>color;
				Program closeHitProgram;
				Program anyHitProgram;
				CloseHit closeHit;
				AnyHit anyHit;
				SimpleMaterial(RTcontext* _context, PTXManager& _pm)
					:
					Material(_context),
					color(&material, "materialColor"),
					closeHitProgram(_context, _pm, "closeHit"),
					anyHitProgram(_context, _pm, "anyHit"),
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
			Transform trans;
			Program rayAllocator;
			Program miss;
			Program triangleAttrib;
			Buffer resultBuffer;
			Buffer texBuffer;
			SimpleMaterial material;
			GeometryTriangles triangles;
			//BMPCubeData cubeData;
			//TextureCube cube;
			RTtexturesampler sampler;
			Variable<RTcontext> result;
			Variable<RTcontext> texTest;
			Variable<RTcontext> frame;
			Variable<RTcontext> texid;
			Variable<RTcontext> backgroundColor;
			Variable<RTcontext> offset;
			Variable<RTcontext> groupGPU;
			GeometryInstance instance;
			GeometryGroup geoGroup;
			Acceleration geoGroupAccel;
			Acceleration groupAccel;
			Group group;
			STL ahh;
			BMP testBMP;
			BMPCube testCube;
			unsigned int frameNum;
			Whitted(::OpenGL::SourceManager* _sm, FrameScale const& _size)
				:
				renderer(_sm, _size),
				pm(&_sm->folder),
				context({ &rayAllocator }, 2),
				trans({ context, {60.0},{0.02,0.99,0.01},{0.3},{0,0,0},700.0 }),
				rayAllocator(context, pm, "rayAllocator"),
				miss(context, pm, "miss"),
				triangleAttrib(context, pm, "attrib"),
				resultBuffer(context, RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, renderer),
				texBuffer(context, RT_BUFFER_INPUT | RT_BUFFER_CUBEMAP, RT_FORMAT_UNSIGNED_BYTE4),
				material(context, pm),
				triangles(context, 1, RT_GEOMETRY_BUILD_FLAG_NONE,
					{
						{"vertexBuffer",RT_BUFFER_INPUT, RT_FORMAT_FLOAT3}
					}),
				//cubeData("resources/room/"),
				//cube(&cubeData, 2, RGBA32f, 1, cubeData.bmp[0].header.width, cubeData.bmp[0].header.height),
				result(context, "result"),
				texTest(context, "ahh"),
				frame(context, "frame"),
				texid(context, "texid"),
				backgroundColor(context, "background"),
				offset(context, "offset"),
				groupGPU(context, "group"),
				instance(context),
				geoGroup(context),
				group(context),
				geoGroupAccel(context, Acceleration::Trbvh),
				groupAccel(context, Acceleration::Trbvh),
				ahh(pm.folder->find("resources/Stanford_bunny_3.stl").readSTL()),
				testBMP("resources/lightSource.bmp"),
				testCube("resources/room/"),
				frameNum(0)
			{
				renderer.prepare();
				//context.printAllDeviceInfo();
				context.init();
				trans.init(_size);
				rtContextSetMissProgram(context, CloseRay, miss);
				rtGeometryTrianglesSetAttributeProgram(triangles, triangleAttrib);
				resultBuffer.setSize(_size.w, _size.h);

				//cube.dataInit(0, TextureInputBGRInt, TextureInputUByte);
				//rtTextureSamplerCreateFromGLImage(context, cube.texture, RT_TARGET_GL_TEXTURE_CUBE_MAP, &cubeSampler);
				//rtTextureSamplerSetFilteringModes(cubeSampler, RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
				//RTtexturesampler sampler;
				testBMP.printInfo();
				testCube.bmp[0].printInfo();
				testCube.bmp[1].printInfo();
				testCube.bmp[2].printInfo();
				testCube.bmp[3].printInfo();
				testCube.bmp[4].printInfo();
				testCube.bmp[5].printInfo();
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
				//texTest.setObject((RTobject*)& sampler);

				triangles.addSTL("vertexBuffer", ahh, ahh.triangles.length);
				instance.setTriangles(triangles);
				instance.setMaterial({ &material });
				geoGroup.setInstance({ &instance });
				geoGroup.setAccel(geoGroupAccel);
				group.setAccel(groupAccel);
				group.setGeoGroup({ &geoGroup });
				result.setObject(resultBuffer);
				groupGPU.setObject(group);
				backgroundColor.set3f(0.0f, 0.0f, 0.0f);
				material.color.set3f(1.0f, 1.0f, 1.0f);
				offset.set1f(1e-4f);
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
				triangles.destory();
				resultBuffer.destory();
				instance.destory();
				geoGroup.destory();
				group.destory();
				geoGroupAccel.destory();
				groupAccel.destory();
				rayAllocator.destory();
				context.destory();
			}
		};
	}
	struct Whitted :OpenGL
	{
		SourceManager sm;
		OptiX::Whitted whitted;
		Whitted(FrameScale const& _size)
			:
			sm(),
			whitted(&sm, _size)
		{
		}
		virtual void init(FrameScale const& _size) override
		{
			whitted.resize(_size);
		}
		virtual void run() override
		{
			whitted.run();
		}
		void terminate()
		{
			whitted.terminate();
		}
		virtual void frameSize(int _w, int _h)override
		{
			whitted.resize({ _w,_h });
		}
		virtual void framePos(int, int) override {}
		virtual void frameFocus(int) override {}
		virtual void mouseButton(int _button, int _action, int _mods)override
		{
			switch (_button)
			{
				case GLFW_MOUSE_BUTTON_LEFT:whitted.trans.mouse.refreshButton(0, _action); break;
				case GLFW_MOUSE_BUTTON_MIDDLE:whitted.trans.mouse.refreshButton(1, _action); break;
				case GLFW_MOUSE_BUTTON_RIGHT:whitted.trans.mouse.refreshButton(2, _action); break;
			}
		}
		virtual void mousePos(double _x, double _y)override
		{
			whitted.trans.mouse.refreshPos(_x, _y);
		}
		virtual void mouseScroll(double _x, double _y)override
		{
			if (_y != 0.0)
				whitted.trans.scroll.refresh(_y);
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
					case GLFW_KEY_A:whitted.trans.key.refresh(0, _action); break;
					case GLFW_KEY_D:whitted.trans.key.refresh(1, _action); break;
					case GLFW_KEY_W:whitted.trans.key.refresh(2, _action); break;
					case GLFW_KEY_S:whitted.trans.key.refresh(3, _action); break;
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
		"Frame",
		{
			{1920,1080},
			true,false
		}
	};
	Window::WindowManager wm(winParameters);
	OpenGL::Whitted whitted({ 1920,1080 });
	wm.init(0, &whitted);
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
	whitted.terminate();
	return 0;
}

