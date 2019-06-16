#include <cstdio>
#include <cstdlib>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <GL/_Texture.h>
#include <GL/_OptiX.h>
#include <_Math.h>
#include <_Time.h>
#include <_Array.h>
#include "Define.h"

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
			struct FrameData :VariableBase::Data
			{
				unsigned int frame;
				virtual void* pointer()override
				{
					return &frame;
				}
				virtual unsigned long long size()override
				{
					return sizeof(frame);
				}
			};
			struct ColorData :VariableBase::Data
			{
				Define::Color color;
				virtual void* pointer()override
				{
					return &color;
				}
				virtual unsigned long long size()override
				{
					return sizeof(color);
				}
			};
			struct TransformData :VariableBase::Data
			{
				Define::Eye eye;
				virtual void* pointer()override
				{
					return&eye;
				}
				virtual unsigned long long size()override
				{
					return sizeof(eye);
				}
			};

			DefautRenderer renderer;
			PTXManager pm;
			Context context;
			Program rayAllocator;
			Program miss;
			Buffer resultBuffer;
			Buffer vertexBuffer;
			SimpleMaterial cc;
			FrameData frameData;
			ColorData colorData;
			TransformData transformData;
			GeometryTriangles triangles;
			Variable<RTcontext> result;
			Variable<FrameData> frame;
			Variable<ColorData> backgroundColor;
			Variable<RTcontext> offset;
			Variable<TransformData>transform;
			Variable<RTcontext> groupGPU;
			Variable<RTgeometrytriangles> triangleVertices;
			GeometryInstance instance;
			GeometryGroup geoGroup;
			Acceleration geoGroupAccel;
			Acceleration groupAccel;
			Group group;
			Frame(::OpenGL::SourceManager* _sm, FrameScale const& _size)
				:
				renderer(_sm, _size),
				pm(&_sm->folder),
				context({ &rayAllocator }, 2),
				rayAllocator(context, pm, "rayAllocator"),
				miss(context, pm, "miss"),
				resultBuffer(context, RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, renderer),
				vertexBuffer(context, RT_BUFFER_INPUT, RT_FORMAT_FLOAT3),
				cc(context, pm),
				triangles(context, 2, 1, RT_GEOMETRY_BUILD_FLAG_NONE),
				result(context, "result"),
				frame(context, "frame", &frameData),
				backgroundColor(context, "background", &colorData),
				offset(context, "offset"),
				transform(context, "eye", &transformData),
				groupGPU(context, "group"),
				triangleVertices(triangles, "vertices"),
				instance(context),
				geoGroup(context),
				group(context),
				geoGroupAccel(context, Acceleration::Trbvh),
				groupAccel(context, Acceleration::Trbvh)
			{
				renderer.prepare();
				//context.printAllDeviceInfo();
				context.init();
				rtContextSetMissProgram(context, CloseRay, miss);
				resultBuffer.setSize(_size.w, _size.h);

				vertexBuffer.setSize(6);
				Math::vec3<float>* v = (Math::vec3<float>*)vertexBuffer.map();
				v[0] = { 0,0,-1 };
				v[1] = { 1,0,-1 };
				v[2] = { 0,1,-1 };
				v[3] = { 0,0,-0.5 };
				v[4] = { -1,0,-0.5 };
				v[5] = { 0,-1,-0.5 };
				vertexBuffer.unmap();

				triangles.setVertices(&vertexBuffer, 6, 0, 12);
				instance.setTriangles(triangles);
				instance.setMaterial({ &cc });
				geoGroup.setInstance({ &instance });
				geoGroup.setAccel(geoGroupAccel);
				group.setAccel(groupAccel);
				group.setGeoGroup({ &geoGroup });
				result.setObject(resultBuffer);
				groupGPU.setObject(group);
				triangleVertices.setObject(vertexBuffer);
				frameData.frame = 0;
				frame.set();
				transformData.eye =
				{
					{0,0,0},
					-200
				};
				transform.set();
				colorData.color = { 0,1,1 };
				backgroundColor.set();
				cc.color.set3f(1.0f, 0.0f, 0.0f);
				offset.set1f(1e-5);
				context.validate();
			}
			virtual void run()override
			{
				FrameScale size(renderer.size());
				context.launch(0, size.w, size.h);
				renderer.updated = true;
				renderer.use();
				renderer.run();
			}
			virtual void resize(FrameScale const& _size)override
			{
				resultBuffer.unreg();
				renderer.resize(_size);
				resultBuffer.setSize(_size.w, _size.h);
				resultBuffer.reg();
			}
			virtual void terminate()override
			{
				triangles.destory();
				resultBuffer.destory();
				vertexBuffer.destory();
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
			/*switch (_button)
			{
				case GLFW_MOUSE_BUTTON_LEFT:renderer.trans.mouse.refreshButton(0, _action); break;
				case GLFW_MOUSE_BUTTON_MIDDLE:renderer.trans.mouse.refreshButton(1, _action); break;
				case GLFW_MOUSE_BUTTON_RIGHT:renderer.trans.mouse.refreshButton(2, _action); break;
			}*/
		}
		virtual void mousePos(double _x, double _y)override
		{
			//renderer.trans.mouse.refreshPos(_x, _y);
		}
		virtual void mouseScroll(double _x, double _y)override
		{
			//if (_y != 0.0)
				//renderer.trans.scroll.refresh(_y);
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
						//case GLFW_KEY_A:renderer.trans.key.refresh(0, _action); break;
						//case GLFW_KEY_D:renderer.trans.key.refresh(1, _action); break;
						//case GLFW_KEY_W:renderer.trans.key.refresh(2, _action); break;
						//case GLFW_KEY_S:renderer.trans.key.refresh(3, _action); break;
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
			{400,400},
			true,false
		}
	};
	Window::WindowManager wm(winParameters);
	OpenGL::Frame test({ 400,400 });
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

