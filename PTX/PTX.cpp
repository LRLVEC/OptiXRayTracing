#include <optix.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sutil.h>
#include <_File.h>

int main(int argc, char* argv[])
{
	RTcontext context = 0;
	/* Primary RTAPI objects */
	RTprogram ray_gen_program;
	RTbuffer  buffer;
	/* Parameters */
	RTvariable result_buffer;
	RTvariable draw_color;
	int width = 512u;
	int height = 384u;
	sutil::initGlut(&argc, argv);
	/* Create our objects and set state */
	rtContextCreate(&context);
	rtContextSetRayTypeCount(context, 1);
	rtContextSetEntryPointCount(context, 1);

	

	rtBufferCreate(context, RT_BUFFER_OUTPUT, &buffer);
	rtBufferSetFormat(buffer, RT_FORMAT_FLOAT4);
	rtBufferSetSize2D(buffer, width, height);
	rtContextDeclareVariable(context, "result_buffer", &result_buffer);
	rtVariableSetObject(result_buffer, buffer);

	File ahh("./");
	String<char>bhh(ahh.find("DrawColor.cu.ptx").readText());

	//const char* ptx = sutil::getPtxString("optixHello", "draw_color.cu");//Fuck sutil
	rtProgramCreateFromPTXString(context, bhh, "draw_solid_color", &ray_gen_program);
	rtProgramDeclareVariable(ray_gen_program, "draw_color", &draw_color);
	rtVariableSet3f(draw_color, 0, 1, 0);
	rtContextSetRayGenerationProgram(context, 0, ray_gen_program);
	/* Run */
	rtContextValidate(context);
	rtContextLaunch2D(context, 0 /* entry point */, width, height);
	/* Display image */
	sutil::displayBufferGlut(argv[0], buffer);
	/* Clean up */
	rtBufferDestroy(buffer);
	rtProgramDestroy(ray_gen_program);
	rtContextDestroy(context);
	return(0);
}
