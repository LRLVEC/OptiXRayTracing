#include <cstdio>
#include <cstdlib>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <GL/_Texture.h>
#include <OptiX/_OptiX.h>
#include <_Math.h>
#include <_Time.h>
#include <_Array.h>

namespace OpenGL
{
	namespace OptiX
	{
		struct HelloWorld :RayTracer
		{
			DefautRenderer renderer;
			PTXManager pm;
			Context context;
			Program drawColor;
			Buffer buffer;
			Variable<RTcontext> result;
			Variable<RTprogram> color;
			float t;
			HelloWorld(::OpenGL::SourceManager* _sm, FrameScale const& _size)
				:
				renderer(_sm, _size),
				pm(&_sm->folder),
				context({ &drawColor }, 1),
				drawColor(context, pm, "drawColor"),
				buffer(context, RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, renderer),
				result(context, "result"),
				color(drawColor, "color"),
				t(0)
			{
				renderer.prepare();
				context.init();
				buffer.setSize(_size.w, _size.h);
				result.setObject(buffer);
				color.set3f(0, 1, 1);
				context.validate();
			}
			virtual void run()override
			{
				if (t > 2 * Math::Pi)t -= 2 * Math::Pi;
				color.set3f((1 - sinf(t += 0.01)) / 2, (1 + sinf(t += 0.01)) / 2, 1);
				FrameScale size(renderer.size());
				context.launch(0, size.w, size.h);
				renderer.updated = true;
				renderer.use();
				renderer.run();
			}
			virtual void resize(FrameScale const& _size)override
			{
				buffer.unreg();
				renderer.resize(_size);
				buffer.reg();
				buffer.setSize(_size.w, _size.w);
			}
			virtual void terminate()override
			{
				buffer.destory();
				drawColor.destory();
				context.destory();
			}
		};
	}
	struct HelloWorld :OpenGL
	{
		SourceManager sm;
		OptiX::HelloWorld hw;
		HelloWorld(FrameScale const& _size)
			:
			sm(),
			hw(&sm, _size)
		{
		}
		virtual void init(FrameScale const& _size) override
		{
			hw.resize(_size);
		}
		virtual void run() override
		{
			hw.run();
		}
		void terminate()
		{
			hw.terminate();
		}
		virtual void frameSize(int _w, int _h)override
		{
			hw.resize({ _w,_h });
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
		"PBOTest",
		{
			{400,400},
			true,false
		}
	};
	Window::WindowManager wm(winParameters);
	OpenGL::HelloWorld test({ 400,400 });
	wm.init(0, &test);
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

