#include <cstdio>
#include <cstdlib>
#include <GL/_OpenGL.h>
#include <GL/_Window.h>
#include <GL/_Texture.h>
#include <optixu/optixpp_namespace.h>
#include <optix.h>
#include <_Math.h>
#include <_Time.h>
#include <_Array.h>
#include <_Pair.h>

namespace OpenGL
{
	struct PBOTest :OpenGL
	{
		struct Renderer :Program
		{
			struct TriangleData :Buffer::Data
			{
				using Vertex = Math::vec2<float>;
				using Triangle = Array<Vertex, 3>;
				Array<Triangle, 2> triangles;
				TriangleData()
					:
					Data(StaticDraw),
					triangles({ {{-1,-1},{1,-1},{1,1}},{{1,1},{-1,1},{-1,-1}} })
				{
				}
				virtual void* pointer()override
				{
					return (void*)triangles.data;
				}
				virtual unsigned int size()override
				{
					return sizeof(Triangle)* triangles.length;
				}
			};

			TriangleData triangles;
			Buffer buffer;
			BufferConfig bufferArray;
			VertexAttrib positions;

			Renderer(SourceManager* _sourceManage)
				:
				Program(_sourceManage, "Triangle", Vector<VertexAttrib*>{&positions}),
				triangles(),
				buffer(&triangles),
				bufferArray(&buffer, ArrayBuffer),
				positions(&bufferArray, 0, VertexAttrib::two,
					VertexAttrib::Float, false, sizeof(TriangleData::Vertex), 0, 0)
			{
				init();
			}
			virtual void initBufferData()override
			{
			}
			virtual void run() override
			{
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			void resize(int _w, int _h)
			{
				glViewport(0, 0, _w, _h);
			}
		};

		SourceManager sm;
		Renderer renderer;
		RTcontext context;
		RTprogram ray_gen_program;
		GLuint pbo;
		Texture tex;
		TextureConfig<TextureStorage2D>texConfig;
		RTbuffer  buffer;
		RTvariable result_buffer;
		RTvariable draw_color;

		PBOTest()
			:
			sm(),
			renderer(&sm),
			context(0),
			tex(nullptr, 0),
			texConfig(&tex, Texture2D, RGBA32f, 1, 100, 100)
		{
		}
		virtual void init(FrameScale const& _size) override
		{
			glDeleteTextures(1, &tex.texture);
			texConfig.width = _size.w;
			texConfig.height = _size.h;
			tex.create();
			texConfig.bind();
			texConfig.allocData();
			renderer.use();
			tex.bindUnit();

			
			rtContextCreate(&context);
			rtContextSetRayTypeCount(context, 1);
			rtContextSetEntryPointCount(context, 1);
			glGenBuffers(1, &pbo);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, _size.w * _size.h * 16, nullptr, GL_DYNAMIC_DRAW);
			rtBufferCreateFromGLBO(context, RT_BUFFER_OUTPUT, pbo, &buffer);
			//rtBufferCreate(context, RT_BUFFER_OUTPUT, &buffer);
			rtBufferSetFormat(buffer, RT_FORMAT_FLOAT4);
			rtBufferSetSize2D(buffer, _size.w, _size.h);
			rtContextDeclareVariable(context, "result_buffer", &result_buffer);
			rtVariableSetObject(result_buffer, buffer);
			File ahh("./");
			String<char>bhh(ahh.find("DrawColor.cu.ptx").readText());
			rtProgramCreateFromPTXString(context, bhh, "draw_solid_color", &ray_gen_program);
			rtProgramDeclareVariable(ray_gen_program, "draw_color", &draw_color);
			rtVariableSet3f(draw_color, 0, 1, 0);
			rtContextSetRayGenerationProgram(context, 0, ray_gen_program);
			rtContextValidate(context);
			rtContextLaunch2D(context, 0 /* entry point */, _size.w, _size.h);

			texConfig.dataInit(0, TextureInputRGBA, TextureInputFloat);



			rtBufferDestroy(buffer);
			rtProgramDestroy(ray_gen_program);
			rtContextDestroy(context);

			glViewport(0, 0, _size.w, _size.h);/*
			renderer.trans.init(_size);
			renderer.transformUnifrom.dataInit();*/
			renderer.bufferArray.dataInit();
		}
		virtual void run() override;
		virtual void frameSize(int, int) override;
		virtual void framePos(int, int) override;
		virtual void frameFocus(int) override;
		virtual void mouseButton(int, int, int) override;
		virtual void mousePos(double, double) override;
		virtual void mouseScroll(double, double) override;
		virtual void key(GLFWwindow*, int, int, int, int) override;
	};

	inline void PBOTest::run()
	{
		/*
		glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);*/
		renderer.use();
		//renderer.refreshBuffer();
		renderer.run();
	}
	inline void PBOTest::frameSize(int _w, int _h)
	{
		renderer.resize(_w, _h);
	}
	inline void PBOTest::framePos(int, int)
	{
	}
	inline void PBOTest::frameFocus(int)
	{
	}
	inline void PBOTest::mouseButton(int _button, int _action, int _mods)
	{
		/*switch (_button)
		{
			case GLFW_MOUSE_BUTTON_LEFT:renderer.trans.mouse.refreshButton(0, _action); break;
			case GLFW_MOUSE_BUTTON_MIDDLE:renderer.trans.mouse.refreshButton(1, _action); break;
			case GLFW_MOUSE_BUTTON_RIGHT:renderer.trans.mouse.refreshButton(2, _action); break;
		}*/
	}
	inline void PBOTest::mousePos(double _x, double _y)
	{
		//renderer.trans.mouse.refreshPos(_x, _y);
	}
	inline void PBOTest::mouseScroll(double _x, double _y)
	{
		//if (_y != 0.0)
			//renderer.trans.scroll.refresh(_y);
	}
	inline void PBOTest::key(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
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
	OpenGL::PBOTest test;
	init.printRenderer();
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
	return 0;
}

