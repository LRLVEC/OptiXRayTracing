#include <cstdio>
#include <cstdlib>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <GL/_Texture.h>
#include <OptiX/_OptiX.h>
#include <OptiX/_Define.h>
#include <_Math.h>
#include <_Time.h>
#include <_Array.h>
#include <_STL.h>

namespace OpenGL
{
	namespace OptiX
	{
		struct Frame :RayTracer
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
			SimpleMaterial material;
			GeometryTriangles triangles;
			Variable<RTcontext> result;
			Variable<RTcontext> frame;
			Variable<RTcontext> backgroundColor;
			Variable<RTcontext> offset;
			Variable<RTcontext> groupGPU;
			GeometryInstance instance;
			GeometryGroup geoGroup;
			Acceleration geoGroupAccel;
			Acceleration groupAccel;
			Group group;
			STL ahh;
			unsigned int frameNum;
			Frame(::OpenGL::SourceManager* _sm, FrameScale const& _size)
				:
				renderer(_sm, _size),
				pm(&_sm->folder),
				context({ &rayAllocator }, 2),
				trans({ context, {60.0},{0.02,0.9,0.01},{0.3},{0,0,0},700.0 }),
				rayAllocator(context, pm, "rayAllocator"),
				miss(context, pm, "miss"),
				triangleAttrib(context, pm, "attrib"),
				resultBuffer(context, RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, renderer),
				material(context, pm),
				triangles(context, 1, RT_GEOMETRY_BUILD_FLAG_NONE,
					{
						{"vertexBuffer",RT_BUFFER_INPUT, RT_FORMAT_FLOAT3}
					}),
				result(context, "result"),
				frame(context, "frame"),
				backgroundColor(context, "background"),
				offset(context, "offset"),
				groupGPU(context, "group"),
				instance(context),
				geoGroup(context),
				group(context),
				geoGroupAccel(context, Acceleration::Trbvh),
				groupAccel(context, Acceleration::Trbvh),
				ahh(pm.folder->find("resources/Stanford_bunny_3.stl").readSTL()),
				frameNum(0)
			{
				renderer.prepare();
				//context.printAllDeviceInfo();
				context.init();
				trans.init(_size);
				rtContextSetMissProgram(context, CloseRay, miss);
				rtGeometryTrianglesSetAttributeProgram(triangles, triangleAttrib);
				resultBuffer.setSize(_size.w, _size.h);

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
				offset.set1f(1e-5f);
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
	struct Frame :OpenGL
	{
		SourceManager sm;
		OptiX::Frame frame;
		Frame(FrameScale const& _size)
			:
			sm(),
			frame(&sm, _size)
		{
		}
		virtual void init(FrameScale const& _size) override
		{
			frame.resize(_size);
		}
		virtual void run() override
		{
			frame.run();
		}
		void terminate()
		{
			frame.terminate();
		}
		virtual void frameSize(int _w, int _h)override
		{
			frame.resize({ _w,_h });
		}
		virtual void framePos(int, int) override {}
		virtual void frameFocus(int) override {}
		virtual void mouseButton(int _button, int _action, int _mods)override
		{
			switch (_button)
			{
				case GLFW_MOUSE_BUTTON_LEFT:frame.trans.mouse.refreshButton(0, _action); break;
				case GLFW_MOUSE_BUTTON_MIDDLE:frame.trans.mouse.refreshButton(1, _action); break;
				case GLFW_MOUSE_BUTTON_RIGHT:frame.trans.mouse.refreshButton(2, _action); break;
			}
		}
		virtual void mousePos(double _x, double _y)override
		{
			frame.trans.mouse.refreshPos(_x, _y);
		}
		virtual void mouseScroll(double _x, double _y)override
		{
			if (_y != 0.0)
				frame.trans.scroll.refresh(_y);
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
					case GLFW_KEY_A:frame.trans.key.refresh(0, _action); break;
					case GLFW_KEY_D:frame.trans.key.refresh(1, _action); break;
					case GLFW_KEY_W:frame.trans.key.refresh(2, _action); break;
					case GLFW_KEY_S:frame.trans.key.refresh(3, _action); break;
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
	OpenGL::Frame test({ 1920,1080 });
	wm.init(0, &test);
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
	test.terminate();
	return 0;
}

