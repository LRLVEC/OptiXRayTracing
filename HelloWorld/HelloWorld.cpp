#include <cstdio>
#include <cstdlib>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <GL/_Texture.h>
#include <GL/_OptiX.h>
#include <_Math.h>
#include <_Time.h>
#include <_Array.h>

namespace OpenGL
{
	namespace OptiX
	{

	}
	struct HelloWorld :OpenGL
	{
		SourceManager sm;
		OptiX::DefautRenderer renderer;
		RTcontext context;
		RTprogram ray_gen_program;
		RTbuffer  buffer;
		RTvariable result_buffer;
		RTvariable draw_color;

		HelloWorld(FrameScale const& _size)
			:
			sm(),
			renderer(&sm, _size),
			context(0)
		{
		}
		virtual void init(FrameScale const& _size) override
		{
			renderer.prepare();
			rtContextCreate(&context);
			rtContextSetRayTypeCount(context, 1);
			rtContextSetEntryPointCount(context, 1);
			rtBufferCreateFromGLBO(context, RT_BUFFER_OUTPUT, renderer, &buffer);
			//rtBufferCreate(context, RT_BUFFER_OUTPUT, &buffer);
			rtBufferSetFormat(buffer, RT_FORMAT_FLOAT4);
			rtBufferSetSize2D(buffer, _size.w, _size.h);
			rtContextDeclareVariable(context, "result_buffer", &result_buffer);
			rtVariableSetObject(result_buffer, buffer);
			File ahh("./ptx/");
			String<char>bhh(ahh.find("DrawColor.ptx").readText());
			rtProgramCreateFromPTXString(context, bhh, "draw_solid_color", &ray_gen_program);
			rtProgramDeclareVariable(ray_gen_program, "draw_color", &draw_color);
			rtVariableSet3f(draw_color, 0, 1, 0);
			rtContextSetRayGenerationProgram(context, 0, ray_gen_program);
			rtContextValidate(context);
			rtContextLaunch2D(context, 0 /* entry point */, _size.w, _size.h);
			renderer.updated = true;
			glViewport(0, 0, _size.w, _size.h);
		}
		virtual void run() override
		{
			renderer.use();
			renderer.run();
		}
		void terminate()
		{
			rtBufferDestroy(buffer);
			rtProgramDestroy(ray_gen_program);
			rtContextDestroy(context);
		}
		virtual void frameSize(int _w, int _h)override
		{
			renderer.resize(_w, _h);
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
			false,false
		}
	};
	Window::WindowManager wm(winParameters);
	OpenGL::HelloWorld test({ 400,400 });
	wm.init(0, &test);
	glfwSwapInterval(1);
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

