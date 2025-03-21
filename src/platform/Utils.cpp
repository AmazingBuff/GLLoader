//
// Created by Hash Liu on 2025/3/5.
//

#include <GLFunctions.h>
#include <GLExtFunctions.h>

#include "Utils.h"

#ifdef _WIN32
#include <windows.h>
static HMODULE s_gl_lib;
#else
#include <dlfcn.h>
static void* s_gl_lib;
#endif

using PFNGETPROCADDRESSPROC = void* (APIENTRYP)(const char*);
static PFNGETPROCADDRESSPROC s_get_proc_address;


static void* get_proc(const char* namez)
{
	void* result = nullptr;
	if (s_gl_lib == nullptr)
		return nullptr;

	if (s_get_proc_address != nullptr)
		result = s_get_proc_address(namez);
	if (result == nullptr)
	{
#if defined(_WIN32) || defined(__CYGWIN__)
		result = (void*)GetProcAddress((HMODULE)s_gl_lib, namez);
#else
		result = dlsym(s_gl_lib, namez);
#endif
	}

	return result;
}

static bool find_version(PFNGLGETSTRINGPROC getString, int* major, int* minor)
{
	const char* prefixes[] = {
		"OpenGL ES-CM ",
		"OpenGL ES-CL ",
		"OpenGL ES ",
		nullptr
	};

	const char* version = (const char*)getString(GL_VERSION);
	if (!version)
		return false;

	for (int i = 0; prefixes[i]; i++)
	{
		const size_t length = strlen(prefixes[i]);
		if (strncmp(version, prefixes[i], length) == 0)
		{
			version += length;
			break;
		}
	}

	/* PR #18 */
#ifdef _MSC_VER
	sscanf_s(version, "%d.%d", major, minor);
#else
	sscanf(version, "%d.%d", major, minor);
#endif
	return true;
}

static bool open_gl(int* major, int* minor)
{
#if defined(_WIN32)
	s_gl_lib = LoadLibraryA("opengl32.dll");
#else
#ifdef __APPLE__
	static const char* Names[] = {"libGL.dylib", "libGL.so"};
#else
	static const char* Names[] = {"libGL.so.1", "libGL.so"};
#endif
	unsigned int index = 0;
	for(index = 0; index < (sizeof(Names) / sizeof(Names[0])); index++)
	{
		s_gl_lib = dlopen(Names[index], RTLD_NOW | RTLD_GLOBAL);

		if(s_gl_lib != nullptr)
			break;
	}
#endif
	if (s_gl_lib != nullptr)
	{
#if defined(_WIN32)
		s_get_proc_address = (PFNGETPROCADDRESSPROC)GetProcAddress(s_gl_lib, "wglGetProcAddress");
#elif defined(_UNIX)
		s_get_proc_address = (PFNGETPROCADDRESSPROC)dlsym(s_gl_lib, "glXGetProcAddressARB");
#endif
		PFNGLGETSTRINGPROC getString = (PFNGLGETSTRINGPROC)get_proc("glGetString");
		if (getString && getString(GL_VERSION))
			return find_version(getString, major, minor);
	}

	return false;
}

static void close_gl()
{
	if (s_gl_lib != nullptr) 
	{
#ifdef _WIN32
		FreeLibrary(s_gl_lib);
#else
		dlclose(s_gl_lib);
#endif
		s_gl_lib = nullptr;
	}
}


static bool open_gl_es(int* major, int* minor)
{
#if defined(_WIN32)
	s_gl_lib = LoadLibraryA("libEGL.dll");
#else
#if defined(__APPLE__)
	static const char* Names[] = {"libEGL.dylib", "libEGL.so"};
#elif defined(__linux__)
	static const char* Names[] = {"libEGL.so.1", "libEGL.so"};
#endif
    unsigned int index = 0;
	for(index = 0; index < (sizeof(Names) / sizeof(Names[0])); index++)
	{
		s_gl_lib = dlopen(Names[index], RTLD_NOW | RTLD_GLOBAL);

		if(s_gl_lib != nullptr)
			break;
	}
#endif
	if (s_gl_lib != nullptr)
	{
#if defined(_WIN32)
		s_get_proc_address = (PFNGETPROCADDRESSPROC)GetProcAddress(s_gl_lib, "eglGetProcAddress");
#else
		s_get_proc_address = (PFNGETPROCADDRESSPROC)dlsym(s_gl_lib, "eglGetProcAddress");
#endif
		PFNGLGETSTRINGPROC getString = (PFNGLGETSTRINGPROC)get_proc("glGetString");
		if (getString && getString(GL_VERSION))
			return find_version(getString, major, minor);
	}

	return false;
}

static bool has_ext(const std::vector<const char*>& exts, const char* ext)
{
	return std::any_of(exts.begin(), exts.end(),
							   [&](const char* str)
							   {
								   if (str == ext)
								   	return true;
								   return false;
							   });
}


namespace GL
{
    // fork from glad
	using LoadProc = void*(*)(const char*);

	static void load_GL_ES_2_0(LoadProc load, GLFunctions* func)
	{
		func->glActiveTexture = (PFNGLACTIVETEXTUREPROC)load("glActiveTexture");
		func->glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
		func->glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)load("glBindAttribLocation");
		func->glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
		func->glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)load("glBindFramebuffer");
		func->glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)load("glBindRenderbuffer");
		func->glBindTexture = (PFNGLBINDTEXTUREPROC)load("glBindTexture");
		func->glBlendColor = (PFNGLBLENDCOLORPROC)load("glBlendColor");
		func->glBlendEquation = (PFNGLBLENDEQUATIONPROC)load("glBlendEquation");
		func->glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)load("glBlendEquationSeparate");
		func->glBlendFunc = (PFNGLBLENDFUNCPROC)load("glBlendFunc");
		func->glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)load("glBlendFuncSeparate");
		func->glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
		func->glBufferSubData = (PFNGLBUFFERSUBDATAPROC)load("glBufferSubData");
		func->glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)load("glCheckFramebufferStatus");
		func->glClear = (PFNGLCLEARPROC)load("glClear");
		func->glClearColor = (PFNGLCLEARCOLORPROC)load("glClearColor");
		func->glClearDepthf = (PFNGLCLEARDEPTHFPROC)load("glClearDepthf");
		func->glClearStencil = (PFNGLCLEARSTENCILPROC)load("glClearStencil");
		func->glColorMask = (PFNGLCOLORMASKPROC)load("glColorMask");
		func->glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
		func->glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)load("glCompressedTexImage2D");
		func->glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)load("glCompressedTexSubImage2D");
		func->glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC)load("glCopyTexImage2D");
		func->glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC)load("glCopyTexSubImage2D");
		func->glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
		func->glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
		func->glCullFace = (PFNGLCULLFACEPROC)load("glCullFace");
		func->glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)load("glDeleteBuffers");
		func->glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)load("glDeleteFramebuffers");
		func->glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load("glDeleteProgram");
		func->glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)load("glDeleteRenderbuffers");
		func->glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
		func->glDeleteTextures = (PFNGLDELETETEXTURESPROC)load("glDeleteTextures");
		func->glDepthFunc = (PFNGLDEPTHFUNCPROC)load("glDepthFunc");
		func->glDepthMask = (PFNGLDEPTHMASKPROC)load("glDepthMask");
		func->glDepthRangef = (PFNGLDEPTHRANGEFPROC)load("glDepthRangef");
		func->glDetachShader = (PFNGLDETACHSHADERPROC)load("glDetachShader");
		func->glDisable = (PFNGLDISABLEPROC)load("glDisable");
		func->glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)load("glDisableVertexAttribArray");
		func->glDrawArrays = (PFNGLDRAWARRAYSPROC)load("glDrawArrays");
		func->glDrawElements = (PFNGLDRAWELEMENTSPROC)load("glDrawElements");
		func->glEnable = (PFNGLENABLEPROC)load("glEnable");
		func->glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
		func->glFinish = (PFNGLFINISHPROC)load("glFinish");
		func->glFlush = (PFNGLFLUSHPROC)load("glFlush");
		func->glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)load("glFramebufferRenderbuffer");
		func->glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)load("glFramebufferTexture2D");
		func->glFrontFace = (PFNGLFRONTFACEPROC)load("glFrontFace");
		func->glGenBuffers = (PFNGLGENBUFFERSPROC)load("glGenBuffers");
		func->glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)load("glGenerateMipmap");
		func->glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)load("glGenFramebuffers");
		func->glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)load("glGenRenderbuffers");
		func->glGenTextures = (PFNGLGENTEXTURESPROC)load("glGenTextures");
		func->glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)load("glGetActiveAttrib");
		func->glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)load("glGetActiveUniform");
		func->glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)load("glGetAttachedShaders");
		func->glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)load("glGetAttribLocation");
		func->glGetBooleanv = (PFNGLGETBOOLEANVPROC)load("glGetBooleanv");
		func->glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)load("glGetBufferParameteriv");
		func->glGetError = (PFNGLGETERRORPROC)load("glGetError");
		func->glGetFloatv = (PFNGLGETFLOATVPROC)load("glGetFloatv");
		func->glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)load("glGetFramebufferAttachmentParameteriv");
		func->glGetIntegerv = (PFNGLGETINTEGERVPROC)load("glGetIntegerv");
		func->glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load("glGetProgramiv");
		func->glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)load("glGetProgramInfoLog");
		func->glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)load("glGetRenderbufferParameteriv");
		func->glGetShaderiv = (PFNGLGETSHADERIVPROC)load("glGetShaderiv");
		func->glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)load("glGetShaderInfoLog");
		func->glGetShaderPrecisionFormat = (PFNGLGETSHADERPRECISIONFORMATPROC)load("glGetShaderPrecisionFormat");
		func->glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)load("glGetShaderSource");
		func->glGetString = (PFNGLGETSTRINGPROC)load("glGetString");
		func->glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC)load("glGetTexParameterfv");
		func->glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC)load("glGetTexParameteriv");
		func->glGetUniformfv = (PFNGLGETUNIFORMFVPROC)load("glGetUniformfv");
		func->glGetUniformiv = (PFNGLGETUNIFORMIVPROC)load("glGetUniformiv");
		func->glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");
		func->glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)load("glGetVertexAttribfv");
		func->glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)load("glGetVertexAttribiv");
		func->glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)load("glGetVertexAttribPointerv");
		func->glHint = (PFNGLHINTPROC)load("glHint");
		func->glIsBuffer = (PFNGLISBUFFERPROC)load("glIsBuffer");
		func->glIsEnabled = (PFNGLISENABLEDPROC)load("glIsEnabled");
		func->glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)load("glIsFramebuffer");
		func->glIsProgram = (PFNGLISPROGRAMPROC)load("glIsProgram");
		func->glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)load("glIsRenderbuffer");
		func->glIsShader = (PFNGLISSHADERPROC)load("glIsShader");
		func->glIsTexture = (PFNGLISTEXTUREPROC)load("glIsTexture");
		func->glLineWidth = (PFNGLLINEWIDTHPROC)load("glLineWidth");
		func->glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
		func->glPixelStorei = (PFNGLPIXELSTOREIPROC)load("glPixelStorei");
		func->glPolygonOffset = (PFNGLPOLYGONOFFSETPROC)load("glPolygonOffset");
		func->glReadPixels = (PFNGLREADPIXELSPROC)load("glReadPixels");
		func->glReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC)load("glReleaseShaderCompiler");
		func->glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)load("glRenderbufferStorage");
		func->glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)load("glSampleCoverage");
		func->glScissor = (PFNGLSCISSORPROC)load("glScissor");
		func->glShaderBinary = (PFNGLSHADERBINARYPROC)load("glShaderBinary");
		func->glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
		func->glStencilFunc = (PFNGLSTENCILFUNCPROC)load("glStencilFunc");
		func->glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)load("glStencilFuncSeparate");
		func->glStencilMask = (PFNGLSTENCILMASKPROC)load("glStencilMask");
		func->glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)load("glStencilMaskSeparate");
		func->glStencilOp = (PFNGLSTENCILOPPROC)load("glStencilOp");
		func->glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)load("glStencilOpSeparate");
		func->glTexImage2D = (PFNGLTEXIMAGE2DPROC)load("glTexImage2D");
		func->glTexParameterf = (PFNGLTEXPARAMETERFPROC)load("glTexParameterf");
		func->glTexParameterfv = (PFNGLTEXPARAMETERFVPROC)load("glTexParameterfv");
		func->glTexParameteri = (PFNGLTEXPARAMETERIPROC)load("glTexParameteri");
		func->glTexParameteriv = (PFNGLTEXPARAMETERIVPROC)load("glTexParameteriv");
		func->glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)load("glTexSubImage2D");
		func->glUniform1f = (PFNGLUNIFORM1FPROC)load("glUniform1f");
		func->glUniform1fv = (PFNGLUNIFORM1FVPROC)load("glUniform1fv");
		func->glUniform1i = (PFNGLUNIFORM1IPROC)load("glUniform1i");
		func->glUniform1iv = (PFNGLUNIFORM1IVPROC)load("glUniform1iv");
		func->glUniform2f = (PFNGLUNIFORM2FPROC)load("glUniform2f");
		func->glUniform2fv = (PFNGLUNIFORM2FVPROC)load("glUniform2fv");
		func->glUniform2i = (PFNGLUNIFORM2IPROC)load("glUniform2i");
		func->glUniform2iv = (PFNGLUNIFORM2IVPROC)load("glUniform2iv");
		func->glUniform3f = (PFNGLUNIFORM3FPROC)load("glUniform3f");
		func->glUniform3fv = (PFNGLUNIFORM3FVPROC)load("glUniform3fv");
		func->glUniform3i = (PFNGLUNIFORM3IPROC)load("glUniform3i");
		func->glUniform3iv = (PFNGLUNIFORM3IVPROC)load("glUniform3iv");
		func->glUniform4f = (PFNGLUNIFORM4FPROC)load("glUniform4f");
		func->glUniform4fv = (PFNGLUNIFORM4FVPROC)load("glUniform4fv");
		func->glUniform4i = (PFNGLUNIFORM4IPROC)load("glUniform4i");
		func->glUniform4iv = (PFNGLUNIFORM4IVPROC)load("glUniform4iv");
		func->glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)load("glUniformMatrix2fv");
		func->glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)load("glUniformMatrix3fv");
		func->glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)load("glUniformMatrix4fv");
		func->glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");
		func->glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)load("glValidateProgram");
		func->glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)load("glVertexAttrib1f");
		func->glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)load("glVertexAttrib1fv");
		func->glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)load("glVertexAttrib2f");
		func->glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)load("glVertexAttrib2fv");
		func->glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)load("glVertexAttrib3f");
		func->glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)load("glVertexAttrib3fv");
		func->glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)load("glVertexAttrib4f");
		func->glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)load("glVertexAttrib4fv");
		func->glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");
		func->glViewport = (PFNGLVIEWPORTPROC)load("glViewport");
	}
	static void load_GL_ES_3_0(LoadProc load, GLFunctions* func)
	{
		func->glReadBuffer = (PFNGLREADBUFFERPROC)load("glReadBuffer");
		func->glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)load("glDrawRangeElements");
		func->glTexImage3D = (PFNGLTEXIMAGE3DPROC)load("glTexImage3D");
		func->glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)load("glTexSubImage3D");
		func->glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)load("glCopyTexSubImage3D");
		func->glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)load("glCompressedTexImage3D");
		func->glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)load("glCompressedTexSubImage3D");
		func->glGenQueries = (PFNGLGENQUERIESPROC)load("glGenQueries");
		func->glDeleteQueries = (PFNGLDELETEQUERIESPROC)load("glDeleteQueries");
		func->glIsQuery = (PFNGLISQUERYPROC)load("glIsQuery");
		func->glBeginQuery = (PFNGLBEGINQUERYPROC)load("glBeginQuery");
		func->glEndQuery = (PFNGLENDQUERYPROC)load("glEndQuery");
		func->glGetQueryiv = (PFNGLGETQUERYIVPROC)load("glGetQueryiv");
		func->glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC)load("glGetQueryObjectuiv");
		func->glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)load("glUnmapBuffer");
		func->glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC)load("glGetBufferPointerv");
		func->glDrawBuffers = (PFNGLDRAWBUFFERSPROC)load("glDrawBuffers");
		func->glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)load("glUniformMatrix2x3fv");
		func->glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)load("glUniformMatrix3x2fv");
		func->glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)load("glUniformMatrix2x4fv");
		func->glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)load("glUniformMatrix4x2fv");
		func->glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)load("glUniformMatrix3x4fv");
		func->glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)load("glUniformMatrix4x3fv");
		func->glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)load("glBlitFramebuffer");
		func->glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)load("glRenderbufferStorageMultisample");
		func->glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)load("glFramebufferTextureLayer");
		func->glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)load("glMapBufferRange");
		func->glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)load("glFlushMappedBufferRange");
		func->glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
		func->glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)load("glDeleteVertexArrays");
		func->glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
		func->glIsVertexArray = (PFNGLISVERTEXARRAYPROC)load("glIsVertexArray");
		func->glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)load("glGetIntegeri_v");
		func->glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC)load("glBeginTransformFeedback");
		func->glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC)load("glEndTransformFeedback");
		func->glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)load("glBindBufferRange");
		func->glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)load("glBindBufferBase");
		func->glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)load("glTransformFeedbackVaryings");
		func->glGetTransformFeedbackVarying = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)load("glGetTransformFeedbackVarying");
		func->glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)load("glVertexAttribIPointer");
		func->glGetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIVPROC)load("glGetVertexAttribIiv");
		func->glGetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIVPROC)load("glGetVertexAttribIuiv");
		func->glVertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC)load("glVertexAttribI4i");
		func->glVertexAttribI4ui = (PFNGLVERTEXATTRIBI4UIPROC)load("glVertexAttribI4ui");
		func->glVertexAttribI4iv = (PFNGLVERTEXATTRIBI4IVPROC)load("glVertexAttribI4iv");
		func->glVertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIVPROC)load("glVertexAttribI4uiv");
		func->glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC)load("glGetUniformuiv");
		func->glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC)load("glGetFragDataLocation");
		func->glUniform1ui = (PFNGLUNIFORM1UIPROC)load("glUniform1ui");
		func->glUniform2ui = (PFNGLUNIFORM2UIPROC)load("glUniform2ui");
		func->glUniform3ui = (PFNGLUNIFORM3UIPROC)load("glUniform3ui");
		func->glUniform4ui = (PFNGLUNIFORM4UIPROC)load("glUniform4ui");
		func->glUniform1uiv = (PFNGLUNIFORM1UIVPROC)load("glUniform1uiv");
		func->glUniform2uiv = (PFNGLUNIFORM2UIVPROC)load("glUniform2uiv");
		func->glUniform3uiv = (PFNGLUNIFORM3UIVPROC)load("glUniform3uiv");
		func->glUniform4uiv = (PFNGLUNIFORM4UIVPROC)load("glUniform4uiv");
		func->glClearBufferiv = (PFNGLCLEARBUFFERIVPROC)load("glClearBufferiv");
		func->glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC)load("glClearBufferuiv");
		func->glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)load("glClearBufferfv");
		func->glClearBufferfi = (PFNGLCLEARBUFFERFIPROC)load("glClearBufferfi");
		func->glGetStringi = (PFNGLGETSTRINGIPROC)load("glGetStringi");
		func->glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)load("glCopyBufferSubData");
		func->glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC)load("glGetUniformIndices");
		func->glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)load("glGetActiveUniformsiv");
		func->glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)load("glGetUniformBlockIndex");
		func->glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)load("glGetActiveUniformBlockiv");
		func->glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)load("glGetActiveUniformBlockName");
		func->glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)load("glUniformBlockBinding");
		func->glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)load("glDrawArraysInstanced");
		func->glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)load("glDrawElementsInstanced");
		func->glFenceSync = (PFNGLFENCESYNCPROC)load("glFenceSync");
		func->glIsSync = (PFNGLISSYNCPROC)load("glIsSync");
		func->glDeleteSync = (PFNGLDELETESYNCPROC)load("glDeleteSync");
		func->glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)load("glClientWaitSync");
		func->glWaitSync = (PFNGLWAITSYNCPROC)load("glWaitSync");
		func->glGetInteger64v = (PFNGLGETINTEGER64VPROC)load("glGetInteger64v");
		func->glGetSynciv = (PFNGLGETSYNCIVPROC)load("glGetSynciv");
		func->glGetInteger64i_v = (PFNGLGETINTEGER64I_VPROC)load("glGetInteger64i_v");
		func->glGetBufferParameteri64v = (PFNGLGETBUFFERPARAMETERI64VPROC)load("glGetBufferParameteri64v");
		func->glGenSamplers = (PFNGLGENSAMPLERSPROC)load("glGenSamplers");
		func->glDeleteSamplers = (PFNGLDELETESAMPLERSPROC)load("glDeleteSamplers");
		func->glIsSampler = (PFNGLISSAMPLERPROC)load("glIsSampler");
		func->glBindSampler = (PFNGLBINDSAMPLERPROC)load("glBindSampler");
		func->glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC)load("glSamplerParameteri");
		func->glSamplerParameteriv = (PFNGLSAMPLERPARAMETERIVPROC)load("glSamplerParameteriv");
		func->glSamplerParameterf = (PFNGLSAMPLERPARAMETERFPROC)load("glSamplerParameterf");
		func->glSamplerParameterfv = (PFNGLSAMPLERPARAMETERFVPROC)load("glSamplerParameterfv");
		func->glGetSamplerParameteriv = (PFNGLGETSAMPLERPARAMETERIVPROC)load("glGetSamplerParameteriv");
		func->glGetSamplerParameterfv = (PFNGLGETSAMPLERPARAMETERFVPROC)load("glGetSamplerParameterfv");
		func->glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)load("glVertexAttribDivisor");
		func->glBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC)load("glBindTransformFeedback");
		func->glDeleteTransformFeedbacks = (PFNGLDELETETRANSFORMFEEDBACKSPROC)load("glDeleteTransformFeedbacks");
		func->glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC)load("glGenTransformFeedbacks");
		func->glIsTransformFeedback = (PFNGLISTRANSFORMFEEDBACKPROC)load("glIsTransformFeedback");
		func->glPauseTransformFeedback = (PFNGLPAUSETRANSFORMFEEDBACKPROC)load("glPauseTransformFeedback");
		func->glResumeTransformFeedback = (PFNGLRESUMETRANSFORMFEEDBACKPROC)load("glResumeTransformFeedback");
		func->glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC)load("glGetProgramBinary");
		func->glProgramBinary = (PFNGLPROGRAMBINARYPROC)load("glProgramBinary");
		func->glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)load("glProgramParameteri");
		func->glInvalidateFramebuffer = (PFNGLINVALIDATEFRAMEBUFFERPROC)load("glInvalidateFramebuffer");
		func->glInvalidateSubFramebuffer = (PFNGLINVALIDATESUBFRAMEBUFFERPROC)load("glInvalidateSubFramebuffer");
		func->glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)load("glTexStorage2D");
		func->glTexStorage3D = (PFNGLTEXSTORAGE3DPROC)load("glTexStorage3D");
		func->glGetInternalformativ = (PFNGLGETINTERNALFORMATIVPROC)load("glGetInternalformativ");
	}
	static void load_GL_ES_3_1(LoadProc load, GLFunctions* func)
	{
		func->glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)load("glDispatchCompute");
		func->glDispatchComputeIndirect = (PFNGLDISPATCHCOMPUTEINDIRECTPROC)load("glDispatchComputeIndirect");
		func->glDrawArraysIndirect = (PFNGLDRAWARRAYSINDIRECTPROC)load("glDrawArraysIndirect");
		func->glDrawElementsIndirect = (PFNGLDRAWELEMENTSINDIRECTPROC)load("glDrawElementsIndirect");
		func->glFramebufferParameteri = (PFNGLFRAMEBUFFERPARAMETERIPROC)load("glFramebufferParameteri");
		func->glGetFramebufferParameteriv = (PFNGLGETFRAMEBUFFERPARAMETERIVPROC)load("glGetFramebufferParameteriv");
		func->glGetProgramInterfaceiv = (PFNGLGETPROGRAMINTERFACEIVPROC)load("glGetProgramInterfaceiv");
		func->glGetProgramResourceIndex = (PFNGLGETPROGRAMRESOURCEINDEXPROC)load("glGetProgramResourceIndex");
		func->glGetProgramResourceName = (PFNGLGETPROGRAMRESOURCENAMEPROC)load("glGetProgramResourceName");
		func->glGetProgramResourceiv = (PFNGLGETPROGRAMRESOURCEIVPROC)load("glGetProgramResourceiv");
		func->glGetProgramResourceLocation = (PFNGLGETPROGRAMRESOURCELOCATIONPROC)load("glGetProgramResourceLocation");
		func->glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC)load("glUseProgramStages");
		func->glActiveShaderProgram = (PFNGLACTIVESHADERPROGRAMPROC)load("glActiveShaderProgram");
		func->glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)load("glCreateShaderProgramv");
		func->glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC)load("glBindProgramPipeline");
		func->glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC)load("glDeleteProgramPipelines");
		func->glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC)load("glGenProgramPipelines");
		func->glIsProgramPipeline = (PFNGLISPROGRAMPIPELINEPROC)load("glIsProgramPipeline");
		func->glGetProgramPipelineiv = (PFNGLGETPROGRAMPIPELINEIVPROC)load("glGetProgramPipelineiv");
		func->glProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC)load("glProgramUniform1i");
		func->glProgramUniform2i = (PFNGLPROGRAMUNIFORM2IPROC)load("glProgramUniform2i");
		func->glProgramUniform3i = (PFNGLPROGRAMUNIFORM3IPROC)load("glProgramUniform3i");
		func->glProgramUniform4i = (PFNGLPROGRAMUNIFORM4IPROC)load("glProgramUniform4i");
		func->glProgramUniform1ui = (PFNGLPROGRAMUNIFORM1UIPROC)load("glProgramUniform1ui");
		func->glProgramUniform2ui = (PFNGLPROGRAMUNIFORM2UIPROC)load("glProgramUniform2ui");
		func->glProgramUniform3ui = (PFNGLPROGRAMUNIFORM3UIPROC)load("glProgramUniform3ui");
		func->glProgramUniform4ui = (PFNGLPROGRAMUNIFORM4UIPROC)load("glProgramUniform4ui");
		func->glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC)load("glProgramUniform1f");
		func->glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC)load("glProgramUniform2f");
		func->glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC)load("glProgramUniform3f");
		func->glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC)load("glProgramUniform4f");
		func->glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC)load("glProgramUniform1iv");
		func->glProgramUniform2iv = (PFNGLPROGRAMUNIFORM2IVPROC)load("glProgramUniform2iv");
		func->glProgramUniform3iv = (PFNGLPROGRAMUNIFORM3IVPROC)load("glProgramUniform3iv");
		func->glProgramUniform4iv = (PFNGLPROGRAMUNIFORM4IVPROC)load("glProgramUniform4iv");
		func->glProgramUniform1uiv = (PFNGLPROGRAMUNIFORM1UIVPROC)load("glProgramUniform1uiv");
		func->glProgramUniform2uiv = (PFNGLPROGRAMUNIFORM2UIVPROC)load("glProgramUniform2uiv");
		func->glProgramUniform3uiv = (PFNGLPROGRAMUNIFORM3UIVPROC)load("glProgramUniform3uiv");
		func->glProgramUniform4uiv = (PFNGLPROGRAMUNIFORM4UIVPROC)load("glProgramUniform4uiv");
		func->glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC)load("glProgramUniform1fv");
		func->glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC)load("glProgramUniform2fv");
		func->glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC)load("glProgramUniform3fv");
		func->glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC)load("glProgramUniform4fv");
		func->glProgramUniformMatrix2fv = (PFNGLPROGRAMUNIFORMMATRIX2FVPROC)load("glProgramUniformMatrix2fv");
		func->glProgramUniformMatrix3fv = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC)load("glProgramUniformMatrix3fv");
		func->glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC)load("glProgramUniformMatrix4fv");
		func->glProgramUniformMatrix2x3fv = (PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC)load("glProgramUniformMatrix2x3fv");
		func->glProgramUniformMatrix3x2fv = (PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC)load("glProgramUniformMatrix3x2fv");
		func->glProgramUniformMatrix2x4fv = (PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC)load("glProgramUniformMatrix2x4fv");
		func->glProgramUniformMatrix4x2fv = (PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC)load("glProgramUniformMatrix4x2fv");
		func->glProgramUniformMatrix3x4fv = (PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC)load("glProgramUniformMatrix3x4fv");
		func->glProgramUniformMatrix4x3fv = (PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC)load("glProgramUniformMatrix4x3fv");
		func->glValidateProgramPipeline = (PFNGLVALIDATEPROGRAMPIPELINEPROC)load("glValidateProgramPipeline");
		func->glGetProgramPipelineInfoLog = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC)load("glGetProgramPipelineInfoLog");
		func->glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)load("glBindImageTexture");
		func->glGetBooleani_v = (PFNGLGETBOOLEANI_VPROC)load("glGetBooleani_v");
		func->glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)load("glMemoryBarrier");
		func->glMemoryBarrierByRegion = (PFNGLMEMORYBARRIERBYREGIONPROC)load("glMemoryBarrierByRegion");
		func->glTexStorage2DMultisample = (PFNGLTEXSTORAGE2DMULTISAMPLEPROC)load("glTexStorage2DMultisample");
		func->glGetMultisamplefv = (PFNGLGETMULTISAMPLEFVPROC)load("glGetMultisamplefv");
		func->glSampleMaski = (PFNGLSAMPLEMASKIPROC)load("glSampleMaski");
		func->glGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC)load("glGetTexLevelParameteriv");
		func->glGetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFVPROC)load("glGetTexLevelParameterfv");
		func->glBindVertexBuffer = (PFNGLBINDVERTEXBUFFERPROC)load("glBindVertexBuffer");
		func->glVertexAttribFormat = (PFNGLVERTEXATTRIBFORMATPROC)load("glVertexAttribFormat");
		func->glVertexAttribIFormat = (PFNGLVERTEXATTRIBIFORMATPROC)load("glVertexAttribIFormat");
		func->glVertexAttribBinding = (PFNGLVERTEXATTRIBBINDINGPROC)load("glVertexAttribBinding");
		func->glVertexBindingDivisor = (PFNGLVERTEXBINDINGDIVISORPROC)load("glVertexBindingDivisor");
	}
	static void load_GL_ES_3_2(LoadProc load, GLFunctions* func)
	{
		func->glBlendBarrier = (PFNGLBLENDBARRIERPROC)load("glBlendBarrier");
		func->glCopyImageSubData = (PFNGLCOPYIMAGESUBDATAPROC)load("glCopyImageSubData");
		func->glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC)load("glDebugMessageControl");
		func->glDebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC)load("glDebugMessageInsert");
		func->glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)load("glDebugMessageCallback");
		func->glGetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC)load("glGetDebugMessageLog");
		func->glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC)load("glPushDebugGroup");
		func->glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROC)load("glPopDebugGroup");
		func->glObjectLabel = (PFNGLOBJECTLABELPROC)load("glObjectLabel");
		func->glGetObjectLabel = (PFNGLGETOBJECTLABELPROC)load("glGetObjectLabel");
		func->glObjectPtrLabel = (PFNGLOBJECTPTRLABELPROC)load("glObjectPtrLabel");
		func->glGetObjectPtrLabel = (PFNGLGETOBJECTPTRLABELPROC)load("glGetObjectPtrLabel");
		func->glGetPointerv = (PFNGLGETPOINTERVPROC)load("glGetPointerv");
		func->glEnablei = (PFNGLENABLEIPROC)load("glEnablei");
		func->glDisablei = (PFNGLDISABLEIPROC)load("glDisablei");
		func->glBlendEquationi = (PFNGLBLENDEQUATIONIPROC)load("glBlendEquationi");
		func->glBlendEquationSeparatei = (PFNGLBLENDEQUATIONSEPARATEIPROC)load("glBlendEquationSeparatei");
		func->glBlendFunci = (PFNGLBLENDFUNCIPROC)load("glBlendFunci");
		func->glBlendFuncSeparatei = (PFNGLBLENDFUNCSEPARATEIPROC)load("glBlendFuncSeparatei");
		func->glColorMaski = (PFNGLCOLORMASKIPROC)load("glColorMaski");
		func->glIsEnabledi = (PFNGLISENABLEDIPROC)load("glIsEnabledi");
		func->glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC)load("glDrawElementsBaseVertex");
		func->glDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC)load("glDrawRangeElementsBaseVertex");
		func->glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)load("glDrawElementsInstancedBaseVertex");
		func->glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)load("glFramebufferTexture");
		func->glPrimitiveBoundingBox = (PFNGLPRIMITIVEBOUNDINGBOXPROC)load("glPrimitiveBoundingBox");
		func->glGetGraphicsResetStatus = (PFNGLGETGRAPHICSRESETSTATUSPROC)load("glGetGraphicsResetStatus");
		func->glReadnPixels = (PFNGLREADNPIXELSPROC)load("glReadnPixels");
		func->glGetnUniformfv = (PFNGLGETNUNIFORMFVPROC)load("glGetnUniformfv");
		func->glGetnUniformiv = (PFNGLGETNUNIFORMIVPROC)load("glGetnUniformiv");
		func->glGetnUniformuiv = (PFNGLGETNUNIFORMUIVPROC)load("glGetnUniformuiv");
		func->glMinSampleShading = (PFNGLMINSAMPLESHADINGPROC)load("glMinSampleShading");
		func->glPatchParameteri = (PFNGLPATCHPARAMETERIPROC)load("glPatchParameteri");
		func->glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC)load("glTexParameterIiv");
		func->glTexParameterIuiv = (PFNGLTEXPARAMETERIUIVPROC)load("glTexParameterIuiv");
		func->glGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIVPROC)load("glGetTexParameterIiv");
		func->glGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIVPROC)load("glGetTexParameterIuiv");
		func->glSamplerParameterIiv = (PFNGLSAMPLERPARAMETERIIVPROC)load("glSamplerParameterIiv");
		func->glSamplerParameterIuiv = (PFNGLSAMPLERPARAMETERIUIVPROC)load("glSamplerParameterIuiv");
		func->glGetSamplerParameterIiv = (PFNGLGETSAMPLERPARAMETERIIVPROC)load("glGetSamplerParameterIiv");
		func->glGetSamplerParameterIuiv = (PFNGLGETSAMPLERPARAMETERIUIVPROC)load("glGetSamplerParameterIuiv");
		func->glTexBuffer = (PFNGLTEXBUFFERPROC)load("glTexBuffer");
		func->glTexBufferRange = (PFNGLTEXBUFFERRANGEPROC)load("glTexBufferRange");
		func->glTexStorage3DMultisample = (PFNGLTEXSTORAGE3DMULTISAMPLEPROC)load("glTexStorage3DMultisample");
	}
	static GLFunctions* load_GL_ES_funcs(int major, int minor)
	{
		GLFunctions* func = new GLFunctions;

#define LOAD_GL_ES_FUNC(maj, min) if ((major == maj && minor >= min) || major > maj) load_GL_ES_##maj##_##min(get_proc, func)

		LOAD_GL_ES_FUNC(2, 0);
		LOAD_GL_ES_FUNC(3, 0);
		LOAD_GL_ES_FUNC(3, 1);
		LOAD_GL_ES_FUNC(3, 2);

#undef LOAD_GL_ES_FUNC

		return func;
	}


	static void load_GL_OES_EGL_image(LoadProc load, GLExtFunctions* func)
	{
		func->glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)load("glEGLImageTargetTexture2DOES");
		func->glEGLImageTargetRenderbufferStorageOES = (PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC)load("glEGLImageTargetRenderbufferStorageOES");
	}
	static GLExtFunctions* load_GL_ES_EXT_funcs(GLFunctions* func, int major)
	{
		std::vector<const char*> exts;
		if (major >= 3)
		{
			int num = 0;
			func->glGetIntegerv(GL_NUM_EXTENSIONS, &num);
			exts.resize(num);
			for (int i = 0; i < num; i++)
				exts[i] = (const char*)func->glGetStringi(GL_EXTENSIONS, i);
		}

		if (!exts.empty())
		{
			GLExtFunctions* ext_func = new GLExtFunctions;
#define LOAD_GL_ES_EXT_FUNC(ext, ...) if(has_ext(exts, #ext)) load_##ext(get_proc, __VA_ARGS__)
			LOAD_GL_ES_EXT_FUNC(GL_OES_EGL_image, ext_func);


#undef LOAD_GL_ES_EXT_FUNC
			return ext_func;
		}

		return nullptr;
	}


	// gl
	static void load_GL_1_0(LoadProc load, GLFunctions* func)
    {
		func->glCullFace = (PFNGLCULLFACEPROC)load("glCullFace");
		func->glFrontFace = (PFNGLFRONTFACEPROC)load("glFrontFace");
		func->glHint = (PFNGLHINTPROC)load("glHint");
		func->glLineWidth = (PFNGLLINEWIDTHPROC)load("glLineWidth");
		func->glPointSize = (PFNGLPOINTSIZEPROC)load("glPointSize");
		func->glPolygonMode = (PFNGLPOLYGONMODEPROC)load("glPolygonMode");
		func->glScissor = (PFNGLSCISSORPROC)load("glScissor");
		func->glTexParameterf = (PFNGLTEXPARAMETERFPROC)load("glTexParameterf");
		func->glTexParameterfv = (PFNGLTEXPARAMETERFVPROC)load("glTexParameterfv");
		func->glTexParameteri = (PFNGLTEXPARAMETERIPROC)load("glTexParameteri");
		func->glTexParameteriv = (PFNGLTEXPARAMETERIVPROC)load("glTexParameteriv");
		func->glTexImage1D = (PFNGLTEXIMAGE1DPROC)load("glTexImage1D");
		func->glTexImage2D = (PFNGLTEXIMAGE2DPROC)load("glTexImage2D");
		func->glDrawBuffer = (PFNGLDRAWBUFFERPROC)load("glDrawBuffer");
		func->glClear = (PFNGLCLEARPROC)load("glClear");
		func->glClearColor = (PFNGLCLEARCOLORPROC)load("glClearColor");
		func->glClearStencil = (PFNGLCLEARSTENCILPROC)load("glClearStencil");
		func->glClearDepth = (PFNGLCLEARDEPTHPROC)load("glClearDepth");
		func->glStencilMask = (PFNGLSTENCILMASKPROC)load("glStencilMask");
		func->glColorMask = (PFNGLCOLORMASKPROC)load("glColorMask");
		func->glDepthMask = (PFNGLDEPTHMASKPROC)load("glDepthMask");
		func->glDisable = (PFNGLDISABLEPROC)load("glDisable");
		func->glEnable = (PFNGLENABLEPROC)load("glEnable");
		func->glFinish = (PFNGLFINISHPROC)load("glFinish");
		func->glFlush = (PFNGLFLUSHPROC)load("glFlush");
		func->glBlendFunc = (PFNGLBLENDFUNCPROC)load("glBlendFunc");
		func->glLogicOp = (PFNGLLOGICOPPROC)load("glLogicOp");
		func->glStencilFunc = (PFNGLSTENCILFUNCPROC)load("glStencilFunc");
		func->glStencilOp = (PFNGLSTENCILOPPROC)load("glStencilOp");
		func->glDepthFunc = (PFNGLDEPTHFUNCPROC)load("glDepthFunc");
		func->glPixelStoref = (PFNGLPIXELSTOREFPROC)load("glPixelStoref");
		func->glPixelStorei = (PFNGLPIXELSTOREIPROC)load("glPixelStorei");
		func->glReadBuffer = (PFNGLREADBUFFERPROC)load("glReadBuffer");
		func->glReadPixels = (PFNGLREADPIXELSPROC)load("glReadPixels");
		func->glGetBooleanv = (PFNGLGETBOOLEANVPROC)load("glGetBooleanv");
		func->glGetDoublev = (PFNGLGETDOUBLEVPROC)load("glGetDoublev");
		func->glGetError = (PFNGLGETERRORPROC)load("glGetError");
		func->glGetFloatv = (PFNGLGETFLOATVPROC)load("glGetFloatv");
		func->glGetIntegerv = (PFNGLGETINTEGERVPROC)load("glGetIntegerv");
		func->glGetString = (PFNGLGETSTRINGPROC)load("glGetString");
		func->glGetTexImage = (PFNGLGETTEXIMAGEPROC)load("glGetTexImage");
		func->glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC)load("glGetTexParameterfv");
		func->glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC)load("glGetTexParameteriv");
		func->glGetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFVPROC)load("glGetTexLevelParameterfv");
		func->glGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC)load("glGetTexLevelParameteriv");
		func->glIsEnabled = (PFNGLISENABLEDPROC)load("glIsEnabled");
		func->glDepthRange = (PFNGLDEPTHRANGEPROC)load("glDepthRange");
		func->glViewport = (PFNGLVIEWPORTPROC)load("glViewport");
	}
	static void load_GL_1_1(LoadProc load, GLFunctions* func)
    {
		func->glDrawArrays = (PFNGLDRAWARRAYSPROC)load("glDrawArrays");
		func->glDrawElements = (PFNGLDRAWELEMENTSPROC)load("glDrawElements");
		func->glPolygonOffset = (PFNGLPOLYGONOFFSETPROC)load("glPolygonOffset");
		func->glCopyTexImage1D = (PFNGLCOPYTEXIMAGE1DPROC)load("glCopyTexImage1D");
		func->glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC)load("glCopyTexImage2D");
		func->glCopyTexSubImage1D = (PFNGLCOPYTEXSUBIMAGE1DPROC)load("glCopyTexSubImage1D");
		func->glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC)load("glCopyTexSubImage2D");
		func->glTexSubImage1D = (PFNGLTEXSUBIMAGE1DPROC)load("glTexSubImage1D");
		func->glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)load("glTexSubImage2D");
		func->glBindTexture = (PFNGLBINDTEXTUREPROC)load("glBindTexture");
		func->glDeleteTextures = (PFNGLDELETETEXTURESPROC)load("glDeleteTextures");
		func->glGenTextures = (PFNGLGENTEXTURESPROC)load("glGenTextures");
		func->glIsTexture = (PFNGLISTEXTUREPROC)load("glIsTexture");
	}
	static void load_GL_1_2(LoadProc load, GLFunctions* func)
    {
		func->glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)load("glDrawRangeElements");
		func->glTexImage3D = (PFNGLTEXIMAGE3DPROC)load("glTexImage3D");
		func->glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)load("glTexSubImage3D");
		func->glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)load("glCopyTexSubImage3D");
	}
	static void load_GL_1_3(LoadProc load, GLFunctions* func)
    {
		func->glActiveTexture = (PFNGLACTIVETEXTUREPROC)load("glActiveTexture");
		func->glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)load("glSampleCoverage");
		func->glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)load("glCompressedTexImage3D");
		func->glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)load("glCompressedTexImage2D");
		func->glCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1DPROC)load("glCompressedTexImage1D");
		func->glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)load("glCompressedTexSubImage3D");
		func->glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)load("glCompressedTexSubImage2D");
		func->glCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC)load("glCompressedTexSubImage1D");
		func->glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)load("glGetCompressedTexImage");
	}
	static void load_GL_1_4(LoadProc load, GLFunctions* func)
    {
		func->glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)load("glBlendFuncSeparate");
		func->glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC)load("glMultiDrawArrays");
		func->glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC)load("glMultiDrawElements");
		func->glPointParameterf = (PFNGLPOINTPARAMETERFPROC)load("glPointParameterf");
		func->glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC)load("glPointParameterfv");
		func->glPointParameteri = (PFNGLPOINTPARAMETERIPROC)load("glPointParameteri");
		func->glPointParameteriv = (PFNGLPOINTPARAMETERIVPROC)load("glPointParameteriv");
		func->glBlendColor = (PFNGLBLENDCOLORPROC)load("glBlendColor");
		func->glBlendEquation = (PFNGLBLENDEQUATIONPROC)load("glBlendEquation");
	}
	static void load_GL_1_5(LoadProc load, GLFunctions* func)
    {
		func->glGenQueries = (PFNGLGENQUERIESPROC)load("glGenQueries");
		func->glDeleteQueries = (PFNGLDELETEQUERIESPROC)load("glDeleteQueries");
		func->glIsQuery = (PFNGLISQUERYPROC)load("glIsQuery");
		func->glBeginQuery = (PFNGLBEGINQUERYPROC)load("glBeginQuery");
		func->glEndQuery = (PFNGLENDQUERYPROC)load("glEndQuery");
		func->glGetQueryiv = (PFNGLGETQUERYIVPROC)load("glGetQueryiv");
		func->glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC)load("glGetQueryObjectiv");
		func->glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC)load("glGetQueryObjectuiv");
		func->glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
		func->glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)load("glDeleteBuffers");
		func->glGenBuffers = (PFNGLGENBUFFERSPROC)load("glGenBuffers");
		func->glIsBuffer = (PFNGLISBUFFERPROC)load("glIsBuffer");
		func->glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
		func->glBufferSubData = (PFNGLBUFFERSUBDATAPROC)load("glBufferSubData");
		func->glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)load("glGetBufferSubData");
		func->glMapBuffer = (PFNGLMAPBUFFERPROC)load("glMapBuffer");
		func->glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)load("glUnmapBuffer");
		func->glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)load("glGetBufferParameteriv");
		func->glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC)load("glGetBufferPointerv");
	}
	static void load_GL_2_0(LoadProc load, GLFunctions* func)
    {
		func->glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)load("glBlendEquationSeparate");
		func->glDrawBuffers = (PFNGLDRAWBUFFERSPROC)load("glDrawBuffers");
		func->glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)load("glStencilOpSeparate");
		func->glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)load("glStencilFuncSeparate");
		func->glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)load("glStencilMaskSeparate");
		func->glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
		func->glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)load("glBindAttribLocation");
		func->glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
		func->glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
		func->glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
		func->glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load("glDeleteProgram");
		func->glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
		func->glDetachShader = (PFNGLDETACHSHADERPROC)load("glDetachShader");
		func->glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)load("glDisableVertexAttribArray");
		func->glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
		func->glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)load("glGetActiveAttrib");
		func->glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)load("glGetActiveUniform");
		func->glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)load("glGetAttachedShaders");
		func->glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)load("glGetAttribLocation");
		func->glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load("glGetProgramiv");
		func->glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)load("glGetProgramInfoLog");
		func->glGetShaderiv = (PFNGLGETSHADERIVPROC)load("glGetShaderiv");
		func->glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)load("glGetShaderInfoLog");
		func->glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)load("glGetShaderSource");
		func->glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");
		func->glGetUniformfv = (PFNGLGETUNIFORMFVPROC)load("glGetUniformfv");
		func->glGetUniformiv = (PFNGLGETUNIFORMIVPROC)load("glGetUniformiv");
		func->glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC)load("glGetVertexAttribdv");
		func->glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)load("glGetVertexAttribfv");
		func->glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)load("glGetVertexAttribiv");
		func->glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)load("glGetVertexAttribPointerv");
		func->glIsProgram = (PFNGLISPROGRAMPROC)load("glIsProgram");
		func->glIsShader = (PFNGLISSHADERPROC)load("glIsShader");
		func->glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
		func->glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
		func->glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");
		func->glUniform1f = (PFNGLUNIFORM1FPROC)load("glUniform1f");
		func->glUniform2f = (PFNGLUNIFORM2FPROC)load("glUniform2f");
		func->glUniform3f = (PFNGLUNIFORM3FPROC)load("glUniform3f");
		func->glUniform4f = (PFNGLUNIFORM4FPROC)load("glUniform4f");
		func->glUniform1i = (PFNGLUNIFORM1IPROC)load("glUniform1i");
		func->glUniform2i = (PFNGLUNIFORM2IPROC)load("glUniform2i");
		func->glUniform3i = (PFNGLUNIFORM3IPROC)load("glUniform3i");
		func->glUniform4i = (PFNGLUNIFORM4IPROC)load("glUniform4i");
		func->glUniform1fv = (PFNGLUNIFORM1FVPROC)load("glUniform1fv");
		func->glUniform2fv = (PFNGLUNIFORM2FVPROC)load("glUniform2fv");
		func->glUniform3fv = (PFNGLUNIFORM3FVPROC)load("glUniform3fv");
		func->glUniform4fv = (PFNGLUNIFORM4FVPROC)load("glUniform4fv");
		func->glUniform1iv = (PFNGLUNIFORM1IVPROC)load("glUniform1iv");
		func->glUniform2iv = (PFNGLUNIFORM2IVPROC)load("glUniform2iv");
		func->glUniform3iv = (PFNGLUNIFORM3IVPROC)load("glUniform3iv");
		func->glUniform4iv = (PFNGLUNIFORM4IVPROC)load("glUniform4iv");
		func->glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)load("glUniformMatrix2fv");
		func->glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)load("glUniformMatrix3fv");
		func->glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)load("glUniformMatrix4fv");
		func->glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)load("glValidateProgram");
		func->glVertexAttrib1d = (PFNGLVERTEXATTRIB1DPROC)load("glVertexAttrib1d");
		func->glVertexAttrib1dv = (PFNGLVERTEXATTRIB1DVPROC)load("glVertexAttrib1dv");
		func->glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)load("glVertexAttrib1f");
		func->glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)load("glVertexAttrib1fv");
		func->glVertexAttrib1s = (PFNGLVERTEXATTRIB1SPROC)load("glVertexAttrib1s");
		func->glVertexAttrib1sv = (PFNGLVERTEXATTRIB1SVPROC)load("glVertexAttrib1sv");
		func->glVertexAttrib2d = (PFNGLVERTEXATTRIB2DPROC)load("glVertexAttrib2d");
		func->glVertexAttrib2dv = (PFNGLVERTEXATTRIB2DVPROC)load("glVertexAttrib2dv");
		func->glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)load("glVertexAttrib2f");
		func->glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)load("glVertexAttrib2fv");
		func->glVertexAttrib2s = (PFNGLVERTEXATTRIB2SPROC)load("glVertexAttrib2s");
		func->glVertexAttrib2sv = (PFNGLVERTEXATTRIB2SVPROC)load("glVertexAttrib2sv");
		func->glVertexAttrib3d = (PFNGLVERTEXATTRIB3DPROC)load("glVertexAttrib3d");
		func->glVertexAttrib3dv = (PFNGLVERTEXATTRIB3DVPROC)load("glVertexAttrib3dv");
		func->glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)load("glVertexAttrib3f");
		func->glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)load("glVertexAttrib3fv");
		func->glVertexAttrib3s = (PFNGLVERTEXATTRIB3SPROC)load("glVertexAttrib3s");
		func->glVertexAttrib3sv = (PFNGLVERTEXATTRIB3SVPROC)load("glVertexAttrib3sv");
		func->glVertexAttrib4Nbv = (PFNGLVERTEXATTRIB4NBVPROC)load("glVertexAttrib4Nbv");
		func->glVertexAttrib4Niv = (PFNGLVERTEXATTRIB4NIVPROC)load("glVertexAttrib4Niv");
		func->glVertexAttrib4Nsv = (PFNGLVERTEXATTRIB4NSVPROC)load("glVertexAttrib4Nsv");
		func->glVertexAttrib4Nub = (PFNGLVERTEXATTRIB4NUBPROC)load("glVertexAttrib4Nub");
		func->glVertexAttrib4Nubv = (PFNGLVERTEXATTRIB4NUBVPROC)load("glVertexAttrib4Nubv");
		func->glVertexAttrib4Nuiv = (PFNGLVERTEXATTRIB4NUIVPROC)load("glVertexAttrib4Nuiv");
		func->glVertexAttrib4Nusv = (PFNGLVERTEXATTRIB4NUSVPROC)load("glVertexAttrib4Nusv");
		func->glVertexAttrib4bv = (PFNGLVERTEXATTRIB4BVPROC)load("glVertexAttrib4bv");
		func->glVertexAttrib4d = (PFNGLVERTEXATTRIB4DPROC)load("glVertexAttrib4d");
		func->glVertexAttrib4dv = (PFNGLVERTEXATTRIB4DVPROC)load("glVertexAttrib4dv");
		func->glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)load("glVertexAttrib4f");
		func->glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)load("glVertexAttrib4fv");
		func->glVertexAttrib4iv = (PFNGLVERTEXATTRIB4IVPROC)load("glVertexAttrib4iv");
		func->glVertexAttrib4s = (PFNGLVERTEXATTRIB4SPROC)load("glVertexAttrib4s");
		func->glVertexAttrib4sv = (PFNGLVERTEXATTRIB4SVPROC)load("glVertexAttrib4sv");
		func->glVertexAttrib4ubv = (PFNGLVERTEXATTRIB4UBVPROC)load("glVertexAttrib4ubv");
		func->glVertexAttrib4uiv = (PFNGLVERTEXATTRIB4UIVPROC)load("glVertexAttrib4uiv");
		func->glVertexAttrib4usv = (PFNGLVERTEXATTRIB4USVPROC)load("glVertexAttrib4usv");
		func->glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");
	}
	static void load_GL_2_1(LoadProc load, GLFunctions* func)
    {
		func->glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)load("glUniformMatrix2x3fv");
		func->glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)load("glUniformMatrix3x2fv");
		func->glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)load("glUniformMatrix2x4fv");
		func->glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)load("glUniformMatrix4x2fv");
		func->glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)load("glUniformMatrix3x4fv");
		func->glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)load("glUniformMatrix4x3fv");
	}
	static void load_GL_3_0(LoadProc load, GLFunctions* func)
    {
		func->glColorMaski = (PFNGLCOLORMASKIPROC)load("glColorMaski");
		func->glGetBooleani_v = (PFNGLGETBOOLEANI_VPROC)load("glGetBooleani_v");
		func->glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)load("glGetIntegeri_v");
		func->glEnablei = (PFNGLENABLEIPROC)load("glEnablei");
		func->glDisablei = (PFNGLDISABLEIPROC)load("glDisablei");
		func->glIsEnabledi = (PFNGLISENABLEDIPROC)load("glIsEnabledi");
		func->glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC)load("glBeginTransformFeedback");
		func->glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC)load("glEndTransformFeedback");
		func->glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)load("glBindBufferRange");
		func->glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)load("glBindBufferBase");
		func->glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)load("glTransformFeedbackVaryings");
		func->glGetTransformFeedbackVarying = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)load("glGetTransformFeedbackVarying");
		func->glClampColor = (PFNGLCLAMPCOLORPROC)load("glClampColor");
		func->glBeginConditionalRender = (PFNGLBEGINCONDITIONALRENDERPROC)load("glBeginConditionalRender");
		func->glEndConditionalRender = (PFNGLENDCONDITIONALRENDERPROC)load("glEndConditionalRender");
		func->glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)load("glVertexAttribIPointer");
		func->glGetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIVPROC)load("glGetVertexAttribIiv");
		func->glGetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIVPROC)load("glGetVertexAttribIuiv");
		func->glVertexAttribI1i = (PFNGLVERTEXATTRIBI1IPROC)load("glVertexAttribI1i");
		func->glVertexAttribI2i = (PFNGLVERTEXATTRIBI2IPROC)load("glVertexAttribI2i");
		func->glVertexAttribI3i = (PFNGLVERTEXATTRIBI3IPROC)load("glVertexAttribI3i");
		func->glVertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC)load("glVertexAttribI4i");
		func->glVertexAttribI1ui = (PFNGLVERTEXATTRIBI1UIPROC)load("glVertexAttribI1ui");
		func->glVertexAttribI2ui = (PFNGLVERTEXATTRIBI2UIPROC)load("glVertexAttribI2ui");
		func->glVertexAttribI3ui = (PFNGLVERTEXATTRIBI3UIPROC)load("glVertexAttribI3ui");
		func->glVertexAttribI4ui = (PFNGLVERTEXATTRIBI4UIPROC)load("glVertexAttribI4ui");
		func->glVertexAttribI1iv = (PFNGLVERTEXATTRIBI1IVPROC)load("glVertexAttribI1iv");
		func->glVertexAttribI2iv = (PFNGLVERTEXATTRIBI2IVPROC)load("glVertexAttribI2iv");
		func->glVertexAttribI3iv = (PFNGLVERTEXATTRIBI3IVPROC)load("glVertexAttribI3iv");
		func->glVertexAttribI4iv = (PFNGLVERTEXATTRIBI4IVPROC)load("glVertexAttribI4iv");
		func->glVertexAttribI1uiv = (PFNGLVERTEXATTRIBI1UIVPROC)load("glVertexAttribI1uiv");
		func->glVertexAttribI2uiv = (PFNGLVERTEXATTRIBI2UIVPROC)load("glVertexAttribI2uiv");
		func->glVertexAttribI3uiv = (PFNGLVERTEXATTRIBI3UIVPROC)load("glVertexAttribI3uiv");
		func->glVertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIVPROC)load("glVertexAttribI4uiv");
		func->glVertexAttribI4bv = (PFNGLVERTEXATTRIBI4BVPROC)load("glVertexAttribI4bv");
		func->glVertexAttribI4sv = (PFNGLVERTEXATTRIBI4SVPROC)load("glVertexAttribI4sv");
		func->glVertexAttribI4ubv = (PFNGLVERTEXATTRIBI4UBVPROC)load("glVertexAttribI4ubv");
		func->glVertexAttribI4usv = (PFNGLVERTEXATTRIBI4USVPROC)load("glVertexAttribI4usv");
		func->glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC)load("glGetUniformuiv");
		func->glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC)load("glBindFragDataLocation");
		func->glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC)load("glGetFragDataLocation");
		func->glUniform1ui = (PFNGLUNIFORM1UIPROC)load("glUniform1ui");
		func->glUniform2ui = (PFNGLUNIFORM2UIPROC)load("glUniform2ui");
		func->glUniform3ui = (PFNGLUNIFORM3UIPROC)load("glUniform3ui");
		func->glUniform4ui = (PFNGLUNIFORM4UIPROC)load("glUniform4ui");
		func->glUniform1uiv = (PFNGLUNIFORM1UIVPROC)load("glUniform1uiv");
		func->glUniform2uiv = (PFNGLUNIFORM2UIVPROC)load("glUniform2uiv");
		func->glUniform3uiv = (PFNGLUNIFORM3UIVPROC)load("glUniform3uiv");
		func->glUniform4uiv = (PFNGLUNIFORM4UIVPROC)load("glUniform4uiv");
		func->glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC)load("glTexParameterIiv");
		func->glTexParameterIuiv = (PFNGLTEXPARAMETERIUIVPROC)load("glTexParameterIuiv");
		func->glGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIVPROC)load("glGetTexParameterIiv");
		func->glGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIVPROC)load("glGetTexParameterIuiv");
		func->glClearBufferiv = (PFNGLCLEARBUFFERIVPROC)load("glClearBufferiv");
		func->glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC)load("glClearBufferuiv");
		func->glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)load("glClearBufferfv");
		func->glClearBufferfi = (PFNGLCLEARBUFFERFIPROC)load("glClearBufferfi");
		func->glGetStringi = (PFNGLGETSTRINGIPROC)load("glGetStringi");
		func->glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)load("glIsRenderbuffer");
		func->glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)load("glBindRenderbuffer");
		func->glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)load("glDeleteRenderbuffers");
		func->glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)load("glGenRenderbuffers");
		func->glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)load("glRenderbufferStorage");
		func->glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)load("glGetRenderbufferParameteriv");
		func->glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)load("glIsFramebuffer");
		func->glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)load("glBindFramebuffer");
		func->glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)load("glDeleteFramebuffers");
		func->glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)load("glGenFramebuffers");
		func->glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)load("glCheckFramebufferStatus");
		func->glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)load("glFramebufferTexture1D");
		func->glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)load("glFramebufferTexture2D");
		func->glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)load("glFramebufferTexture3D");
		func->glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)load("glFramebufferRenderbuffer");
		func->glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)load("glGetFramebufferAttachmentParameteriv");
		func->glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)load("glGenerateMipmap");
		func->glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)load("glBlitFramebuffer");
		func->glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)load("glRenderbufferStorageMultisample");
		func->glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)load("glFramebufferTextureLayer");
		func->glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)load("glMapBufferRange");
		func->glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)load("glFlushMappedBufferRange");
		func->glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
		func->glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)load("glDeleteVertexArrays");
		func->glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
		func->glIsVertexArray = (PFNGLISVERTEXARRAYPROC)load("glIsVertexArray");
	}
	static void load_GL_3_1(LoadProc load, GLFunctions* func)
    {
		func->glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)load("glDrawArraysInstanced");
		func->glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)load("glDrawElementsInstanced");
		func->glTexBuffer = (PFNGLTEXBUFFERPROC)load("glTexBuffer");
		func->glPrimitiveRestartIndex = (PFNGLPRIMITIVERESTARTINDEXPROC)load("glPrimitiveRestartIndex");
		func->glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)load("glCopyBufferSubData");
		func->glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC)load("glGetUniformIndices");
		func->glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)load("glGetActiveUniformsiv");
		func->glGetActiveUniformName = (PFNGLGETACTIVEUNIFORMNAMEPROC)load("glGetActiveUniformName");
		func->glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)load("glGetUniformBlockIndex");
		func->glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)load("glGetActiveUniformBlockiv");
		func->glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)load("glGetActiveUniformBlockName");
		func->glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)load("glUniformBlockBinding");
		func->glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)load("glBindBufferRange");
		func->glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)load("glBindBufferBase");
		func->glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)load("glGetIntegeri_v");
	}
	static void load_GL_3_2(LoadProc load, GLFunctions* func)
    {
		func->glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC)load("glDrawElementsBaseVertex");
		func->glDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC)load("glDrawRangeElementsBaseVertex");
		func->glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)load("glDrawElementsInstancedBaseVertex");
		func->glMultiDrawElementsBaseVertex = (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC)load("glMultiDrawElementsBaseVertex");
		func->glProvokingVertex = (PFNGLPROVOKINGVERTEXPROC)load("glProvokingVertex");
		func->glFenceSync = (PFNGLFENCESYNCPROC)load("glFenceSync");
		func->glIsSync = (PFNGLISSYNCPROC)load("glIsSync");
		func->glDeleteSync = (PFNGLDELETESYNCPROC)load("glDeleteSync");
		func->glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)load("glClientWaitSync");
		func->glWaitSync = (PFNGLWAITSYNCPROC)load("glWaitSync");
		func->glGetInteger64v = (PFNGLGETINTEGER64VPROC)load("glGetInteger64v");
		func->glGetSynciv = (PFNGLGETSYNCIVPROC)load("glGetSynciv");
		func->glGetInteger64i_v = (PFNGLGETINTEGER64I_VPROC)load("glGetInteger64i_v");
		func->glGetBufferParameteri64v = (PFNGLGETBUFFERPARAMETERI64VPROC)load("glGetBufferParameteri64v");
		func->glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)load("glFramebufferTexture");
		func->glTexImage2DMultisample = (PFNGLTEXIMAGE2DMULTISAMPLEPROC)load("glTexImage2DMultisample");
		func->glTexImage3DMultisample = (PFNGLTEXIMAGE3DMULTISAMPLEPROC)load("glTexImage3DMultisample");
		func->glGetMultisamplefv = (PFNGLGETMULTISAMPLEFVPROC)load("glGetMultisamplefv");
		func->glSampleMaski = (PFNGLSAMPLEMASKIPROC)load("glSampleMaski");
	}
	static void load_GL_3_3(LoadProc load, GLFunctions* func)
    {
		func->glBindFragDataLocationIndexed = (PFNGLBINDFRAGDATALOCATIONINDEXEDPROC)load("glBindFragDataLocationIndexed");
		func->glGetFragDataIndex = (PFNGLGETFRAGDATAINDEXPROC)load("glGetFragDataIndex");
		func->glGenSamplers = (PFNGLGENSAMPLERSPROC)load("glGenSamplers");
		func->glDeleteSamplers = (PFNGLDELETESAMPLERSPROC)load("glDeleteSamplers");
		func->glIsSampler = (PFNGLISSAMPLERPROC)load("glIsSampler");
		func->glBindSampler = (PFNGLBINDSAMPLERPROC)load("glBindSampler");
		func->glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC)load("glSamplerParameteri");
		func->glSamplerParameteriv = (PFNGLSAMPLERPARAMETERIVPROC)load("glSamplerParameteriv");
		func->glSamplerParameterf = (PFNGLSAMPLERPARAMETERFPROC)load("glSamplerParameterf");
		func->glSamplerParameterfv = (PFNGLSAMPLERPARAMETERFVPROC)load("glSamplerParameterfv");
		func->glSamplerParameterIiv = (PFNGLSAMPLERPARAMETERIIVPROC)load("glSamplerParameterIiv");
		func->glSamplerParameterIuiv = (PFNGLSAMPLERPARAMETERIUIVPROC)load("glSamplerParameterIuiv");
		func->glGetSamplerParameteriv = (PFNGLGETSAMPLERPARAMETERIVPROC)load("glGetSamplerParameteriv");
		func->glGetSamplerParameterIiv = (PFNGLGETSAMPLERPARAMETERIIVPROC)load("glGetSamplerParameterIiv");
		func->glGetSamplerParameterfv = (PFNGLGETSAMPLERPARAMETERFVPROC)load("glGetSamplerParameterfv");
		func->glGetSamplerParameterIuiv = (PFNGLGETSAMPLERPARAMETERIUIVPROC)load("glGetSamplerParameterIuiv");
		func->glQueryCounter = (PFNGLQUERYCOUNTERPROC)load("glQueryCounter");
		func->glGetQueryObjecti64v = (PFNGLGETQUERYOBJECTI64VPROC)load("glGetQueryObjecti64v");
		func->glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC)load("glGetQueryObjectui64v");
		func->glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)load("glVertexAttribDivisor");
		func->glVertexAttribP1ui = (PFNGLVERTEXATTRIBP1UIPROC)load("glVertexAttribP1ui");
		func->glVertexAttribP1uiv = (PFNGLVERTEXATTRIBP1UIVPROC)load("glVertexAttribP1uiv");
		func->glVertexAttribP2ui = (PFNGLVERTEXATTRIBP2UIPROC)load("glVertexAttribP2ui");
		func->glVertexAttribP2uiv = (PFNGLVERTEXATTRIBP2UIVPROC)load("glVertexAttribP2uiv");
		func->glVertexAttribP3ui = (PFNGLVERTEXATTRIBP3UIPROC)load("glVertexAttribP3ui");
		func->glVertexAttribP3uiv = (PFNGLVERTEXATTRIBP3UIVPROC)load("glVertexAttribP3uiv");
		func->glVertexAttribP4ui = (PFNGLVERTEXATTRIBP4UIPROC)load("glVertexAttribP4ui");
		func->glVertexAttribP4uiv = (PFNGLVERTEXATTRIBP4UIVPROC)load("glVertexAttribP4uiv");
    }
	static void load_GL_4_0(LoadProc load, GLFunctions* func)
    {
		func->glMinSampleShading = (PFNGLMINSAMPLESHADINGPROC)load("glMinSampleShading");
		func->glBlendEquationi = (PFNGLBLENDEQUATIONIPROC)load("glBlendEquationi");
		func->glBlendEquationSeparatei = (PFNGLBLENDEQUATIONSEPARATEIPROC)load("glBlendEquationSeparatei");
		func->glBlendFunci = (PFNGLBLENDFUNCIPROC)load("glBlendFunci");
		func->glBlendFuncSeparatei = (PFNGLBLENDFUNCSEPARATEIPROC)load("glBlendFuncSeparatei");
		func->glDrawArraysIndirect = (PFNGLDRAWARRAYSINDIRECTPROC)load("glDrawArraysIndirect");
		func->glDrawElementsIndirect = (PFNGLDRAWELEMENTSINDIRECTPROC)load("glDrawElementsIndirect");
		func->glUniform1d = (PFNGLUNIFORM1DPROC)load("glUniform1d");
		func->glUniform2d = (PFNGLUNIFORM2DPROC)load("glUniform2d");
		func->glUniform3d = (PFNGLUNIFORM3DPROC)load("glUniform3d");
		func->glUniform4d = (PFNGLUNIFORM4DPROC)load("glUniform4d");
		func->glUniform1dv = (PFNGLUNIFORM1DVPROC)load("glUniform1dv");
		func->glUniform2dv = (PFNGLUNIFORM2DVPROC)load("glUniform2dv");
		func->glUniform3dv = (PFNGLUNIFORM3DVPROC)load("glUniform3dv");
		func->glUniform4dv = (PFNGLUNIFORM4DVPROC)load("glUniform4dv");
		func->glUniformMatrix2dv = (PFNGLUNIFORMMATRIX2DVPROC)load("glUniformMatrix2dv");
		func->glUniformMatrix3dv = (PFNGLUNIFORMMATRIX3DVPROC)load("glUniformMatrix3dv");
		func->glUniformMatrix4dv = (PFNGLUNIFORMMATRIX4DVPROC)load("glUniformMatrix4dv");
		func->glUniformMatrix2x3dv = (PFNGLUNIFORMMATRIX2X3DVPROC)load("glUniformMatrix2x3dv");
		func->glUniformMatrix2x4dv = (PFNGLUNIFORMMATRIX2X4DVPROC)load("glUniformMatrix2x4dv");
		func->glUniformMatrix3x2dv = (PFNGLUNIFORMMATRIX3X2DVPROC)load("glUniformMatrix3x2dv");
		func->glUniformMatrix3x4dv = (PFNGLUNIFORMMATRIX3X4DVPROC)load("glUniformMatrix3x4dv");
		func->glUniformMatrix4x2dv = (PFNGLUNIFORMMATRIX4X2DVPROC)load("glUniformMatrix4x2dv");
		func->glUniformMatrix4x3dv = (PFNGLUNIFORMMATRIX4X3DVPROC)load("glUniformMatrix4x3dv");
		func->glGetUniformdv = (PFNGLGETUNIFORMDVPROC)load("glGetUniformdv");
		func->glGetSubroutineUniformLocation = (PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC)load("glGetSubroutineUniformLocation");
		func->glGetSubroutineIndex = (PFNGLGETSUBROUTINEINDEXPROC)load("glGetSubroutineIndex");
		func->glGetActiveSubroutineUniformiv = (PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC)load("glGetActiveSubroutineUniformiv");
		func->glGetActiveSubroutineUniformName = (PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC)load("glGetActiveSubroutineUniformName");
		func->glGetActiveSubroutineName = (PFNGLGETACTIVESUBROUTINENAMEPROC)load("glGetActiveSubroutineName");
		func->glUniformSubroutinesuiv = (PFNGLUNIFORMSUBROUTINESUIVPROC)load("glUniformSubroutinesuiv");
		func->glGetUniformSubroutineuiv = (PFNGLGETUNIFORMSUBROUTINEUIVPROC)load("glGetUniformSubroutineuiv");
		func->glGetProgramStageiv = (PFNGLGETPROGRAMSTAGEIVPROC)load("glGetProgramStageiv");
		func->glPatchParameteri = (PFNGLPATCHPARAMETERIPROC)load("glPatchParameteri");
		func->glPatchParameterfv = (PFNGLPATCHPARAMETERFVPROC)load("glPatchParameterfv");
		func->glBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC)load("glBindTransformFeedback");
		func->glDeleteTransformFeedbacks = (PFNGLDELETETRANSFORMFEEDBACKSPROC)load("glDeleteTransformFeedbacks");
		func->glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC)load("glGenTransformFeedbacks");
		func->glIsTransformFeedback = (PFNGLISTRANSFORMFEEDBACKPROC)load("glIsTransformFeedback");
		func->glPauseTransformFeedback = (PFNGLPAUSETRANSFORMFEEDBACKPROC)load("glPauseTransformFeedback");
		func->glResumeTransformFeedback = (PFNGLRESUMETRANSFORMFEEDBACKPROC)load("glResumeTransformFeedback");
		func->glDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACKPROC)load("glDrawTransformFeedback");
		func->glDrawTransformFeedbackStream = (PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC)load("glDrawTransformFeedbackStream");
		func->glBeginQueryIndexed = (PFNGLBEGINQUERYINDEXEDPROC)load("glBeginQueryIndexed");
		func->glEndQueryIndexed = (PFNGLENDQUERYINDEXEDPROC)load("glEndQueryIndexed");
		func->glGetQueryIndexediv = (PFNGLGETQUERYINDEXEDIVPROC)load("glGetQueryIndexediv");
	}
	static void load_GL_4_1(LoadProc load, GLFunctions* func)
    {
		func->glReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC)load("glReleaseShaderCompiler");
		func->glShaderBinary = (PFNGLSHADERBINARYPROC)load("glShaderBinary");
		func->glGetShaderPrecisionFormat = (PFNGLGETSHADERPRECISIONFORMATPROC)load("glGetShaderPrecisionFormat");
		func->glDepthRangef = (PFNGLDEPTHRANGEFPROC)load("glDepthRangef");
		func->glClearDepthf = (PFNGLCLEARDEPTHFPROC)load("glClearDepthf");
		func->glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC)load("glGetProgramBinary");
		func->glProgramBinary = (PFNGLPROGRAMBINARYPROC)load("glProgramBinary");
		func->glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)load("glProgramParameteri");
		func->glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC)load("glUseProgramStages");
		func->glActiveShaderProgram = (PFNGLACTIVESHADERPROGRAMPROC)load("glActiveShaderProgram");
		func->glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)load("glCreateShaderProgramv");
		func->glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC)load("glBindProgramPipeline");
		func->glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC)load("glDeleteProgramPipelines");
		func->glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC)load("glGenProgramPipelines");
		func->glIsProgramPipeline = (PFNGLISPROGRAMPIPELINEPROC)load("glIsProgramPipeline");
		func->glGetProgramPipelineiv = (PFNGLGETPROGRAMPIPELINEIVPROC)load("glGetProgramPipelineiv");
		func->glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)load("glProgramParameteri");
		func->glProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC)load("glProgramUniform1i");
		func->glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC)load("glProgramUniform1iv");
		func->glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC)load("glProgramUniform1f");
		func->glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC)load("glProgramUniform1fv");
		func->glProgramUniform1d = (PFNGLPROGRAMUNIFORM1DPROC)load("glProgramUniform1d");
		func->glProgramUniform1dv = (PFNGLPROGRAMUNIFORM1DVPROC)load("glProgramUniform1dv");
		func->glProgramUniform1ui = (PFNGLPROGRAMUNIFORM1UIPROC)load("glProgramUniform1ui");
		func->glProgramUniform1uiv = (PFNGLPROGRAMUNIFORM1UIVPROC)load("glProgramUniform1uiv");
		func->glProgramUniform2i = (PFNGLPROGRAMUNIFORM2IPROC)load("glProgramUniform2i");
		func->glProgramUniform2iv = (PFNGLPROGRAMUNIFORM2IVPROC)load("glProgramUniform2iv");
		func->glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC)load("glProgramUniform2f");
		func->glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC)load("glProgramUniform2fv");
		func->glProgramUniform2d = (PFNGLPROGRAMUNIFORM2DPROC)load("glProgramUniform2d");
		func->glProgramUniform2dv = (PFNGLPROGRAMUNIFORM2DVPROC)load("glProgramUniform2dv");
		func->glProgramUniform2ui = (PFNGLPROGRAMUNIFORM2UIPROC)load("glProgramUniform2ui");
		func->glProgramUniform2uiv = (PFNGLPROGRAMUNIFORM2UIVPROC)load("glProgramUniform2uiv");
		func->glProgramUniform3i = (PFNGLPROGRAMUNIFORM3IPROC)load("glProgramUniform3i");
		func->glProgramUniform3iv = (PFNGLPROGRAMUNIFORM3IVPROC)load("glProgramUniform3iv");
		func->glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC)load("glProgramUniform3f");
		func->glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC)load("glProgramUniform3fv");
		func->glProgramUniform3d = (PFNGLPROGRAMUNIFORM3DPROC)load("glProgramUniform3d");
		func->glProgramUniform3dv = (PFNGLPROGRAMUNIFORM3DVPROC)load("glProgramUniform3dv");
		func->glProgramUniform3ui = (PFNGLPROGRAMUNIFORM3UIPROC)load("glProgramUniform3ui");
		func->glProgramUniform3uiv = (PFNGLPROGRAMUNIFORM3UIVPROC)load("glProgramUniform3uiv");
		func->glProgramUniform4i = (PFNGLPROGRAMUNIFORM4IPROC)load("glProgramUniform4i");
		func->glProgramUniform4iv = (PFNGLPROGRAMUNIFORM4IVPROC)load("glProgramUniform4iv");
		func->glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC)load("glProgramUniform4f");
		func->glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC)load("glProgramUniform4fv");
		func->glProgramUniform4d = (PFNGLPROGRAMUNIFORM4DPROC)load("glProgramUniform4d");
		func->glProgramUniform4dv = (PFNGLPROGRAMUNIFORM4DVPROC)load("glProgramUniform4dv");
		func->glProgramUniform4ui = (PFNGLPROGRAMUNIFORM4UIPROC)load("glProgramUniform4ui");
		func->glProgramUniform4uiv = (PFNGLPROGRAMUNIFORM4UIVPROC)load("glProgramUniform4uiv");
		func->glProgramUniformMatrix2fv = (PFNGLPROGRAMUNIFORMMATRIX2FVPROC)load("glProgramUniformMatrix2fv");
		func->glProgramUniformMatrix3fv = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC)load("glProgramUniformMatrix3fv");
		func->glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC)load("glProgramUniformMatrix4fv");
		func->glProgramUniformMatrix2dv = (PFNGLPROGRAMUNIFORMMATRIX2DVPROC)load("glProgramUniformMatrix2dv");
		func->glProgramUniformMatrix3dv = (PFNGLPROGRAMUNIFORMMATRIX3DVPROC)load("glProgramUniformMatrix3dv");
		func->glProgramUniformMatrix4dv = (PFNGLPROGRAMUNIFORMMATRIX4DVPROC)load("glProgramUniformMatrix4dv");
		func->glProgramUniformMatrix2x3fv = (PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC)load("glProgramUniformMatrix2x3fv");
		func->glProgramUniformMatrix3x2fv = (PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC)load("glProgramUniformMatrix3x2fv");
		func->glProgramUniformMatrix2x4fv = (PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC)load("glProgramUniformMatrix2x4fv");
		func->glProgramUniformMatrix4x2fv = (PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC)load("glProgramUniformMatrix4x2fv");
		func->glProgramUniformMatrix3x4fv = (PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC)load("glProgramUniformMatrix3x4fv");
		func->glProgramUniformMatrix4x3fv = (PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC)load("glProgramUniformMatrix4x3fv");
		func->glProgramUniformMatrix2x3dv = (PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC)load("glProgramUniformMatrix2x3dv");
		func->glProgramUniformMatrix3x2dv = (PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC)load("glProgramUniformMatrix3x2dv");
		func->glProgramUniformMatrix2x4dv = (PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC)load("glProgramUniformMatrix2x4dv");
		func->glProgramUniformMatrix4x2dv = (PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC)load("glProgramUniformMatrix4x2dv");
		func->glProgramUniformMatrix3x4dv = (PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC)load("glProgramUniformMatrix3x4dv");
		func->glProgramUniformMatrix4x3dv = (PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC)load("glProgramUniformMatrix4x3dv");
		func->glValidateProgramPipeline = (PFNGLVALIDATEPROGRAMPIPELINEPROC)load("glValidateProgramPipeline");
		func->glGetProgramPipelineInfoLog = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC)load("glGetProgramPipelineInfoLog");
		func->glVertexAttribL1d = (PFNGLVERTEXATTRIBL1DPROC)load("glVertexAttribL1d");
		func->glVertexAttribL2d = (PFNGLVERTEXATTRIBL2DPROC)load("glVertexAttribL2d");
		func->glVertexAttribL3d = (PFNGLVERTEXATTRIBL3DPROC)load("glVertexAttribL3d");
		func->glVertexAttribL4d = (PFNGLVERTEXATTRIBL4DPROC)load("glVertexAttribL4d");
		func->glVertexAttribL1dv = (PFNGLVERTEXATTRIBL1DVPROC)load("glVertexAttribL1dv");
		func->glVertexAttribL2dv = (PFNGLVERTEXATTRIBL2DVPROC)load("glVertexAttribL2dv");
		func->glVertexAttribL3dv = (PFNGLVERTEXATTRIBL3DVPROC)load("glVertexAttribL3dv");
		func->glVertexAttribL4dv = (PFNGLVERTEXATTRIBL4DVPROC)load("glVertexAttribL4dv");
		func->glVertexAttribLPointer = (PFNGLVERTEXATTRIBLPOINTERPROC)load("glVertexAttribLPointer");
		func->glGetVertexAttribLdv = (PFNGLGETVERTEXATTRIBLDVPROC)load("glGetVertexAttribLdv");
		func->glViewportArrayv = (PFNGLVIEWPORTARRAYVPROC)load("glViewportArrayv");
		func->glViewportIndexedf = (PFNGLVIEWPORTINDEXEDFPROC)load("glViewportIndexedf");
		func->glViewportIndexedfv = (PFNGLVIEWPORTINDEXEDFVPROC)load("glViewportIndexedfv");
		func->glScissorArrayv = (PFNGLSCISSORARRAYVPROC)load("glScissorArrayv");
		func->glScissorIndexed = (PFNGLSCISSORINDEXEDPROC)load("glScissorIndexed");
		func->glScissorIndexedv = (PFNGLSCISSORINDEXEDVPROC)load("glScissorIndexedv");
		func->glDepthRangeArrayv = (PFNGLDEPTHRANGEARRAYVPROC)load("glDepthRangeArrayv");
		func->glDepthRangeIndexed = (PFNGLDEPTHRANGEINDEXEDPROC)load("glDepthRangeIndexed");
		func->glGetFloati_v = (PFNGLGETFLOATI_VPROC)load("glGetFloati_v");
		func->glGetDoublei_v = (PFNGLGETDOUBLEI_VPROC)load("glGetDoublei_v");
	}
	static void load_GL_4_2(LoadProc load, GLFunctions* func)
    {
		func->glDrawArraysInstancedBaseInstance = (PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC)load("glDrawArraysInstancedBaseInstance");
		func->glDrawElementsInstancedBaseInstance = (PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC)load("glDrawElementsInstancedBaseInstance");
		func->glDrawElementsInstancedBaseVertexBaseInstance = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC)load("glDrawElementsInstancedBaseVertexBaseInstance");
		func->glGetInternalformativ = (PFNGLGETINTERNALFORMATIVPROC)load("glGetInternalformativ");
		func->glGetActiveAtomicCounterBufferiv = (PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC)load("glGetActiveAtomicCounterBufferiv");
		func->glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)load("glBindImageTexture");
		func->glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)load("glMemoryBarrier");
		func->glTexStorage1D = (PFNGLTEXSTORAGE1DPROC)load("glTexStorage1D");
		func->glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)load("glTexStorage2D");
		func->glTexStorage3D = (PFNGLTEXSTORAGE3DPROC)load("glTexStorage3D");
		func->glDrawTransformFeedbackInstanced = (PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC)load("glDrawTransformFeedbackInstanced");
		func->glDrawTransformFeedbackStreamInstanced = (PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC)load("glDrawTransformFeedbackStreamInstanced");
	}
	static void load_GL_4_3(LoadProc load, GLFunctions* func)
    {
		func->glClearBufferData = (PFNGLCLEARBUFFERDATAPROC)load("glClearBufferData");
		func->glClearBufferSubData = (PFNGLCLEARBUFFERSUBDATAPROC)load("glClearBufferSubData");
		func->glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)load("glDispatchCompute");
		func->glDispatchComputeIndirect = (PFNGLDISPATCHCOMPUTEINDIRECTPROC)load("glDispatchComputeIndirect");
		func->glCopyImageSubData = (PFNGLCOPYIMAGESUBDATAPROC)load("glCopyImageSubData");
		func->glFramebufferParameteri = (PFNGLFRAMEBUFFERPARAMETERIPROC)load("glFramebufferParameteri");
		func->glGetFramebufferParameteriv = (PFNGLGETFRAMEBUFFERPARAMETERIVPROC)load("glGetFramebufferParameteriv");
		func->glGetInternalformati64v = (PFNGLGETINTERNALFORMATI64VPROC)load("glGetInternalformati64v");
		func->glInvalidateTexSubImage = (PFNGLINVALIDATETEXSUBIMAGEPROC)load("glInvalidateTexSubImage");
		func->glInvalidateTexImage = (PFNGLINVALIDATETEXIMAGEPROC)load("glInvalidateTexImage");
		func->glInvalidateBufferSubData = (PFNGLINVALIDATEBUFFERSUBDATAPROC)load("glInvalidateBufferSubData");
		func->glInvalidateBufferData = (PFNGLINVALIDATEBUFFERDATAPROC)load("glInvalidateBufferData");
		func->glInvalidateFramebuffer = (PFNGLINVALIDATEFRAMEBUFFERPROC)load("glInvalidateFramebuffer");
		func->glInvalidateSubFramebuffer = (PFNGLINVALIDATESUBFRAMEBUFFERPROC)load("glInvalidateSubFramebuffer");
		func->glMultiDrawArraysIndirect = (PFNGLMULTIDRAWARRAYSINDIRECTPROC)load("glMultiDrawArraysIndirect");
		func->glMultiDrawElementsIndirect = (PFNGLMULTIDRAWELEMENTSINDIRECTPROC)load("glMultiDrawElementsIndirect");
		func->glGetProgramInterfaceiv = (PFNGLGETPROGRAMINTERFACEIVPROC)load("glGetProgramInterfaceiv");
		func->glGetProgramResourceIndex = (PFNGLGETPROGRAMRESOURCEINDEXPROC)load("glGetProgramResourceIndex");
		func->glGetProgramResourceName = (PFNGLGETPROGRAMRESOURCENAMEPROC)load("glGetProgramResourceName");
		func->glGetProgramResourceiv = (PFNGLGETPROGRAMRESOURCEIVPROC)load("glGetProgramResourceiv");
		func->glGetProgramResourceLocation = (PFNGLGETPROGRAMRESOURCELOCATIONPROC)load("glGetProgramResourceLocation");
		func->glGetProgramResourceLocationIndex = (PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC)load("glGetProgramResourceLocationIndex");
		func->glShaderStorageBlockBinding = (PFNGLSHADERSTORAGEBLOCKBINDINGPROC)load("glShaderStorageBlockBinding");
		func->glTexBufferRange = (PFNGLTEXBUFFERRANGEPROC)load("glTexBufferRange");
		func->glTexStorage2DMultisample = (PFNGLTEXSTORAGE2DMULTISAMPLEPROC)load("glTexStorage2DMultisample");
		func->glTexStorage3DMultisample = (PFNGLTEXSTORAGE3DMULTISAMPLEPROC)load("glTexStorage3DMultisample");
		func->glTextureView = (PFNGLTEXTUREVIEWPROC)load("glTextureView");
		func->glBindVertexBuffer = (PFNGLBINDVERTEXBUFFERPROC)load("glBindVertexBuffer");
		func->glVertexAttribFormat = (PFNGLVERTEXATTRIBFORMATPROC)load("glVertexAttribFormat");
		func->glVertexAttribIFormat = (PFNGLVERTEXATTRIBIFORMATPROC)load("glVertexAttribIFormat");
		func->glVertexAttribLFormat = (PFNGLVERTEXATTRIBLFORMATPROC)load("glVertexAttribLFormat");
		func->glVertexAttribBinding = (PFNGLVERTEXATTRIBBINDINGPROC)load("glVertexAttribBinding");
		func->glVertexBindingDivisor = (PFNGLVERTEXBINDINGDIVISORPROC)load("glVertexBindingDivisor");
		func->glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC)load("glDebugMessageControl");
		func->glDebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC)load("glDebugMessageInsert");
		func->glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)load("glDebugMessageCallback");
		func->glGetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC)load("glGetDebugMessageLog");
		func->glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC)load("glPushDebugGroup");
		func->glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROC)load("glPopDebugGroup");
		func->glObjectLabel = (PFNGLOBJECTLABELPROC)load("glObjectLabel");
		func->glGetObjectLabel = (PFNGLGETOBJECTLABELPROC)load("glGetObjectLabel");
		func->glObjectPtrLabel = (PFNGLOBJECTPTRLABELPROC)load("glObjectPtrLabel");
		func->glGetObjectPtrLabel = (PFNGLGETOBJECTPTRLABELPROC)load("glGetObjectPtrLabel");
		func->glGetPointerv = (PFNGLGETPOINTERVPROC)load("glGetPointerv");
	}
	static void load_GL_4_4(LoadProc load, GLFunctions* func)
    {
		func->glBufferStorage = (PFNGLBUFFERSTORAGEPROC)load("glBufferStorage");
		func->glClearTexImage = (PFNGLCLEARTEXIMAGEPROC)load("glClearTexImage");
		func->glClearTexSubImage = (PFNGLCLEARTEXSUBIMAGEPROC)load("glClearTexSubImage");
		func->glBindBuffersBase = (PFNGLBINDBUFFERSBASEPROC)load("glBindBuffersBase");
		func->glBindBuffersRange = (PFNGLBINDBUFFERSRANGEPROC)load("glBindBuffersRange");
		func->glBindTextures = (PFNGLBINDTEXTURESPROC)load("glBindTextures");
		func->glBindSamplers = (PFNGLBINDSAMPLERSPROC)load("glBindSamplers");
		func->glBindImageTextures = (PFNGLBINDIMAGETEXTURESPROC)load("glBindImageTextures");
		func->glBindVertexBuffers = (PFNGLBINDVERTEXBUFFERSPROC)load("glBindVertexBuffers");
	}
	static void load_GL_4_5(LoadProc load, GLFunctions* func)
    {
		func->glClipControl = (PFNGLCLIPCONTROLPROC)load("glClipControl");
		func->glCreateTransformFeedbacks = (PFNGLCREATETRANSFORMFEEDBACKSPROC)load("glCreateTransformFeedbacks");
		func->glTransformFeedbackBufferBase = (PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC)load("glTransformFeedbackBufferBase");
		func->glTransformFeedbackBufferRange = (PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC)load("glTransformFeedbackBufferRange");
		func->glGetTransformFeedbackiv = (PFNGLGETTRANSFORMFEEDBACKIVPROC)load("glGetTransformFeedbackiv");
		func->glGetTransformFeedbacki_v = (PFNGLGETTRANSFORMFEEDBACKI_VPROC)load("glGetTransformFeedbacki_v");
		func->glGetTransformFeedbacki64_v = (PFNGLGETTRANSFORMFEEDBACKI64_VPROC)load("glGetTransformFeedbacki64_v");
		func->glCreateBuffers = (PFNGLCREATEBUFFERSPROC)load("glCreateBuffers");
		func->glNamedBufferStorage = (PFNGLNAMEDBUFFERSTORAGEPROC)load("glNamedBufferStorage");
		func->glNamedBufferData = (PFNGLNAMEDBUFFERDATAPROC)load("glNamedBufferData");
		func->glNamedBufferSubData = (PFNGLNAMEDBUFFERSUBDATAPROC)load("glNamedBufferSubData");
		func->glCopyNamedBufferSubData = (PFNGLCOPYNAMEDBUFFERSUBDATAPROC)load("glCopyNamedBufferSubData");
		func->glClearNamedBufferData = (PFNGLCLEARNAMEDBUFFERDATAPROC)load("glClearNamedBufferData");
		func->glClearNamedBufferSubData = (PFNGLCLEARNAMEDBUFFERSUBDATAPROC)load("glClearNamedBufferSubData");
		func->glMapNamedBuffer = (PFNGLMAPNAMEDBUFFERPROC)load("glMapNamedBuffer");
		func->glMapNamedBufferRange = (PFNGLMAPNAMEDBUFFERRANGEPROC)load("glMapNamedBufferRange");
		func->glUnmapNamedBuffer = (PFNGLUNMAPNAMEDBUFFERPROC)load("glUnmapNamedBuffer");
		func->glFlushMappedNamedBufferRange = (PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC)load("glFlushMappedNamedBufferRange");
		func->glGetNamedBufferParameteriv = (PFNGLGETNAMEDBUFFERPARAMETERIVPROC)load("glGetNamedBufferParameteriv");
		func->glGetNamedBufferParameteri64v = (PFNGLGETNAMEDBUFFERPARAMETERI64VPROC)load("glGetNamedBufferParameteri64v");
		func->glGetNamedBufferPointerv = (PFNGLGETNAMEDBUFFERPOINTERVPROC)load("glGetNamedBufferPointerv");
		func->glGetNamedBufferSubData = (PFNGLGETNAMEDBUFFERSUBDATAPROC)load("glGetNamedBufferSubData");
		func->glCreateFramebuffers = (PFNGLCREATEFRAMEBUFFERSPROC)load("glCreateFramebuffers");
		func->glNamedFramebufferRenderbuffer = (PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC)load("glNamedFramebufferRenderbuffer");
		func->glNamedFramebufferParameteri = (PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC)load("glNamedFramebufferParameteri");
		func->glNamedFramebufferTexture = (PFNGLNAMEDFRAMEBUFFERTEXTUREPROC)load("glNamedFramebufferTexture");
		func->glNamedFramebufferTextureLayer = (PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC)load("glNamedFramebufferTextureLayer");
		func->glNamedFramebufferDrawBuffer = (PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC)load("glNamedFramebufferDrawBuffer");
		func->glNamedFramebufferDrawBuffers = (PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC)load("glNamedFramebufferDrawBuffers");
		func->glNamedFramebufferReadBuffer = (PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC)load("glNamedFramebufferReadBuffer");
		func->glInvalidateNamedFramebufferData = (PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC)load("glInvalidateNamedFramebufferData");
		func->glInvalidateNamedFramebufferSubData = (PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC)load("glInvalidateNamedFramebufferSubData");
		func->glClearNamedFramebufferiv = (PFNGLCLEARNAMEDFRAMEBUFFERIVPROC)load("glClearNamedFramebufferiv");
		func->glClearNamedFramebufferuiv = (PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC)load("glClearNamedFramebufferuiv");
		func->glClearNamedFramebufferfv = (PFNGLCLEARNAMEDFRAMEBUFFERFVPROC)load("glClearNamedFramebufferfv");
		func->glClearNamedFramebufferfi = (PFNGLCLEARNAMEDFRAMEBUFFERFIPROC)load("glClearNamedFramebufferfi");
		func->glBlitNamedFramebuffer = (PFNGLBLITNAMEDFRAMEBUFFERPROC)load("glBlitNamedFramebuffer");
		func->glCheckNamedFramebufferStatus = (PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC)load("glCheckNamedFramebufferStatus");
		func->glGetNamedFramebufferParameteriv = (PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC)load("glGetNamedFramebufferParameteriv");
		func->glGetNamedFramebufferAttachmentParameteriv = (PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC)load("glGetNamedFramebufferAttachmentParameteriv");
		func->glCreateRenderbuffers = (PFNGLCREATERENDERBUFFERSPROC)load("glCreateRenderbuffers");
		func->glNamedRenderbufferStorage = (PFNGLNAMEDRENDERBUFFERSTORAGEPROC)load("glNamedRenderbufferStorage");
		func->glNamedRenderbufferStorageMultisample = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC)load("glNamedRenderbufferStorageMultisample");
		func->glGetNamedRenderbufferParameteriv = (PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC)load("glGetNamedRenderbufferParameteriv");
		func->glCreateTextures = (PFNGLCREATETEXTURESPROC)load("glCreateTextures");
		func->glTextureBuffer = (PFNGLTEXTUREBUFFERPROC)load("glTextureBuffer");
		func->glTextureBufferRange = (PFNGLTEXTUREBUFFERRANGEPROC)load("glTextureBufferRange");
		func->glTextureStorage1D = (PFNGLTEXTURESTORAGE1DPROC)load("glTextureStorage1D");
		func->glTextureStorage2D = (PFNGLTEXTURESTORAGE2DPROC)load("glTextureStorage2D");
		func->glTextureStorage3D = (PFNGLTEXTURESTORAGE3DPROC)load("glTextureStorage3D");
		func->glTextureStorage2DMultisample = (PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC)load("glTextureStorage2DMultisample");
		func->glTextureStorage3DMultisample = (PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC)load("glTextureStorage3DMultisample");
		func->glTextureSubImage1D = (PFNGLTEXTURESUBIMAGE1DPROC)load("glTextureSubImage1D");
		func->glTextureSubImage2D = (PFNGLTEXTURESUBIMAGE2DPROC)load("glTextureSubImage2D");
		func->glTextureSubImage3D = (PFNGLTEXTURESUBIMAGE3DPROC)load("glTextureSubImage3D");
		func->glCompressedTextureSubImage1D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC)load("glCompressedTextureSubImage1D");
		func->glCompressedTextureSubImage2D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC)load("glCompressedTextureSubImage2D");
		func->glCompressedTextureSubImage3D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC)load("glCompressedTextureSubImage3D");
		func->glCopyTextureSubImage1D = (PFNGLCOPYTEXTURESUBIMAGE1DPROC)load("glCopyTextureSubImage1D");
		func->glCopyTextureSubImage2D = (PFNGLCOPYTEXTURESUBIMAGE2DPROC)load("glCopyTextureSubImage2D");
		func->glCopyTextureSubImage3D = (PFNGLCOPYTEXTURESUBIMAGE3DPROC)load("glCopyTextureSubImage3D");
		func->glTextureParameterf = (PFNGLTEXTUREPARAMETERFPROC)load("glTextureParameterf");
		func->glTextureParameterfv = (PFNGLTEXTUREPARAMETERFVPROC)load("glTextureParameterfv");
		func->glTextureParameteri = (PFNGLTEXTUREPARAMETERIPROC)load("glTextureParameteri");
		func->glTextureParameterIiv = (PFNGLTEXTUREPARAMETERIIVPROC)load("glTextureParameterIiv");
		func->glTextureParameterIuiv = (PFNGLTEXTUREPARAMETERIUIVPROC)load("glTextureParameterIuiv");
		func->glTextureParameteriv = (PFNGLTEXTUREPARAMETERIVPROC)load("glTextureParameteriv");
		func->glGenerateTextureMipmap = (PFNGLGENERATETEXTUREMIPMAPPROC)load("glGenerateTextureMipmap");
		func->glBindTextureUnit = (PFNGLBINDTEXTUREUNITPROC)load("glBindTextureUnit");
		func->glGetTextureImage = (PFNGLGETTEXTUREIMAGEPROC)load("glGetTextureImage");
		func->glGetCompressedTextureImage = (PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC)load("glGetCompressedTextureImage");
		func->glGetTextureLevelParameterfv = (PFNGLGETTEXTURELEVELPARAMETERFVPROC)load("glGetTextureLevelParameterfv");
		func->glGetTextureLevelParameteriv = (PFNGLGETTEXTURELEVELPARAMETERIVPROC)load("glGetTextureLevelParameteriv");
		func->glGetTextureParameterfv = (PFNGLGETTEXTUREPARAMETERFVPROC)load("glGetTextureParameterfv");
		func->glGetTextureParameterIiv = (PFNGLGETTEXTUREPARAMETERIIVPROC)load("glGetTextureParameterIiv");
		func->glGetTextureParameterIuiv = (PFNGLGETTEXTUREPARAMETERIUIVPROC)load("glGetTextureParameterIuiv");
		func->glGetTextureParameteriv = (PFNGLGETTEXTUREPARAMETERIVPROC)load("glGetTextureParameteriv");
		func->glCreateVertexArrays = (PFNGLCREATEVERTEXARRAYSPROC)load("glCreateVertexArrays");
		func->glDisableVertexArrayAttrib = (PFNGLDISABLEVERTEXARRAYATTRIBPROC)load("glDisableVertexArrayAttrib");
		func->glEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIBPROC)load("glEnableVertexArrayAttrib");
		func->glVertexArrayElementBuffer = (PFNGLVERTEXARRAYELEMENTBUFFERPROC)load("glVertexArrayElementBuffer");
		func->glVertexArrayVertexBuffer = (PFNGLVERTEXARRAYVERTEXBUFFERPROC)load("glVertexArrayVertexBuffer");
		func->glVertexArrayVertexBuffers = (PFNGLVERTEXARRAYVERTEXBUFFERSPROC)load("glVertexArrayVertexBuffers");
		func->glVertexArrayAttribBinding = (PFNGLVERTEXARRAYATTRIBBINDINGPROC)load("glVertexArrayAttribBinding");
		func->glVertexArrayAttribFormat = (PFNGLVERTEXARRAYATTRIBFORMATPROC)load("glVertexArrayAttribFormat");
		func->glVertexArrayAttribIFormat = (PFNGLVERTEXARRAYATTRIBIFORMATPROC)load("glVertexArrayAttribIFormat");
		func->glVertexArrayAttribLFormat = (PFNGLVERTEXARRAYATTRIBLFORMATPROC)load("glVertexArrayAttribLFormat");
		func->glVertexArrayBindingDivisor = (PFNGLVERTEXARRAYBINDINGDIVISORPROC)load("glVertexArrayBindingDivisor");
		func->glGetVertexArrayiv = (PFNGLGETVERTEXARRAYIVPROC)load("glGetVertexArrayiv");
		func->glGetVertexArrayIndexediv = (PFNGLGETVERTEXARRAYINDEXEDIVPROC)load("glGetVertexArrayIndexediv");
		func->glGetVertexArrayIndexed64iv = (PFNGLGETVERTEXARRAYINDEXED64IVPROC)load("glGetVertexArrayIndexed64iv");
		func->glCreateSamplers = (PFNGLCREATESAMPLERSPROC)load("glCreateSamplers");
		func->glCreateProgramPipelines = (PFNGLCREATEPROGRAMPIPELINESPROC)load("glCreateProgramPipelines");
		func->glCreateQueries = (PFNGLCREATEQUERIESPROC)load("glCreateQueries");
		func->glGetQueryBufferObjecti64v = (PFNGLGETQUERYBUFFEROBJECTI64VPROC)load("glGetQueryBufferObjecti64v");
		func->glGetQueryBufferObjectiv = (PFNGLGETQUERYBUFFEROBJECTIVPROC)load("glGetQueryBufferObjectiv");
		func->glGetQueryBufferObjectui64v = (PFNGLGETQUERYBUFFEROBJECTUI64VPROC)load("glGetQueryBufferObjectui64v");
		func->glGetQueryBufferObjectuiv = (PFNGLGETQUERYBUFFEROBJECTUIVPROC)load("glGetQueryBufferObjectuiv");
		func->glMemoryBarrierByRegion = (PFNGLMEMORYBARRIERBYREGIONPROC)load("glMemoryBarrierByRegion");
		func->glGetTextureSubImage = (PFNGLGETTEXTURESUBIMAGEPROC)load("glGetTextureSubImage");
		func->glGetCompressedTextureSubImage = (PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC)load("glGetCompressedTextureSubImage");
		func->glGetGraphicsResetStatus = (PFNGLGETGRAPHICSRESETSTATUSPROC)load("glGetGraphicsResetStatus");
		func->glGetnCompressedTexImage = (PFNGLGETNCOMPRESSEDTEXIMAGEPROC)load("glGetnCompressedTexImage");
		func->glGetnTexImage = (PFNGLGETNTEXIMAGEPROC)load("glGetnTexImage");
		func->glGetnUniformdv = (PFNGLGETNUNIFORMDVPROC)load("glGetnUniformdv");
		func->glGetnUniformfv = (PFNGLGETNUNIFORMFVPROC)load("glGetnUniformfv");
		func->glGetnUniformiv = (PFNGLGETNUNIFORMIVPROC)load("glGetnUniformiv");
		func->glGetnUniformuiv = (PFNGLGETNUNIFORMUIVPROC)load("glGetnUniformuiv");
		func->glReadnPixels = (PFNGLREADNPIXELSPROC)load("glReadnPixels");
		func->glTextureBarrier = (PFNGLTEXTUREBARRIERPROC)load("glTextureBarrier");
	}
	static void load_GL_4_6(LoadProc load, GLFunctions* func)
    {
		func->glSpecializeShader = (PFNGLSPECIALIZESHADERPROC)load("glSpecializeShader");
		func->glMultiDrawArraysIndirectCount = (PFNGLMULTIDRAWARRAYSINDIRECTCOUNTPROC)load("glMultiDrawArraysIndirectCount");
		func->glMultiDrawElementsIndirectCount = (PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTPROC)load("glMultiDrawElementsIndirectCount");
		func->glPolygonOffsetClamp = (PFNGLPOLYGONOFFSETCLAMPPROC)load("glPolygonOffsetClamp");
	}
	static GLFunctions* load_GL_funcs(int major, int minor)
	{
		GLFunctions* func = new GLFunctions;

#define LOAD_GL_FUNC(maj, min) if ((major == maj && minor >= min) || major > maj) load_GL_##maj##_##min(get_proc, func)

		LOAD_GL_FUNC(1, 0);
		LOAD_GL_FUNC(1, 1);
		LOAD_GL_FUNC(1, 2);
		LOAD_GL_FUNC(1, 3);
		LOAD_GL_FUNC(1, 4);
		LOAD_GL_FUNC(1, 5);
		LOAD_GL_FUNC(2, 0);
		LOAD_GL_FUNC(2, 1);
		LOAD_GL_FUNC(3, 0);
		LOAD_GL_FUNC(3, 1);
		LOAD_GL_FUNC(3, 2);
		LOAD_GL_FUNC(3, 3);
		LOAD_GL_FUNC(4, 0);
		LOAD_GL_FUNC(4, 1);
		LOAD_GL_FUNC(4, 2);
		LOAD_GL_FUNC(4, 3);
		LOAD_GL_FUNC(4, 4);
		LOAD_GL_FUNC(4, 5);
		LOAD_GL_FUNC(4, 6);

#undef LOAD_GL_FUNC

		return func;
	}

	
	static void load_GL_3DFX_tbuffer(LoadProc load, GLExtFunctions* func)
    {
		func->glTbufferMask3DFX = (PFNGLTBUFFERMASK3DFXPROC)load("glTbufferMask3DFX");
	}
	static void load_GL_AMD_debug_output(LoadProc load, GLExtFunctions* func)
    {
	    func->glDebugMessageEnableAMD = (PFNGLDEBUGMESSAGEENABLEAMDPROC)load("glDebugMessageEnableAMD");
		func->glDebugMessageInsertAMD = (PFNGLDEBUGMESSAGEINSERTAMDPROC)load("glDebugMessageInsertAMD");
		func->glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)load("glDebugMessageCallbackAMD");
		func->glGetDebugMessageLogAMD = (PFNGLGETDEBUGMESSAGELOGAMDPROC)load("glGetDebugMessageLogAMD");
	}
	static void load_GL_AMD_draw_buffers_blend(LoadProc load, GLExtFunctions* func)
    {
		func->glBlendFuncIndexedAMD = (PFNGLBLENDFUNCINDEXEDAMDPROC)load("glBlendFuncIndexedAMD");
		func->glBlendFuncSeparateIndexedAMD = (PFNGLBLENDFUNCSEPARATEINDEXEDAMDPROC)load("glBlendFuncSeparateIndexedAMD");
		func->glBlendEquationIndexedAMD = (PFNGLBLENDEQUATIONINDEXEDAMDPROC)load("glBlendEquationIndexedAMD");
		func->glBlendEquationSeparateIndexedAMD = (PFNGLBLENDEQUATIONSEPARATEINDEXEDAMDPROC)load("glBlendEquationSeparateIndexedAMD");
	}
	static void load_GL_AMD_framebuffer_multisample_advanced(LoadProc load, GLExtFunctions* func)
    {
		func->glRenderbufferStorageMultisampleAdvancedAMD = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC)load("glRenderbufferStorageMultisampleAdvancedAMD");
		func->glNamedRenderbufferStorageMultisampleAdvancedAMD = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC)load("glNamedRenderbufferStorageMultisampleAdvancedAMD");
	}
	static void load_GL_AMD_framebuffer_sample_positions(LoadProc load, GLExtFunctions* func)
    {
		func->glFramebufferSamplePositionsfvAMD = (PFNGLFRAMEBUFFERSAMPLEPOSITIONSFVAMDPROC)load("glFramebufferSamplePositionsfvAMD");
		func->glNamedFramebufferSamplePositionsfvAMD = (PFNGLNAMEDFRAMEBUFFERSAMPLEPOSITIONSFVAMDPROC)load("glNamedFramebufferSamplePositionsfvAMD");
		func->glGetFramebufferParameterfvAMD = (PFNGLGETFRAMEBUFFERPARAMETERFVAMDPROC)load("glGetFramebufferParameterfvAMD");
		func->glGetNamedFramebufferParameterfvAMD = (PFNGLGETNAMEDFRAMEBUFFERPARAMETERFVAMDPROC)load("glGetNamedFramebufferParameterfvAMD");
	}
	static void load_GL_AMD_gpu_shader_int64(LoadProc load, GLExtFunctions* func)
    {
		func->glUniform1i64NV = (PFNGLUNIFORM1I64NVPROC)load("glUniform1i64NV");
		func->glUniform2i64NV = (PFNGLUNIFORM2I64NVPROC)load("glUniform2i64NV");
		func->glUniform3i64NV = (PFNGLUNIFORM3I64NVPROC)load("glUniform3i64NV");
		func->glUniform4i64NV = (PFNGLUNIFORM4I64NVPROC)load("glUniform4i64NV");
		func->glUniform1i64vNV = (PFNGLUNIFORM1I64VNVPROC)load("glUniform1i64vNV");
		func->glUniform2i64vNV = (PFNGLUNIFORM2I64VNVPROC)load("glUniform2i64vNV");
		func->glUniform3i64vNV = (PFNGLUNIFORM3I64VNVPROC)load("glUniform3i64vNV");
		func->glUniform4i64vNV = (PFNGLUNIFORM4I64VNVPROC)load("glUniform4i64vNV");
		func->glUniform1ui64NV = (PFNGLUNIFORM1UI64NVPROC)load("glUniform1ui64NV");
		func->glUniform2ui64NV = (PFNGLUNIFORM2UI64NVPROC)load("glUniform2ui64NV");
		func->glUniform3ui64NV = (PFNGLUNIFORM3UI64NVPROC)load("glUniform3ui64NV");
		func->glUniform4ui64NV = (PFNGLUNIFORM4UI64NVPROC)load("glUniform4ui64NV");
		func->glUniform1ui64vNV = (PFNGLUNIFORM1UI64VNVPROC)load("glUniform1ui64vNV");
		func->glUniform2ui64vNV = (PFNGLUNIFORM2UI64VNVPROC)load("glUniform2ui64vNV");
		func->glUniform3ui64vNV = (PFNGLUNIFORM3UI64VNVPROC)load("glUniform3ui64vNV");
		func->glUniform4ui64vNV = (PFNGLUNIFORM4UI64VNVPROC)load("glUniform4ui64vNV");
		func->glGetUniformi64vNV = (PFNGLGETUNIFORMI64VNVPROC)load("glGetUniformi64vNV");
		func->glGetUniformui64vNV = (PFNGLGETUNIFORMUI64VNVPROC)load("glGetUniformui64vNV");
		func->glProgramUniform1i64NV = (PFNGLPROGRAMUNIFORM1I64NVPROC)load("glProgramUniform1i64NV");
		func->glProgramUniform2i64NV = (PFNGLPROGRAMUNIFORM2I64NVPROC)load("glProgramUniform2i64NV");
		func->glProgramUniform3i64NV = (PFNGLPROGRAMUNIFORM3I64NVPROC)load("glProgramUniform3i64NV");
		func->glProgramUniform4i64NV = (PFNGLPROGRAMUNIFORM4I64NVPROC)load("glProgramUniform4i64NV");
		func->glProgramUniform1i64vNV = (PFNGLPROGRAMUNIFORM1I64VNVPROC)load("glProgramUniform1i64vNV");
		func->glProgramUniform2i64vNV = (PFNGLPROGRAMUNIFORM2I64VNVPROC)load("glProgramUniform2i64vNV");
		func->glProgramUniform3i64vNV = (PFNGLPROGRAMUNIFORM3I64VNVPROC)load("glProgramUniform3i64vNV");
		func->glProgramUniform4i64vNV = (PFNGLPROGRAMUNIFORM4I64VNVPROC)load("glProgramUniform4i64vNV");
		func->glProgramUniform1ui64NV = (PFNGLPROGRAMUNIFORM1UI64NVPROC)load("glProgramUniform1ui64NV");
		func->glProgramUniform2ui64NV = (PFNGLPROGRAMUNIFORM2UI64NVPROC)load("glProgramUniform2ui64NV");
		func->glProgramUniform3ui64NV = (PFNGLPROGRAMUNIFORM3UI64NVPROC)load("glProgramUniform3ui64NV");
		func->glProgramUniform4ui64NV = (PFNGLPROGRAMUNIFORM4UI64NVPROC)load("glProgramUniform4ui64NV");
		func->glProgramUniform1ui64vNV = (PFNGLPROGRAMUNIFORM1UI64VNVPROC)load("glProgramUniform1ui64vNV");
		func->glProgramUniform2ui64vNV = (PFNGLPROGRAMUNIFORM2UI64VNVPROC)load("glProgramUniform2ui64vNV");
		func->glProgramUniform3ui64vNV = (PFNGLPROGRAMUNIFORM3UI64VNVPROC)load("glProgramUniform3ui64vNV");
		func->glProgramUniform4ui64vNV = (PFNGLPROGRAMUNIFORM4UI64VNVPROC)load("glProgramUniform4ui64vNV");
	}
	static void load_GL_AMD_interleaved_elements(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexAttribParameteriAMD = (PFNGLVERTEXATTRIBPARAMETERIAMDPROC)load("glVertexAttribParameteriAMD");
	}
	static void load_GL_AMD_multi_draw_indirect(LoadProc load, GLExtFunctions* func)
    {
		func->glMultiDrawArraysIndirectAMD = (PFNGLMULTIDRAWARRAYSINDIRECTAMDPROC)load("glMultiDrawArraysIndirectAMD");
		func->glMultiDrawElementsIndirectAMD = (PFNGLMULTIDRAWELEMENTSINDIRECTAMDPROC)load("glMultiDrawElementsIndirectAMD");
	}
	static void load_GL_AMD_name_gen_delete(LoadProc load, GLExtFunctions* func)
    {
		func->glGenNamesAMD = (PFNGLGENNAMESAMDPROC)load("glGenNamesAMD");
		func->glDeleteNamesAMD = (PFNGLDELETENAMESAMDPROC)load("glDeleteNamesAMD");
		func->glIsNameAMD = (PFNGLISNAMEAMDPROC)load("glIsNameAMD");
	}
	static void load_GL_AMD_occlusion_query_event(LoadProc load, GLExtFunctions* func)
    {
		func->glQueryObjectParameteruiAMD = (PFNGLQUERYOBJECTPARAMETERUIAMDPROC)load("glQueryObjectParameteruiAMD");
	}
	static void load_GL_AMD_performance_monitor(LoadProc load, GLExtFunctions* func)
    {
		func->glGetPerfMonitorGroupsAMD = (PFNGLGETPERFMONITORGROUPSAMDPROC)load("glGetPerfMonitorGroupsAMD");
		func->glGetPerfMonitorCountersAMD = (PFNGLGETPERFMONITORCOUNTERSAMDPROC)load("glGetPerfMonitorCountersAMD");
		func->glGetPerfMonitorGroupStringAMD = (PFNGLGETPERFMONITORGROUPSTRINGAMDPROC)load("glGetPerfMonitorGroupStringAMD");
		func->glGetPerfMonitorCounterStringAMD = (PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC)load("glGetPerfMonitorCounterStringAMD");
		func->glGetPerfMonitorCounterInfoAMD = (PFNGLGETPERFMONITORCOUNTERINFOAMDPROC)load("glGetPerfMonitorCounterInfoAMD");
		func->glGenPerfMonitorsAMD = (PFNGLGENPERFMONITORSAMDPROC)load("glGenPerfMonitorsAMD");
		func->glDeletePerfMonitorsAMD = (PFNGLDELETEPERFMONITORSAMDPROC)load("glDeletePerfMonitorsAMD");
		func->glSelectPerfMonitorCountersAMD = (PFNGLSELECTPERFMONITORCOUNTERSAMDPROC)load("glSelectPerfMonitorCountersAMD");
		func->glBeginPerfMonitorAMD = (PFNGLBEGINPERFMONITORAMDPROC)load("glBeginPerfMonitorAMD");
		func->glEndPerfMonitorAMD = (PFNGLENDPERFMONITORAMDPROC)load("glEndPerfMonitorAMD");
		func->glGetPerfMonitorCounterDataAMD = (PFNGLGETPERFMONITORCOUNTERDATAAMDPROC)load("glGetPerfMonitorCounterDataAMD");
	}
	static void load_GL_AMD_sample_positions(LoadProc load, GLExtFunctions* func)
    {
		func->glSetMultisamplefvAMD = (PFNGLSETMULTISAMPLEFVAMDPROC)load("glSetMultisamplefvAMD");
	}
	static void load_GL_AMD_sparse_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glTexStorageSparseAMD = (PFNGLTEXSTORAGESPARSEAMDPROC)load("glTexStorageSparseAMD");
		func->glTextureStorageSparseAMD = (PFNGLTEXTURESTORAGESPARSEAMDPROC)load("glTextureStorageSparseAMD");
	}
	static void load_GL_AMD_stencil_operation_extended(LoadProc load, GLExtFunctions* func)
    {
		func->glStencilOpValueAMD = (PFNGLSTENCILOPVALUEAMDPROC)load("glStencilOpValueAMD");
	}
	static void load_GL_AMD_vertex_shader_tessellator(LoadProc load, GLExtFunctions* func)
    {
		func->glTessellationFactorAMD = (PFNGLTESSELLATIONFACTORAMDPROC)load("glTessellationFactorAMD");
		func->glTessellationModeAMD = (PFNGLTESSELLATIONMODEAMDPROC)load("glTessellationModeAMD");
	}
	static void load_GL_APPLE_element_array(LoadProc load, GLExtFunctions* func)
    {
		func->glElementPointerAPPLE = (PFNGLELEMENTPOINTERAPPLEPROC)load("glElementPointerAPPLE");
		func->glDrawElementArrayAPPLE = (PFNGLDRAWELEMENTARRAYAPPLEPROC)load("glDrawElementArrayAPPLE");
		func->glDrawRangeElementArrayAPPLE = (PFNGLDRAWRANGEELEMENTARRAYAPPLEPROC)load("glDrawRangeElementArrayAPPLE");
		func->glMultiDrawElementArrayAPPLE = (PFNGLMULTIDRAWELEMENTARRAYAPPLEPROC)load("glMultiDrawElementArrayAPPLE");
		func->glMultiDrawRangeElementArrayAPPLE = (PFNGLMULTIDRAWRANGEELEMENTARRAYAPPLEPROC)load("glMultiDrawRangeElementArrayAPPLE");
	}
	static void load_GL_APPLE_fence(LoadProc load, GLExtFunctions* func)
    {
		func->glGenFencesAPPLE = (PFNGLGENFENCESAPPLEPROC)load("glGenFencesAPPLE");
		func->glDeleteFencesAPPLE = (PFNGLDELETEFENCESAPPLEPROC)load("glDeleteFencesAPPLE");
		func->glSetFenceAPPLE = (PFNGLSETFENCEAPPLEPROC)load("glSetFenceAPPLE");
		func->glIsFenceAPPLE = (PFNGLISFENCEAPPLEPROC)load("glIsFenceAPPLE");
		func->glTestFenceAPPLE = (PFNGLTESTFENCEAPPLEPROC)load("glTestFenceAPPLE");
		func->glFinishFenceAPPLE = (PFNGLFINISHFENCEAPPLEPROC)load("glFinishFenceAPPLE");
		func->glTestObjectAPPLE = (PFNGLTESTOBJECTAPPLEPROC)load("glTestObjectAPPLE");
		func->glFinishObjectAPPLE = (PFNGLFINISHOBJECTAPPLEPROC)load("glFinishObjectAPPLE");
	}
	static void load_GL_APPLE_flush_buffer_range(LoadProc load, GLExtFunctions* func)
    {
		func->glBufferParameteriAPPLE = (PFNGLBUFFERPARAMETERIAPPLEPROC)load("glBufferParameteriAPPLE");
		func->glFlushMappedBufferRangeAPPLE = (PFNGLFLUSHMAPPEDBUFFERRANGEAPPLEPROC)load("glFlushMappedBufferRangeAPPLE");
	}
	static void load_GL_APPLE_object_purgeable(LoadProc load, GLExtFunctions* func)
    {
		func->glObjectPurgeableAPPLE = (PFNGLOBJECTPURGEABLEAPPLEPROC)load("glObjectPurgeableAPPLE");
		func->glObjectUnpurgeableAPPLE = (PFNGLOBJECTUNPURGEABLEAPPLEPROC)load("glObjectUnpurgeableAPPLE");
		func->glGetObjectParameterivAPPLE = (PFNGLGETOBJECTPARAMETERIVAPPLEPROC)load("glGetObjectParameterivAPPLE");
	}
	static void load_GL_APPLE_texture_range(LoadProc load, GLExtFunctions* func)
    {
		func->glTextureRangeAPPLE = (PFNGLTEXTURERANGEAPPLEPROC)load("glTextureRangeAPPLE");
		func->glGetTexParameterPointervAPPLE = (PFNGLGETTEXPARAMETERPOINTERVAPPLEPROC)load("glGetTexParameterPointervAPPLE");
	}
	static void load_GL_APPLE_vertex_array_object(LoadProc load, GLExtFunctions* func)
    {
		func->glBindVertexArrayAPPLE = (PFNGLBINDVERTEXARRAYAPPLEPROC)load("glBindVertexArrayAPPLE");
		func->glDeleteVertexArraysAPPLE = (PFNGLDELETEVERTEXARRAYSAPPLEPROC)load("glDeleteVertexArraysAPPLE");
		func->glGenVertexArraysAPPLE = (PFNGLGENVERTEXARRAYSAPPLEPROC)load("glGenVertexArraysAPPLE");
		func->glIsVertexArrayAPPLE = (PFNGLISVERTEXARRAYAPPLEPROC)load("glIsVertexArrayAPPLE");
	}
	static void load_GL_APPLE_vertex_array_range(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexArrayRangeAPPLE = (PFNGLVERTEXARRAYRANGEAPPLEPROC)load("glVertexArrayRangeAPPLE");
		func->glFlushVertexArrayRangeAPPLE = (PFNGLFLUSHVERTEXARRAYRANGEAPPLEPROC)load("glFlushVertexArrayRangeAPPLE");
		func->glVertexArrayParameteriAPPLE = (PFNGLVERTEXARRAYPARAMETERIAPPLEPROC)load("glVertexArrayParameteriAPPLE");
	}
	static void load_GL_APPLE_vertex_program_evaluators(LoadProc load, GLExtFunctions* func)
    {
		func->glEnableVertexAttribAPPLE = (PFNGLENABLEVERTEXATTRIBAPPLEPROC)load("glEnableVertexAttribAPPLE");
		func->glDisableVertexAttribAPPLE = (PFNGLDISABLEVERTEXATTRIBAPPLEPROC)load("glDisableVertexAttribAPPLE");
		func->glIsVertexAttribEnabledAPPLE = (PFNGLISVERTEXATTRIBENABLEDAPPLEPROC)load("glIsVertexAttribEnabledAPPLE");
		func->glMapVertexAttrib1dAPPLE = (PFNGLMAPVERTEXATTRIB1DAPPLEPROC)load("glMapVertexAttrib1dAPPLE");
		func->glMapVertexAttrib1fAPPLE = (PFNGLMAPVERTEXATTRIB1FAPPLEPROC)load("glMapVertexAttrib1fAPPLE");
		func->glMapVertexAttrib2dAPPLE = (PFNGLMAPVERTEXATTRIB2DAPPLEPROC)load("glMapVertexAttrib2dAPPLE");
		func->glMapVertexAttrib2fAPPLE = (PFNGLMAPVERTEXATTRIB2FAPPLEPROC)load("glMapVertexAttrib2fAPPLE");
	}
	static void load_GL_ARB_ES2_compatibility(LoadProc load, GLFunctions* func)
    {
		func->glReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC)load("glReleaseShaderCompiler");
		func->glShaderBinary = (PFNGLSHADERBINARYPROC)load("glShaderBinary");
		func->glGetShaderPrecisionFormat = (PFNGLGETSHADERPRECISIONFORMATPROC)load("glGetShaderPrecisionFormat");
		func->glDepthRangef = (PFNGLDEPTHRANGEFPROC)load("glDepthRangef");
		func->glClearDepthf = (PFNGLCLEARDEPTHFPROC)load("glClearDepthf");
	}
	static void load_GL_ARB_ES3_1_compatibility(LoadProc load, GLFunctions* func)
    {
		func->glMemoryBarrierByRegion = (PFNGLMEMORYBARRIERBYREGIONPROC)load("glMemoryBarrierByRegion");
	}
	static void load_GL_ARB_ES3_2_compatibility(LoadProc load, GLExtFunctions* func)
    {
		func->glPrimitiveBoundingBoxARB = (PFNGLPRIMITIVEBOUNDINGBOXARBPROC)load("glPrimitiveBoundingBoxARB");
	}
	static void load_GL_ARB_base_instance(LoadProc load, GLFunctions* func)
    {
		func->glDrawArraysInstancedBaseInstance = (PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC)load("glDrawArraysInstancedBaseInstance");
		func->glDrawElementsInstancedBaseInstance = (PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC)load("glDrawElementsInstancedBaseInstance");
		func->glDrawElementsInstancedBaseVertexBaseInstance = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC)load("glDrawElementsInstancedBaseVertexBaseInstance");
	}
	static void load_GL_ARB_bindless_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glGetTextureHandleARB = (PFNGLGETTEXTUREHANDLEARBPROC)load("glGetTextureHandleARB");
		func->glGetTextureSamplerHandleARB = (PFNGLGETTEXTURESAMPLERHANDLEARBPROC)load("glGetTextureSamplerHandleARB");
		func->glMakeTextureHandleResidentARB = (PFNGLMAKETEXTUREHANDLERESIDENTARBPROC)load("glMakeTextureHandleResidentARB");
		func->glMakeTextureHandleNonResidentARB = (PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC)load("glMakeTextureHandleNonResidentARB");
		func->glGetImageHandleARB = (PFNGLGETIMAGEHANDLEARBPROC)load("glGetImageHandleARB");
		func->glMakeImageHandleResidentARB = (PFNGLMAKEIMAGEHANDLERESIDENTARBPROC)load("glMakeImageHandleResidentARB");
		func->glMakeImageHandleNonResidentARB = (PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC)load("glMakeImageHandleNonResidentARB");
		func->glUniformHandleui64ARB = (PFNGLUNIFORMHANDLEUI64ARBPROC)load("glUniformHandleui64ARB");
		func->glUniformHandleui64vARB = (PFNGLUNIFORMHANDLEUI64VARBPROC)load("glUniformHandleui64vARB");
		func->glProgramUniformHandleui64ARB = (PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC)load("glProgramUniformHandleui64ARB");
		func->glProgramUniformHandleui64vARB = (PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC)load("glProgramUniformHandleui64vARB");
		func->glIsTextureHandleResidentARB = (PFNGLISTEXTUREHANDLERESIDENTARBPROC)load("glIsTextureHandleResidentARB");
		func->glIsImageHandleResidentARB = (PFNGLISIMAGEHANDLERESIDENTARBPROC)load("glIsImageHandleResidentARB");
		func->glVertexAttribL1ui64ARB = (PFNGLVERTEXATTRIBL1UI64ARBPROC)load("glVertexAttribL1ui64ARB");
		func->glVertexAttribL1ui64vARB = (PFNGLVERTEXATTRIBL1UI64VARBPROC)load("glVertexAttribL1ui64vARB");
		func->glGetVertexAttribLui64vARB = (PFNGLGETVERTEXATTRIBLUI64VARBPROC)load("glGetVertexAttribLui64vARB");
	}
	static void load_GL_ARB_blend_func_extended(LoadProc load, GLFunctions* func)
    {
		func->glBindFragDataLocationIndexed = (PFNGLBINDFRAGDATALOCATIONINDEXEDPROC)load("glBindFragDataLocationIndexed");
		func->glGetFragDataIndex = (PFNGLGETFRAGDATAINDEXPROC)load("glGetFragDataIndex");
	}
	static void load_GL_ARB_buffer_storage(LoadProc load, GLFunctions* func)
    {
		func->glBufferStorage = (PFNGLBUFFERSTORAGEPROC)load("glBufferStorage");
	}
	static void load_GL_ARB_cl_event(LoadProc load, GLExtFunctions* func)
    {
		func->glCreateSyncFromCLeventARB = (PFNGLCREATESYNCFROMCLEVENTARBPROC)load("glCreateSyncFromCLeventARB");
	}
	static void load_GL_ARB_clear_buffer_object(LoadProc load, GLFunctions* func)
    {
		func->glClearBufferData = (PFNGLCLEARBUFFERDATAPROC)load("glClearBufferData");
		func->glClearBufferSubData = (PFNGLCLEARBUFFERSUBDATAPROC)load("glClearBufferSubData");
	}
	static void load_GL_ARB_clear_texture(LoadProc load, GLFunctions* func)
    {
		func->glClearTexImage = (PFNGLCLEARTEXIMAGEPROC)load("glClearTexImage");
		func->glClearTexSubImage = (PFNGLCLEARTEXSUBIMAGEPROC)load("glClearTexSubImage");
	}
	static void load_GL_ARB_clip_control(LoadProc load, GLFunctions* func)
    {
		func->glClipControl = (PFNGLCLIPCONTROLPROC)load("glClipControl");
	}
	static void load_GL_ARB_color_buffer_float(LoadProc load, GLExtFunctions* func)
    {
		func->glClampColorARB = (PFNGLCLAMPCOLORARBPROC)load("glClampColorARB");
	}
	static void load_GL_ARB_compute_shader(LoadProc load, GLFunctions* func)
    {
		func->glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)load("glDispatchCompute");
		func->glDispatchComputeIndirect = (PFNGLDISPATCHCOMPUTEINDIRECTPROC)load("glDispatchComputeIndirect");
	}
	static void load_GL_ARB_compute_variable_group_size(LoadProc load, GLExtFunctions* func)
    {
		func->glDispatchComputeGroupSizeARB = (PFNGLDISPATCHCOMPUTEGROUPSIZEARBPROC)load("glDispatchComputeGroupSizeARB");
	}
	static void load_GL_ARB_copy_buffer(LoadProc load, GLFunctions* func)
    {
		func->glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)load("glCopyBufferSubData");
	}
	static void load_GL_ARB_copy_image(LoadProc load, GLFunctions* func)
    {
		func->glCopyImageSubData = (PFNGLCOPYIMAGESUBDATAPROC)load("glCopyImageSubData");
	}
	static void load_GL_ARB_debug_output(LoadProc load, GLExtFunctions* func)
    {
		func->glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARBPROC)load("glDebugMessageControlARB");
		func->glDebugMessageInsertARB = (PFNGLDEBUGMESSAGEINSERTARBPROC)load("glDebugMessageInsertARB");
		func->glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)load("glDebugMessageCallbackARB");
		func->glGetDebugMessageLogARB = (PFNGLGETDEBUGMESSAGELOGARBPROC)load("glGetDebugMessageLogARB");
	}
	static void load_GL_ARB_direct_state_access(LoadProc load, GLFunctions* func)
    {
		func->glCreateTransformFeedbacks = (PFNGLCREATETRANSFORMFEEDBACKSPROC)load("glCreateTransformFeedbacks");
		func->glTransformFeedbackBufferBase = (PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC)load("glTransformFeedbackBufferBase");
		func->glTransformFeedbackBufferRange = (PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC)load("glTransformFeedbackBufferRange");
		func->glGetTransformFeedbackiv = (PFNGLGETTRANSFORMFEEDBACKIVPROC)load("glGetTransformFeedbackiv");
		func->glGetTransformFeedbacki_v = (PFNGLGETTRANSFORMFEEDBACKI_VPROC)load("glGetTransformFeedbacki_v");
		func->glGetTransformFeedbacki64_v = (PFNGLGETTRANSFORMFEEDBACKI64_VPROC)load("glGetTransformFeedbacki64_v");
		func->glCreateBuffers = (PFNGLCREATEBUFFERSPROC)load("glCreateBuffers");
		func->glNamedBufferStorage = (PFNGLNAMEDBUFFERSTORAGEPROC)load("glNamedBufferStorage");
		func->glNamedBufferData = (PFNGLNAMEDBUFFERDATAPROC)load("glNamedBufferData");
		func->glNamedBufferSubData = (PFNGLNAMEDBUFFERSUBDATAPROC)load("glNamedBufferSubData");
		func->glCopyNamedBufferSubData = (PFNGLCOPYNAMEDBUFFERSUBDATAPROC)load("glCopyNamedBufferSubData");
		func->glClearNamedBufferData = (PFNGLCLEARNAMEDBUFFERDATAPROC)load("glClearNamedBufferData");
		func->glClearNamedBufferSubData = (PFNGLCLEARNAMEDBUFFERSUBDATAPROC)load("glClearNamedBufferSubData");
		func->glMapNamedBuffer = (PFNGLMAPNAMEDBUFFERPROC)load("glMapNamedBuffer");
		func->glMapNamedBufferRange = (PFNGLMAPNAMEDBUFFERRANGEPROC)load("glMapNamedBufferRange");
		func->glUnmapNamedBuffer = (PFNGLUNMAPNAMEDBUFFERPROC)load("glUnmapNamedBuffer");
		func->glFlushMappedNamedBufferRange = (PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC)load("glFlushMappedNamedBufferRange");
		func->glGetNamedBufferParameteriv = (PFNGLGETNAMEDBUFFERPARAMETERIVPROC)load("glGetNamedBufferParameteriv");
		func->glGetNamedBufferParameteri64v = (PFNGLGETNAMEDBUFFERPARAMETERI64VPROC)load("glGetNamedBufferParameteri64v");
		func->glGetNamedBufferPointerv = (PFNGLGETNAMEDBUFFERPOINTERVPROC)load("glGetNamedBufferPointerv");
		func->glGetNamedBufferSubData = (PFNGLGETNAMEDBUFFERSUBDATAPROC)load("glGetNamedBufferSubData");
		func->glCreateFramebuffers = (PFNGLCREATEFRAMEBUFFERSPROC)load("glCreateFramebuffers");
		func->glNamedFramebufferRenderbuffer = (PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC)load("glNamedFramebufferRenderbuffer");
		func->glNamedFramebufferParameteri = (PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC)load("glNamedFramebufferParameteri");
		func->glNamedFramebufferTexture = (PFNGLNAMEDFRAMEBUFFERTEXTUREPROC)load("glNamedFramebufferTexture");
		func->glNamedFramebufferTextureLayer = (PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC)load("glNamedFramebufferTextureLayer");
		func->glNamedFramebufferDrawBuffer = (PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC)load("glNamedFramebufferDrawBuffer");
		func->glNamedFramebufferDrawBuffers = (PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC)load("glNamedFramebufferDrawBuffers");
		func->glNamedFramebufferReadBuffer = (PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC)load("glNamedFramebufferReadBuffer");
		func->glInvalidateNamedFramebufferData = (PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC)load("glInvalidateNamedFramebufferData");
		func->glInvalidateNamedFramebufferSubData = (PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC)load("glInvalidateNamedFramebufferSubData");
		func->glClearNamedFramebufferiv = (PFNGLCLEARNAMEDFRAMEBUFFERIVPROC)load("glClearNamedFramebufferiv");
		func->glClearNamedFramebufferuiv = (PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC)load("glClearNamedFramebufferuiv");
		func->glClearNamedFramebufferfv = (PFNGLCLEARNAMEDFRAMEBUFFERFVPROC)load("glClearNamedFramebufferfv");
		func->glClearNamedFramebufferfi = (PFNGLCLEARNAMEDFRAMEBUFFERFIPROC)load("glClearNamedFramebufferfi");
		func->glBlitNamedFramebuffer = (PFNGLBLITNAMEDFRAMEBUFFERPROC)load("glBlitNamedFramebuffer");
		func->glCheckNamedFramebufferStatus = (PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC)load("glCheckNamedFramebufferStatus");
		func->glGetNamedFramebufferParameteriv = (PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC)load("glGetNamedFramebufferParameteriv");
		func->glGetNamedFramebufferAttachmentParameteriv = (PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC)load("glGetNamedFramebufferAttachmentParameteriv");
		func->glCreateRenderbuffers = (PFNGLCREATERENDERBUFFERSPROC)load("glCreateRenderbuffers");
		func->glNamedRenderbufferStorage = (PFNGLNAMEDRENDERBUFFERSTORAGEPROC)load("glNamedRenderbufferStorage");
		func->glNamedRenderbufferStorageMultisample = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC)load("glNamedRenderbufferStorageMultisample");
		func->glGetNamedRenderbufferParameteriv = (PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC)load("glGetNamedRenderbufferParameteriv");
		func->glCreateTextures = (PFNGLCREATETEXTURESPROC)load("glCreateTextures");
		func->glTextureBuffer = (PFNGLTEXTUREBUFFERPROC)load("glTextureBuffer");
		func->glTextureBufferRange = (PFNGLTEXTUREBUFFERRANGEPROC)load("glTextureBufferRange");
		func->glTextureStorage1D = (PFNGLTEXTURESTORAGE1DPROC)load("glTextureStorage1D");
		func->glTextureStorage2D = (PFNGLTEXTURESTORAGE2DPROC)load("glTextureStorage2D");
		func->glTextureStorage3D = (PFNGLTEXTURESTORAGE3DPROC)load("glTextureStorage3D");
		func->glTextureStorage2DMultisample = (PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC)load("glTextureStorage2DMultisample");
		func->glTextureStorage3DMultisample = (PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC)load("glTextureStorage3DMultisample");
		func->glTextureSubImage1D = (PFNGLTEXTURESUBIMAGE1DPROC)load("glTextureSubImage1D");
		func->glTextureSubImage2D = (PFNGLTEXTURESUBIMAGE2DPROC)load("glTextureSubImage2D");
		func->glTextureSubImage3D = (PFNGLTEXTURESUBIMAGE3DPROC)load("glTextureSubImage3D");
		func->glCompressedTextureSubImage1D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC)load("glCompressedTextureSubImage1D");
		func->glCompressedTextureSubImage2D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC)load("glCompressedTextureSubImage2D");
		func->glCompressedTextureSubImage3D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC)load("glCompressedTextureSubImage3D");
		func->glCopyTextureSubImage1D = (PFNGLCOPYTEXTURESUBIMAGE1DPROC)load("glCopyTextureSubImage1D");
		func->glCopyTextureSubImage2D = (PFNGLCOPYTEXTURESUBIMAGE2DPROC)load("glCopyTextureSubImage2D");
		func->glCopyTextureSubImage3D = (PFNGLCOPYTEXTURESUBIMAGE3DPROC)load("glCopyTextureSubImage3D");
		func->glTextureParameterf = (PFNGLTEXTUREPARAMETERFPROC)load("glTextureParameterf");
		func->glTextureParameterfv = (PFNGLTEXTUREPARAMETERFVPROC)load("glTextureParameterfv");
		func->glTextureParameteri = (PFNGLTEXTUREPARAMETERIPROC)load("glTextureParameteri");
		func->glTextureParameterIiv = (PFNGLTEXTUREPARAMETERIIVPROC)load("glTextureParameterIiv");
		func->glTextureParameterIuiv = (PFNGLTEXTUREPARAMETERIUIVPROC)load("glTextureParameterIuiv");
		func->glTextureParameteriv = (PFNGLTEXTUREPARAMETERIVPROC)load("glTextureParameteriv");
		func->glGenerateTextureMipmap = (PFNGLGENERATETEXTUREMIPMAPPROC)load("glGenerateTextureMipmap");
		func->glBindTextureUnit = (PFNGLBINDTEXTUREUNITPROC)load("glBindTextureUnit");
		func->glGetTextureImage = (PFNGLGETTEXTUREIMAGEPROC)load("glGetTextureImage");
		func->glGetCompressedTextureImage = (PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC)load("glGetCompressedTextureImage");
		func->glGetTextureLevelParameterfv = (PFNGLGETTEXTURELEVELPARAMETERFVPROC)load("glGetTextureLevelParameterfv");
		func->glGetTextureLevelParameteriv = (PFNGLGETTEXTURELEVELPARAMETERIVPROC)load("glGetTextureLevelParameteriv");
		func->glGetTextureParameterfv = (PFNGLGETTEXTUREPARAMETERFVPROC)load("glGetTextureParameterfv");
		func->glGetTextureParameterIiv = (PFNGLGETTEXTUREPARAMETERIIVPROC)load("glGetTextureParameterIiv");
		func->glGetTextureParameterIuiv = (PFNGLGETTEXTUREPARAMETERIUIVPROC)load("glGetTextureParameterIuiv");
		func->glGetTextureParameteriv = (PFNGLGETTEXTUREPARAMETERIVPROC)load("glGetTextureParameteriv");
		func->glCreateVertexArrays = (PFNGLCREATEVERTEXARRAYSPROC)load("glCreateVertexArrays");
		func->glDisableVertexArrayAttrib = (PFNGLDISABLEVERTEXARRAYATTRIBPROC)load("glDisableVertexArrayAttrib");
		func->glEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIBPROC)load("glEnableVertexArrayAttrib");
		func->glVertexArrayElementBuffer = (PFNGLVERTEXARRAYELEMENTBUFFERPROC)load("glVertexArrayElementBuffer");
		func->glVertexArrayVertexBuffer = (PFNGLVERTEXARRAYVERTEXBUFFERPROC)load("glVertexArrayVertexBuffer");
		func->glVertexArrayVertexBuffers = (PFNGLVERTEXARRAYVERTEXBUFFERSPROC)load("glVertexArrayVertexBuffers");
		func->glVertexArrayAttribBinding = (PFNGLVERTEXARRAYATTRIBBINDINGPROC)load("glVertexArrayAttribBinding");
		func->glVertexArrayAttribFormat = (PFNGLVERTEXARRAYATTRIBFORMATPROC)load("glVertexArrayAttribFormat");
		func->glVertexArrayAttribIFormat = (PFNGLVERTEXARRAYATTRIBIFORMATPROC)load("glVertexArrayAttribIFormat");
		func->glVertexArrayAttribLFormat = (PFNGLVERTEXARRAYATTRIBLFORMATPROC)load("glVertexArrayAttribLFormat");
		func->glVertexArrayBindingDivisor = (PFNGLVERTEXARRAYBINDINGDIVISORPROC)load("glVertexArrayBindingDivisor");
		func->glGetVertexArrayiv = (PFNGLGETVERTEXARRAYIVPROC)load("glGetVertexArrayiv");
		func->glGetVertexArrayIndexediv = (PFNGLGETVERTEXARRAYINDEXEDIVPROC)load("glGetVertexArrayIndexediv");
		func->glGetVertexArrayIndexed64iv = (PFNGLGETVERTEXARRAYINDEXED64IVPROC)load("glGetVertexArrayIndexed64iv");
		func->glCreateSamplers = (PFNGLCREATESAMPLERSPROC)load("glCreateSamplers");
		func->glCreateProgramPipelines = (PFNGLCREATEPROGRAMPIPELINESPROC)load("glCreateProgramPipelines");
		func->glCreateQueries = (PFNGLCREATEQUERIESPROC)load("glCreateQueries");
		func->glGetQueryBufferObjecti64v = (PFNGLGETQUERYBUFFEROBJECTI64VPROC)load("glGetQueryBufferObjecti64v");
		func->glGetQueryBufferObjectiv = (PFNGLGETQUERYBUFFEROBJECTIVPROC)load("glGetQueryBufferObjectiv");
		func->glGetQueryBufferObjectui64v = (PFNGLGETQUERYBUFFEROBJECTUI64VPROC)load("glGetQueryBufferObjectui64v");
		func->glGetQueryBufferObjectuiv = (PFNGLGETQUERYBUFFEROBJECTUIVPROC)load("glGetQueryBufferObjectuiv");
	}
	static void load_GL_ARB_draw_buffers(LoadProc load, GLExtFunctions* func)
    {
		func->glDrawBuffersARB = (PFNGLDRAWBUFFERSARBPROC)load("glDrawBuffersARB");
	}
	static void load_GL_ARB_draw_buffers_blend(LoadProc load, GLExtFunctions* func)
    {
		func->glBlendEquationiARB = (PFNGLBLENDEQUATIONIARBPROC)load("glBlendEquationiARB");
		func->glBlendEquationSeparateiARB = (PFNGLBLENDEQUATIONSEPARATEIARBPROC)load("glBlendEquationSeparateiARB");
		func->glBlendFunciARB = (PFNGLBLENDFUNCIARBPROC)load("glBlendFunciARB");
		func->glBlendFuncSeparateiARB = (PFNGLBLENDFUNCSEPARATEIARBPROC)load("glBlendFuncSeparateiARB");
	}
	static void load_GL_ARB_draw_elements_base_vertex(LoadProc load, GLFunctions* func)
    {
		func->glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC)load("glDrawElementsBaseVertex");
		func->glDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC)load("glDrawRangeElementsBaseVertex");
		func->glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)load("glDrawElementsInstancedBaseVertex");
		func->glMultiDrawElementsBaseVertex = (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC)load("glMultiDrawElementsBaseVertex");
	}
	static void load_GL_ARB_draw_indirect(LoadProc load, GLFunctions* func)
    {
		func->glDrawArraysIndirect = (PFNGLDRAWARRAYSINDIRECTPROC)load("glDrawArraysIndirect");
		func->glDrawElementsIndirect = (PFNGLDRAWELEMENTSINDIRECTPROC)load("glDrawElementsIndirect");
	}
	static void load_GL_ARB_draw_instanced(LoadProc load, GLExtFunctions* func)
    {
		func->glDrawArraysInstancedARB = (PFNGLDRAWARRAYSINSTANCEDARBPROC)load("glDrawArraysInstancedARB");
		func->glDrawElementsInstancedARB = (PFNGLDRAWELEMENTSINSTANCEDARBPROC)load("glDrawElementsInstancedARB");
	}
	static void load_GL_ARB_fragment_program(LoadProc load, GLExtFunctions* func)
    {
		func->glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)load("glProgramStringARB");
		func->glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)load("glBindProgramARB");
		func->glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)load("glDeleteProgramsARB");
		func->glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)load("glGenProgramsARB");
		func->glProgramEnvParameter4dARB = (PFNGLPROGRAMENVPARAMETER4DARBPROC)load("glProgramEnvParameter4dARB");
		func->glProgramEnvParameter4dvARB = (PFNGLPROGRAMENVPARAMETER4DVARBPROC)load("glProgramEnvParameter4dvARB");
		func->glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)load("glProgramEnvParameter4fARB");
		func->glProgramEnvParameter4fvARB = (PFNGLPROGRAMENVPARAMETER4FVARBPROC)load("glProgramEnvParameter4fvARB");
		func->glProgramLocalParameter4dARB = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC)load("glProgramLocalParameter4dARB");
		func->glProgramLocalParameter4dvARB = (PFNGLPROGRAMLOCALPARAMETER4DVARBPROC)load("glProgramLocalParameter4dvARB");
		func->glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)load("glProgramLocalParameter4fARB");
		func->glProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)load("glProgramLocalParameter4fvARB");
		func->glGetProgramEnvParameterdvARB = (PFNGLGETPROGRAMENVPARAMETERDVARBPROC)load("glGetProgramEnvParameterdvARB");
		func->glGetProgramEnvParameterfvARB = (PFNGLGETPROGRAMENVPARAMETERFVARBPROC)load("glGetProgramEnvParameterfvARB");
		func->glGetProgramLocalParameterdvARB = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC)load("glGetProgramLocalParameterdvARB");
		func->glGetProgramLocalParameterfvARB = (PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)load("glGetProgramLocalParameterfvARB");
		func->glGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC)load("glGetProgramivARB");
		func->glGetProgramStringARB = (PFNGLGETPROGRAMSTRINGARBPROC)load("glGetProgramStringARB");
		func->glIsProgramARB = (PFNGLISPROGRAMARBPROC)load("glIsProgramARB");
	}
	static void load_GL_ARB_framebuffer_no_attachments(LoadProc load, GLFunctions* func)
    {
		func->glFramebufferParameteri = (PFNGLFRAMEBUFFERPARAMETERIPROC)load("glFramebufferParameteri");
		func->glGetFramebufferParameteriv = (PFNGLGETFRAMEBUFFERPARAMETERIVPROC)load("glGetFramebufferParameteriv");
	}
	static void load_GL_ARB_framebuffer_object(LoadProc load, GLFunctions* func)
    {
		func->glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)load("glIsRenderbuffer");
		func->glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)load("glBindRenderbuffer");
		func->glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)load("glDeleteRenderbuffers");
		func->glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)load("glGenRenderbuffers");
		func->glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)load("glRenderbufferStorage");
		func->glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)load("glGetRenderbufferParameteriv");
		func->glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)load("glIsFramebuffer");
		func->glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)load("glBindFramebuffer");
		func->glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)load("glDeleteFramebuffers");
		func->glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)load("glGenFramebuffers");
		func->glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)load("glCheckFramebufferStatus");
		func->glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)load("glFramebufferTexture1D");
		func->glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)load("glFramebufferTexture2D");
		func->glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)load("glFramebufferTexture3D");
		func->glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)load("glFramebufferRenderbuffer");
		func->glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)load("glGetFramebufferAttachmentParameteriv");
		func->glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)load("glGenerateMipmap");
		func->glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)load("glBlitFramebuffer");
		func->glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)load("glRenderbufferStorageMultisample");
		func->glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)load("glFramebufferTextureLayer");
	}
	static void load_GL_ARB_geometry_shader4(LoadProc load, GLExtFunctions* func)
    {
		func->glProgramParameteriARB = (PFNGLPROGRAMPARAMETERIARBPROC)load("glProgramParameteriARB");
		func->glFramebufferTextureARB = (PFNGLFRAMEBUFFERTEXTUREARBPROC)load("glFramebufferTextureARB");
		func->glFramebufferTextureLayerARB = (PFNGLFRAMEBUFFERTEXTURELAYERARBPROC)load("glFramebufferTextureLayerARB");
		func->glFramebufferTextureFaceARB = (PFNGLFRAMEBUFFERTEXTUREFACEARBPROC)load("glFramebufferTextureFaceARB");
	}
	static void load_GL_ARB_get_program_binary(LoadProc load, GLFunctions* func)
    {
		func->glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC)load("glGetProgramBinary");
		func->glProgramBinary = (PFNGLPROGRAMBINARYPROC)load("glProgramBinary");
		func->glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)load("glProgramParameteri");
	}
	static void load_GL_ARB_get_texture_sub_image(LoadProc load, GLFunctions* func)
    {
		func->glGetTextureSubImage = (PFNGLGETTEXTURESUBIMAGEPROC)load("glGetTextureSubImage");
		func->glGetCompressedTextureSubImage = (PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC)load("glGetCompressedTextureSubImage");
	}
	static void load_GL_ARB_gl_spirv(LoadProc load, GLExtFunctions* func)
    {
		func->glSpecializeShaderARB = (PFNGLSPECIALIZESHADERARBPROC)load("glSpecializeShaderARB");
	}
	static void load_GL_ARB_gpu_shader_fp64(LoadProc load, GLFunctions* func)
    {
		func->glUniform1d = (PFNGLUNIFORM1DPROC)load("glUniform1d");
		func->glUniform2d = (PFNGLUNIFORM2DPROC)load("glUniform2d");
		func->glUniform3d = (PFNGLUNIFORM3DPROC)load("glUniform3d");
		func->glUniform4d = (PFNGLUNIFORM4DPROC)load("glUniform4d");
		func->glUniform1dv = (PFNGLUNIFORM1DVPROC)load("glUniform1dv");
		func->glUniform2dv = (PFNGLUNIFORM2DVPROC)load("glUniform2dv");
		func->glUniform3dv = (PFNGLUNIFORM3DVPROC)load("glUniform3dv");
		func->glUniform4dv = (PFNGLUNIFORM4DVPROC)load("glUniform4dv");
		func->glUniformMatrix2dv = (PFNGLUNIFORMMATRIX2DVPROC)load("glUniformMatrix2dv");
		func->glUniformMatrix3dv = (PFNGLUNIFORMMATRIX3DVPROC)load("glUniformMatrix3dv");
		func->glUniformMatrix4dv = (PFNGLUNIFORMMATRIX4DVPROC)load("glUniformMatrix4dv");
		func->glUniformMatrix2x3dv = (PFNGLUNIFORMMATRIX2X3DVPROC)load("glUniformMatrix2x3dv");
		func->glUniformMatrix2x4dv = (PFNGLUNIFORMMATRIX2X4DVPROC)load("glUniformMatrix2x4dv");
		func->glUniformMatrix3x2dv = (PFNGLUNIFORMMATRIX3X2DVPROC)load("glUniformMatrix3x2dv");
		func->glUniformMatrix3x4dv = (PFNGLUNIFORMMATRIX3X4DVPROC)load("glUniformMatrix3x4dv");
		func->glUniformMatrix4x2dv = (PFNGLUNIFORMMATRIX4X2DVPROC)load("glUniformMatrix4x2dv");
		func->glUniformMatrix4x3dv = (PFNGLUNIFORMMATRIX4X3DVPROC)load("glUniformMatrix4x3dv");
		func->glGetUniformdv = (PFNGLGETUNIFORMDVPROC)load("glGetUniformdv");
	}
	static void load_GL_ARB_gpu_shader_int64(LoadProc load, GLExtFunctions* func)
    {
		func->glUniform1i64ARB = (PFNGLUNIFORM1I64ARBPROC)load("glUniform1i64ARB");
		func->glUniform2i64ARB = (PFNGLUNIFORM2I64ARBPROC)load("glUniform2i64ARB");
		func->glUniform3i64ARB = (PFNGLUNIFORM3I64ARBPROC)load("glUniform3i64ARB");
		func->glUniform4i64ARB = (PFNGLUNIFORM4I64ARBPROC)load("glUniform4i64ARB");
		func->glUniform1i64vARB = (PFNGLUNIFORM1I64VARBPROC)load("glUniform1i64vARB");
		func->glUniform2i64vARB = (PFNGLUNIFORM2I64VARBPROC)load("glUniform2i64vARB");
		func->glUniform3i64vARB = (PFNGLUNIFORM3I64VARBPROC)load("glUniform3i64vARB");
		func->glUniform4i64vARB = (PFNGLUNIFORM4I64VARBPROC)load("glUniform4i64vARB");
		func->glUniform1ui64ARB = (PFNGLUNIFORM1UI64ARBPROC)load("glUniform1ui64ARB");
		func->glUniform2ui64ARB = (PFNGLUNIFORM2UI64ARBPROC)load("glUniform2ui64ARB");
		func->glUniform3ui64ARB = (PFNGLUNIFORM3UI64ARBPROC)load("glUniform3ui64ARB");
		func->glUniform4ui64ARB = (PFNGLUNIFORM4UI64ARBPROC)load("glUniform4ui64ARB");
		func->glUniform1ui64vARB = (PFNGLUNIFORM1UI64VARBPROC)load("glUniform1ui64vARB");
		func->glUniform2ui64vARB = (PFNGLUNIFORM2UI64VARBPROC)load("glUniform2ui64vARB");
		func->glUniform3ui64vARB = (PFNGLUNIFORM3UI64VARBPROC)load("glUniform3ui64vARB");
		func->glUniform4ui64vARB = (PFNGLUNIFORM4UI64VARBPROC)load("glUniform4ui64vARB");
		func->glGetUniformi64vARB = (PFNGLGETUNIFORMI64VARBPROC)load("glGetUniformi64vARB");
		func->glGetUniformui64vARB = (PFNGLGETUNIFORMUI64VARBPROC)load("glGetUniformui64vARB");
		func->glGetnUniformi64vARB = (PFNGLGETNUNIFORMI64VARBPROC)load("glGetnUniformi64vARB");
		func->glGetnUniformui64vARB = (PFNGLGETNUNIFORMUI64VARBPROC)load("glGetnUniformui64vARB");
		func->glProgramUniform1i64ARB = (PFNGLPROGRAMUNIFORM1I64ARBPROC)load("glProgramUniform1i64ARB");
		func->glProgramUniform2i64ARB = (PFNGLPROGRAMUNIFORM2I64ARBPROC)load("glProgramUniform2i64ARB");
		func->glProgramUniform3i64ARB = (PFNGLPROGRAMUNIFORM3I64ARBPROC)load("glProgramUniform3i64ARB");
		func->glProgramUniform4i64ARB = (PFNGLPROGRAMUNIFORM4I64ARBPROC)load("glProgramUniform4i64ARB");
		func->glProgramUniform1i64vARB = (PFNGLPROGRAMUNIFORM1I64VARBPROC)load("glProgramUniform1i64vARB");
		func->glProgramUniform2i64vARB = (PFNGLPROGRAMUNIFORM2I64VARBPROC)load("glProgramUniform2i64vARB");
		func->glProgramUniform3i64vARB = (PFNGLPROGRAMUNIFORM3I64VARBPROC)load("glProgramUniform3i64vARB");
		func->glProgramUniform4i64vARB = (PFNGLPROGRAMUNIFORM4I64VARBPROC)load("glProgramUniform4i64vARB");
		func->glProgramUniform1ui64ARB = (PFNGLPROGRAMUNIFORM1UI64ARBPROC)load("glProgramUniform1ui64ARB");
		func->glProgramUniform2ui64ARB = (PFNGLPROGRAMUNIFORM2UI64ARBPROC)load("glProgramUniform2ui64ARB");
		func->glProgramUniform3ui64ARB = (PFNGLPROGRAMUNIFORM3UI64ARBPROC)load("glProgramUniform3ui64ARB");
		func->glProgramUniform4ui64ARB = (PFNGLPROGRAMUNIFORM4UI64ARBPROC)load("glProgramUniform4ui64ARB");
		func->glProgramUniform1ui64vARB = (PFNGLPROGRAMUNIFORM1UI64VARBPROC)load("glProgramUniform1ui64vARB");
		func->glProgramUniform2ui64vARB = (PFNGLPROGRAMUNIFORM2UI64VARBPROC)load("glProgramUniform2ui64vARB");
		func->glProgramUniform3ui64vARB = (PFNGLPROGRAMUNIFORM3UI64VARBPROC)load("glProgramUniform3ui64vARB");
		func->glProgramUniform4ui64vARB = (PFNGLPROGRAMUNIFORM4UI64VARBPROC)load("glProgramUniform4ui64vARB");
	}
	static void load_GL_ARB_imaging(LoadProc load, GLFunctions* func)
    {
		func->glBlendColor = (PFNGLBLENDCOLORPROC)load("glBlendColor");
		func->glBlendEquation = (PFNGLBLENDEQUATIONPROC)load("glBlendEquation");
	}
	static void load_GL_ARB_indirect_parameters(LoadProc load, GLExtFunctions* func)
    {
		func->glMultiDrawArraysIndirectCountARB = (PFNGLMULTIDRAWARRAYSINDIRECTCOUNTARBPROC)load("glMultiDrawArraysIndirectCountARB");
		func->glMultiDrawElementsIndirectCountARB = (PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTARBPROC)load("glMultiDrawElementsIndirectCountARB");
	}
	static void load_GL_ARB_instanced_arrays(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexAttribDivisorARB = (PFNGLVERTEXATTRIBDIVISORARBPROC)load("glVertexAttribDivisorARB");
	}
	static void load_GL_ARB_internalformat_query(LoadProc load, GLFunctions* func)
    {
		func->glGetInternalformativ = (PFNGLGETINTERNALFORMATIVPROC)load("glGetInternalformativ");
	}
	static void load_GL_ARB_internalformat_query2(LoadProc load, GLFunctions* func)
    {
		func->glGetInternalformati64v = (PFNGLGETINTERNALFORMATI64VPROC)load("glGetInternalformati64v");
	}
	static void load_GL_ARB_invalidate_subdata(LoadProc load, GLFunctions* func)
    {
		func->glInvalidateTexSubImage = (PFNGLINVALIDATETEXSUBIMAGEPROC)load("glInvalidateTexSubImage");
		func->glInvalidateTexImage = (PFNGLINVALIDATETEXIMAGEPROC)load("glInvalidateTexImage");
		func->glInvalidateBufferSubData = (PFNGLINVALIDATEBUFFERSUBDATAPROC)load("glInvalidateBufferSubData");
		func->glInvalidateBufferData = (PFNGLINVALIDATEBUFFERDATAPROC)load("glInvalidateBufferData");
		func->glInvalidateFramebuffer = (PFNGLINVALIDATEFRAMEBUFFERPROC)load("glInvalidateFramebuffer");
		func->glInvalidateSubFramebuffer = (PFNGLINVALIDATESUBFRAMEBUFFERPROC)load("glInvalidateSubFramebuffer");
	}
	static void load_GL_ARB_map_buffer_range(LoadProc load, GLFunctions* func)
    {
		func->glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)load("glMapBufferRange");
		func->glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)load("glFlushMappedBufferRange");
	}
	static void load_GL_ARB_matrix_palette(LoadProc load, GLExtFunctions* func)
    {
		func->glCurrentPaletteMatrixARB = (PFNGLCURRENTPALETTEMATRIXARBPROC)load("glCurrentPaletteMatrixARB");
		func->glMatrixIndexubvARB = (PFNGLMATRIXINDEXUBVARBPROC)load("glMatrixIndexubvARB");
		func->glMatrixIndexusvARB = (PFNGLMATRIXINDEXUSVARBPROC)load("glMatrixIndexusvARB");
		func->glMatrixIndexuivARB = (PFNGLMATRIXINDEXUIVARBPROC)load("glMatrixIndexuivARB");
		func->glMatrixIndexPointerARB = (PFNGLMATRIXINDEXPOINTERARBPROC)load("glMatrixIndexPointerARB");
	}
	static void load_GL_ARB_multi_bind(LoadProc load, GLFunctions* func)
    {
		func->glBindBuffersBase = (PFNGLBINDBUFFERSBASEPROC)load("glBindBuffersBase");
		func->glBindBuffersRange = (PFNGLBINDBUFFERSRANGEPROC)load("glBindBuffersRange");
		func->glBindTextures = (PFNGLBINDTEXTURESPROC)load("glBindTextures");
		func->glBindSamplers = (PFNGLBINDSAMPLERSPROC)load("glBindSamplers");
		func->glBindImageTextures = (PFNGLBINDIMAGETEXTURESPROC)load("glBindImageTextures");
		func->glBindVertexBuffers = (PFNGLBINDVERTEXBUFFERSPROC)load("glBindVertexBuffers");
	}
	static void load_GL_ARB_multi_draw_indirect(LoadProc load, GLFunctions* func)
    {
		func->glMultiDrawArraysIndirect = (PFNGLMULTIDRAWARRAYSINDIRECTPROC)load("glMultiDrawArraysIndirect");
		func->glMultiDrawElementsIndirect = (PFNGLMULTIDRAWELEMENTSINDIRECTPROC)load("glMultiDrawElementsIndirect");
	}
	static void load_GL_ARB_multisample(LoadProc load, GLExtFunctions* func)
    {
		func->glSampleCoverageARB = (PFNGLSAMPLECOVERAGEARBPROC)load("glSampleCoverageARB");
	}
	static void load_GL_ARB_multitexture(LoadProc load, GLExtFunctions* func)
    {
		func->glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)load("glActiveTextureARB");
		func->glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)load("glClientActiveTextureARB");
		func->glMultiTexCoord1dARB = (PFNGLMULTITEXCOORD1DARBPROC)load("glMultiTexCoord1dARB");
		func->glMultiTexCoord1dvARB = (PFNGLMULTITEXCOORD1DVARBPROC)load("glMultiTexCoord1dvARB");
		func->glMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC)load("glMultiTexCoord1fARB");
		func->glMultiTexCoord1fvARB = (PFNGLMULTITEXCOORD1FVARBPROC)load("glMultiTexCoord1fvARB");
		func->glMultiTexCoord1iARB = (PFNGLMULTITEXCOORD1IARBPROC)load("glMultiTexCoord1iARB");
		func->glMultiTexCoord1ivARB = (PFNGLMULTITEXCOORD1IVARBPROC)load("glMultiTexCoord1ivARB");
		func->glMultiTexCoord1sARB = (PFNGLMULTITEXCOORD1SARBPROC)load("glMultiTexCoord1sARB");
		func->glMultiTexCoord1svARB = (PFNGLMULTITEXCOORD1SVARBPROC)load("glMultiTexCoord1svARB");
		func->glMultiTexCoord2dARB = (PFNGLMULTITEXCOORD2DARBPROC)load("glMultiTexCoord2dARB");
		func->glMultiTexCoord2dvARB = (PFNGLMULTITEXCOORD2DVARBPROC)load("glMultiTexCoord2dvARB");
		func->glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)load("glMultiTexCoord2fARB");
		func->glMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVARBPROC)load("glMultiTexCoord2fvARB");
		func->glMultiTexCoord2iARB = (PFNGLMULTITEXCOORD2IARBPROC)load("glMultiTexCoord2iARB");
		func->glMultiTexCoord2ivARB = (PFNGLMULTITEXCOORD2IVARBPROC)load("glMultiTexCoord2ivARB");
		func->glMultiTexCoord2sARB = (PFNGLMULTITEXCOORD2SARBPROC)load("glMultiTexCoord2sARB");
		func->glMultiTexCoord2svARB = (PFNGLMULTITEXCOORD2SVARBPROC)load("glMultiTexCoord2svARB");
		func->glMultiTexCoord3dARB = (PFNGLMULTITEXCOORD3DARBPROC)load("glMultiTexCoord3dARB");
		func->glMultiTexCoord3dvARB = (PFNGLMULTITEXCOORD3DVARBPROC)load("glMultiTexCoord3dvARB");
		func->glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)load("glMultiTexCoord3fARB");
		func->glMultiTexCoord3fvARB = (PFNGLMULTITEXCOORD3FVARBPROC)load("glMultiTexCoord3fvARB");
		func->glMultiTexCoord3iARB = (PFNGLMULTITEXCOORD3IARBPROC)load("glMultiTexCoord3iARB");
		func->glMultiTexCoord3ivARB = (PFNGLMULTITEXCOORD3IVARBPROC)load("glMultiTexCoord3ivARB");
		func->glMultiTexCoord3sARB = (PFNGLMULTITEXCOORD3SARBPROC)load("glMultiTexCoord3sARB");
		func->glMultiTexCoord3svARB = (PFNGLMULTITEXCOORD3SVARBPROC)load("glMultiTexCoord3svARB");
		func->glMultiTexCoord4dARB = (PFNGLMULTITEXCOORD4DARBPROC)load("glMultiTexCoord4dARB");
		func->glMultiTexCoord4dvARB = (PFNGLMULTITEXCOORD4DVARBPROC)load("glMultiTexCoord4dvARB");
		func->glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)load("glMultiTexCoord4fARB");
		func->glMultiTexCoord4fvARB = (PFNGLMULTITEXCOORD4FVARBPROC)load("glMultiTexCoord4fvARB");
		func->glMultiTexCoord4iARB = (PFNGLMULTITEXCOORD4IARBPROC)load("glMultiTexCoord4iARB");
		func->glMultiTexCoord4ivARB = (PFNGLMULTITEXCOORD4IVARBPROC)load("glMultiTexCoord4ivARB");
		func->glMultiTexCoord4sARB = (PFNGLMULTITEXCOORD4SARBPROC)load("glMultiTexCoord4sARB");
		func->glMultiTexCoord4svARB = (PFNGLMULTITEXCOORD4SVARBPROC)load("glMultiTexCoord4svARB");
	}
	static void load_GL_ARB_occlusion_query(LoadProc load, GLExtFunctions* func)
    {
		func->glGenQueriesARB = (PFNGLGENQUERIESARBPROC)load("glGenQueriesARB");
		func->glDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC)load("glDeleteQueriesARB");
		func->glIsQueryARB = (PFNGLISQUERYARBPROC)load("glIsQueryARB");
		func->glBeginQueryARB = (PFNGLBEGINQUERYARBPROC)load("glBeginQueryARB");
		func->glEndQueryARB = (PFNGLENDQUERYARBPROC)load("glEndQueryARB");
		func->glGetQueryivARB = (PFNGLGETQUERYIVARBPROC)load("glGetQueryivARB");
		func->glGetQueryObjectivARB = (PFNGLGETQUERYOBJECTIVARBPROC)load("glGetQueryObjectivARB");
		func->glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)load("glGetQueryObjectuivARB");
	}
	static void load_GL_ARB_parallel_shader_compile(LoadProc load, GLExtFunctions* func)
    {
		func->glMaxShaderCompilerThreadsARB = (PFNGLMAXSHADERCOMPILERTHREADSARBPROC)load("glMaxShaderCompilerThreadsARB");
	}
	static void load_GL_ARB_point_parameters(LoadProc load, GLExtFunctions* func)
    {
		func->glPointParameterfARB = (PFNGLPOINTPARAMETERFARBPROC)load("glPointParameterfARB");
		func->glPointParameterfvARB = (PFNGLPOINTPARAMETERFVARBPROC)load("glPointParameterfvARB");
	}
	static void load_GL_ARB_polygon_offset_clamp(LoadProc load, GLFunctions* func)
    {
		func->glPolygonOffsetClamp = (PFNGLPOLYGONOFFSETCLAMPPROC)load("glPolygonOffsetClamp");
	}
	static void load_GL_ARB_program_interface_query(LoadProc load, GLFunctions* func)
    {
		func->glGetProgramInterfaceiv = (PFNGLGETPROGRAMINTERFACEIVPROC)load("glGetProgramInterfaceiv");
		func->glGetProgramResourceIndex = (PFNGLGETPROGRAMRESOURCEINDEXPROC)load("glGetProgramResourceIndex");
		func->glGetProgramResourceName = (PFNGLGETPROGRAMRESOURCENAMEPROC)load("glGetProgramResourceName");
		func->glGetProgramResourceiv = (PFNGLGETPROGRAMRESOURCEIVPROC)load("glGetProgramResourceiv");
		func->glGetProgramResourceLocation = (PFNGLGETPROGRAMRESOURCELOCATIONPROC)load("glGetProgramResourceLocation");
		func->glGetProgramResourceLocationIndex = (PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC)load("glGetProgramResourceLocationIndex");
	}
	static void load_GL_ARB_provoking_vertex(LoadProc load, GLFunctions* func)
    {
		func->glProvokingVertex = (PFNGLPROVOKINGVERTEXPROC)load("glProvokingVertex");
	}
	static void load_GL_ARB_robustness(LoadProc load, GLExtFunctions* func)
    {
		func->glGetGraphicsResetStatusARB = (PFNGLGETGRAPHICSRESETSTATUSARBPROC)load("glGetGraphicsResetStatusARB");
		func->glGetnTexImageARB = (PFNGLGETNTEXIMAGEARBPROC)load("glGetnTexImageARB");
		func->glReadnPixelsARB = (PFNGLREADNPIXELSARBPROC)load("glReadnPixelsARB");
		func->glGetnCompressedTexImageARB = (PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC)load("glGetnCompressedTexImageARB");
		func->glGetnUniformfvARB = (PFNGLGETNUNIFORMFVARBPROC)load("glGetnUniformfvARB");
		func->glGetnUniformivARB = (PFNGLGETNUNIFORMIVARBPROC)load("glGetnUniformivARB");
		func->glGetnUniformuivARB = (PFNGLGETNUNIFORMUIVARBPROC)load("glGetnUniformuivARB");
		func->glGetnUniformdvARB = (PFNGLGETNUNIFORMDVARBPROC)load("glGetnUniformdvARB");
    }
	static void load_GL_ARB_sample_locations(LoadProc load, GLExtFunctions* func)
    {
		func->glFramebufferSampleLocationsfvARB = (PFNGLFRAMEBUFFERSAMPLELOCATIONSFVARBPROC)load("glFramebufferSampleLocationsfvARB");
		func->glNamedFramebufferSampleLocationsfvARB = (PFNGLNAMEDFRAMEBUFFERSAMPLELOCATIONSFVARBPROC)load("glNamedFramebufferSampleLocationsfvARB");
		func->glEvaluateDepthValuesARB = (PFNGLEVALUATEDEPTHVALUESARBPROC)load("glEvaluateDepthValuesARB");
	}
	static void load_GL_ARB_sample_shading(LoadProc load, GLExtFunctions* func)
    {
		func->glMinSampleShadingARB = (PFNGLMINSAMPLESHADINGARBPROC)load("glMinSampleShadingARB");
	}
	static void load_GL_ARB_sampler_objects(LoadProc load, GLFunctions* func)
    {
		func->glGenSamplers = (PFNGLGENSAMPLERSPROC)load("glGenSamplers");
		func->glDeleteSamplers = (PFNGLDELETESAMPLERSPROC)load("glDeleteSamplers");
		func->glIsSampler = (PFNGLISSAMPLERPROC)load("glIsSampler");
		func->glBindSampler = (PFNGLBINDSAMPLERPROC)load("glBindSampler");
		func->glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC)load("glSamplerParameteri");
		func->glSamplerParameteriv = (PFNGLSAMPLERPARAMETERIVPROC)load("glSamplerParameteriv");
		func->glSamplerParameterf = (PFNGLSAMPLERPARAMETERFPROC)load("glSamplerParameterf");
		func->glSamplerParameterfv = (PFNGLSAMPLERPARAMETERFVPROC)load("glSamplerParameterfv");
		func->glSamplerParameterIiv = (PFNGLSAMPLERPARAMETERIIVPROC)load("glSamplerParameterIiv");
		func->glSamplerParameterIuiv = (PFNGLSAMPLERPARAMETERIUIVPROC)load("glSamplerParameterIuiv");
		func->glGetSamplerParameteriv = (PFNGLGETSAMPLERPARAMETERIVPROC)load("glGetSamplerParameteriv");
		func->glGetSamplerParameterIiv = (PFNGLGETSAMPLERPARAMETERIIVPROC)load("glGetSamplerParameterIiv");
		func->glGetSamplerParameterfv = (PFNGLGETSAMPLERPARAMETERFVPROC)load("glGetSamplerParameterfv");
		func->glGetSamplerParameterIuiv = (PFNGLGETSAMPLERPARAMETERIUIVPROC)load("glGetSamplerParameterIuiv");
	}
	static void load_GL_ARB_separate_shader_objects(LoadProc load, GLFunctions* func)
    {
		func->glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC)load("glUseProgramStages");
		func->glActiveShaderProgram = (PFNGLACTIVESHADERPROGRAMPROC)load("glActiveShaderProgram");
		func->glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)load("glCreateShaderProgramv");
		func->glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC)load("glBindProgramPipeline");
		func->glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC)load("glDeleteProgramPipelines");
		func->glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC)load("glGenProgramPipelines");
		func->glIsProgramPipeline = (PFNGLISPROGRAMPIPELINEPROC)load("glIsProgramPipeline");
		func->glGetProgramPipelineiv = (PFNGLGETPROGRAMPIPELINEIVPROC)load("glGetProgramPipelineiv");
		func->glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)load("glProgramParameteri");
		func->glProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC)load("glProgramUniform1i");
		func->glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC)load("glProgramUniform1iv");
		func->glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC)load("glProgramUniform1f");
		func->glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC)load("glProgramUniform1fv");
		func->glProgramUniform1d = (PFNGLPROGRAMUNIFORM1DPROC)load("glProgramUniform1d");
		func->glProgramUniform1dv = (PFNGLPROGRAMUNIFORM1DVPROC)load("glProgramUniform1dv");
		func->glProgramUniform1ui = (PFNGLPROGRAMUNIFORM1UIPROC)load("glProgramUniform1ui");
		func->glProgramUniform1uiv = (PFNGLPROGRAMUNIFORM1UIVPROC)load("glProgramUniform1uiv");
		func->glProgramUniform2i = (PFNGLPROGRAMUNIFORM2IPROC)load("glProgramUniform2i");
		func->glProgramUniform2iv = (PFNGLPROGRAMUNIFORM2IVPROC)load("glProgramUniform2iv");
		func->glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC)load("glProgramUniform2f");
		func->glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC)load("glProgramUniform2fv");
		func->glProgramUniform2d = (PFNGLPROGRAMUNIFORM2DPROC)load("glProgramUniform2d");
		func->glProgramUniform2dv = (PFNGLPROGRAMUNIFORM2DVPROC)load("glProgramUniform2dv");
		func->glProgramUniform2ui = (PFNGLPROGRAMUNIFORM2UIPROC)load("glProgramUniform2ui");
		func->glProgramUniform2uiv = (PFNGLPROGRAMUNIFORM2UIVPROC)load("glProgramUniform2uiv");
		func->glProgramUniform3i = (PFNGLPROGRAMUNIFORM3IPROC)load("glProgramUniform3i");
		func->glProgramUniform3iv = (PFNGLPROGRAMUNIFORM3IVPROC)load("glProgramUniform3iv");
		func->glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC)load("glProgramUniform3f");
		func->glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC)load("glProgramUniform3fv");
		func->glProgramUniform3d = (PFNGLPROGRAMUNIFORM3DPROC)load("glProgramUniform3d");
		func->glProgramUniform3dv = (PFNGLPROGRAMUNIFORM3DVPROC)load("glProgramUniform3dv");
		func->glProgramUniform3ui = (PFNGLPROGRAMUNIFORM3UIPROC)load("glProgramUniform3ui");
		func->glProgramUniform3uiv = (PFNGLPROGRAMUNIFORM3UIVPROC)load("glProgramUniform3uiv");
		func->glProgramUniform4i = (PFNGLPROGRAMUNIFORM4IPROC)load("glProgramUniform4i");
		func->glProgramUniform4iv = (PFNGLPROGRAMUNIFORM4IVPROC)load("glProgramUniform4iv");
		func->glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC)load("glProgramUniform4f");
		func->glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC)load("glProgramUniform4fv");
		func->glProgramUniform4d = (PFNGLPROGRAMUNIFORM4DPROC)load("glProgramUniform4d");
		func->glProgramUniform4dv = (PFNGLPROGRAMUNIFORM4DVPROC)load("glProgramUniform4dv");
		func->glProgramUniform4ui = (PFNGLPROGRAMUNIFORM4UIPROC)load("glProgramUniform4ui");
		func->glProgramUniform4uiv = (PFNGLPROGRAMUNIFORM4UIVPROC)load("glProgramUniform4uiv");
		func->glProgramUniformMatrix2fv = (PFNGLPROGRAMUNIFORMMATRIX2FVPROC)load("glProgramUniformMatrix2fv");
		func->glProgramUniformMatrix3fv = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC)load("glProgramUniformMatrix3fv");
		func->glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC)load("glProgramUniformMatrix4fv");
		func->glProgramUniformMatrix2dv = (PFNGLPROGRAMUNIFORMMATRIX2DVPROC)load("glProgramUniformMatrix2dv");
		func->glProgramUniformMatrix3dv = (PFNGLPROGRAMUNIFORMMATRIX3DVPROC)load("glProgramUniformMatrix3dv");
		func->glProgramUniformMatrix4dv = (PFNGLPROGRAMUNIFORMMATRIX4DVPROC)load("glProgramUniformMatrix4dv");
		func->glProgramUniformMatrix2x3fv = (PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC)load("glProgramUniformMatrix2x3fv");
		func->glProgramUniformMatrix3x2fv = (PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC)load("glProgramUniformMatrix3x2fv");
		func->glProgramUniformMatrix2x4fv = (PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC)load("glProgramUniformMatrix2x4fv");
		func->glProgramUniformMatrix4x2fv = (PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC)load("glProgramUniformMatrix4x2fv");
		func->glProgramUniformMatrix3x4fv = (PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC)load("glProgramUniformMatrix3x4fv");
		func->glProgramUniformMatrix4x3fv = (PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC)load("glProgramUniformMatrix4x3fv");
		func->glProgramUniformMatrix2x3dv = (PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC)load("glProgramUniformMatrix2x3dv");
		func->glProgramUniformMatrix3x2dv = (PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC)load("glProgramUniformMatrix3x2dv");
		func->glProgramUniformMatrix2x4dv = (PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC)load("glProgramUniformMatrix2x4dv");
		func->glProgramUniformMatrix4x2dv = (PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC)load("glProgramUniformMatrix4x2dv");
		func->glProgramUniformMatrix3x4dv = (PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC)load("glProgramUniformMatrix3x4dv");
		func->glProgramUniformMatrix4x3dv = (PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC)load("glProgramUniformMatrix4x3dv");
		func->glValidateProgramPipeline = (PFNGLVALIDATEPROGRAMPIPELINEPROC)load("glValidateProgramPipeline");
		func->glGetProgramPipelineInfoLog = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC)load("glGetProgramPipelineInfoLog");
	}
	static void load_GL_ARB_shader_atomic_counters(LoadProc load, GLFunctions* func)
    {
		func->glGetActiveAtomicCounterBufferiv = (PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC)load("glGetActiveAtomicCounterBufferiv");
	}
	static void load_GL_ARB_shader_image_load_store(LoadProc load, GLFunctions* func)
    {
		func->glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)load("glBindImageTexture");
		func->glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)load("glMemoryBarrier");
	}
	static void load_GL_ARB_shader_objects(LoadProc load, GLExtFunctions* func)
    {
		func->glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)load("glDeleteObjectARB");
		func->glGetHandleARB = (PFNGLGETHANDLEARBPROC)load("glGetHandleARB");
		func->glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)load("glDetachObjectARB");
		func->glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)load("glCreateShaderObjectARB");
		func->glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)load("glShaderSourceARB");
		func->glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)load("glCompileShaderARB");
		func->glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)load("glCreateProgramObjectARB");
		func->glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)load("glAttachObjectARB");
		func->glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)load("glLinkProgramARB");
		func->glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)load("glUseProgramObjectARB");
		func->glValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)load("glValidateProgramARB");
		func->glUniform1fARB = (PFNGLUNIFORM1FARBPROC)load("glUniform1fARB");
		func->glUniform2fARB = (PFNGLUNIFORM2FARBPROC)load("glUniform2fARB");
		func->glUniform3fARB = (PFNGLUNIFORM3FARBPROC)load("glUniform3fARB");
		func->glUniform4fARB = (PFNGLUNIFORM4FARBPROC)load("glUniform4fARB");
		func->glUniform1iARB = (PFNGLUNIFORM1IARBPROC)load("glUniform1iARB");
		func->glUniform2iARB = (PFNGLUNIFORM2IARBPROC)load("glUniform2iARB");
		func->glUniform3iARB = (PFNGLUNIFORM3IARBPROC)load("glUniform3iARB");
		func->glUniform4iARB = (PFNGLUNIFORM4IARBPROC)load("glUniform4iARB");
		func->glUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)load("glUniform1fvARB");
		func->glUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)load("glUniform2fvARB");
		func->glUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)load("glUniform3fvARB");
		func->glUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)load("glUniform4fvARB");
		func->glUniform1ivARB = (PFNGLUNIFORM1IVARBPROC)load("glUniform1ivARB");
		func->glUniform2ivARB = (PFNGLUNIFORM2IVARBPROC)load("glUniform2ivARB");
		func->glUniform3ivARB = (PFNGLUNIFORM3IVARBPROC)load("glUniform3ivARB");
		func->glUniform4ivARB = (PFNGLUNIFORM4IVARBPROC)load("glUniform4ivARB");
		func->glUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC)load("glUniformMatrix2fvARB");
		func->glUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC)load("glUniformMatrix3fvARB");
		func->glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)load("glUniformMatrix4fvARB");
		func->glGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC)load("glGetObjectParameterfvARB");
		func->glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)load("glGetObjectParameterivARB");
		func->glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)load("glGetInfoLogARB");
		func->glGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC)load("glGetAttachedObjectsARB");
		func->glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)load("glGetUniformLocationARB");
		func->glGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC)load("glGetActiveUniformARB");
		func->glGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC)load("glGetUniformfvARB");
		func->glGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC)load("glGetUniformivARB");
		func->glGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC)load("glGetShaderSourceARB");
	}
	static void load_GL_ARB_shader_storage_buffer_object(LoadProc load, GLFunctions* func)
    {
		func->glShaderStorageBlockBinding = (PFNGLSHADERSTORAGEBLOCKBINDINGPROC)load("glShaderStorageBlockBinding");
	}
	static void load_GL_ARB_shader_subroutine(LoadProc load, GLFunctions* func)
    {
		func->glGetSubroutineUniformLocation = (PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC)load("glGetSubroutineUniformLocation");
		func->glGetSubroutineIndex = (PFNGLGETSUBROUTINEINDEXPROC)load("glGetSubroutineIndex");
		func->glGetActiveSubroutineUniformiv = (PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC)load("glGetActiveSubroutineUniformiv");
		func->glGetActiveSubroutineUniformName = (PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC)load("glGetActiveSubroutineUniformName");
		func->glGetActiveSubroutineName = (PFNGLGETACTIVESUBROUTINENAMEPROC)load("glGetActiveSubroutineName");
		func->glUniformSubroutinesuiv = (PFNGLUNIFORMSUBROUTINESUIVPROC)load("glUniformSubroutinesuiv");
		func->glGetUniformSubroutineuiv = (PFNGLGETUNIFORMSUBROUTINEUIVPROC)load("glGetUniformSubroutineuiv");
		func->glGetProgramStageiv = (PFNGLGETPROGRAMSTAGEIVPROC)load("glGetProgramStageiv");
	}
	static void load_GL_ARB_shading_language_include(LoadProc load, GLExtFunctions* func)
    {
		func->glNamedStringARB = (PFNGLNAMEDSTRINGARBPROC)load("glNamedStringARB");
		func->glDeleteNamedStringARB = (PFNGLDELETENAMEDSTRINGARBPROC)load("glDeleteNamedStringARB");
		func->glCompileShaderIncludeARB = (PFNGLCOMPILESHADERINCLUDEARBPROC)load("glCompileShaderIncludeARB");
		func->glIsNamedStringARB = (PFNGLISNAMEDSTRINGARBPROC)load("glIsNamedStringARB");
		func->glGetNamedStringARB = (PFNGLGETNAMEDSTRINGARBPROC)load("glGetNamedStringARB");
		func->glGetNamedStringivARB = (PFNGLGETNAMEDSTRINGIVARBPROC)load("glGetNamedStringivARB");
	}
	static void load_GL_ARB_sparse_buffer(LoadProc load, GLExtFunctions* func)
    {
		func->glBufferPageCommitmentARB = (PFNGLBUFFERPAGECOMMITMENTARBPROC)load("glBufferPageCommitmentARB");
		func->glNamedBufferPageCommitmentEXT = (PFNGLNAMEDBUFFERPAGECOMMITMENTEXTPROC)load("glNamedBufferPageCommitmentEXT");
		func->glNamedBufferPageCommitmentARB = (PFNGLNAMEDBUFFERPAGECOMMITMENTARBPROC)load("glNamedBufferPageCommitmentARB");
	}
	static void load_GL_ARB_sparse_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glTexPageCommitmentARB = (PFNGLTEXPAGECOMMITMENTARBPROC)load("glTexPageCommitmentARB");
	}
	static void load_GL_ARB_sync(LoadProc load, GLFunctions* func)
    {
		func->glFenceSync = (PFNGLFENCESYNCPROC)load("glFenceSync");
		func->glIsSync = (PFNGLISSYNCPROC)load("glIsSync");
		func->glDeleteSync = (PFNGLDELETESYNCPROC)load("glDeleteSync");
		func->glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)load("glClientWaitSync");
		func->glWaitSync = (PFNGLWAITSYNCPROC)load("glWaitSync");
		func->glGetInteger64v = (PFNGLGETINTEGER64VPROC)load("glGetInteger64v");
		func->glGetSynciv = (PFNGLGETSYNCIVPROC)load("glGetSynciv");
	}
	static void load_GL_ARB_tessellation_shader(LoadProc load, GLFunctions* func)
    {
		func->glPatchParameteri = (PFNGLPATCHPARAMETERIPROC)load("glPatchParameteri");
		func->glPatchParameterfv = (PFNGLPATCHPARAMETERFVPROC)load("glPatchParameterfv");
	}
	static void load_GL_ARB_texture_barrier(LoadProc load, GLFunctions* func)
    {
		func->glTextureBarrier = (PFNGLTEXTUREBARRIERPROC)load("glTextureBarrier");
	}
	static void load_GL_ARB_texture_buffer_object(LoadProc load, GLExtFunctions* func)
    {
		func->glTexBufferARB = (PFNGLTEXBUFFERARBPROC)load("glTexBufferARB");
	}
	static void load_GL_ARB_texture_buffer_range(LoadProc load, GLFunctions* func)
    {
		func->glTexBufferRange = (PFNGLTEXBUFFERRANGEPROC)load("glTexBufferRange");
	}
	static void load_GL_ARB_texture_compression(LoadProc load, GLExtFunctions* func)
    {
		func->glCompressedTexImage3DARB = (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)load("glCompressedTexImage3DARB");
		func->glCompressedTexImage2DARB = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)load("glCompressedTexImage2DARB");
		func->glCompressedTexImage1DARB = (PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)load("glCompressedTexImage1DARB");
		func->glCompressedTexSubImage3DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)load("glCompressedTexSubImage3DARB");
		func->glCompressedTexSubImage2DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)load("glCompressedTexSubImage2DARB");
		func->glCompressedTexSubImage1DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)load("glCompressedTexSubImage1DARB");
		func->glGetCompressedTexImageARB = (PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)load("glGetCompressedTexImageARB");
	}
	static void load_GL_ARB_texture_multisample(LoadProc load, GLFunctions* func)
    {
		func->glTexImage2DMultisample = (PFNGLTEXIMAGE2DMULTISAMPLEPROC)load("glTexImage2DMultisample");
		func->glTexImage3DMultisample = (PFNGLTEXIMAGE3DMULTISAMPLEPROC)load("glTexImage3DMultisample");
		func->glGetMultisamplefv = (PFNGLGETMULTISAMPLEFVPROC)load("glGetMultisamplefv");
		func->glSampleMaski = (PFNGLSAMPLEMASKIPROC)load("glSampleMaski");
	}
	static void load_GL_ARB_texture_storage(LoadProc load, GLFunctions* func)
    {
		func->glTexStorage1D = (PFNGLTEXSTORAGE1DPROC)load("glTexStorage1D");
		func->glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)load("glTexStorage2D");
		func->glTexStorage3D = (PFNGLTEXSTORAGE3DPROC)load("glTexStorage3D");
	}
	static void load_GL_ARB_texture_storage_multisample(LoadProc load, GLFunctions* func)
    {
		func->glTexStorage2DMultisample = (PFNGLTEXSTORAGE2DMULTISAMPLEPROC)load("glTexStorage2DMultisample");
		func->glTexStorage3DMultisample = (PFNGLTEXSTORAGE3DMULTISAMPLEPROC)load("glTexStorage3DMultisample");
	}
	static void load_GL_ARB_texture_view(LoadProc load, GLFunctions* func)
    {
		func->glTextureView = (PFNGLTEXTUREVIEWPROC)load("glTextureView");
	}
	static void load_GL_ARB_timer_query(LoadProc load, GLFunctions* func)
    {
		func->glQueryCounter = (PFNGLQUERYCOUNTERPROC)load("glQueryCounter");
		func->glGetQueryObjecti64v = (PFNGLGETQUERYOBJECTI64VPROC)load("glGetQueryObjecti64v");
		func->glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC)load("glGetQueryObjectui64v");
	}
	static void load_GL_ARB_transform_feedback2(LoadProc load, GLFunctions* func)
    {
		func->glBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC)load("glBindTransformFeedback");
		func->glDeleteTransformFeedbacks = (PFNGLDELETETRANSFORMFEEDBACKSPROC)load("glDeleteTransformFeedbacks");
		func->glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC)load("glGenTransformFeedbacks");
		func->glIsTransformFeedback = (PFNGLISTRANSFORMFEEDBACKPROC)load("glIsTransformFeedback");
		func->glPauseTransformFeedback = (PFNGLPAUSETRANSFORMFEEDBACKPROC)load("glPauseTransformFeedback");
		func->glResumeTransformFeedback = (PFNGLRESUMETRANSFORMFEEDBACKPROC)load("glResumeTransformFeedback");
		func->glDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACKPROC)load("glDrawTransformFeedback");
	}
	static void load_GL_ARB_transform_feedback3(LoadProc load, GLFunctions* func)
    {
		func->glDrawTransformFeedbackStream = (PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC)load("glDrawTransformFeedbackStream");
		func->glBeginQueryIndexed = (PFNGLBEGINQUERYINDEXEDPROC)load("glBeginQueryIndexed");
		func->glEndQueryIndexed = (PFNGLENDQUERYINDEXEDPROC)load("glEndQueryIndexed");
		func->glGetQueryIndexediv = (PFNGLGETQUERYINDEXEDIVPROC)load("glGetQueryIndexediv");
	}
	static void load_GL_ARB_transform_feedback_instanced(LoadProc load, GLFunctions* func)
    {
		func->glDrawTransformFeedbackInstanced = (PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC)load("glDrawTransformFeedbackInstanced");
		func->glDrawTransformFeedbackStreamInstanced = (PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC)load("glDrawTransformFeedbackStreamInstanced");
	}
	static void load_GL_ARB_transpose_matrix(LoadProc load, GLExtFunctions* func)
    {
		func->glLoadTransposeMatrixfARB = (PFNGLLOADTRANSPOSEMATRIXFARBPROC)load("glLoadTransposeMatrixfARB");
		func->glLoadTransposeMatrixdARB = (PFNGLLOADTRANSPOSEMATRIXDARBPROC)load("glLoadTransposeMatrixdARB");
		func->glMultTransposeMatrixfARB = (PFNGLMULTTRANSPOSEMATRIXFARBPROC)load("glMultTransposeMatrixfARB");
		func->glMultTransposeMatrixdARB = (PFNGLMULTTRANSPOSEMATRIXDARBPROC)load("glMultTransposeMatrixdARB");
	}
	static void load_GL_ARB_uniform_buffer_object(LoadProc load, GLFunctions* func)
    {
		func->glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC)load("glGetUniformIndices");
		func->glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)load("glGetActiveUniformsiv");
		func->glGetActiveUniformName = (PFNGLGETACTIVEUNIFORMNAMEPROC)load("glGetActiveUniformName");
		func->glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)load("glGetUniformBlockIndex");
		func->glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)load("glGetActiveUniformBlockiv");
		func->glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)load("glGetActiveUniformBlockName");
		func->glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)load("glUniformBlockBinding");
		func->glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)load("glBindBufferRange");
		func->glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)load("glBindBufferBase");
		func->glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)load("glGetIntegeri_v");
	}
	static void load_GL_ARB_vertex_array_object(LoadProc load, GLFunctions* func)
    {
		func->glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
		func->glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)load("glDeleteVertexArrays");
		func->glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
		func->glIsVertexArray = (PFNGLISVERTEXARRAYPROC)load("glIsVertexArray");
	}
	static void load_GL_ARB_vertex_attrib_64bit(LoadProc load, GLFunctions* func)
    {
		func->glVertexAttribL1d = (PFNGLVERTEXATTRIBL1DPROC)load("glVertexAttribL1d");
		func->glVertexAttribL2d = (PFNGLVERTEXATTRIBL2DPROC)load("glVertexAttribL2d");
		func->glVertexAttribL3d = (PFNGLVERTEXATTRIBL3DPROC)load("glVertexAttribL3d");
		func->glVertexAttribL4d = (PFNGLVERTEXATTRIBL4DPROC)load("glVertexAttribL4d");
		func->glVertexAttribL1dv = (PFNGLVERTEXATTRIBL1DVPROC)load("glVertexAttribL1dv");
		func->glVertexAttribL2dv = (PFNGLVERTEXATTRIBL2DVPROC)load("glVertexAttribL2dv");
		func->glVertexAttribL3dv = (PFNGLVERTEXATTRIBL3DVPROC)load("glVertexAttribL3dv");
		func->glVertexAttribL4dv = (PFNGLVERTEXATTRIBL4DVPROC)load("glVertexAttribL4dv");
		func->glVertexAttribLPointer = (PFNGLVERTEXATTRIBLPOINTERPROC)load("glVertexAttribLPointer");
		func->glGetVertexAttribLdv = (PFNGLGETVERTEXATTRIBLDVPROC)load("glGetVertexAttribLdv");
	}
	static void load_GL_ARB_vertex_attrib_binding(LoadProc load, GLFunctions* func)
    {
		func->glBindVertexBuffer = (PFNGLBINDVERTEXBUFFERPROC)load("glBindVertexBuffer");
		func->glVertexAttribFormat = (PFNGLVERTEXATTRIBFORMATPROC)load("glVertexAttribFormat");
		func->glVertexAttribIFormat = (PFNGLVERTEXATTRIBIFORMATPROC)load("glVertexAttribIFormat");
		func->glVertexAttribLFormat = (PFNGLVERTEXATTRIBLFORMATPROC)load("glVertexAttribLFormat");
		func->glVertexAttribBinding = (PFNGLVERTEXATTRIBBINDINGPROC)load("glVertexAttribBinding");
		func->glVertexBindingDivisor = (PFNGLVERTEXBINDINGDIVISORPROC)load("glVertexBindingDivisor");
	}
	static void load_GL_ARB_vertex_blend(LoadProc load, GLExtFunctions* func)
    {
		func->glWeightbvARB = (PFNGLWEIGHTBVARBPROC)load("glWeightbvARB");
		func->glWeightsvARB = (PFNGLWEIGHTSVARBPROC)load("glWeightsvARB");
		func->glWeightivARB = (PFNGLWEIGHTIVARBPROC)load("glWeightivARB");
		func->glWeightfvARB = (PFNGLWEIGHTFVARBPROC)load("glWeightfvARB");
		func->glWeightdvARB = (PFNGLWEIGHTDVARBPROC)load("glWeightdvARB");
		func->glWeightubvARB = (PFNGLWEIGHTUBVARBPROC)load("glWeightubvARB");
		func->glWeightusvARB = (PFNGLWEIGHTUSVARBPROC)load("glWeightusvARB");
		func->glWeightuivARB = (PFNGLWEIGHTUIVARBPROC)load("glWeightuivARB");
		func->glWeightPointerARB = (PFNGLWEIGHTPOINTERARBPROC)load("glWeightPointerARB");
		func->glVertexBlendARB = (PFNGLVERTEXBLENDARBPROC)load("glVertexBlendARB");
	}
	static void load_GL_ARB_vertex_buffer_object(LoadProc load, GLExtFunctions* func)
    {
		func->glBindBufferARB = (PFNGLBINDBUFFERARBPROC)load("glBindBufferARB");
		func->glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)load("glDeleteBuffersARB");
		func->glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)load("glGenBuffersARB");
		func->glIsBufferARB = (PFNGLISBUFFERARBPROC)load("glIsBufferARB");
		func->glBufferDataARB = (PFNGLBUFFERDATAARBPROC)load("glBufferDataARB");
		func->glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)load("glBufferSubDataARB");
		func->glGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC)load("glGetBufferSubDataARB");
		func->glMapBufferARB = (PFNGLMAPBUFFERARBPROC)load("glMapBufferARB");
		func->glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)load("glUnmapBufferARB");
		func->glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)load("glGetBufferParameterivARB");
		func->glGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC)load("glGetBufferPointervARB");
	}
	static void load_GL_ARB_vertex_program(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexAttrib1dARB = (PFNGLVERTEXATTRIB1DARBPROC)load("glVertexAttrib1dARB");
		func->glVertexAttrib1dvARB = (PFNGLVERTEXATTRIB1DVARBPROC)load("glVertexAttrib1dvARB");
		func->glVertexAttrib1fARB = (PFNGLVERTEXATTRIB1FARBPROC)load("glVertexAttrib1fARB");
		func->glVertexAttrib1fvARB = (PFNGLVERTEXATTRIB1FVARBPROC)load("glVertexAttrib1fvARB");
		func->glVertexAttrib1sARB = (PFNGLVERTEXATTRIB1SARBPROC)load("glVertexAttrib1sARB");
		func->glVertexAttrib1svARB = (PFNGLVERTEXATTRIB1SVARBPROC)load("glVertexAttrib1svARB");
		func->glVertexAttrib2dARB = (PFNGLVERTEXATTRIB2DARBPROC)load("glVertexAttrib2dARB");
		func->glVertexAttrib2dvARB = (PFNGLVERTEXATTRIB2DVARBPROC)load("glVertexAttrib2dvARB");
		func->glVertexAttrib2fARB = (PFNGLVERTEXATTRIB2FARBPROC)load("glVertexAttrib2fARB");
		func->glVertexAttrib2fvARB = (PFNGLVERTEXATTRIB2FVARBPROC)load("glVertexAttrib2fvARB");
		func->glVertexAttrib2sARB = (PFNGLVERTEXATTRIB2SARBPROC)load("glVertexAttrib2sARB");
		func->glVertexAttrib2svARB = (PFNGLVERTEXATTRIB2SVARBPROC)load("glVertexAttrib2svARB");
		func->glVertexAttrib3dARB = (PFNGLVERTEXATTRIB3DARBPROC)load("glVertexAttrib3dARB");
		func->glVertexAttrib3dvARB = (PFNGLVERTEXATTRIB3DVARBPROC)load("glVertexAttrib3dvARB");
		func->glVertexAttrib3fARB = (PFNGLVERTEXATTRIB3FARBPROC)load("glVertexAttrib3fARB");
		func->glVertexAttrib3fvARB = (PFNGLVERTEXATTRIB3FVARBPROC)load("glVertexAttrib3fvARB");
		func->glVertexAttrib3sARB = (PFNGLVERTEXATTRIB3SARBPROC)load("glVertexAttrib3sARB");
		func->glVertexAttrib3svARB = (PFNGLVERTEXATTRIB3SVARBPROC)load("glVertexAttrib3svARB");
		func->glVertexAttrib4NbvARB = (PFNGLVERTEXATTRIB4NBVARBPROC)load("glVertexAttrib4NbvARB");
		func->glVertexAttrib4NivARB = (PFNGLVERTEXATTRIB4NIVARBPROC)load("glVertexAttrib4NivARB");
		func->glVertexAttrib4NsvARB = (PFNGLVERTEXATTRIB4NSVARBPROC)load("glVertexAttrib4NsvARB");
		func->glVertexAttrib4NubARB = (PFNGLVERTEXATTRIB4NUBARBPROC)load("glVertexAttrib4NubARB");
		func->glVertexAttrib4NubvARB = (PFNGLVERTEXATTRIB4NUBVARBPROC)load("glVertexAttrib4NubvARB");
		func->glVertexAttrib4NuivARB = (PFNGLVERTEXATTRIB4NUIVARBPROC)load("glVertexAttrib4NuivARB");
		func->glVertexAttrib4NusvARB = (PFNGLVERTEXATTRIB4NUSVARBPROC)load("glVertexAttrib4NusvARB");
		func->glVertexAttrib4bvARB = (PFNGLVERTEXATTRIB4BVARBPROC)load("glVertexAttrib4bvARB");
		func->glVertexAttrib4dARB = (PFNGLVERTEXATTRIB4DARBPROC)load("glVertexAttrib4dARB");
		func->glVertexAttrib4dvARB = (PFNGLVERTEXATTRIB4DVARBPROC)load("glVertexAttrib4dvARB");
		func->glVertexAttrib4fARB = (PFNGLVERTEXATTRIB4FARBPROC)load("glVertexAttrib4fARB");
		func->glVertexAttrib4fvARB = (PFNGLVERTEXATTRIB4FVARBPROC)load("glVertexAttrib4fvARB");
		func->glVertexAttrib4ivARB = (PFNGLVERTEXATTRIB4IVARBPROC)load("glVertexAttrib4ivARB");
		func->glVertexAttrib4sARB = (PFNGLVERTEXATTRIB4SARBPROC)load("glVertexAttrib4sARB");
		func->glVertexAttrib4svARB = (PFNGLVERTEXATTRIB4SVARBPROC)load("glVertexAttrib4svARB");
		func->glVertexAttrib4ubvARB = (PFNGLVERTEXATTRIB4UBVARBPROC)load("glVertexAttrib4ubvARB");
		func->glVertexAttrib4uivARB = (PFNGLVERTEXATTRIB4UIVARBPROC)load("glVertexAttrib4uivARB");
		func->glVertexAttrib4usvARB = (PFNGLVERTEXATTRIB4USVARBPROC)load("glVertexAttrib4usvARB");
		func->glVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC)load("glVertexAttribPointerARB");
		func->glEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)load("glEnableVertexAttribArrayARB");
		func->glDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)load("glDisableVertexAttribArrayARB");
		func->glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)load("glProgramStringARB");
		func->glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)load("glBindProgramARB");
		func->glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)load("glDeleteProgramsARB");
		func->glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)load("glGenProgramsARB");
		func->glProgramEnvParameter4dARB = (PFNGLPROGRAMENVPARAMETER4DARBPROC)load("glProgramEnvParameter4dARB");
		func->glProgramEnvParameter4dvARB = (PFNGLPROGRAMENVPARAMETER4DVARBPROC)load("glProgramEnvParameter4dvARB");
		func->glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)load("glProgramEnvParameter4fARB");
		func->glProgramEnvParameter4fvARB = (PFNGLPROGRAMENVPARAMETER4FVARBPROC)load("glProgramEnvParameter4fvARB");
		func->glProgramLocalParameter4dARB = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC)load("glProgramLocalParameter4dARB");
		func->glProgramLocalParameter4dvARB = (PFNGLPROGRAMLOCALPARAMETER4DVARBPROC)load("glProgramLocalParameter4dvARB");
		func->glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)load("glProgramLocalParameter4fARB");
		func->glProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)load("glProgramLocalParameter4fvARB");
		func->glGetProgramEnvParameterdvARB = (PFNGLGETPROGRAMENVPARAMETERDVARBPROC)load("glGetProgramEnvParameterdvARB");
		func->glGetProgramEnvParameterfvARB = (PFNGLGETPROGRAMENVPARAMETERFVARBPROC)load("glGetProgramEnvParameterfvARB");
		func->glGetProgramLocalParameterdvARB = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC)load("glGetProgramLocalParameterdvARB");
		func->glGetProgramLocalParameterfvARB = (PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)load("glGetProgramLocalParameterfvARB");
		func->glGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC)load("glGetProgramivARB");
		func->glGetProgramStringARB = (PFNGLGETPROGRAMSTRINGARBPROC)load("glGetProgramStringARB");
		func->glGetVertexAttribdvARB = (PFNGLGETVERTEXATTRIBDVARBPROC)load("glGetVertexAttribdvARB");
		func->glGetVertexAttribfvARB = (PFNGLGETVERTEXATTRIBFVARBPROC)load("glGetVertexAttribfvARB");
		func->glGetVertexAttribivARB = (PFNGLGETVERTEXATTRIBIVARBPROC)load("glGetVertexAttribivARB");
		func->glGetVertexAttribPointervARB = (PFNGLGETVERTEXATTRIBPOINTERVARBPROC)load("glGetVertexAttribPointervARB");
		func->glIsProgramARB = (PFNGLISPROGRAMARBPROC)load("glIsProgramARB");
	}
	static void load_GL_ARB_vertex_shader(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexAttrib1fARB = (PFNGLVERTEXATTRIB1FARBPROC)load("glVertexAttrib1fARB");
		func->glVertexAttrib1sARB = (PFNGLVERTEXATTRIB1SARBPROC)load("glVertexAttrib1sARB");
		func->glVertexAttrib1dARB = (PFNGLVERTEXATTRIB1DARBPROC)load("glVertexAttrib1dARB");
		func->glVertexAttrib2fARB = (PFNGLVERTEXATTRIB2FARBPROC)load("glVertexAttrib2fARB");
		func->glVertexAttrib2sARB = (PFNGLVERTEXATTRIB2SARBPROC)load("glVertexAttrib2sARB");
		func->glVertexAttrib2dARB = (PFNGLVERTEXATTRIB2DARBPROC)load("glVertexAttrib2dARB");
		func->glVertexAttrib3fARB = (PFNGLVERTEXATTRIB3FARBPROC)load("glVertexAttrib3fARB");
		func->glVertexAttrib3sARB = (PFNGLVERTEXATTRIB3SARBPROC)load("glVertexAttrib3sARB");
		func->glVertexAttrib3dARB = (PFNGLVERTEXATTRIB3DARBPROC)load("glVertexAttrib3dARB");
		func->glVertexAttrib4fARB = (PFNGLVERTEXATTRIB4FARBPROC)load("glVertexAttrib4fARB");
		func->glVertexAttrib4sARB = (PFNGLVERTEXATTRIB4SARBPROC)load("glVertexAttrib4sARB");
		func->glVertexAttrib4dARB = (PFNGLVERTEXATTRIB4DARBPROC)load("glVertexAttrib4dARB");
		func->glVertexAttrib4NubARB = (PFNGLVERTEXATTRIB4NUBARBPROC)load("glVertexAttrib4NubARB");
		func->glVertexAttrib1fvARB = (PFNGLVERTEXATTRIB1FVARBPROC)load("glVertexAttrib1fvARB");
		func->glVertexAttrib1svARB = (PFNGLVERTEXATTRIB1SVARBPROC)load("glVertexAttrib1svARB");
		func->glVertexAttrib1dvARB = (PFNGLVERTEXATTRIB1DVARBPROC)load("glVertexAttrib1dvARB");
		func->glVertexAttrib2fvARB = (PFNGLVERTEXATTRIB2FVARBPROC)load("glVertexAttrib2fvARB");
		func->glVertexAttrib2svARB = (PFNGLVERTEXATTRIB2SVARBPROC)load("glVertexAttrib2svARB");
		func->glVertexAttrib2dvARB = (PFNGLVERTEXATTRIB2DVARBPROC)load("glVertexAttrib2dvARB");
		func->glVertexAttrib3fvARB = (PFNGLVERTEXATTRIB3FVARBPROC)load("glVertexAttrib3fvARB");
		func->glVertexAttrib3svARB = (PFNGLVERTEXATTRIB3SVARBPROC)load("glVertexAttrib3svARB");
		func->glVertexAttrib3dvARB = (PFNGLVERTEXATTRIB3DVARBPROC)load("glVertexAttrib3dvARB");
		func->glVertexAttrib4fvARB = (PFNGLVERTEXATTRIB4FVARBPROC)load("glVertexAttrib4fvARB");
		func->glVertexAttrib4svARB = (PFNGLVERTEXATTRIB4SVARBPROC)load("glVertexAttrib4svARB");
		func->glVertexAttrib4dvARB = (PFNGLVERTEXATTRIB4DVARBPROC)load("glVertexAttrib4dvARB");
		func->glVertexAttrib4ivARB = (PFNGLVERTEXATTRIB4IVARBPROC)load("glVertexAttrib4ivARB");
		func->glVertexAttrib4bvARB = (PFNGLVERTEXATTRIB4BVARBPROC)load("glVertexAttrib4bvARB");
		func->glVertexAttrib4ubvARB = (PFNGLVERTEXATTRIB4UBVARBPROC)load("glVertexAttrib4ubvARB");
		func->glVertexAttrib4usvARB = (PFNGLVERTEXATTRIB4USVARBPROC)load("glVertexAttrib4usvARB");
		func->glVertexAttrib4uivARB = (PFNGLVERTEXATTRIB4UIVARBPROC)load("glVertexAttrib4uivARB");
		func->glVertexAttrib4NbvARB = (PFNGLVERTEXATTRIB4NBVARBPROC)load("glVertexAttrib4NbvARB");
		func->glVertexAttrib4NsvARB = (PFNGLVERTEXATTRIB4NSVARBPROC)load("glVertexAttrib4NsvARB");
		func->glVertexAttrib4NivARB = (PFNGLVERTEXATTRIB4NIVARBPROC)load("glVertexAttrib4NivARB");
		func->glVertexAttrib4NubvARB = (PFNGLVERTEXATTRIB4NUBVARBPROC)load("glVertexAttrib4NubvARB");
		func->glVertexAttrib4NusvARB = (PFNGLVERTEXATTRIB4NUSVARBPROC)load("glVertexAttrib4NusvARB");
		func->glVertexAttrib4NuivARB = (PFNGLVERTEXATTRIB4NUIVARBPROC)load("glVertexAttrib4NuivARB");
		func->glVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC)load("glVertexAttribPointerARB");
		func->glEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)load("glEnableVertexAttribArrayARB");
		func->glDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)load("glDisableVertexAttribArrayARB");
		func->glBindAttribLocationARB = (PFNGLBINDATTRIBLOCATIONARBPROC)load("glBindAttribLocationARB");
		func->glGetActiveAttribARB = (PFNGLGETACTIVEATTRIBARBPROC)load("glGetActiveAttribARB");
		func->glGetAttribLocationARB = (PFNGLGETATTRIBLOCATIONARBPROC)load("glGetAttribLocationARB");
		func->glGetVertexAttribdvARB = (PFNGLGETVERTEXATTRIBDVARBPROC)load("glGetVertexAttribdvARB");
		func->glGetVertexAttribfvARB = (PFNGLGETVERTEXATTRIBFVARBPROC)load("glGetVertexAttribfvARB");
		func->glGetVertexAttribivARB = (PFNGLGETVERTEXATTRIBIVARBPROC)load("glGetVertexAttribivARB");
		func->glGetVertexAttribPointervARB = (PFNGLGETVERTEXATTRIBPOINTERVARBPROC)load("glGetVertexAttribPointervARB");
	}
	static void load_GL_ARB_vertex_type_2_10_10_10_rev(LoadProc load, GLFunctions* func)
    {
		func->glVertexAttribP1ui = (PFNGLVERTEXATTRIBP1UIPROC)load("glVertexAttribP1ui");
		func->glVertexAttribP1uiv = (PFNGLVERTEXATTRIBP1UIVPROC)load("glVertexAttribP1uiv");
		func->glVertexAttribP2ui = (PFNGLVERTEXATTRIBP2UIPROC)load("glVertexAttribP2ui");
		func->glVertexAttribP2uiv = (PFNGLVERTEXATTRIBP2UIVPROC)load("glVertexAttribP2uiv");
		func->glVertexAttribP3ui = (PFNGLVERTEXATTRIBP3UIPROC)load("glVertexAttribP3ui");
		func->glVertexAttribP3uiv = (PFNGLVERTEXATTRIBP3UIVPROC)load("glVertexAttribP3uiv");
		func->glVertexAttribP4ui = (PFNGLVERTEXATTRIBP4UIPROC)load("glVertexAttribP4ui");
		func->glVertexAttribP4uiv = (PFNGLVERTEXATTRIBP4UIVPROC)load("glVertexAttribP4uiv");
    }
	static void load_GL_ARB_viewport_array(LoadProc load, GLFunctions* func, GLExtFunctions* ext_func)
    {
		func->glViewportArrayv = (PFNGLVIEWPORTARRAYVPROC)load("glViewportArrayv");
		func->glViewportIndexedf = (PFNGLVIEWPORTINDEXEDFPROC)load("glViewportIndexedf");
		func->glViewportIndexedfv = (PFNGLVIEWPORTINDEXEDFVPROC)load("glViewportIndexedfv");
		func->glScissorArrayv = (PFNGLSCISSORARRAYVPROC)load("glScissorArrayv");
		func->glScissorIndexed = (PFNGLSCISSORINDEXEDPROC)load("glScissorIndexed");
		func->glScissorIndexedv = (PFNGLSCISSORINDEXEDVPROC)load("glScissorIndexedv");
		func->glDepthRangeArrayv = (PFNGLDEPTHRANGEARRAYVPROC)load("glDepthRangeArrayv");
		func->glDepthRangeIndexed = (PFNGLDEPTHRANGEINDEXEDPROC)load("glDepthRangeIndexed");
		func->glGetFloati_v = (PFNGLGETFLOATI_VPROC)load("glGetFloati_v");
		func->glGetDoublei_v = (PFNGLGETDOUBLEI_VPROC)load("glGetDoublei_v");
		ext_func->glDepthRangeArraydvNV = (PFNGLDEPTHRANGEARRAYDVNVPROC)load("glDepthRangeArraydvNV");
		ext_func->glDepthRangeIndexeddNV = (PFNGLDEPTHRANGEINDEXEDDNVPROC)load("glDepthRangeIndexeddNV");
	}
	static void load_GL_ARB_window_pos(LoadProc load, GLExtFunctions* func)
    {
		func->glWindowPos2dARB = (PFNGLWINDOWPOS2DARBPROC)load("glWindowPos2dARB");
		func->glWindowPos2dvARB = (PFNGLWINDOWPOS2DVARBPROC)load("glWindowPos2dvARB");
		func->glWindowPos2fARB = (PFNGLWINDOWPOS2FARBPROC)load("glWindowPos2fARB");
		func->glWindowPos2fvARB = (PFNGLWINDOWPOS2FVARBPROC)load("glWindowPos2fvARB");
		func->glWindowPos2iARB = (PFNGLWINDOWPOS2IARBPROC)load("glWindowPos2iARB");
		func->glWindowPos2ivARB = (PFNGLWINDOWPOS2IVARBPROC)load("glWindowPos2ivARB");
		func->glWindowPos2sARB = (PFNGLWINDOWPOS2SARBPROC)load("glWindowPos2sARB");
		func->glWindowPos2svARB = (PFNGLWINDOWPOS2SVARBPROC)load("glWindowPos2svARB");
		func->glWindowPos3dARB = (PFNGLWINDOWPOS3DARBPROC)load("glWindowPos3dARB");
		func->glWindowPos3dvARB = (PFNGLWINDOWPOS3DVARBPROC)load("glWindowPos3dvARB");
		func->glWindowPos3fARB = (PFNGLWINDOWPOS3FARBPROC)load("glWindowPos3fARB");
		func->glWindowPos3fvARB = (PFNGLWINDOWPOS3FVARBPROC)load("glWindowPos3fvARB");
		func->glWindowPos3iARB = (PFNGLWINDOWPOS3IARBPROC)load("glWindowPos3iARB");
		func->glWindowPos3ivARB = (PFNGLWINDOWPOS3IVARBPROC)load("glWindowPos3ivARB");
		func->glWindowPos3sARB = (PFNGLWINDOWPOS3SARBPROC)load("glWindowPos3sARB");
		func->glWindowPos3svARB = (PFNGLWINDOWPOS3SVARBPROC)load("glWindowPos3svARB");
	}
	static void load_GL_ATI_draw_buffers(LoadProc load, GLExtFunctions* func)
    {
		func->glDrawBuffersATI = (PFNGLDRAWBUFFERSATIPROC)load("glDrawBuffersATI");
	}
	static void load_GL_ATI_element_array(LoadProc load, GLExtFunctions* func)
    {
		func->glElementPointerATI = (PFNGLELEMENTPOINTERATIPROC)load("glElementPointerATI");
		func->glDrawElementArrayATI = (PFNGLDRAWELEMENTARRAYATIPROC)load("glDrawElementArrayATI");
		func->glDrawRangeElementArrayATI = (PFNGLDRAWRANGEELEMENTARRAYATIPROC)load("glDrawRangeElementArrayATI");
	}
	static void load_GL_ATI_envmap_bumpmap(LoadProc load, GLExtFunctions* func)
    {
		func->glTexBumpParameterivATI = (PFNGLTEXBUMPPARAMETERIVATIPROC)load("glTexBumpParameterivATI");
		func->glTexBumpParameterfvATI = (PFNGLTEXBUMPPARAMETERFVATIPROC)load("glTexBumpParameterfvATI");
		func->glGetTexBumpParameterivATI = (PFNGLGETTEXBUMPPARAMETERIVATIPROC)load("glGetTexBumpParameterivATI");
		func->glGetTexBumpParameterfvATI = (PFNGLGETTEXBUMPPARAMETERFVATIPROC)load("glGetTexBumpParameterfvATI");
	}
	static void load_GL_ATI_fragment_shader(LoadProc load, GLExtFunctions* func)
    {
		func->glGenFragmentShadersATI = (PFNGLGENFRAGMENTSHADERSATIPROC)load("glGenFragmentShadersATI");
		func->glBindFragmentShaderATI = (PFNGLBINDFRAGMENTSHADERATIPROC)load("glBindFragmentShaderATI");
		func->glDeleteFragmentShaderATI = (PFNGLDELETEFRAGMENTSHADERATIPROC)load("glDeleteFragmentShaderATI");
		func->glBeginFragmentShaderATI = (PFNGLBEGINFRAGMENTSHADERATIPROC)load("glBeginFragmentShaderATI");
		func->glEndFragmentShaderATI = (PFNGLENDFRAGMENTSHADERATIPROC)load("glEndFragmentShaderATI");
		func->glPassTexCoordATI = (PFNGLPASSTEXCOORDATIPROC)load("glPassTexCoordATI");
		func->glSampleMapATI = (PFNGLSAMPLEMAPATIPROC)load("glSampleMapATI");
		func->glColorFragmentOp1ATI = (PFNGLCOLORFRAGMENTOP1ATIPROC)load("glColorFragmentOp1ATI");
		func->glColorFragmentOp2ATI = (PFNGLCOLORFRAGMENTOP2ATIPROC)load("glColorFragmentOp2ATI");
		func->glColorFragmentOp3ATI = (PFNGLCOLORFRAGMENTOP3ATIPROC)load("glColorFragmentOp3ATI");
		func->glAlphaFragmentOp1ATI = (PFNGLALPHAFRAGMENTOP1ATIPROC)load("glAlphaFragmentOp1ATI");
		func->glAlphaFragmentOp2ATI = (PFNGLALPHAFRAGMENTOP2ATIPROC)load("glAlphaFragmentOp2ATI");
		func->glAlphaFragmentOp3ATI = (PFNGLALPHAFRAGMENTOP3ATIPROC)load("glAlphaFragmentOp3ATI");
		func->glSetFragmentShaderConstantATI = (PFNGLSETFRAGMENTSHADERCONSTANTATIPROC)load("glSetFragmentShaderConstantATI");
	}
	static void load_GL_ATI_map_object_buffer(LoadProc load, GLExtFunctions* func)
    {
		func->glMapObjectBufferATI = (PFNGLMAPOBJECTBUFFERATIPROC)load("glMapObjectBufferATI");
		func->glUnmapObjectBufferATI = (PFNGLUNMAPOBJECTBUFFERATIPROC)load("glUnmapObjectBufferATI");
	}
	static void load_GL_ATI_pn_triangles(LoadProc load, GLExtFunctions* func)
    {
		func->glPNTrianglesiATI = (PFNGLPNTRIANGLESIATIPROC)load("glPNTrianglesiATI");
		func->glPNTrianglesfATI = (PFNGLPNTRIANGLESFATIPROC)load("glPNTrianglesfATI");
	}
	static void load_GL_ATI_separate_stencil(LoadProc load, GLExtFunctions* func)
    {
		func->glStencilOpSeparateATI = (PFNGLSTENCILOPSEPARATEATIPROC)load("glStencilOpSeparateATI");
		func->glStencilFuncSeparateATI = (PFNGLSTENCILFUNCSEPARATEATIPROC)load("glStencilFuncSeparateATI");
	}
	static void load_GL_ATI_vertex_array_object(LoadProc load, GLExtFunctions* func)
    {
		func->glNewObjectBufferATI = (PFNGLNEWOBJECTBUFFERATIPROC)load("glNewObjectBufferATI");
		func->glIsObjectBufferATI = (PFNGLISOBJECTBUFFERATIPROC)load("glIsObjectBufferATI");
		func->glUpdateObjectBufferATI = (PFNGLUPDATEOBJECTBUFFERATIPROC)load("glUpdateObjectBufferATI");
		func->glGetObjectBufferfvATI = (PFNGLGETOBJECTBUFFERFVATIPROC)load("glGetObjectBufferfvATI");
		func->glGetObjectBufferivATI = (PFNGLGETOBJECTBUFFERIVATIPROC)load("glGetObjectBufferivATI");
		func->glFreeObjectBufferATI = (PFNGLFREEOBJECTBUFFERATIPROC)load("glFreeObjectBufferATI");
		func->glArrayObjectATI = (PFNGLARRAYOBJECTATIPROC)load("glArrayObjectATI");
		func->glGetArrayObjectfvATI = (PFNGLGETARRAYOBJECTFVATIPROC)load("glGetArrayObjectfvATI");
		func->glGetArrayObjectivATI = (PFNGLGETARRAYOBJECTIVATIPROC)load("glGetArrayObjectivATI");
		func->glVariantArrayObjectATI = (PFNGLVARIANTARRAYOBJECTATIPROC)load("glVariantArrayObjectATI");
		func->glGetVariantArrayObjectfvATI = (PFNGLGETVARIANTARRAYOBJECTFVATIPROC)load("glGetVariantArrayObjectfvATI");
		func->glGetVariantArrayObjectivATI = (PFNGLGETVARIANTARRAYOBJECTIVATIPROC)load("glGetVariantArrayObjectivATI");
	}
	static void load_GL_ATI_vertex_attrib_array_object(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexAttribArrayObjectATI = (PFNGLVERTEXATTRIBARRAYOBJECTATIPROC)load("glVertexAttribArrayObjectATI");
		func->glGetVertexAttribArrayObjectfvATI = (PFNGLGETVERTEXATTRIBARRAYOBJECTFVATIPROC)load("glGetVertexAttribArrayObjectfvATI");
		func->glGetVertexAttribArrayObjectivATI = (PFNGLGETVERTEXATTRIBARRAYOBJECTIVATIPROC)load("glGetVertexAttribArrayObjectivATI");
	}
	static void load_GL_ATI_vertex_streams(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexStream1sATI = (PFNGLVERTEXSTREAM1SATIPROC)load("glVertexStream1sATI");
		func->glVertexStream1svATI = (PFNGLVERTEXSTREAM1SVATIPROC)load("glVertexStream1svATI");
		func->glVertexStream1iATI = (PFNGLVERTEXSTREAM1IATIPROC)load("glVertexStream1iATI");
		func->glVertexStream1ivATI = (PFNGLVERTEXSTREAM1IVATIPROC)load("glVertexStream1ivATI");
		func->glVertexStream1fATI = (PFNGLVERTEXSTREAM1FATIPROC)load("glVertexStream1fATI");
		func->glVertexStream1fvATI = (PFNGLVERTEXSTREAM1FVATIPROC)load("glVertexStream1fvATI");
		func->glVertexStream1dATI = (PFNGLVERTEXSTREAM1DATIPROC)load("glVertexStream1dATI");
		func->glVertexStream1dvATI = (PFNGLVERTEXSTREAM1DVATIPROC)load("glVertexStream1dvATI");
		func->glVertexStream2sATI = (PFNGLVERTEXSTREAM2SATIPROC)load("glVertexStream2sATI");
		func->glVertexStream2svATI = (PFNGLVERTEXSTREAM2SVATIPROC)load("glVertexStream2svATI");
		func->glVertexStream2iATI = (PFNGLVERTEXSTREAM2IATIPROC)load("glVertexStream2iATI");
		func->glVertexStream2ivATI = (PFNGLVERTEXSTREAM2IVATIPROC)load("glVertexStream2ivATI");
		func->glVertexStream2fATI = (PFNGLVERTEXSTREAM2FATIPROC)load("glVertexStream2fATI");
		func->glVertexStream2fvATI = (PFNGLVERTEXSTREAM2FVATIPROC)load("glVertexStream2fvATI");
		func->glVertexStream2dATI = (PFNGLVERTEXSTREAM2DATIPROC)load("glVertexStream2dATI");
		func->glVertexStream2dvATI = (PFNGLVERTEXSTREAM2DVATIPROC)load("glVertexStream2dvATI");
		func->glVertexStream3sATI = (PFNGLVERTEXSTREAM3SATIPROC)load("glVertexStream3sATI");
		func->glVertexStream3svATI = (PFNGLVERTEXSTREAM3SVATIPROC)load("glVertexStream3svATI");
		func->glVertexStream3iATI = (PFNGLVERTEXSTREAM3IATIPROC)load("glVertexStream3iATI");
		func->glVertexStream3ivATI = (PFNGLVERTEXSTREAM3IVATIPROC)load("glVertexStream3ivATI");
		func->glVertexStream3fATI = (PFNGLVERTEXSTREAM3FATIPROC)load("glVertexStream3fATI");
		func->glVertexStream3fvATI = (PFNGLVERTEXSTREAM3FVATIPROC)load("glVertexStream3fvATI");
		func->glVertexStream3dATI = (PFNGLVERTEXSTREAM3DATIPROC)load("glVertexStream3dATI");
		func->glVertexStream3dvATI = (PFNGLVERTEXSTREAM3DVATIPROC)load("glVertexStream3dvATI");
		func->glVertexStream4sATI = (PFNGLVERTEXSTREAM4SATIPROC)load("glVertexStream4sATI");
		func->glVertexStream4svATI = (PFNGLVERTEXSTREAM4SVATIPROC)load("glVertexStream4svATI");
		func->glVertexStream4iATI = (PFNGLVERTEXSTREAM4IATIPROC)load("glVertexStream4iATI");
		func->glVertexStream4ivATI = (PFNGLVERTEXSTREAM4IVATIPROC)load("glVertexStream4ivATI");
		func->glVertexStream4fATI = (PFNGLVERTEXSTREAM4FATIPROC)load("glVertexStream4fATI");
		func->glVertexStream4fvATI = (PFNGLVERTEXSTREAM4FVATIPROC)load("glVertexStream4fvATI");
		func->glVertexStream4dATI = (PFNGLVERTEXSTREAM4DATIPROC)load("glVertexStream4dATI");
		func->glVertexStream4dvATI = (PFNGLVERTEXSTREAM4DVATIPROC)load("glVertexStream4dvATI");
		func->glNormalStream3bATI = (PFNGLNORMALSTREAM3BATIPROC)load("glNormalStream3bATI");
		func->glNormalStream3bvATI = (PFNGLNORMALSTREAM3BVATIPROC)load("glNormalStream3bvATI");
		func->glNormalStream3sATI = (PFNGLNORMALSTREAM3SATIPROC)load("glNormalStream3sATI");
		func->glNormalStream3svATI = (PFNGLNORMALSTREAM3SVATIPROC)load("glNormalStream3svATI");
		func->glNormalStream3iATI = (PFNGLNORMALSTREAM3IATIPROC)load("glNormalStream3iATI");
		func->glNormalStream3ivATI = (PFNGLNORMALSTREAM3IVATIPROC)load("glNormalStream3ivATI");
		func->glNormalStream3fATI = (PFNGLNORMALSTREAM3FATIPROC)load("glNormalStream3fATI");
		func->glNormalStream3fvATI = (PFNGLNORMALSTREAM3FVATIPROC)load("glNormalStream3fvATI");
		func->glNormalStream3dATI = (PFNGLNORMALSTREAM3DATIPROC)load("glNormalStream3dATI");
		func->glNormalStream3dvATI = (PFNGLNORMALSTREAM3DVATIPROC)load("glNormalStream3dvATI");
		func->glClientActiveVertexStreamATI = (PFNGLCLIENTACTIVEVERTEXSTREAMATIPROC)load("glClientActiveVertexStreamATI");
		func->glVertexBlendEnviATI = (PFNGLVERTEXBLENDENVIATIPROC)load("glVertexBlendEnviATI");
		func->glVertexBlendEnvfATI = (PFNGLVERTEXBLENDENVFATIPROC)load("glVertexBlendEnvfATI");
	}
	static void load_GL_EXT_egl_image_storage(LoadProc load, GLExtFunctions* func)
    {
		func->glEGLImageTargetTexStorageEXT = (PFNGLEGLIMAGETARGETTEXSTORAGEEXTPROC)load("glEGLImageTargetTexStorageEXT");
		func->glEGLImageTargetTextureStorageEXT = (PFNGLEGLIMAGETARGETTEXTURESTORAGEEXTPROC)load("glEGLImageTargetTextureStorageEXT");
	}
	static void load_GL_EXT_bindable_uniform(LoadProc load, GLExtFunctions* func)
    {
		func->glUniformBufferEXT = (PFNGLUNIFORMBUFFEREXTPROC)load("glUniformBufferEXT");
		func->glGetUniformBufferSizeEXT = (PFNGLGETUNIFORMBUFFERSIZEEXTPROC)load("glGetUniformBufferSizeEXT");
		func->glGetUniformOffsetEXT = (PFNGLGETUNIFORMOFFSETEXTPROC)load("glGetUniformOffsetEXT");
	}
	static void load_GL_EXT_blend_color(LoadProc load, GLExtFunctions* func)
    {
		func->glBlendColorEXT = (PFNGLBLENDCOLOREXTPROC)load("glBlendColorEXT");
	}
	static void load_GL_EXT_blend_equation_separate(LoadProc load, GLExtFunctions* func)
    {
		func->glBlendEquationSeparateEXT = (PFNGLBLENDEQUATIONSEPARATEEXTPROC)load("glBlendEquationSeparateEXT");
	}
	static void load_GL_EXT_blend_func_separate(LoadProc load, GLExtFunctions* func)
    {
		func->glBlendFuncSeparateEXT = (PFNGLBLENDFUNCSEPARATEEXTPROC)load("glBlendFuncSeparateEXT");
	}
	static void load_GL_EXT_blend_minmax(LoadProc load, GLExtFunctions* func)
    {
		func->glBlendEquationEXT = (PFNGLBLENDEQUATIONEXTPROC)load("glBlendEquationEXT");
	}
	static void load_GL_EXT_color_subtable(LoadProc load, GLExtFunctions* func)
    {
		func->glColorSubTableEXT = (PFNGLCOLORSUBTABLEEXTPROC)load("glColorSubTableEXT");
		func->glCopyColorSubTableEXT = (PFNGLCOPYCOLORSUBTABLEEXTPROC)load("glCopyColorSubTableEXT");
	}
	static void load_GL_EXT_compiled_vertex_array(LoadProc load, GLExtFunctions* func)
    {
		func->glLockArraysEXT = (PFNGLLOCKARRAYSEXTPROC)load("glLockArraysEXT");
		func->glUnlockArraysEXT = (PFNGLUNLOCKARRAYSEXTPROC)load("glUnlockArraysEXT");
	}
	static void load_GL_EXT_convolution(LoadProc load, GLExtFunctions* func)
    {
		func->glConvolutionFilter1DEXT = (PFNGLCONVOLUTIONFILTER1DEXTPROC)load("glConvolutionFilter1DEXT");
		func->glConvolutionFilter2DEXT = (PFNGLCONVOLUTIONFILTER2DEXTPROC)load("glConvolutionFilter2DEXT");
		func->glConvolutionParameterfEXT = (PFNGLCONVOLUTIONPARAMETERFEXTPROC)load("glConvolutionParameterfEXT");
		func->glConvolutionParameterfvEXT = (PFNGLCONVOLUTIONPARAMETERFVEXTPROC)load("glConvolutionParameterfvEXT");
		func->glConvolutionParameteriEXT = (PFNGLCONVOLUTIONPARAMETERIEXTPROC)load("glConvolutionParameteriEXT");
		func->glConvolutionParameterivEXT = (PFNGLCONVOLUTIONPARAMETERIVEXTPROC)load("glConvolutionParameterivEXT");
		func->glCopyConvolutionFilter1DEXT = (PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC)load("glCopyConvolutionFilter1DEXT");
		func->glCopyConvolutionFilter2DEXT = (PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC)load("glCopyConvolutionFilter2DEXT");
		func->glGetConvolutionFilterEXT = (PFNGLGETCONVOLUTIONFILTEREXTPROC)load("glGetConvolutionFilterEXT");
		func->glGetConvolutionParameterfvEXT = (PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC)load("glGetConvolutionParameterfvEXT");
		func->glGetConvolutionParameterivEXT = (PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC)load("glGetConvolutionParameterivEXT");
		func->glGetSeparableFilterEXT = (PFNGLGETSEPARABLEFILTEREXTPROC)load("glGetSeparableFilterEXT");
		func->glSeparableFilter2DEXT = (PFNGLSEPARABLEFILTER2DEXTPROC)load("glSeparableFilter2DEXT");
	}
	static void load_GL_EXT_coordinate_frame(LoadProc load, GLExtFunctions* func)
    {
		func->glTangent3bEXT = (PFNGLTANGENT3BEXTPROC)load("glTangent3bEXT");
		func->glTangent3bvEXT = (PFNGLTANGENT3BVEXTPROC)load("glTangent3bvEXT");
		func->glTangent3dEXT = (PFNGLTANGENT3DEXTPROC)load("glTangent3dEXT");
		func->glTangent3dvEXT = (PFNGLTANGENT3DVEXTPROC)load("glTangent3dvEXT");
		func->glTangent3fEXT = (PFNGLTANGENT3FEXTPROC)load("glTangent3fEXT");
		func->glTangent3fvEXT = (PFNGLTANGENT3FVEXTPROC)load("glTangent3fvEXT");
		func->glTangent3iEXT = (PFNGLTANGENT3IEXTPROC)load("glTangent3iEXT");
		func->glTangent3ivEXT = (PFNGLTANGENT3IVEXTPROC)load("glTangent3ivEXT");
		func->glTangent3sEXT = (PFNGLTANGENT3SEXTPROC)load("glTangent3sEXT");
		func->glTangent3svEXT = (PFNGLTANGENT3SVEXTPROC)load("glTangent3svEXT");
		func->glBinormal3bEXT = (PFNGLBINORMAL3BEXTPROC)load("glBinormal3bEXT");
		func->glBinormal3bvEXT = (PFNGLBINORMAL3BVEXTPROC)load("glBinormal3bvEXT");
		func->glBinormal3dEXT = (PFNGLBINORMAL3DEXTPROC)load("glBinormal3dEXT");
		func->glBinormal3dvEXT = (PFNGLBINORMAL3DVEXTPROC)load("glBinormal3dvEXT");
		func->glBinormal3fEXT = (PFNGLBINORMAL3FEXTPROC)load("glBinormal3fEXT");
		func->glBinormal3fvEXT = (PFNGLBINORMAL3FVEXTPROC)load("glBinormal3fvEXT");
		func->glBinormal3iEXT = (PFNGLBINORMAL3IEXTPROC)load("glBinormal3iEXT");
		func->glBinormal3ivEXT = (PFNGLBINORMAL3IVEXTPROC)load("glBinormal3ivEXT");
		func->glBinormal3sEXT = (PFNGLBINORMAL3SEXTPROC)load("glBinormal3sEXT");
		func->glBinormal3svEXT = (PFNGLBINORMAL3SVEXTPROC)load("glBinormal3svEXT");
		func->glTangentPointerEXT = (PFNGLTANGENTPOINTEREXTPROC)load("glTangentPointerEXT");
		func->glBinormalPointerEXT = (PFNGLBINORMALPOINTEREXTPROC)load("glBinormalPointerEXT");
	}
	static void load_GL_EXT_copy_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glCopyTexImage1DEXT = (PFNGLCOPYTEXIMAGE1DEXTPROC)load("glCopyTexImage1DEXT");
		func->glCopyTexImage2DEXT = (PFNGLCOPYTEXIMAGE2DEXTPROC)load("glCopyTexImage2DEXT");
		func->glCopyTexSubImage1DEXT = (PFNGLCOPYTEXSUBIMAGE1DEXTPROC)load("glCopyTexSubImage1DEXT");
		func->glCopyTexSubImage2DEXT = (PFNGLCOPYTEXSUBIMAGE2DEXTPROC)load("glCopyTexSubImage2DEXT");
		func->glCopyTexSubImage3DEXT = (PFNGLCOPYTEXSUBIMAGE3DEXTPROC)load("glCopyTexSubImage3DEXT");
	}
	static void load_GL_EXT_cull_vertex(LoadProc load, GLExtFunctions* func)
    {
		func->glCullParameterdvEXT = (PFNGLCULLPARAMETERDVEXTPROC)load("glCullParameterdvEXT");
		func->glCullParameterfvEXT = (PFNGLCULLPARAMETERFVEXTPROC)load("glCullParameterfvEXT");
	}
	static void load_GL_EXT_debug_label(LoadProc load, GLExtFunctions* func)
    {
		func->glLabelObjectEXT = (PFNGLLABELOBJECTEXTPROC)load("glLabelObjectEXT");
		func->glGetObjectLabelEXT = (PFNGLGETOBJECTLABELEXTPROC)load("glGetObjectLabelEXT");
	}
	static void load_GL_EXT_debug_marker(LoadProc load, GLExtFunctions* func)
    {
		func->glInsertEventMarkerEXT = (PFNGLINSERTEVENTMARKEREXTPROC)load("glInsertEventMarkerEXT");
		func->glPushGroupMarkerEXT = (PFNGLPUSHGROUPMARKEREXTPROC)load("glPushGroupMarkerEXT");
		func->glPopGroupMarkerEXT = (PFNGLPOPGROUPMARKEREXTPROC)load("glPopGroupMarkerEXT");
	}
	static void load_GL_EXT_depth_bounds_test(LoadProc load, GLExtFunctions* func)
    {
		func->glDepthBoundsEXT = (PFNGLDEPTHBOUNDSEXTPROC)load("glDepthBoundsEXT");
	}
	static void load_GL_EXT_direct_state_access(LoadProc load, GLExtFunctions* func)
    {
		func->glMatrixLoadfEXT = (PFNGLMATRIXLOADFEXTPROC)load("glMatrixLoadfEXT");
		func->glMatrixLoaddEXT = (PFNGLMATRIXLOADDEXTPROC)load("glMatrixLoaddEXT");
		func->glMatrixMultfEXT = (PFNGLMATRIXMULTFEXTPROC)load("glMatrixMultfEXT");
		func->glMatrixMultdEXT = (PFNGLMATRIXMULTDEXTPROC)load("glMatrixMultdEXT");
		func->glMatrixLoadIdentityEXT = (PFNGLMATRIXLOADIDENTITYEXTPROC)load("glMatrixLoadIdentityEXT");
		func->glMatrixRotatefEXT = (PFNGLMATRIXROTATEFEXTPROC)load("glMatrixRotatefEXT");
		func->glMatrixRotatedEXT = (PFNGLMATRIXROTATEDEXTPROC)load("glMatrixRotatedEXT");
		func->glMatrixScalefEXT = (PFNGLMATRIXSCALEFEXTPROC)load("glMatrixScalefEXT");
		func->glMatrixScaledEXT = (PFNGLMATRIXSCALEDEXTPROC)load("glMatrixScaledEXT");
		func->glMatrixTranslatefEXT = (PFNGLMATRIXTRANSLATEFEXTPROC)load("glMatrixTranslatefEXT");
		func->glMatrixTranslatedEXT = (PFNGLMATRIXTRANSLATEDEXTPROC)load("glMatrixTranslatedEXT");
		func->glMatrixFrustumEXT = (PFNGLMATRIXFRUSTUMEXTPROC)load("glMatrixFrustumEXT");
		func->glMatrixOrthoEXT = (PFNGLMATRIXORTHOEXTPROC)load("glMatrixOrthoEXT");
		func->glMatrixPopEXT = (PFNGLMATRIXPOPEXTPROC)load("glMatrixPopEXT");
		func->glMatrixPushEXT = (PFNGLMATRIXPUSHEXTPROC)load("glMatrixPushEXT");
		func->glClientAttribDefaultEXT = (PFNGLCLIENTATTRIBDEFAULTEXTPROC)load("glClientAttribDefaultEXT");
		func->glPushClientAttribDefaultEXT = (PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC)load("glPushClientAttribDefaultEXT");
		func->glTextureParameterfEXT = (PFNGLTEXTUREPARAMETERFEXTPROC)load("glTextureParameterfEXT");
		func->glTextureParameterfvEXT = (PFNGLTEXTUREPARAMETERFVEXTPROC)load("glTextureParameterfvEXT");
		func->glTextureParameteriEXT = (PFNGLTEXTUREPARAMETERIEXTPROC)load("glTextureParameteriEXT");
		func->glTextureParameterivEXT = (PFNGLTEXTUREPARAMETERIVEXTPROC)load("glTextureParameterivEXT");
		func->glTextureImage1DEXT = (PFNGLTEXTUREIMAGE1DEXTPROC)load("glTextureImage1DEXT");
		func->glTextureImage2DEXT = (PFNGLTEXTUREIMAGE2DEXTPROC)load("glTextureImage2DEXT");
		func->glTextureSubImage1DEXT = (PFNGLTEXTURESUBIMAGE1DEXTPROC)load("glTextureSubImage1DEXT");
		func->glTextureSubImage2DEXT = (PFNGLTEXTURESUBIMAGE2DEXTPROC)load("glTextureSubImage2DEXT");
		func->glCopyTextureImage1DEXT = (PFNGLCOPYTEXTUREIMAGE1DEXTPROC)load("glCopyTextureImage1DEXT");
		func->glCopyTextureImage2DEXT = (PFNGLCOPYTEXTUREIMAGE2DEXTPROC)load("glCopyTextureImage2DEXT");
		func->glCopyTextureSubImage1DEXT = (PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC)load("glCopyTextureSubImage1DEXT");
		func->glCopyTextureSubImage2DEXT = (PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC)load("glCopyTextureSubImage2DEXT");
		func->glGetTextureImageEXT = (PFNGLGETTEXTUREIMAGEEXTPROC)load("glGetTextureImageEXT");
		func->glGetTextureParameterfvEXT = (PFNGLGETTEXTUREPARAMETERFVEXTPROC)load("glGetTextureParameterfvEXT");
		func->glGetTextureParameterivEXT = (PFNGLGETTEXTUREPARAMETERIVEXTPROC)load("glGetTextureParameterivEXT");
		func->glGetTextureLevelParameterfvEXT = (PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC)load("glGetTextureLevelParameterfvEXT");
		func->glGetTextureLevelParameterivEXT = (PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC)load("glGetTextureLevelParameterivEXT");
		func->glTextureImage3DEXT = (PFNGLTEXTUREIMAGE3DEXTPROC)load("glTextureImage3DEXT");
		func->glTextureSubImage3DEXT = (PFNGLTEXTURESUBIMAGE3DEXTPROC)load("glTextureSubImage3DEXT");
		func->glCopyTextureSubImage3DEXT = (PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC)load("glCopyTextureSubImage3DEXT");
		func->glBindMultiTextureEXT = (PFNGLBINDMULTITEXTUREEXTPROC)load("glBindMultiTextureEXT");
		func->glMultiTexCoordPointerEXT = (PFNGLMULTITEXCOORDPOINTEREXTPROC)load("glMultiTexCoordPointerEXT");
		func->glMultiTexEnvfEXT = (PFNGLMULTITEXENVFEXTPROC)load("glMultiTexEnvfEXT");
		func->glMultiTexEnvfvEXT = (PFNGLMULTITEXENVFVEXTPROC)load("glMultiTexEnvfvEXT");
		func->glMultiTexEnviEXT = (PFNGLMULTITEXENVIEXTPROC)load("glMultiTexEnviEXT");
		func->glMultiTexEnvivEXT = (PFNGLMULTITEXENVIVEXTPROC)load("glMultiTexEnvivEXT");
		func->glMultiTexGendEXT = (PFNGLMULTITEXGENDEXTPROC)load("glMultiTexGendEXT");
		func->glMultiTexGendvEXT = (PFNGLMULTITEXGENDVEXTPROC)load("glMultiTexGendvEXT");
		func->glMultiTexGenfEXT = (PFNGLMULTITEXGENFEXTPROC)load("glMultiTexGenfEXT");
		func->glMultiTexGenfvEXT = (PFNGLMULTITEXGENFVEXTPROC)load("glMultiTexGenfvEXT");
		func->glMultiTexGeniEXT = (PFNGLMULTITEXGENIEXTPROC)load("glMultiTexGeniEXT");
		func->glMultiTexGenivEXT = (PFNGLMULTITEXGENIVEXTPROC)load("glMultiTexGenivEXT");
		func->glGetMultiTexEnvfvEXT = (PFNGLGETMULTITEXENVFVEXTPROC)load("glGetMultiTexEnvfvEXT");
		func->glGetMultiTexEnvivEXT = (PFNGLGETMULTITEXENVIVEXTPROC)load("glGetMultiTexEnvivEXT");
		func->glGetMultiTexGendvEXT = (PFNGLGETMULTITEXGENDVEXTPROC)load("glGetMultiTexGendvEXT");
		func->glGetMultiTexGenfvEXT = (PFNGLGETMULTITEXGENFVEXTPROC)load("glGetMultiTexGenfvEXT");
		func->glGetMultiTexGenivEXT = (PFNGLGETMULTITEXGENIVEXTPROC)load("glGetMultiTexGenivEXT");
		func->glMultiTexParameteriEXT = (PFNGLMULTITEXPARAMETERIEXTPROC)load("glMultiTexParameteriEXT");
		func->glMultiTexParameterivEXT = (PFNGLMULTITEXPARAMETERIVEXTPROC)load("glMultiTexParameterivEXT");
		func->glMultiTexParameterfEXT = (PFNGLMULTITEXPARAMETERFEXTPROC)load("glMultiTexParameterfEXT");
		func->glMultiTexParameterfvEXT = (PFNGLMULTITEXPARAMETERFVEXTPROC)load("glMultiTexParameterfvEXT");
		func->glMultiTexImage1DEXT = (PFNGLMULTITEXIMAGE1DEXTPROC)load("glMultiTexImage1DEXT");
		func->glMultiTexImage2DEXT = (PFNGLMULTITEXIMAGE2DEXTPROC)load("glMultiTexImage2DEXT");
		func->glMultiTexSubImage1DEXT = (PFNGLMULTITEXSUBIMAGE1DEXTPROC)load("glMultiTexSubImage1DEXT");
		func->glMultiTexSubImage2DEXT = (PFNGLMULTITEXSUBIMAGE2DEXTPROC)load("glMultiTexSubImage2DEXT");
		func->glCopyMultiTexImage1DEXT = (PFNGLCOPYMULTITEXIMAGE1DEXTPROC)load("glCopyMultiTexImage1DEXT");
		func->glCopyMultiTexImage2DEXT = (PFNGLCOPYMULTITEXIMAGE2DEXTPROC)load("glCopyMultiTexImage2DEXT");
		func->glCopyMultiTexSubImage1DEXT = (PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC)load("glCopyMultiTexSubImage1DEXT");
		func->glCopyMultiTexSubImage2DEXT = (PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC)load("glCopyMultiTexSubImage2DEXT");
		func->glGetMultiTexImageEXT = (PFNGLGETMULTITEXIMAGEEXTPROC)load("glGetMultiTexImageEXT");
		func->glGetMultiTexParameterfvEXT = (PFNGLGETMULTITEXPARAMETERFVEXTPROC)load("glGetMultiTexParameterfvEXT");
		func->glGetMultiTexParameterivEXT = (PFNGLGETMULTITEXPARAMETERIVEXTPROC)load("glGetMultiTexParameterivEXT");
		func->glGetMultiTexLevelParameterfvEXT = (PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC)load("glGetMultiTexLevelParameterfvEXT");
		func->glGetMultiTexLevelParameterivEXT = (PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC)load("glGetMultiTexLevelParameterivEXT");
		func->glMultiTexImage3DEXT = (PFNGLMULTITEXIMAGE3DEXTPROC)load("glMultiTexImage3DEXT");
		func->glMultiTexSubImage3DEXT = (PFNGLMULTITEXSUBIMAGE3DEXTPROC)load("glMultiTexSubImage3DEXT");
		func->glCopyMultiTexSubImage3DEXT = (PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC)load("glCopyMultiTexSubImage3DEXT");
		func->glEnableClientStateIndexedEXT = (PFNGLENABLECLIENTSTATEINDEXEDEXTPROC)load("glEnableClientStateIndexedEXT");
		func->glDisableClientStateIndexedEXT = (PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC)load("glDisableClientStateIndexedEXT");
		func->glGetFloatIndexedvEXT = (PFNGLGETFLOATINDEXEDVEXTPROC)load("glGetFloatIndexedvEXT");
		func->glGetDoubleIndexedvEXT = (PFNGLGETDOUBLEINDEXEDVEXTPROC)load("glGetDoubleIndexedvEXT");
		func->glGetPointerIndexedvEXT = (PFNGLGETPOINTERINDEXEDVEXTPROC)load("glGetPointerIndexedvEXT");
		func->glEnableIndexedEXT = (PFNGLENABLEINDEXEDEXTPROC)load("glEnableIndexedEXT");
		func->glDisableIndexedEXT = (PFNGLDISABLEINDEXEDEXTPROC)load("glDisableIndexedEXT");
		func->glIsEnabledIndexedEXT = (PFNGLISENABLEDINDEXEDEXTPROC)load("glIsEnabledIndexedEXT");
		func->glGetIntegerIndexedvEXT = (PFNGLGETINTEGERINDEXEDVEXTPROC)load("glGetIntegerIndexedvEXT");
		func->glGetBooleanIndexedvEXT = (PFNGLGETBOOLEANINDEXEDVEXTPROC)load("glGetBooleanIndexedvEXT");
		func->glCompressedTextureImage3DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC)load("glCompressedTextureImage3DEXT");
		func->glCompressedTextureImage2DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC)load("glCompressedTextureImage2DEXT");
		func->glCompressedTextureImage1DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC)load("glCompressedTextureImage1DEXT");
		func->glCompressedTextureSubImage3DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC)load("glCompressedTextureSubImage3DEXT");
		func->glCompressedTextureSubImage2DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC)load("glCompressedTextureSubImage2DEXT");
		func->glCompressedTextureSubImage1DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC)load("glCompressedTextureSubImage1DEXT");
		func->glGetCompressedTextureImageEXT = (PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC)load("glGetCompressedTextureImageEXT");
		func->glCompressedMultiTexImage3DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC)load("glCompressedMultiTexImage3DEXT");
		func->glCompressedMultiTexImage2DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC)load("glCompressedMultiTexImage2DEXT");
		func->glCompressedMultiTexImage1DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC)load("glCompressedMultiTexImage1DEXT");
		func->glCompressedMultiTexSubImage3DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC)load("glCompressedMultiTexSubImage3DEXT");
		func->glCompressedMultiTexSubImage2DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC)load("glCompressedMultiTexSubImage2DEXT");
		func->glCompressedMultiTexSubImage1DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC)load("glCompressedMultiTexSubImage1DEXT");
		func->glGetCompressedMultiTexImageEXT = (PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC)load("glGetCompressedMultiTexImageEXT");
		func->glMatrixLoadTransposefEXT = (PFNGLMATRIXLOADTRANSPOSEFEXTPROC)load("glMatrixLoadTransposefEXT");
		func->glMatrixLoadTransposedEXT = (PFNGLMATRIXLOADTRANSPOSEDEXTPROC)load("glMatrixLoadTransposedEXT");
		func->glMatrixMultTransposefEXT = (PFNGLMATRIXMULTTRANSPOSEFEXTPROC)load("glMatrixMultTransposefEXT");
		func->glMatrixMultTransposedEXT = (PFNGLMATRIXMULTTRANSPOSEDEXTPROC)load("glMatrixMultTransposedEXT");
		func->glNamedBufferDataEXT = (PFNGLNAMEDBUFFERDATAEXTPROC)load("glNamedBufferDataEXT");
		func->glNamedBufferSubDataEXT = (PFNGLNAMEDBUFFERSUBDATAEXTPROC)load("glNamedBufferSubDataEXT");
		func->glMapNamedBufferEXT = (PFNGLMAPNAMEDBUFFEREXTPROC)load("glMapNamedBufferEXT");
		func->glUnmapNamedBufferEXT = (PFNGLUNMAPNAMEDBUFFEREXTPROC)load("glUnmapNamedBufferEXT");
		func->glGetNamedBufferParameterivEXT = (PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC)load("glGetNamedBufferParameterivEXT");
		func->glGetNamedBufferPointervEXT = (PFNGLGETNAMEDBUFFERPOINTERVEXTPROC)load("glGetNamedBufferPointervEXT");
		func->glGetNamedBufferSubDataEXT = (PFNGLGETNAMEDBUFFERSUBDATAEXTPROC)load("glGetNamedBufferSubDataEXT");
		func->glProgramUniform1fEXT = (PFNGLPROGRAMUNIFORM1FEXTPROC)load("glProgramUniform1fEXT");
		func->glProgramUniform2fEXT = (PFNGLPROGRAMUNIFORM2FEXTPROC)load("glProgramUniform2fEXT");
		func->glProgramUniform3fEXT = (PFNGLPROGRAMUNIFORM3FEXTPROC)load("glProgramUniform3fEXT");
		func->glProgramUniform4fEXT = (PFNGLPROGRAMUNIFORM4FEXTPROC)load("glProgramUniform4fEXT");
		func->glProgramUniform1iEXT = (PFNGLPROGRAMUNIFORM1IEXTPROC)load("glProgramUniform1iEXT");
		func->glProgramUniform2iEXT = (PFNGLPROGRAMUNIFORM2IEXTPROC)load("glProgramUniform2iEXT");
		func->glProgramUniform3iEXT = (PFNGLPROGRAMUNIFORM3IEXTPROC)load("glProgramUniform3iEXT");
		func->glProgramUniform4iEXT = (PFNGLPROGRAMUNIFORM4IEXTPROC)load("glProgramUniform4iEXT");
		func->glProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC)load("glProgramUniform1fvEXT");
		func->glProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC)load("glProgramUniform2fvEXT");
		func->glProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC)load("glProgramUniform3fvEXT");
		func->glProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC)load("glProgramUniform4fvEXT");
		func->glProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC)load("glProgramUniform1ivEXT");
		func->glProgramUniform2ivEXT = (PFNGLPROGRAMUNIFORM2IVEXTPROC)load("glProgramUniform2ivEXT");
		func->glProgramUniform3ivEXT = (PFNGLPROGRAMUNIFORM3IVEXTPROC)load("glProgramUniform3ivEXT");
		func->glProgramUniform4ivEXT = (PFNGLPROGRAMUNIFORM4IVEXTPROC)load("glProgramUniform4ivEXT");
		func->glProgramUniformMatrix2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC)load("glProgramUniformMatrix2fvEXT");
		func->glProgramUniformMatrix3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC)load("glProgramUniformMatrix3fvEXT");
		func->glProgramUniformMatrix4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC)load("glProgramUniformMatrix4fvEXT");
		func->glProgramUniformMatrix2x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC)load("glProgramUniformMatrix2x3fvEXT");
		func->glProgramUniformMatrix3x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC)load("glProgramUniformMatrix3x2fvEXT");
		func->glProgramUniformMatrix2x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC)load("glProgramUniformMatrix2x4fvEXT");
		func->glProgramUniformMatrix4x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC)load("glProgramUniformMatrix4x2fvEXT");
		func->glProgramUniformMatrix3x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC)load("glProgramUniformMatrix3x4fvEXT");
		func->glProgramUniformMatrix4x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC)load("glProgramUniformMatrix4x3fvEXT");
		func->glTextureBufferEXT = (PFNGLTEXTUREBUFFEREXTPROC)load("glTextureBufferEXT");
		func->glMultiTexBufferEXT = (PFNGLMULTITEXBUFFEREXTPROC)load("glMultiTexBufferEXT");
		func->glTextureParameterIivEXT = (PFNGLTEXTUREPARAMETERIIVEXTPROC)load("glTextureParameterIivEXT");
		func->glTextureParameterIuivEXT = (PFNGLTEXTUREPARAMETERIUIVEXTPROC)load("glTextureParameterIuivEXT");
		func->glGetTextureParameterIivEXT = (PFNGLGETTEXTUREPARAMETERIIVEXTPROC)load("glGetTextureParameterIivEXT");
		func->glGetTextureParameterIuivEXT = (PFNGLGETTEXTUREPARAMETERIUIVEXTPROC)load("glGetTextureParameterIuivEXT");
		func->glMultiTexParameterIivEXT = (PFNGLMULTITEXPARAMETERIIVEXTPROC)load("glMultiTexParameterIivEXT");
		func->glMultiTexParameterIuivEXT = (PFNGLMULTITEXPARAMETERIUIVEXTPROC)load("glMultiTexParameterIuivEXT");
		func->glGetMultiTexParameterIivEXT = (PFNGLGETMULTITEXPARAMETERIIVEXTPROC)load("glGetMultiTexParameterIivEXT");
		func->glGetMultiTexParameterIuivEXT = (PFNGLGETMULTITEXPARAMETERIUIVEXTPROC)load("glGetMultiTexParameterIuivEXT");
		func->glProgramUniform1uiEXT = (PFNGLPROGRAMUNIFORM1UIEXTPROC)load("glProgramUniform1uiEXT");
		func->glProgramUniform2uiEXT = (PFNGLPROGRAMUNIFORM2UIEXTPROC)load("glProgramUniform2uiEXT");
		func->glProgramUniform3uiEXT = (PFNGLPROGRAMUNIFORM3UIEXTPROC)load("glProgramUniform3uiEXT");
		func->glProgramUniform4uiEXT = (PFNGLPROGRAMUNIFORM4UIEXTPROC)load("glProgramUniform4uiEXT");
		func->glProgramUniform1uivEXT = (PFNGLPROGRAMUNIFORM1UIVEXTPROC)load("glProgramUniform1uivEXT");
		func->glProgramUniform2uivEXT = (PFNGLPROGRAMUNIFORM2UIVEXTPROC)load("glProgramUniform2uivEXT");
		func->glProgramUniform3uivEXT = (PFNGLPROGRAMUNIFORM3UIVEXTPROC)load("glProgramUniform3uivEXT");
		func->glProgramUniform4uivEXT = (PFNGLPROGRAMUNIFORM4UIVEXTPROC)load("glProgramUniform4uivEXT");
		func->glNamedProgramLocalParameters4fvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC)load("glNamedProgramLocalParameters4fvEXT");
		func->glNamedProgramLocalParameterI4iEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC)load("glNamedProgramLocalParameterI4iEXT");
		func->glNamedProgramLocalParameterI4ivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC)load("glNamedProgramLocalParameterI4ivEXT");
		func->glNamedProgramLocalParametersI4ivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC)load("glNamedProgramLocalParametersI4ivEXT");
		func->glNamedProgramLocalParameterI4uiEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC)load("glNamedProgramLocalParameterI4uiEXT");
		func->glNamedProgramLocalParameterI4uivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC)load("glNamedProgramLocalParameterI4uivEXT");
		func->glNamedProgramLocalParametersI4uivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC)load("glNamedProgramLocalParametersI4uivEXT");
		func->glGetNamedProgramLocalParameterIivEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC)load("glGetNamedProgramLocalParameterIivEXT");
		func->glGetNamedProgramLocalParameterIuivEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC)load("glGetNamedProgramLocalParameterIuivEXT");
		func->glEnableClientStateiEXT = (PFNGLENABLECLIENTSTATEIEXTPROC)load("glEnableClientStateiEXT");
		func->glDisableClientStateiEXT = (PFNGLDISABLECLIENTSTATEIEXTPROC)load("glDisableClientStateiEXT");
		func->glGetFloati_vEXT = (PFNGLGETFLOATI_VEXTPROC)load("glGetFloati_vEXT");
		func->glGetDoublei_vEXT = (PFNGLGETDOUBLEI_VEXTPROC)load("glGetDoublei_vEXT");
		func->glGetPointeri_vEXT = (PFNGLGETPOINTERI_VEXTPROC)load("glGetPointeri_vEXT");
		func->glNamedProgramStringEXT = (PFNGLNAMEDPROGRAMSTRINGEXTPROC)load("glNamedProgramStringEXT");
		func->glNamedProgramLocalParameter4dEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC)load("glNamedProgramLocalParameter4dEXT");
		func->glNamedProgramLocalParameter4dvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC)load("glNamedProgramLocalParameter4dvEXT");
		func->glNamedProgramLocalParameter4fEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC)load("glNamedProgramLocalParameter4fEXT");
		func->glNamedProgramLocalParameter4fvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC)load("glNamedProgramLocalParameter4fvEXT");
		func->glGetNamedProgramLocalParameterdvEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC)load("glGetNamedProgramLocalParameterdvEXT");
		func->glGetNamedProgramLocalParameterfvEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC)load("glGetNamedProgramLocalParameterfvEXT");
		func->glGetNamedProgramivEXT = (PFNGLGETNAMEDPROGRAMIVEXTPROC)load("glGetNamedProgramivEXT");
		func->glGetNamedProgramStringEXT = (PFNGLGETNAMEDPROGRAMSTRINGEXTPROC)load("glGetNamedProgramStringEXT");
		func->glNamedRenderbufferStorageEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC)load("glNamedRenderbufferStorageEXT");
		func->glGetNamedRenderbufferParameterivEXT = (PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC)load("glGetNamedRenderbufferParameterivEXT");
		func->glNamedRenderbufferStorageMultisampleEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)load("glNamedRenderbufferStorageMultisampleEXT");
		func->glNamedRenderbufferStorageMultisampleCoverageEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC)load("glNamedRenderbufferStorageMultisampleCoverageEXT");
		func->glCheckNamedFramebufferStatusEXT = (PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC)load("glCheckNamedFramebufferStatusEXT");
		func->glNamedFramebufferTexture1DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC)load("glNamedFramebufferTexture1DEXT");
		func->glNamedFramebufferTexture2DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC)load("glNamedFramebufferTexture2DEXT");
		func->glNamedFramebufferTexture3DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC)load("glNamedFramebufferTexture3DEXT");
		func->glNamedFramebufferRenderbufferEXT = (PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC)load("glNamedFramebufferRenderbufferEXT");
		func->glGetNamedFramebufferAttachmentParameterivEXT = (PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)load("glGetNamedFramebufferAttachmentParameterivEXT");
		func->glGenerateTextureMipmapEXT = (PFNGLGENERATETEXTUREMIPMAPEXTPROC)load("glGenerateTextureMipmapEXT");
		func->glGenerateMultiTexMipmapEXT = (PFNGLGENERATEMULTITEXMIPMAPEXTPROC)load("glGenerateMultiTexMipmapEXT");
		func->glFramebufferDrawBufferEXT = (PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC)load("glFramebufferDrawBufferEXT");
		func->glFramebufferDrawBuffersEXT = (PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC)load("glFramebufferDrawBuffersEXT");
		func->glFramebufferReadBufferEXT = (PFNGLFRAMEBUFFERREADBUFFEREXTPROC)load("glFramebufferReadBufferEXT");
		func->glGetFramebufferParameterivEXT = (PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC)load("glGetFramebufferParameterivEXT");
		func->glNamedCopyBufferSubDataEXT = (PFNGLNAMEDCOPYBUFFERSUBDATAEXTPROC)load("glNamedCopyBufferSubDataEXT");
		func->glNamedFramebufferTextureEXT = (PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC)load("glNamedFramebufferTextureEXT");
		func->glNamedFramebufferTextureLayerEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC)load("glNamedFramebufferTextureLayerEXT");
		func->glNamedFramebufferTextureFaceEXT = (PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC)load("glNamedFramebufferTextureFaceEXT");
		func->glTextureRenderbufferEXT = (PFNGLTEXTURERENDERBUFFEREXTPROC)load("glTextureRenderbufferEXT");
		func->glMultiTexRenderbufferEXT = (PFNGLMULTITEXRENDERBUFFEREXTPROC)load("glMultiTexRenderbufferEXT");
		func->glVertexArrayVertexOffsetEXT = (PFNGLVERTEXARRAYVERTEXOFFSETEXTPROC)load("glVertexArrayVertexOffsetEXT");
		func->glVertexArrayColorOffsetEXT = (PFNGLVERTEXARRAYCOLOROFFSETEXTPROC)load("glVertexArrayColorOffsetEXT");
		func->glVertexArrayEdgeFlagOffsetEXT = (PFNGLVERTEXARRAYEDGEFLAGOFFSETEXTPROC)load("glVertexArrayEdgeFlagOffsetEXT");
		func->glVertexArrayIndexOffsetEXT = (PFNGLVERTEXARRAYINDEXOFFSETEXTPROC)load("glVertexArrayIndexOffsetEXT");
		func->glVertexArrayNormalOffsetEXT = (PFNGLVERTEXARRAYNORMALOFFSETEXTPROC)load("glVertexArrayNormalOffsetEXT");
		func->glVertexArrayTexCoordOffsetEXT = (PFNGLVERTEXARRAYTEXCOORDOFFSETEXTPROC)load("glVertexArrayTexCoordOffsetEXT");
		func->glVertexArrayMultiTexCoordOffsetEXT = (PFNGLVERTEXARRAYMULTITEXCOORDOFFSETEXTPROC)load("glVertexArrayMultiTexCoordOffsetEXT");
		func->glVertexArrayFogCoordOffsetEXT = (PFNGLVERTEXARRAYFOGCOORDOFFSETEXTPROC)load("glVertexArrayFogCoordOffsetEXT");
		func->glVertexArraySecondaryColorOffsetEXT = (PFNGLVERTEXARRAYSECONDARYCOLOROFFSETEXTPROC)load("glVertexArraySecondaryColorOffsetEXT");
		func->glVertexArrayVertexAttribOffsetEXT = (PFNGLVERTEXARRAYVERTEXATTRIBOFFSETEXTPROC)load("glVertexArrayVertexAttribOffsetEXT");
		func->glVertexArrayVertexAttribIOffsetEXT = (PFNGLVERTEXARRAYVERTEXATTRIBIOFFSETEXTPROC)load("glVertexArrayVertexAttribIOffsetEXT");
		func->glEnableVertexArrayEXT = (PFNGLENABLEVERTEXARRAYEXTPROC)load("glEnableVertexArrayEXT");
		func->glDisableVertexArrayEXT = (PFNGLDISABLEVERTEXARRAYEXTPROC)load("glDisableVertexArrayEXT");
		func->glEnableVertexArrayAttribEXT = (PFNGLENABLEVERTEXARRAYATTRIBEXTPROC)load("glEnableVertexArrayAttribEXT");
		func->glDisableVertexArrayAttribEXT = (PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC)load("glDisableVertexArrayAttribEXT");
		func->glGetVertexArrayIntegervEXT = (PFNGLGETVERTEXARRAYINTEGERVEXTPROC)load("glGetVertexArrayIntegervEXT");
		func->glGetVertexArrayPointervEXT = (PFNGLGETVERTEXARRAYPOINTERVEXTPROC)load("glGetVertexArrayPointervEXT");
		func->glGetVertexArrayIntegeri_vEXT = (PFNGLGETVERTEXARRAYINTEGERI_VEXTPROC)load("glGetVertexArrayIntegeri_vEXT");
		func->glGetVertexArrayPointeri_vEXT = (PFNGLGETVERTEXARRAYPOINTERI_VEXTPROC)load("glGetVertexArrayPointeri_vEXT");
		func->glMapNamedBufferRangeEXT = (PFNGLMAPNAMEDBUFFERRANGEEXTPROC)load("glMapNamedBufferRangeEXT");
		func->glFlushMappedNamedBufferRangeEXT = (PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEEXTPROC)load("glFlushMappedNamedBufferRangeEXT");
		func->glNamedBufferStorageEXT = (PFNGLNAMEDBUFFERSTORAGEEXTPROC)load("glNamedBufferStorageEXT");
		func->glClearNamedBufferDataEXT = (PFNGLCLEARNAMEDBUFFERDATAEXTPROC)load("glClearNamedBufferDataEXT");
		func->glClearNamedBufferSubDataEXT = (PFNGLCLEARNAMEDBUFFERSUBDATAEXTPROC)load("glClearNamedBufferSubDataEXT");
		func->glNamedFramebufferParameteriEXT = (PFNGLNAMEDFRAMEBUFFERPARAMETERIEXTPROC)load("glNamedFramebufferParameteriEXT");
		func->glGetNamedFramebufferParameterivEXT = (PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVEXTPROC)load("glGetNamedFramebufferParameterivEXT");
		func->glProgramUniform1dEXT = (PFNGLPROGRAMUNIFORM1DEXTPROC)load("glProgramUniform1dEXT");
		func->glProgramUniform2dEXT = (PFNGLPROGRAMUNIFORM2DEXTPROC)load("glProgramUniform2dEXT");
		func->glProgramUniform3dEXT = (PFNGLPROGRAMUNIFORM3DEXTPROC)load("glProgramUniform3dEXT");
		func->glProgramUniform4dEXT = (PFNGLPROGRAMUNIFORM4DEXTPROC)load("glProgramUniform4dEXT");
		func->glProgramUniform1dvEXT = (PFNGLPROGRAMUNIFORM1DVEXTPROC)load("glProgramUniform1dvEXT");
		func->glProgramUniform2dvEXT = (PFNGLPROGRAMUNIFORM2DVEXTPROC)load("glProgramUniform2dvEXT");
		func->glProgramUniform3dvEXT = (PFNGLPROGRAMUNIFORM3DVEXTPROC)load("glProgramUniform3dvEXT");
		func->glProgramUniform4dvEXT = (PFNGLPROGRAMUNIFORM4DVEXTPROC)load("glProgramUniform4dvEXT");
		func->glProgramUniformMatrix2dvEXT = (PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC)load("glProgramUniformMatrix2dvEXT");
		func->glProgramUniformMatrix3dvEXT = (PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC)load("glProgramUniformMatrix3dvEXT");
		func->glProgramUniformMatrix4dvEXT = (PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC)load("glProgramUniformMatrix4dvEXT");
		func->glProgramUniformMatrix2x3dvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC)load("glProgramUniformMatrix2x3dvEXT");
		func->glProgramUniformMatrix2x4dvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC)load("glProgramUniformMatrix2x4dvEXT");
		func->glProgramUniformMatrix3x2dvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC)load("glProgramUniformMatrix3x2dvEXT");
		func->glProgramUniformMatrix3x4dvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC)load("glProgramUniformMatrix3x4dvEXT");
		func->glProgramUniformMatrix4x2dvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC)load("glProgramUniformMatrix4x2dvEXT");
		func->glProgramUniformMatrix4x3dvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC)load("glProgramUniformMatrix4x3dvEXT");
		func->glTextureBufferRangeEXT = (PFNGLTEXTUREBUFFERRANGEEXTPROC)load("glTextureBufferRangeEXT");
		func->glTextureStorage1DEXT = (PFNGLTEXTURESTORAGE1DEXTPROC)load("glTextureStorage1DEXT");
		func->glTextureStorage2DEXT = (PFNGLTEXTURESTORAGE2DEXTPROC)load("glTextureStorage2DEXT");
		func->glTextureStorage3DEXT = (PFNGLTEXTURESTORAGE3DEXTPROC)load("glTextureStorage3DEXT");
		func->glTextureStorage2DMultisampleEXT = (PFNGLTEXTURESTORAGE2DMULTISAMPLEEXTPROC)load("glTextureStorage2DMultisampleEXT");
		func->glTextureStorage3DMultisampleEXT = (PFNGLTEXTURESTORAGE3DMULTISAMPLEEXTPROC)load("glTextureStorage3DMultisampleEXT");
		func->glVertexArrayBindVertexBufferEXT = (PFNGLVERTEXARRAYBINDVERTEXBUFFEREXTPROC)load("glVertexArrayBindVertexBufferEXT");
		func->glVertexArrayVertexAttribFormatEXT = (PFNGLVERTEXARRAYVERTEXATTRIBFORMATEXTPROC)load("glVertexArrayVertexAttribFormatEXT");
		func->glVertexArrayVertexAttribIFormatEXT = (PFNGLVERTEXARRAYVERTEXATTRIBIFORMATEXTPROC)load("glVertexArrayVertexAttribIFormatEXT");
		func->glVertexArrayVertexAttribLFormatEXT = (PFNGLVERTEXARRAYVERTEXATTRIBLFORMATEXTPROC)load("glVertexArrayVertexAttribLFormatEXT");
		func->glVertexArrayVertexAttribBindingEXT = (PFNGLVERTEXARRAYVERTEXATTRIBBINDINGEXTPROC)load("glVertexArrayVertexAttribBindingEXT");
		func->glVertexArrayVertexBindingDivisorEXT = (PFNGLVERTEXARRAYVERTEXBINDINGDIVISOREXTPROC)load("glVertexArrayVertexBindingDivisorEXT");
		func->glVertexArrayVertexAttribLOffsetEXT = (PFNGLVERTEXARRAYVERTEXATTRIBLOFFSETEXTPROC)load("glVertexArrayVertexAttribLOffsetEXT");
		func->glTexturePageCommitmentEXT = (PFNGLTEXTUREPAGECOMMITMENTEXTPROC)load("glTexturePageCommitmentEXT");
		func->glVertexArrayVertexAttribDivisorEXT = (PFNGLVERTEXARRAYVERTEXATTRIBDIVISOREXTPROC)load("glVertexArrayVertexAttribDivisorEXT");
	}
	static void load_GL_EXT_draw_buffers2(LoadProc load, GLExtFunctions* func)
    {
		func->glColorMaskIndexedEXT = (PFNGLCOLORMASKINDEXEDEXTPROC)load("glColorMaskIndexedEXT");
		func->glGetBooleanIndexedvEXT = (PFNGLGETBOOLEANINDEXEDVEXTPROC)load("glGetBooleanIndexedvEXT");
		func->glGetIntegerIndexedvEXT = (PFNGLGETINTEGERINDEXEDVEXTPROC)load("glGetIntegerIndexedvEXT");
		func->glEnableIndexedEXT = (PFNGLENABLEINDEXEDEXTPROC)load("glEnableIndexedEXT");
		func->glDisableIndexedEXT = (PFNGLDISABLEINDEXEDEXTPROC)load("glDisableIndexedEXT");
		func->glIsEnabledIndexedEXT = (PFNGLISENABLEDINDEXEDEXTPROC)load("glIsEnabledIndexedEXT");
	}
	static void load_GL_EXT_draw_instanced(LoadProc load, GLExtFunctions* func)
    {
		func->glDrawArraysInstancedEXT = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)load("glDrawArraysInstancedEXT");
		func->glDrawElementsInstancedEXT = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)load("glDrawElementsInstancedEXT");
	}
	static void load_GL_EXT_draw_range_elements(LoadProc load, GLExtFunctions* func)
    {
		func->glDrawRangeElementsEXT = (PFNGLDRAWRANGEELEMENTSEXTPROC)load("glDrawRangeElementsEXT");
	}
	static void load_GL_EXT_external_buffer(LoadProc load, GLExtFunctions* func)
    {
		func->glBufferStorageExternalEXT = (PFNGLBUFFERSTORAGEEXTERNALEXTPROC)load("glBufferStorageExternalEXT");
		func->glNamedBufferStorageExternalEXT = (PFNGLNAMEDBUFFERSTORAGEEXTERNALEXTPROC)load("glNamedBufferStorageExternalEXT");
	}
	static void load_GL_EXT_fog_coord(LoadProc load, GLExtFunctions* func)
    {
		func->glFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC)load("glFogCoordfEXT");
		func->glFogCoordfvEXT = (PFNGLFOGCOORDFVEXTPROC)load("glFogCoordfvEXT");
		func->glFogCoorddEXT = (PFNGLFOGCOORDDEXTPROC)load("glFogCoorddEXT");
		func->glFogCoorddvEXT = (PFNGLFOGCOORDDVEXTPROC)load("glFogCoorddvEXT");
		func->glFogCoordPointerEXT = (PFNGLFOGCOORDPOINTEREXTPROC)load("glFogCoordPointerEXT");
	}
	static void load_GL_EXT_framebuffer_blit(LoadProc load, GLExtFunctions* func)
    {
		func->glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC)load("glBlitFramebufferEXT");
	}
	static void load_GL_EXT_framebuffer_blit_layers(LoadProc load, GLExtFunctions* func)
    {
		func->glBlitFramebufferLayersEXT = (PFNGLBLITFRAMEBUFFERLAYERSEXTPROC)load("glBlitFramebufferLayersEXT");
		func->glBlitFramebufferLayerEXT = (PFNGLBLITFRAMEBUFFERLAYEREXTPROC)load("glBlitFramebufferLayerEXT");
	}
	static void load_GL_EXT_framebuffer_multisample(LoadProc load, GLExtFunctions* func)
    {
		func->glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)load("glRenderbufferStorageMultisampleEXT");
	}
	static void load_GL_EXT_framebuffer_object(LoadProc load, GLExtFunctions* func)
    {
		func->glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)load("glIsRenderbufferEXT");
		func->glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)load("glBindRenderbufferEXT");
		func->glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)load("glDeleteRenderbuffersEXT");
		func->glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)load("glGenRenderbuffersEXT");
		func->glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)load("glRenderbufferStorageEXT");
		func->glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)load("glGetRenderbufferParameterivEXT");
		func->glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)load("glIsFramebufferEXT");
		func->glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)load("glBindFramebufferEXT");
		func->glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)load("glDeleteFramebuffersEXT");
		func->glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)load("glGenFramebuffersEXT");
		func->glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)load("glCheckFramebufferStatusEXT");
		func->glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)load("glFramebufferTexture1DEXT");
		func->glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)load("glFramebufferTexture2DEXT");
		func->glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)load("glFramebufferTexture3DEXT");
		func->glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)load("glFramebufferRenderbufferEXT");
		func->glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)load("glGetFramebufferAttachmentParameterivEXT");
		func->glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)load("glGenerateMipmapEXT");
	}
	static void load_GL_EXT_geometry_shader4(LoadProc load, GLExtFunctions* func)
    {
		func->glProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC)load("glProgramParameteriEXT");
	}
	static void load_GL_EXT_gpu_program_parameters(LoadProc load, GLExtFunctions* func)
    {
		func->glProgramEnvParameters4fvEXT = (PFNGLPROGRAMENVPARAMETERS4FVEXTPROC)load("glProgramEnvParameters4fvEXT");
		func->glProgramLocalParameters4fvEXT = (PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC)load("glProgramLocalParameters4fvEXT");
	}
	static void load_GL_EXT_gpu_shader4(LoadProc load, GLExtFunctions* func)
    {
		func->glGetUniformuivEXT = (PFNGLGETUNIFORMUIVEXTPROC)load("glGetUniformuivEXT");
		func->glBindFragDataLocationEXT = (PFNGLBINDFRAGDATALOCATIONEXTPROC)load("glBindFragDataLocationEXT");
		func->glGetFragDataLocationEXT = (PFNGLGETFRAGDATALOCATIONEXTPROC)load("glGetFragDataLocationEXT");
		func->glUniform1uiEXT = (PFNGLUNIFORM1UIEXTPROC)load("glUniform1uiEXT");
		func->glUniform2uiEXT = (PFNGLUNIFORM2UIEXTPROC)load("glUniform2uiEXT");
		func->glUniform3uiEXT = (PFNGLUNIFORM3UIEXTPROC)load("glUniform3uiEXT");
		func->glUniform4uiEXT = (PFNGLUNIFORM4UIEXTPROC)load("glUniform4uiEXT");
		func->glUniform1uivEXT = (PFNGLUNIFORM1UIVEXTPROC)load("glUniform1uivEXT");
		func->glUniform2uivEXT = (PFNGLUNIFORM2UIVEXTPROC)load("glUniform2uivEXT");
		func->glUniform3uivEXT = (PFNGLUNIFORM3UIVEXTPROC)load("glUniform3uivEXT");
		func->glUniform4uivEXT = (PFNGLUNIFORM4UIVEXTPROC)load("glUniform4uivEXT");
		func->glVertexAttribI1iEXT = (PFNGLVERTEXATTRIBI1IEXTPROC)load("glVertexAttribI1iEXT");
		func->glVertexAttribI2iEXT = (PFNGLVERTEXATTRIBI2IEXTPROC)load("glVertexAttribI2iEXT");
		func->glVertexAttribI3iEXT = (PFNGLVERTEXATTRIBI3IEXTPROC)load("glVertexAttribI3iEXT");
		func->glVertexAttribI4iEXT = (PFNGLVERTEXATTRIBI4IEXTPROC)load("glVertexAttribI4iEXT");
		func->glVertexAttribI1uiEXT = (PFNGLVERTEXATTRIBI1UIEXTPROC)load("glVertexAttribI1uiEXT");
		func->glVertexAttribI2uiEXT = (PFNGLVERTEXATTRIBI2UIEXTPROC)load("glVertexAttribI2uiEXT");
		func->glVertexAttribI3uiEXT = (PFNGLVERTEXATTRIBI3UIEXTPROC)load("glVertexAttribI3uiEXT");
		func->glVertexAttribI4uiEXT = (PFNGLVERTEXATTRIBI4UIEXTPROC)load("glVertexAttribI4uiEXT");
		func->glVertexAttribI1ivEXT = (PFNGLVERTEXATTRIBI1IVEXTPROC)load("glVertexAttribI1ivEXT");
		func->glVertexAttribI2ivEXT = (PFNGLVERTEXATTRIBI2IVEXTPROC)load("glVertexAttribI2ivEXT");
		func->glVertexAttribI3ivEXT = (PFNGLVERTEXATTRIBI3IVEXTPROC)load("glVertexAttribI3ivEXT");
		func->glVertexAttribI4ivEXT = (PFNGLVERTEXATTRIBI4IVEXTPROC)load("glVertexAttribI4ivEXT");
		func->glVertexAttribI1uivEXT = (PFNGLVERTEXATTRIBI1UIVEXTPROC)load("glVertexAttribI1uivEXT");
		func->glVertexAttribI2uivEXT = (PFNGLVERTEXATTRIBI2UIVEXTPROC)load("glVertexAttribI2uivEXT");
		func->glVertexAttribI3uivEXT = (PFNGLVERTEXATTRIBI3UIVEXTPROC)load("glVertexAttribI3uivEXT");
		func->glVertexAttribI4uivEXT = (PFNGLVERTEXATTRIBI4UIVEXTPROC)load("glVertexAttribI4uivEXT");
		func->glVertexAttribI4bvEXT = (PFNGLVERTEXATTRIBI4BVEXTPROC)load("glVertexAttribI4bvEXT");
		func->glVertexAttribI4svEXT = (PFNGLVERTEXATTRIBI4SVEXTPROC)load("glVertexAttribI4svEXT");
		func->glVertexAttribI4ubvEXT = (PFNGLVERTEXATTRIBI4UBVEXTPROC)load("glVertexAttribI4ubvEXT");
		func->glVertexAttribI4usvEXT = (PFNGLVERTEXATTRIBI4USVEXTPROC)load("glVertexAttribI4usvEXT");
		func->glVertexAttribIPointerEXT = (PFNGLVERTEXATTRIBIPOINTEREXTPROC)load("glVertexAttribIPointerEXT");
		func->glGetVertexAttribIivEXT = (PFNGLGETVERTEXATTRIBIIVEXTPROC)load("glGetVertexAttribIivEXT");
		func->glGetVertexAttribIuivEXT = (PFNGLGETVERTEXATTRIBIUIVEXTPROC)load("glGetVertexAttribIuivEXT");
	}
	static void load_GL_EXT_histogram(LoadProc load, GLExtFunctions* func)
    {
		func->glGetHistogramEXT = (PFNGLGETHISTOGRAMEXTPROC)load("glGetHistogramEXT");
		func->glGetHistogramParameterfvEXT = (PFNGLGETHISTOGRAMPARAMETERFVEXTPROC)load("glGetHistogramParameterfvEXT");
		func->glGetHistogramParameterivEXT = (PFNGLGETHISTOGRAMPARAMETERIVEXTPROC)load("glGetHistogramParameterivEXT");
		func->glGetMinmaxEXT = (PFNGLGETMINMAXEXTPROC)load("glGetMinmaxEXT");
		func->glGetMinmaxParameterfvEXT = (PFNGLGETMINMAXPARAMETERFVEXTPROC)load("glGetMinmaxParameterfvEXT");
		func->glGetMinmaxParameterivEXT = (PFNGLGETMINMAXPARAMETERIVEXTPROC)load("glGetMinmaxParameterivEXT");
		func->glHistogramEXT = (PFNGLHISTOGRAMEXTPROC)load("glHistogramEXT");
		func->glMinmaxEXT = (PFNGLMINMAXEXTPROC)load("glMinmaxEXT");
		func->glResetHistogramEXT = (PFNGLRESETHISTOGRAMEXTPROC)load("glResetHistogramEXT");
		func->glResetMinmaxEXT = (PFNGLRESETMINMAXEXTPROC)load("glResetMinmaxEXT");
	}
	static void load_GL_EXT_index_func(LoadProc load, GLExtFunctions* func)
    {
		func->glIndexFuncEXT = (PFNGLINDEXFUNCEXTPROC)load("glIndexFuncEXT");
	}
	static void load_GL_EXT_index_material(LoadProc load, GLExtFunctions* func)
    {
		func->glIndexMaterialEXT = (PFNGLINDEXMATERIALEXTPROC)load("glIndexMaterialEXT");
	}
	static void load_GL_EXT_light_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glApplyTextureEXT = (PFNGLAPPLYTEXTUREEXTPROC)load("glApplyTextureEXT");
		func->glTextureLightEXT = (PFNGLTEXTURELIGHTEXTPROC)load("glTextureLightEXT");
		func->glTextureMaterialEXT = (PFNGLTEXTUREMATERIALEXTPROC)load("glTextureMaterialEXT");
	}
	static void load_GL_EXT_memory_object(LoadProc load, GLExtFunctions* func)
    {
		func->glGetUnsignedBytevEXT = (PFNGLGETUNSIGNEDBYTEVEXTPROC)load("glGetUnsignedBytevEXT");
		func->glGetUnsignedBytei_vEXT = (PFNGLGETUNSIGNEDBYTEI_VEXTPROC)load("glGetUnsignedBytei_vEXT");
		func->glDeleteMemoryObjectsEXT = (PFNGLDELETEMEMORYOBJECTSEXTPROC)load("glDeleteMemoryObjectsEXT");
		func->glIsMemoryObjectEXT = (PFNGLISMEMORYOBJECTEXTPROC)load("glIsMemoryObjectEXT");
		func->glCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC)load("glCreateMemoryObjectsEXT");
		func->glMemoryObjectParameterivEXT = (PFNGLMEMORYOBJECTPARAMETERIVEXTPROC)load("glMemoryObjectParameterivEXT");
		func->glGetMemoryObjectParameterivEXT = (PFNGLGETMEMORYOBJECTPARAMETERIVEXTPROC)load("glGetMemoryObjectParameterivEXT");
		func->glTexStorageMem2DEXT = (PFNGLTEXSTORAGEMEM2DEXTPROC)load("glTexStorageMem2DEXT");
		func->glTexStorageMem2DMultisampleEXT = (PFNGLTEXSTORAGEMEM2DMULTISAMPLEEXTPROC)load("glTexStorageMem2DMultisampleEXT");
		func->glTexStorageMem3DEXT = (PFNGLTEXSTORAGEMEM3DEXTPROC)load("glTexStorageMem3DEXT");
		func->glTexStorageMem3DMultisampleEXT = (PFNGLTEXSTORAGEMEM3DMULTISAMPLEEXTPROC)load("glTexStorageMem3DMultisampleEXT");
		func->glBufferStorageMemEXT = (PFNGLBUFFERSTORAGEMEMEXTPROC)load("glBufferStorageMemEXT");
		func->glTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC)load("glTextureStorageMem2DEXT");
		func->glTextureStorageMem2DMultisampleEXT = (PFNGLTEXTURESTORAGEMEM2DMULTISAMPLEEXTPROC)load("glTextureStorageMem2DMultisampleEXT");
		func->glTextureStorageMem3DEXT = (PFNGLTEXTURESTORAGEMEM3DEXTPROC)load("glTextureStorageMem3DEXT");
		func->glTextureStorageMem3DMultisampleEXT = (PFNGLTEXTURESTORAGEMEM3DMULTISAMPLEEXTPROC)load("glTextureStorageMem3DMultisampleEXT");
		func->glNamedBufferStorageMemEXT = (PFNGLNAMEDBUFFERSTORAGEMEMEXTPROC)load("glNamedBufferStorageMemEXT");
		func->glTexStorageMem1DEXT = (PFNGLTEXSTORAGEMEM1DEXTPROC)load("glTexStorageMem1DEXT");
		func->glTextureStorageMem1DEXT = (PFNGLTEXTURESTORAGEMEM1DEXTPROC)load("glTextureStorageMem1DEXT");
	}
	static void load_GL_EXT_memory_object_fd(LoadProc load, GLExtFunctions* func)
    {
		func->glImportMemoryFdEXT = (PFNGLIMPORTMEMORYFDEXTPROC)load("glImportMemoryFdEXT");
	}
	static void load_GL_EXT_memory_object_win32(LoadProc load, GLExtFunctions* func)
    {
		func->glImportMemoryWin32HandleEXT = (PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC)load("glImportMemoryWin32HandleEXT");
		func->glImportMemoryWin32NameEXT = (PFNGLIMPORTMEMORYWIN32NAMEEXTPROC)load("glImportMemoryWin32NameEXT");
	}
	static void load_GL_EXT_multi_draw_arrays(LoadProc load, GLExtFunctions* func)
    {
		func->glMultiDrawArraysEXT = (PFNGLMULTIDRAWARRAYSEXTPROC)load("glMultiDrawArraysEXT");
		func->glMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC)load("glMultiDrawElementsEXT");
	}
	static void load_GL_EXT_multisample(LoadProc load, GLExtFunctions* func)
    {
		func->glSampleMaskEXT = (PFNGLSAMPLEMASKEXTPROC)load("glSampleMaskEXT");
		func->glSamplePatternEXT = (PFNGLSAMPLEPATTERNEXTPROC)load("glSamplePatternEXT");
	}
	static void load_GL_EXT_paletted_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glColorTableEXT = (PFNGLCOLORTABLEEXTPROC)load("glColorTableEXT");
		func->glGetColorTableEXT = (PFNGLGETCOLORTABLEEXTPROC)load("glGetColorTableEXT");
		func->glGetColorTableParameterivEXT = (PFNGLGETCOLORTABLEPARAMETERIVEXTPROC)load("glGetColorTableParameterivEXT");
		func->glGetColorTableParameterfvEXT = (PFNGLGETCOLORTABLEPARAMETERFVEXTPROC)load("glGetColorTableParameterfvEXT");
	}
	static void load_GL_EXT_pixel_transform(LoadProc load, GLExtFunctions* func)
    {
		func->glPixelTransformParameteriEXT = (PFNGLPIXELTRANSFORMPARAMETERIEXTPROC)load("glPixelTransformParameteriEXT");
		func->glPixelTransformParameterfEXT = (PFNGLPIXELTRANSFORMPARAMETERFEXTPROC)load("glPixelTransformParameterfEXT");
		func->glPixelTransformParameterivEXT = (PFNGLPIXELTRANSFORMPARAMETERIVEXTPROC)load("glPixelTransformParameterivEXT");
		func->glPixelTransformParameterfvEXT = (PFNGLPIXELTRANSFORMPARAMETERFVEXTPROC)load("glPixelTransformParameterfvEXT");
		func->glGetPixelTransformParameterivEXT = (PFNGLGETPIXELTRANSFORMPARAMETERIVEXTPROC)load("glGetPixelTransformParameterivEXT");
		func->glGetPixelTransformParameterfvEXT = (PFNGLGETPIXELTRANSFORMPARAMETERFVEXTPROC)load("glGetPixelTransformParameterfvEXT");
	}
	static void load_GL_EXT_point_parameters(LoadProc load, GLExtFunctions* func)
    {
		func->glPointParameterfEXT = (PFNGLPOINTPARAMETERFEXTPROC)load("glPointParameterfEXT");
		func->glPointParameterfvEXT = (PFNGLPOINTPARAMETERFVEXTPROC)load("glPointParameterfvEXT");
	}
	static void load_GL_EXT_polygon_offset(LoadProc load, GLExtFunctions* func)
    {
		func->glPolygonOffsetEXT = (PFNGLPOLYGONOFFSETEXTPROC)load("glPolygonOffsetEXT");
	}
	static void load_GL_EXT_polygon_offset_clamp(LoadProc load, GLExtFunctions* func)
    {
		func->glPolygonOffsetClampEXT = (PFNGLPOLYGONOFFSETCLAMPEXTPROC)load("glPolygonOffsetClampEXT");
	}
	static void load_GL_EXT_provoking_vertex(LoadProc load, GLExtFunctions* func)
    {
		func->glProvokingVertexEXT = (PFNGLPROVOKINGVERTEXEXTPROC)load("glProvokingVertexEXT");
	}
	static void load_GL_EXT_raster_multisample(LoadProc load, GLExtFunctions* func)
    {
		func->glRasterSamplesEXT = (PFNGLRASTERSAMPLESEXTPROC)load("glRasterSamplesEXT");
	}
	static void load_GL_EXT_secondary_color(LoadProc load, GLExtFunctions* func)
    {
		func->glSecondaryColor3bEXT = (PFNGLSECONDARYCOLOR3BEXTPROC)load("glSecondaryColor3bEXT");
		func->glSecondaryColor3bvEXT = (PFNGLSECONDARYCOLOR3BVEXTPROC)load("glSecondaryColor3bvEXT");
		func->glSecondaryColor3dEXT = (PFNGLSECONDARYCOLOR3DEXTPROC)load("glSecondaryColor3dEXT");
		func->glSecondaryColor3dvEXT = (PFNGLSECONDARYCOLOR3DVEXTPROC)load("glSecondaryColor3dvEXT");
		func->glSecondaryColor3fEXT = (PFNGLSECONDARYCOLOR3FEXTPROC)load("glSecondaryColor3fEXT");
		func->glSecondaryColor3fvEXT = (PFNGLSECONDARYCOLOR3FVEXTPROC)load("glSecondaryColor3fvEXT");
		func->glSecondaryColor3iEXT = (PFNGLSECONDARYCOLOR3IEXTPROC)load("glSecondaryColor3iEXT");
		func->glSecondaryColor3ivEXT = (PFNGLSECONDARYCOLOR3IVEXTPROC)load("glSecondaryColor3ivEXT");
		func->glSecondaryColor3sEXT = (PFNGLSECONDARYCOLOR3SEXTPROC)load("glSecondaryColor3sEXT");
		func->glSecondaryColor3svEXT = (PFNGLSECONDARYCOLOR3SVEXTPROC)load("glSecondaryColor3svEXT");
		func->glSecondaryColor3ubEXT = (PFNGLSECONDARYCOLOR3UBEXTPROC)load("glSecondaryColor3ubEXT");
		func->glSecondaryColor3ubvEXT = (PFNGLSECONDARYCOLOR3UBVEXTPROC)load("glSecondaryColor3ubvEXT");
		func->glSecondaryColor3uiEXT = (PFNGLSECONDARYCOLOR3UIEXTPROC)load("glSecondaryColor3uiEXT");
		func->glSecondaryColor3uivEXT = (PFNGLSECONDARYCOLOR3UIVEXTPROC)load("glSecondaryColor3uivEXT");
		func->glSecondaryColor3usEXT = (PFNGLSECONDARYCOLOR3USEXTPROC)load("glSecondaryColor3usEXT");
		func->glSecondaryColor3usvEXT = (PFNGLSECONDARYCOLOR3USVEXTPROC)load("glSecondaryColor3usvEXT");
		func->glSecondaryColorPointerEXT = (PFNGLSECONDARYCOLORPOINTEREXTPROC)load("glSecondaryColorPointerEXT");
	}
	static void load_GL_EXT_semaphore(LoadProc load, GLExtFunctions* func)
    {
		func->glGetUnsignedBytevEXT = (PFNGLGETUNSIGNEDBYTEVEXTPROC)load("glGetUnsignedBytevEXT");
		func->glGetUnsignedBytei_vEXT = (PFNGLGETUNSIGNEDBYTEI_VEXTPROC)load("glGetUnsignedBytei_vEXT");
		func->glGenSemaphoresEXT = (PFNGLGENSEMAPHORESEXTPROC)load("glGenSemaphoresEXT");
		func->glDeleteSemaphoresEXT = (PFNGLDELETESEMAPHORESEXTPROC)load("glDeleteSemaphoresEXT");
		func->glIsSemaphoreEXT = (PFNGLISSEMAPHOREEXTPROC)load("glIsSemaphoreEXT");
		func->glSemaphoreParameterui64vEXT = (PFNGLSEMAPHOREPARAMETERUI64VEXTPROC)load("glSemaphoreParameterui64vEXT");
		func->glGetSemaphoreParameterui64vEXT = (PFNGLGETSEMAPHOREPARAMETERUI64VEXTPROC)load("glGetSemaphoreParameterui64vEXT");
		func->glWaitSemaphoreEXT = (PFNGLWAITSEMAPHOREEXTPROC)load("glWaitSemaphoreEXT");
		func->glSignalSemaphoreEXT = (PFNGLSIGNALSEMAPHOREEXTPROC)load("glSignalSemaphoreEXT");
	}
	static void load_GL_EXT_semaphore_fd(LoadProc load, GLExtFunctions* func)
    {
		func->glImportSemaphoreFdEXT = (PFNGLIMPORTSEMAPHOREFDEXTPROC)load("glImportSemaphoreFdEXT");
	}
	static void load_GL_EXT_semaphore_win32(LoadProc load, GLExtFunctions* func)
    {
		func->glImportSemaphoreWin32HandleEXT = (PFNGLIMPORTSEMAPHOREWIN32HANDLEEXTPROC)load("glImportSemaphoreWin32HandleEXT");
		func->glImportSemaphoreWin32NameEXT = (PFNGLIMPORTSEMAPHOREWIN32NAMEEXTPROC)load("glImportSemaphoreWin32NameEXT");
	}
	static void load_GL_EXT_separate_shader_objects(LoadProc load, GLExtFunctions* func)
    {
		func->glUseShaderProgramEXT = (PFNGLUSESHADERPROGRAMEXTPROC)load("glUseShaderProgramEXT");
		func->glActiveProgramEXT = (PFNGLACTIVEPROGRAMEXTPROC)load("glActiveProgramEXT");
		func->glCreateShaderProgramEXT = (PFNGLCREATESHADERPROGRAMEXTPROC)load("glCreateShaderProgramEXT");
		func->glProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC)load("glProgramParameteriEXT");
		func->glProgramUniform1fEXT = (PFNGLPROGRAMUNIFORM1FEXTPROC)load("glProgramUniform1fEXT");
		func->glProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC)load("glProgramUniform1fvEXT");
		func->glProgramUniform1iEXT = (PFNGLPROGRAMUNIFORM1IEXTPROC)load("glProgramUniform1iEXT");
		func->glProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC)load("glProgramUniform1ivEXT");
		func->glProgramUniform2fEXT = (PFNGLPROGRAMUNIFORM2FEXTPROC)load("glProgramUniform2fEXT");
		func->glProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC)load("glProgramUniform2fvEXT");
		func->glProgramUniform2iEXT = (PFNGLPROGRAMUNIFORM2IEXTPROC)load("glProgramUniform2iEXT");
		func->glProgramUniform2ivEXT = (PFNGLPROGRAMUNIFORM2IVEXTPROC)load("glProgramUniform2ivEXT");
		func->glProgramUniform3fEXT = (PFNGLPROGRAMUNIFORM3FEXTPROC)load("glProgramUniform3fEXT");
		func->glProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC)load("glProgramUniform3fvEXT");
		func->glProgramUniform3iEXT = (PFNGLPROGRAMUNIFORM3IEXTPROC)load("glProgramUniform3iEXT");
		func->glProgramUniform3ivEXT = (PFNGLPROGRAMUNIFORM3IVEXTPROC)load("glProgramUniform3ivEXT");
		func->glProgramUniform4fEXT = (PFNGLPROGRAMUNIFORM4FEXTPROC)load("glProgramUniform4fEXT");
		func->glProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC)load("glProgramUniform4fvEXT");
		func->glProgramUniform4iEXT = (PFNGLPROGRAMUNIFORM4IEXTPROC)load("glProgramUniform4iEXT");
		func->glProgramUniform4ivEXT = (PFNGLPROGRAMUNIFORM4IVEXTPROC)load("glProgramUniform4ivEXT");
		func->glProgramUniformMatrix2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC)load("glProgramUniformMatrix2fvEXT");
		func->glProgramUniformMatrix3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC)load("glProgramUniformMatrix3fvEXT");
		func->glProgramUniformMatrix4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC)load("glProgramUniformMatrix4fvEXT");
		func->glProgramUniform1uiEXT = (PFNGLPROGRAMUNIFORM1UIEXTPROC)load("glProgramUniform1uiEXT");
		func->glProgramUniform2uiEXT = (PFNGLPROGRAMUNIFORM2UIEXTPROC)load("glProgramUniform2uiEXT");
		func->glProgramUniform3uiEXT = (PFNGLPROGRAMUNIFORM3UIEXTPROC)load("glProgramUniform3uiEXT");
		func->glProgramUniform4uiEXT = (PFNGLPROGRAMUNIFORM4UIEXTPROC)load("glProgramUniform4uiEXT");
		func->glProgramUniform1uivEXT = (PFNGLPROGRAMUNIFORM1UIVEXTPROC)load("glProgramUniform1uivEXT");
		func->glProgramUniform2uivEXT = (PFNGLPROGRAMUNIFORM2UIVEXTPROC)load("glProgramUniform2uivEXT");
		func->glProgramUniform3uivEXT = (PFNGLPROGRAMUNIFORM3UIVEXTPROC)load("glProgramUniform3uivEXT");
		func->glProgramUniform4uivEXT = (PFNGLPROGRAMUNIFORM4UIVEXTPROC)load("glProgramUniform4uivEXT");
		func->glProgramUniformMatrix2x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC)load("glProgramUniformMatrix2x3fvEXT");
		func->glProgramUniformMatrix3x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC)load("glProgramUniformMatrix3x2fvEXT");
		func->glProgramUniformMatrix2x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC)load("glProgramUniformMatrix2x4fvEXT");
		func->glProgramUniformMatrix4x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC)load("glProgramUniformMatrix4x2fvEXT");
		func->glProgramUniformMatrix3x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC)load("glProgramUniformMatrix3x4fvEXT");
		func->glProgramUniformMatrix4x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC)load("glProgramUniformMatrix4x3fvEXT");
	}
	static void load_GL_EXT_shader_framebuffer_fetch_non_coherent(LoadProc load, GLExtFunctions* func)
    {
		func->glFramebufferFetchBarrierEXT = (PFNGLFRAMEBUFFERFETCHBARRIEREXTPROC)load("glFramebufferFetchBarrierEXT");
	}
	static void load_GL_EXT_shader_image_load_store(LoadProc load, GLExtFunctions* func)
    {
		func->glBindImageTextureEXT = (PFNGLBINDIMAGETEXTUREEXTPROC)load("glBindImageTextureEXT");
		func->glMemoryBarrierEXT = (PFNGLMEMORYBARRIEREXTPROC)load("glMemoryBarrierEXT");
	}
	static void load_GL_EXT_stencil_clear_tag(LoadProc load, GLExtFunctions* func)
    {
		func->glStencilClearTagEXT = (PFNGLSTENCILCLEARTAGEXTPROC)load("glStencilClearTagEXT");
	}
	static void load_GL_EXT_stencil_two_side(LoadProc load, GLExtFunctions* func)
    {
		func->glActiveStencilFaceEXT = (PFNGLACTIVESTENCILFACEEXTPROC)load("glActiveStencilFaceEXT");
	}
	static void load_GL_EXT_subtexture(LoadProc load, GLExtFunctions* func)
    {
		func->glTexSubImage1DEXT = (PFNGLTEXSUBIMAGE1DEXTPROC)load("glTexSubImage1DEXT");
		func->glTexSubImage2DEXT = (PFNGLTEXSUBIMAGE2DEXTPROC)load("glTexSubImage2DEXT");
	}
	static void load_GL_EXT_texture3D(LoadProc load, GLExtFunctions* func)
    {
		func->glTexImage3DEXT = (PFNGLTEXIMAGE3DEXTPROC)load("glTexImage3DEXT");
		func->glTexSubImage3DEXT = (PFNGLTEXSUBIMAGE3DEXTPROC)load("glTexSubImage3DEXT");
	}
	static void load_GL_EXT_texture_array(LoadProc load, GLExtFunctions* func)
    {
		func->glFramebufferTextureLayerEXT = (PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC)load("glFramebufferTextureLayerEXT");
	}
	static void load_GL_EXT_texture_buffer_object(LoadProc load, GLExtFunctions* func)
    {
		func->glTexBufferEXT = (PFNGLTEXBUFFEREXTPROC)load("glTexBufferEXT");
	}
	static void load_GL_EXT_texture_integer(LoadProc load, GLExtFunctions* func)
    {
		func->glTexParameterIivEXT = (PFNGLTEXPARAMETERIIVEXTPROC)load("glTexParameterIivEXT");
		func->glTexParameterIuivEXT = (PFNGLTEXPARAMETERIUIVEXTPROC)load("glTexParameterIuivEXT");
		func->glGetTexParameterIivEXT = (PFNGLGETTEXPARAMETERIIVEXTPROC)load("glGetTexParameterIivEXT");
		func->glGetTexParameterIuivEXT = (PFNGLGETTEXPARAMETERIUIVEXTPROC)load("glGetTexParameterIuivEXT");
		func->glClearColorIiEXT = (PFNGLCLEARCOLORIIEXTPROC)load("glClearColorIiEXT");
		func->glClearColorIuiEXT = (PFNGLCLEARCOLORIUIEXTPROC)load("glClearColorIuiEXT");
	}
	static void load_GL_EXT_texture_object(LoadProc load, GLExtFunctions* func)
    {
		func->glAreTexturesResidentEXT = (PFNGLARETEXTURESRESIDENTEXTPROC)load("glAreTexturesResidentEXT");
		func->glBindTextureEXT = (PFNGLBINDTEXTUREEXTPROC)load("glBindTextureEXT");
		func->glDeleteTexturesEXT = (PFNGLDELETETEXTURESEXTPROC)load("glDeleteTexturesEXT");
		func->glGenTexturesEXT = (PFNGLGENTEXTURESEXTPROC)load("glGenTexturesEXT");
		func->glIsTextureEXT = (PFNGLISTEXTUREEXTPROC)load("glIsTextureEXT");
		func->glPrioritizeTexturesEXT = (PFNGLPRIORITIZETEXTURESEXTPROC)load("glPrioritizeTexturesEXT");
	}
	static void load_GL_EXT_texture_perturb_normal(LoadProc load, GLExtFunctions* func)
    {
		func->glTextureNormalEXT = (PFNGLTEXTURENORMALEXTPROC)load("glTextureNormalEXT");
	}
	static void load_GL_EXT_texture_storage(LoadProc load, GLExtFunctions* func)
    {
		func->glTexStorage1DEXT = (PFNGLTEXSTORAGE1DEXTPROC)load("glTexStorage1DEXT");
		func->glTexStorage2DEXT = (PFNGLTEXSTORAGE2DEXTPROC)load("glTexStorage2DEXT");
		func->glTexStorage3DEXT = (PFNGLTEXSTORAGE3DEXTPROC)load("glTexStorage3DEXT");
		func->glTextureStorage1DEXT = (PFNGLTEXTURESTORAGE1DEXTPROC)load("glTextureStorage1DEXT");
		func->glTextureStorage2DEXT = (PFNGLTEXTURESTORAGE2DEXTPROC)load("glTextureStorage2DEXT");
		func->glTextureStorage3DEXT = (PFNGLTEXTURESTORAGE3DEXTPROC)load("glTextureStorage3DEXT");
	}
	static void load_GL_EXT_timer_query(LoadProc load, GLExtFunctions* func)
    {
		func->glGetQueryObjecti64vEXT = (PFNGLGETQUERYOBJECTI64VEXTPROC)load("glGetQueryObjecti64vEXT");
		func->glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC)load("glGetQueryObjectui64vEXT");
	}
	static void load_GL_EXT_transform_feedback(LoadProc load, GLExtFunctions* func)
    {
		func->glBeginTransformFeedbackEXT = (PFNGLBEGINTRANSFORMFEEDBACKEXTPROC)load("glBeginTransformFeedbackEXT");
		func->glEndTransformFeedbackEXT = (PFNGLENDTRANSFORMFEEDBACKEXTPROC)load("glEndTransformFeedbackEXT");
		func->glBindBufferRangeEXT = (PFNGLBINDBUFFERRANGEEXTPROC)load("glBindBufferRangeEXT");
		func->glBindBufferOffsetEXT = (PFNGLBINDBUFFEROFFSETEXTPROC)load("glBindBufferOffsetEXT");
		func->glBindBufferBaseEXT = (PFNGLBINDBUFFERBASEEXTPROC)load("glBindBufferBaseEXT");
		func->glTransformFeedbackVaryingsEXT = (PFNGLTRANSFORMFEEDBACKVARYINGSEXTPROC)load("glTransformFeedbackVaryingsEXT");
		func->glGetTransformFeedbackVaryingEXT = (PFNGLGETTRANSFORMFEEDBACKVARYINGEXTPROC)load("glGetTransformFeedbackVaryingEXT");
	}
	static void load_GL_EXT_vertex_array(LoadProc load, GLExtFunctions* func)
    {
		func->glArrayElementEXT = (PFNGLARRAYELEMENTEXTPROC)load("glArrayElementEXT");
		func->glColorPointerEXT = (PFNGLCOLORPOINTEREXTPROC)load("glColorPointerEXT");
		func->glDrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC)load("glDrawArraysEXT");
		func->glEdgeFlagPointerEXT = (PFNGLEDGEFLAGPOINTEREXTPROC)load("glEdgeFlagPointerEXT");
		func->glGetPointervEXT = (PFNGLGETPOINTERVEXTPROC)load("glGetPointervEXT");
		func->glIndexPointerEXT = (PFNGLINDEXPOINTEREXTPROC)load("glIndexPointerEXT");
		func->glNormalPointerEXT = (PFNGLNORMALPOINTEREXTPROC)load("glNormalPointerEXT");
		func->glTexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC)load("glTexCoordPointerEXT");
		func->glVertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC)load("glVertexPointerEXT");
	}
	static void load_GL_EXT_vertex_attrib_64bit(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexAttribL1dEXT = (PFNGLVERTEXATTRIBL1DEXTPROC)load("glVertexAttribL1dEXT");
		func->glVertexAttribL2dEXT = (PFNGLVERTEXATTRIBL2DEXTPROC)load("glVertexAttribL2dEXT");
		func->glVertexAttribL3dEXT = (PFNGLVERTEXATTRIBL3DEXTPROC)load("glVertexAttribL3dEXT");
		func->glVertexAttribL4dEXT = (PFNGLVERTEXATTRIBL4DEXTPROC)load("glVertexAttribL4dEXT");
		func->glVertexAttribL1dvEXT = (PFNGLVERTEXATTRIBL1DVEXTPROC)load("glVertexAttribL1dvEXT");
		func->glVertexAttribL2dvEXT = (PFNGLVERTEXATTRIBL2DVEXTPROC)load("glVertexAttribL2dvEXT");
		func->glVertexAttribL3dvEXT = (PFNGLVERTEXATTRIBL3DVEXTPROC)load("glVertexAttribL3dvEXT");
		func->glVertexAttribL4dvEXT = (PFNGLVERTEXATTRIBL4DVEXTPROC)load("glVertexAttribL4dvEXT");
		func->glVertexAttribLPointerEXT = (PFNGLVERTEXATTRIBLPOINTEREXTPROC)load("glVertexAttribLPointerEXT");
		func->glGetVertexAttribLdvEXT = (PFNGLGETVERTEXATTRIBLDVEXTPROC)load("glGetVertexAttribLdvEXT");
	}
	static void load_GL_EXT_vertex_shader(LoadProc load, GLExtFunctions* func)
    {
		func->glBeginVertexShaderEXT = (PFNGLBEGINVERTEXSHADEREXTPROC)load("glBeginVertexShaderEXT");
		func->glEndVertexShaderEXT = (PFNGLENDVERTEXSHADEREXTPROC)load("glEndVertexShaderEXT");
		func->glBindVertexShaderEXT = (PFNGLBINDVERTEXSHADEREXTPROC)load("glBindVertexShaderEXT");
		func->glGenVertexShadersEXT = (PFNGLGENVERTEXSHADERSEXTPROC)load("glGenVertexShadersEXT");
		func->glDeleteVertexShaderEXT = (PFNGLDELETEVERTEXSHADEREXTPROC)load("glDeleteVertexShaderEXT");
		func->glShaderOp1EXT = (PFNGLSHADEROP1EXTPROC)load("glShaderOp1EXT");
		func->glShaderOp2EXT = (PFNGLSHADEROP2EXTPROC)load("glShaderOp2EXT");
		func->glShaderOp3EXT = (PFNGLSHADEROP3EXTPROC)load("glShaderOp3EXT");
		func->glSwizzleEXT = (PFNGLSWIZZLEEXTPROC)load("glSwizzleEXT");
		func->glWriteMaskEXT = (PFNGLWRITEMASKEXTPROC)load("glWriteMaskEXT");
		func->glInsertComponentEXT = (PFNGLINSERTCOMPONENTEXTPROC)load("glInsertComponentEXT");
		func->glExtractComponentEXT = (PFNGLEXTRACTCOMPONENTEXTPROC)load("glExtractComponentEXT");
		func->glGenSymbolsEXT = (PFNGLGENSYMBOLSEXTPROC)load("glGenSymbolsEXT");
		func->glSetInvariantEXT = (PFNGLSETINVARIANTEXTPROC)load("glSetInvariantEXT");
		func->glSetLocalConstantEXT = (PFNGLSETLOCALCONSTANTEXTPROC)load("glSetLocalConstantEXT");
		func->glVariantbvEXT = (PFNGLVARIANTBVEXTPROC)load("glVariantbvEXT");
		func->glVariantsvEXT = (PFNGLVARIANTSVEXTPROC)load("glVariantsvEXT");
		func->glVariantivEXT = (PFNGLVARIANTIVEXTPROC)load("glVariantivEXT");
		func->glVariantfvEXT = (PFNGLVARIANTFVEXTPROC)load("glVariantfvEXT");
		func->glVariantdvEXT = (PFNGLVARIANTDVEXTPROC)load("glVariantdvEXT");
		func->glVariantubvEXT = (PFNGLVARIANTUBVEXTPROC)load("glVariantubvEXT");
		func->glVariantusvEXT = (PFNGLVARIANTUSVEXTPROC)load("glVariantusvEXT");
		func->glVariantuivEXT = (PFNGLVARIANTUIVEXTPROC)load("glVariantuivEXT");
		func->glVariantPointerEXT = (PFNGLVARIANTPOINTEREXTPROC)load("glVariantPointerEXT");
		func->glEnableVariantClientStateEXT = (PFNGLENABLEVARIANTCLIENTSTATEEXTPROC)load("glEnableVariantClientStateEXT");
		func->glDisableVariantClientStateEXT = (PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC)load("glDisableVariantClientStateEXT");
		func->glBindLightParameterEXT = (PFNGLBINDLIGHTPARAMETEREXTPROC)load("glBindLightParameterEXT");
		func->glBindMaterialParameterEXT = (PFNGLBINDMATERIALPARAMETEREXTPROC)load("glBindMaterialParameterEXT");
		func->glBindTexGenParameterEXT = (PFNGLBINDTEXGENPARAMETEREXTPROC)load("glBindTexGenParameterEXT");
		func->glBindTextureUnitParameterEXT = (PFNGLBINDTEXTUREUNITPARAMETEREXTPROC)load("glBindTextureUnitParameterEXT");
		func->glBindParameterEXT = (PFNGLBINDPARAMETEREXTPROC)load("glBindParameterEXT");
		func->glIsVariantEnabledEXT = (PFNGLISVARIANTENABLEDEXTPROC)load("glIsVariantEnabledEXT");
		func->glGetVariantBooleanvEXT = (PFNGLGETVARIANTBOOLEANVEXTPROC)load("glGetVariantBooleanvEXT");
		func->glGetVariantIntegervEXT = (PFNGLGETVARIANTINTEGERVEXTPROC)load("glGetVariantIntegervEXT");
		func->glGetVariantFloatvEXT = (PFNGLGETVARIANTFLOATVEXTPROC)load("glGetVariantFloatvEXT");
		func->glGetVariantPointervEXT = (PFNGLGETVARIANTPOINTERVEXTPROC)load("glGetVariantPointervEXT");
		func->glGetInvariantBooleanvEXT = (PFNGLGETINVARIANTBOOLEANVEXTPROC)load("glGetInvariantBooleanvEXT");
		func->glGetInvariantIntegervEXT = (PFNGLGETINVARIANTINTEGERVEXTPROC)load("glGetInvariantIntegervEXT");
		func->glGetInvariantFloatvEXT = (PFNGLGETINVARIANTFLOATVEXTPROC)load("glGetInvariantFloatvEXT");
		func->glGetLocalConstantBooleanvEXT = (PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC)load("glGetLocalConstantBooleanvEXT");
		func->glGetLocalConstantIntegervEXT = (PFNGLGETLOCALCONSTANTINTEGERVEXTPROC)load("glGetLocalConstantIntegervEXT");
		func->glGetLocalConstantFloatvEXT = (PFNGLGETLOCALCONSTANTFLOATVEXTPROC)load("glGetLocalConstantFloatvEXT");
	}
	static void load_GL_EXT_vertex_weighting(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexWeightfEXT = (PFNGLVERTEXWEIGHTFEXTPROC)load("glVertexWeightfEXT");
		func->glVertexWeightfvEXT = (PFNGLVERTEXWEIGHTFVEXTPROC)load("glVertexWeightfvEXT");
		func->glVertexWeightPointerEXT = (PFNGLVERTEXWEIGHTPOINTEREXTPROC)load("glVertexWeightPointerEXT");
	}
	static void load_GL_EXT_win32_keyed_mutex(LoadProc load, GLExtFunctions* func)
    {
		func->glAcquireKeyedMutexWin32EXT = (PFNGLACQUIREKEYEDMUTEXWIN32EXTPROC)load("glAcquireKeyedMutexWin32EXT");
		func->glReleaseKeyedMutexWin32EXT = (PFNGLRELEASEKEYEDMUTEXWIN32EXTPROC)load("glReleaseKeyedMutexWin32EXT");
	}
	static void load_GL_EXT_window_rectangles(LoadProc load, GLExtFunctions* func)
    {
		func->glWindowRectanglesEXT = (PFNGLWINDOWRECTANGLESEXTPROC)load("glWindowRectanglesEXT");
	}
	static void load_GL_EXT_x11_sync_object(LoadProc load, GLExtFunctions* func)
    {
		func->glImportSyncEXT = (PFNGLIMPORTSYNCEXTPROC)load("glImportSyncEXT");
	}
	static void load_GL_GREMEDY_frame_terminator(LoadProc load, GLExtFunctions* func)
    {
		func->glFrameTerminatorGREMEDY = (PFNGLFRAMETERMINATORGREMEDYPROC)load("glFrameTerminatorGREMEDY");
	}
	static void load_GL_GREMEDY_string_marker(LoadProc load, GLExtFunctions* func)
    {
		func->glStringMarkerGREMEDY = (PFNGLSTRINGMARKERGREMEDYPROC)load("glStringMarkerGREMEDY");
	}
	static void load_GL_HP_image_transform(LoadProc load, GLExtFunctions* func)
    {
		func->glImageTransformParameteriHP = (PFNGLIMAGETRANSFORMPARAMETERIHPPROC)load("glImageTransformParameteriHP");
		func->glImageTransformParameterfHP = (PFNGLIMAGETRANSFORMPARAMETERFHPPROC)load("glImageTransformParameterfHP");
		func->glImageTransformParameterivHP = (PFNGLIMAGETRANSFORMPARAMETERIVHPPROC)load("glImageTransformParameterivHP");
		func->glImageTransformParameterfvHP = (PFNGLIMAGETRANSFORMPARAMETERFVHPPROC)load("glImageTransformParameterfvHP");
		func->glGetImageTransformParameterivHP = (PFNGLGETIMAGETRANSFORMPARAMETERIVHPPROC)load("glGetImageTransformParameterivHP");
		func->glGetImageTransformParameterfvHP = (PFNGLGETIMAGETRANSFORMPARAMETERFVHPPROC)load("glGetImageTransformParameterfvHP");
	}
	static void load_GL_IBM_multimode_draw_arrays(LoadProc load, GLExtFunctions* func)
    {
		func->glMultiModeDrawArraysIBM = (PFNGLMULTIMODEDRAWARRAYSIBMPROC)load("glMultiModeDrawArraysIBM");
		func->glMultiModeDrawElementsIBM = (PFNGLMULTIMODEDRAWELEMENTSIBMPROC)load("glMultiModeDrawElementsIBM");
	}
	static void load_GL_IBM_static_data(LoadProc load, GLExtFunctions* func)
    {
		func->glFlushStaticDataIBM = (PFNGLFLUSHSTATICDATAIBMPROC)load("glFlushStaticDataIBM");
	}
	static void load_GL_IBM_vertex_array_lists(LoadProc load, GLExtFunctions* func)
    {
		func->glColorPointerListIBM = (PFNGLCOLORPOINTERLISTIBMPROC)load("glColorPointerListIBM");
		func->glSecondaryColorPointerListIBM = (PFNGLSECONDARYCOLORPOINTERLISTIBMPROC)load("glSecondaryColorPointerListIBM");
		func->glEdgeFlagPointerListIBM = (PFNGLEDGEFLAGPOINTERLISTIBMPROC)load("glEdgeFlagPointerListIBM");
		func->glFogCoordPointerListIBM = (PFNGLFOGCOORDPOINTERLISTIBMPROC)load("glFogCoordPointerListIBM");
		func->glIndexPointerListIBM = (PFNGLINDEXPOINTERLISTIBMPROC)load("glIndexPointerListIBM");
		func->glNormalPointerListIBM = (PFNGLNORMALPOINTERLISTIBMPROC)load("glNormalPointerListIBM");
		func->glTexCoordPointerListIBM = (PFNGLTEXCOORDPOINTERLISTIBMPROC)load("glTexCoordPointerListIBM");
		func->glVertexPointerListIBM = (PFNGLVERTEXPOINTERLISTIBMPROC)load("glVertexPointerListIBM");
	}
	static void load_gl_ingr_blend_func_separate(LoadProc load, GLExtFunctions* func)
    {
		func->glBlendFuncSeparateINGR = (PFNGLBLENDFUNCSEPARATEINGRPROC)load("glBlendFuncSeparateINGR");
	}
	static void load_GL_INTEL_framebuffer_CMAA(LoadProc load, GLExtFunctions* func)
    {
		func->glApplyFramebufferAttachmentCMAAINTEL = (PFNGLAPPLYFRAMEBUFFERATTACHMENTCMAAINTELPROC)load("glApplyFramebufferAttachmentCMAAINTEL");
	}
	static void load_GL_INTEL_map_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glSyncTextureINTEL = (PFNGLSYNCTEXTUREINTELPROC)load("glSyncTextureINTEL");
		func->glUnmapTexture2DINTEL = (PFNGLUNMAPTEXTURE2DINTELPROC)load("glUnmapTexture2DINTEL");
		func->glMapTexture2DINTEL = (PFNGLMAPTEXTURE2DINTELPROC)load("glMapTexture2DINTEL");
	}
	static void load_GL_INTEL_parallel_arrays(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexPointervINTEL = (PFNGLVERTEXPOINTERVINTELPROC)load("glVertexPointervINTEL");
		func->glNormalPointervINTEL = (PFNGLNORMALPOINTERVINTELPROC)load("glNormalPointervINTEL");
		func->glColorPointervINTEL = (PFNGLCOLORPOINTERVINTELPROC)load("glColorPointervINTEL");
		func->glTexCoordPointervINTEL = (PFNGLTEXCOORDPOINTERVINTELPROC)load("glTexCoordPointervINTEL");
	}
	static void load_GL_INTEL_performance_query(LoadProc load, GLExtFunctions* func)
    {
		func->glBeginPerfQueryINTEL = (PFNGLBEGINPERFQUERYINTELPROC)load("glBeginPerfQueryINTEL");
		func->glCreatePerfQueryINTEL = (PFNGLCREATEPERFQUERYINTELPROC)load("glCreatePerfQueryINTEL");
		func->glDeletePerfQueryINTEL = (PFNGLDELETEPERFQUERYINTELPROC)load("glDeletePerfQueryINTEL");
		func->glEndPerfQueryINTEL = (PFNGLENDPERFQUERYINTELPROC)load("glEndPerfQueryINTEL");
		func->glGetFirstPerfQueryIdINTEL = (PFNGLGETFIRSTPERFQUERYIDINTELPROC)load("glGetFirstPerfQueryIdINTEL");
		func->glGetNextPerfQueryIdINTEL = (PFNGLGETNEXTPERFQUERYIDINTELPROC)load("glGetNextPerfQueryIdINTEL");
		func->glGetPerfCounterInfoINTEL = (PFNGLGETPERFCOUNTERINFOINTELPROC)load("glGetPerfCounterInfoINTEL");
		func->glGetPerfQueryDataINTEL = (PFNGLGETPERFQUERYDATAINTELPROC)load("glGetPerfQueryDataINTEL");
		func->glGetPerfQueryIdByNameINTEL = (PFNGLGETPERFQUERYIDBYNAMEINTELPROC)load("glGetPerfQueryIdByNameINTEL");
		func->glGetPerfQueryInfoINTEL = (PFNGLGETPERFQUERYINFOINTELPROC)load("glGetPerfQueryInfoINTEL");
	}
	static void load_GL_KHR_blend_equation_advanced(LoadProc load, GLExtFunctions* func)
    {
		func->glBlendBarrierKHR = (PFNGLBLENDBARRIERKHRPROC)load("glBlendBarrierKHR");
	}
	static void load_GL_KHR_debug(LoadProc load, GLFunctions* func)
    {
		func->glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC)load("glDebugMessageControl");
		func->glDebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC)load("glDebugMessageInsert");
		func->glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)load("glDebugMessageCallback");
		func->glGetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC)load("glGetDebugMessageLog");
		func->glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC)load("glPushDebugGroup");
		func->glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROC)load("glPopDebugGroup");
		func->glObjectLabel = (PFNGLOBJECTLABELPROC)load("glObjectLabel");
		func->glGetObjectLabel = (PFNGLGETOBJECTLABELPROC)load("glGetObjectLabel");
		func->glObjectPtrLabel = (PFNGLOBJECTPTRLABELPROC)load("glObjectPtrLabel");
		func->glGetObjectPtrLabel = (PFNGLGETOBJECTPTRLABELPROC)load("glGetObjectPtrLabel");
		func->glGetPointerv = (PFNGLGETPOINTERVPROC)load("glGetPointerv");
    }
	static void load_GL_KHR_parallel_shader_compile(LoadProc load, GLExtFunctions* func)
    {
		func->glMaxShaderCompilerThreadsKHR = (PFNGLMAXSHADERCOMPILERTHREADSKHRPROC)load("glMaxShaderCompilerThreadsKHR");
	}
	static void load_GL_KHR_robustness(LoadProc load, GLFunctions* func)
    {
		func->glGetGraphicsResetStatus = (PFNGLGETGRAPHICSRESETSTATUSPROC)load("glGetGraphicsResetStatus");
		func->glReadnPixels = (PFNGLREADNPIXELSPROC)load("glReadnPixels");
		func->glGetnUniformfv = (PFNGLGETNUNIFORMFVPROC)load("glGetnUniformfv");
		func->glGetnUniformiv = (PFNGLGETNUNIFORMIVPROC)load("glGetnUniformiv");
		func->glGetnUniformuiv = (PFNGLGETNUNIFORMUIVPROC)load("glGetnUniformuiv");
	}
	static void load_GL_MESA_framebuffer_flip_y(LoadProc load, GLExtFunctions* func)
    {
		func->glFramebufferParameteriMESA = (PFNGLFRAMEBUFFERPARAMETERIMESAPROC)load("glFramebufferParameteriMESA");
		func->glGetFramebufferParameterivMESA = (PFNGLGETFRAMEBUFFERPARAMETERIVMESAPROC)load("glGetFramebufferParameterivMESA");
	}
	static void load_GL_MESA_resize_buffers(LoadProc load, GLExtFunctions* func)
    {
		func->glResizeBuffersMESA = (PFNGLRESIZEBUFFERSMESAPROC)load("glResizeBuffersMESA");
	}
	static void load_GL_MESA_window_pos(LoadProc load, GLExtFunctions* func)
    {
		func->glWindowPos2dMESA = (PFNGLWINDOWPOS2DMESAPROC)load("glWindowPos2dMESA");
		func->glWindowPos2dvMESA = (PFNGLWINDOWPOS2DVMESAPROC)load("glWindowPos2dvMESA");
		func->glWindowPos2fMESA = (PFNGLWINDOWPOS2FMESAPROC)load("glWindowPos2fMESA");
		func->glWindowPos2fvMESA = (PFNGLWINDOWPOS2FVMESAPROC)load("glWindowPos2fvMESA");
		func->glWindowPos2iMESA = (PFNGLWINDOWPOS2IMESAPROC)load("glWindowPos2iMESA");
		func->glWindowPos2ivMESA = (PFNGLWINDOWPOS2IVMESAPROC)load("glWindowPos2ivMESA");
		func->glWindowPos2sMESA = (PFNGLWINDOWPOS2SMESAPROC)load("glWindowPos2sMESA");
		func->glWindowPos2svMESA = (PFNGLWINDOWPOS2SVMESAPROC)load("glWindowPos2svMESA");
		func->glWindowPos3dMESA = (PFNGLWINDOWPOS3DMESAPROC)load("glWindowPos3dMESA");
		func->glWindowPos3dvMESA = (PFNGLWINDOWPOS3DVMESAPROC)load("glWindowPos3dvMESA");
		func->glWindowPos3fMESA = (PFNGLWINDOWPOS3FMESAPROC)load("glWindowPos3fMESA");
		func->glWindowPos3fvMESA = (PFNGLWINDOWPOS3FVMESAPROC)load("glWindowPos3fvMESA");
		func->glWindowPos3iMESA = (PFNGLWINDOWPOS3IMESAPROC)load("glWindowPos3iMESA");
		func->glWindowPos3ivMESA = (PFNGLWINDOWPOS3IVMESAPROC)load("glWindowPos3ivMESA");
		func->glWindowPos3sMESA = (PFNGLWINDOWPOS3SMESAPROC)load("glWindowPos3sMESA");
		func->glWindowPos3svMESA = (PFNGLWINDOWPOS3SVMESAPROC)load("glWindowPos3svMESA");
		func->glWindowPos4dMESA = (PFNGLWINDOWPOS4DMESAPROC)load("glWindowPos4dMESA");
		func->glWindowPos4dvMESA = (PFNGLWINDOWPOS4DVMESAPROC)load("glWindowPos4dvMESA");
		func->glWindowPos4fMESA = (PFNGLWINDOWPOS4FMESAPROC)load("glWindowPos4fMESA");
		func->glWindowPos4fvMESA = (PFNGLWINDOWPOS4FVMESAPROC)load("glWindowPos4fvMESA");
		func->glWindowPos4iMESA = (PFNGLWINDOWPOS4IMESAPROC)load("glWindowPos4iMESA");
		func->glWindowPos4ivMESA = (PFNGLWINDOWPOS4IVMESAPROC)load("glWindowPos4ivMESA");
		func->glWindowPos4sMESA = (PFNGLWINDOWPOS4SMESAPROC)load("glWindowPos4sMESA");
		func->glWindowPos4svMESA = (PFNGLWINDOWPOS4SVMESAPROC)load("glWindowPos4svMESA");
	}
	static void load_GL_NVX_conditional_render(LoadProc load, GLExtFunctions* func)
    {
		func->glBeginConditionalRenderNVX = (PFNGLBEGINCONDITIONALRENDERNVXPROC)load("glBeginConditionalRenderNVX");
		func->glEndConditionalRenderNVX = (PFNGLENDCONDITIONALRENDERNVXPROC)load("glEndConditionalRenderNVX");
	}
	static void load_GL_NVX_gpu_multicast2(LoadProc load, GLExtFunctions* func)
    {
		func->glUploadGpuMaskNVX = (PFNGLUPLOADGPUMASKNVXPROC)load("glUploadGpuMaskNVX");
		func->glMulticastViewportArrayvNVX = (PFNGLMULTICASTVIEWPORTARRAYVNVXPROC)load("glMulticastViewportArrayvNVX");
		func->glMulticastViewportPositionWScaleNVX = (PFNGLMULTICASTVIEWPORTPOSITIONWSCALENVXPROC)load("glMulticastViewportPositionWScaleNVX");
		func->glMulticastScissorArrayvNVX = (PFNGLMULTICASTSCISSORARRAYVNVXPROC)load("glMulticastScissorArrayvNVX");
		func->glAsyncCopyBufferSubDataNVX = (PFNGLASYNCCOPYBUFFERSUBDATANVXPROC)load("glAsyncCopyBufferSubDataNVX");
		func->glAsyncCopyImageSubDataNVX = (PFNGLASYNCCOPYIMAGESUBDATANVXPROC)load("glAsyncCopyImageSubDataNVX");
	}
	static void load_GL_NVX_linked_gpu_multicast(LoadProc load, GLExtFunctions* func)
    {
		func->glLGPUNamedBufferSubDataNVX = (PFNGLLGPUNAMEDBUFFERSUBDATANVXPROC)load("glLGPUNamedBufferSubDataNVX");
		func->glLGPUCopyImageSubDataNVX = (PFNGLLGPUCOPYIMAGESUBDATANVXPROC)load("glLGPUCopyImageSubDataNVX");
		func->glLGPUInterlockNVX = (PFNGLLGPUINTERLOCKNVXPROC)load("glLGPUInterlockNVX");
	}
	static void load_GL_NVX_progress_fence(LoadProc load, GLExtFunctions* func)
    {
		func->glCreateProgressFenceNVX = (PFNGLCREATEPROGRESSFENCENVXPROC)load("glCreateProgressFenceNVX");
		func->glSignalSemaphoreui64NVX = (PFNGLSIGNALSEMAPHOREUI64NVXPROC)load("glSignalSemaphoreui64NVX");
		func->glWaitSemaphoreui64NVX = (PFNGLWAITSEMAPHOREUI64NVXPROC)load("glWaitSemaphoreui64NVX");
		func->glClientWaitSemaphoreui64NVX = (PFNGLCLIENTWAITSEMAPHOREUI64NVXPROC)load("glClientWaitSemaphoreui64NVX");
	}
	static void load_GL_NV_alpha_to_coverage_dither_control(LoadProc load, GLExtFunctions* func)
    {
		func->glAlphaToCoverageDitherControlNV = (PFNGLALPHATOCOVERAGEDITHERCONTROLNVPROC)load("glAlphaToCoverageDitherControlNV");
	}
	static void load_GL_NV_bindless_multi_draw_indirect(LoadProc load, GLExtFunctions* func)
    {
		func->glMultiDrawArraysIndirectBindlessNV = (PFNGLMULTIDRAWARRAYSINDIRECTBINDLESSNVPROC)load("glMultiDrawArraysIndirectBindlessNV");
		func->glMultiDrawElementsIndirectBindlessNV = (PFNGLMULTIDRAWELEMENTSINDIRECTBINDLESSNVPROC)load("glMultiDrawElementsIndirectBindlessNV");
	}
	static void load_GL_NV_bindless_multi_draw_indirect_count(LoadProc load, GLExtFunctions* func)
    {
		func->glMultiDrawArraysIndirectBindlessCountNV = (PFNGLMULTIDRAWARRAYSINDIRECTBINDLESSCOUNTNVPROC)load("glMultiDrawArraysIndirectBindlessCountNV");
		func->glMultiDrawElementsIndirectBindlessCountNV = (PFNGLMULTIDRAWELEMENTSINDIRECTBINDLESSCOUNTNVPROC)load("glMultiDrawElementsIndirectBindlessCountNV");
	}
	static void load_GL_NV_bindless_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glGetTextureHandleNV = (PFNGLGETTEXTUREHANDLENVPROC)load("glGetTextureHandleNV");
		func->glGetTextureSamplerHandleNV = (PFNGLGETTEXTURESAMPLERHANDLENVPROC)load("glGetTextureSamplerHandleNV");
		func->glMakeTextureHandleResidentNV = (PFNGLMAKETEXTUREHANDLERESIDENTNVPROC)load("glMakeTextureHandleResidentNV");
		func->glMakeTextureHandleNonResidentNV = (PFNGLMAKETEXTUREHANDLENONRESIDENTNVPROC)load("glMakeTextureHandleNonResidentNV");
		func->glGetImageHandleNV = (PFNGLGETIMAGEHANDLENVPROC)load("glGetImageHandleNV");
		func->glMakeImageHandleResidentNV = (PFNGLMAKEIMAGEHANDLERESIDENTNVPROC)load("glMakeImageHandleResidentNV");
		func->glMakeImageHandleNonResidentNV = (PFNGLMAKEIMAGEHANDLENONRESIDENTNVPROC)load("glMakeImageHandleNonResidentNV");
		func->glUniformHandleui64NV = (PFNGLUNIFORMHANDLEUI64NVPROC)load("glUniformHandleui64NV");
		func->glUniformHandleui64vNV = (PFNGLUNIFORMHANDLEUI64VNVPROC)load("glUniformHandleui64vNV");
		func->glProgramUniformHandleui64NV = (PFNGLPROGRAMUNIFORMHANDLEUI64NVPROC)load("glProgramUniformHandleui64NV");
		func->glProgramUniformHandleui64vNV = (PFNGLPROGRAMUNIFORMHANDLEUI64VNVPROC)load("glProgramUniformHandleui64vNV");
		func->glIsTextureHandleResidentNV = (PFNGLISTEXTUREHANDLERESIDENTNVPROC)load("glIsTextureHandleResidentNV");
		func->glIsImageHandleResidentNV = (PFNGLISIMAGEHANDLERESIDENTNVPROC)load("glIsImageHandleResidentNV");
	}
	static void load_GL_NV_blend_equation_advanced(LoadProc load, GLExtFunctions* func)
    {
		func->glBlendParameteriNV = (PFNGLBLENDPARAMETERINVPROC)load("glBlendParameteriNV");
		func->glBlendBarrierNV = (PFNGLBLENDBARRIERNVPROC)load("glBlendBarrierNV");
	}
	static void load_GL_NV_clip_space_w_scaling(LoadProc load, GLExtFunctions* func)
    {
		func->glViewportPositionWScaleNV = (PFNGLVIEWPORTPOSITIONWSCALENVPROC)load("glViewportPositionWScaleNV");
	}
	static void load_GL_NV_command_list(LoadProc load, GLExtFunctions* func)
    {
		func->glCreateStatesNV = (PFNGLCREATESTATESNVPROC)load("glCreateStatesNV");
		func->glDeleteStatesNV = (PFNGLDELETESTATESNVPROC)load("glDeleteStatesNV");
		func->glIsStateNV = (PFNGLISSTATENVPROC)load("glIsStateNV");
		func->glStateCaptureNV = (PFNGLSTATECAPTURENVPROC)load("glStateCaptureNV");
		func->glGetCommandHeaderNV = (PFNGLGETCOMMANDHEADERNVPROC)load("glGetCommandHeaderNV");
		func->glGetStageIndexNV = (PFNGLGETSTAGEINDEXNVPROC)load("glGetStageIndexNV");
		func->glDrawCommandsNV = (PFNGLDRAWCOMMANDSNVPROC)load("glDrawCommandsNV");
		func->glDrawCommandsAddressNV = (PFNGLDRAWCOMMANDSADDRESSNVPROC)load("glDrawCommandsAddressNV");
		func->glDrawCommandsStatesNV = (PFNGLDRAWCOMMANDSSTATESNVPROC)load("glDrawCommandsStatesNV");
		func->glDrawCommandsStatesAddressNV = (PFNGLDRAWCOMMANDSSTATESADDRESSNVPROC)load("glDrawCommandsStatesAddressNV");
		func->glCreateCommandListsNV = (PFNGLCREATECOMMANDLISTSNVPROC)load("glCreateCommandListsNV");
		func->glDeleteCommandListsNV = (PFNGLDELETECOMMANDLISTSNVPROC)load("glDeleteCommandListsNV");
		func->glIsCommandListNV = (PFNGLISCOMMANDLISTNVPROC)load("glIsCommandListNV");
		func->glListDrawCommandsStatesClientNV = (PFNGLLISTDRAWCOMMANDSSTATESCLIENTNVPROC)load("glListDrawCommandsStatesClientNV");
		func->glCommandListSegmentsNV = (PFNGLCOMMANDLISTSEGMENTSNVPROC)load("glCommandListSegmentsNV");
		func->glCompileCommandListNV = (PFNGLCOMPILECOMMANDLISTNVPROC)load("glCompileCommandListNV");
		func->glCallCommandListNV = (PFNGLCALLCOMMANDLISTNVPROC)load("glCallCommandListNV");
	}
	static void load_GL_NV_conditional_render(LoadProc load, GLExtFunctions* func)
    {
		func->glBeginConditionalRenderNV = (PFNGLBEGINCONDITIONALRENDERNVPROC)load("glBeginConditionalRenderNV");
		func->glEndConditionalRenderNV = (PFNGLENDCONDITIONALRENDERNVPROC)load("glEndConditionalRenderNV");
	}
	static void load_GL_NV_conservative_raster(LoadProc load, GLExtFunctions* func)
    {
		func->glSubpixelPrecisionBiasNV = (PFNGLSUBPIXELPRECISIONBIASNVPROC)load("glSubpixelPrecisionBiasNV");
	}
	static void load_GL_NV_conservative_raster_dilate(LoadProc load, GLExtFunctions* func)
    {
		func->glConservativeRasterParameterfNV = (PFNGLCONSERVATIVERASTERPARAMETERFNVPROC)load("glConservativeRasterParameterfNV");
	}
	static void load_GL_NV_conservative_raster_pre_snap_triangles(LoadProc load, GLExtFunctions* func)
    {
		func->glConservativeRasterParameteriNV = (PFNGLCONSERVATIVERASTERPARAMETERINVPROC)load("glConservativeRasterParameteriNV");
	}
	static void load_GL_NV_copy_image(LoadProc load, GLExtFunctions* func)
    {
		func->glCopyImageSubDataNV = (PFNGLCOPYIMAGESUBDATANVPROC)load("glCopyImageSubDataNV");
	}
	static void load_GL_NV_depth_buffer_float(LoadProc load, GLExtFunctions* func)
    {
		func->glDepthRangedNV = (PFNGLDEPTHRANGEDNVPROC)load("glDepthRangedNV");
		func->glClearDepthdNV = (PFNGLCLEARDEPTHDNVPROC)load("glClearDepthdNV");
		func->glDepthBoundsdNV = (PFNGLDEPTHBOUNDSDNVPROC)load("glDepthBoundsdNV");
	}
	static void load_GL_NV_draw_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glDrawTextureNV = (PFNGLDRAWTEXTURENVPROC)load("glDrawTextureNV");
	}
	static void load_GL_NV_draw_vulkan_image(LoadProc load, GLExtFunctions* func)
    {
		func->glDrawVkImageNV = (PFNGLDRAWVKIMAGENVPROC)load("glDrawVkImageNV");
		func->glGetVkProcAddrNV = (PFNGLGETVKPROCADDRNVPROC)load("glGetVkProcAddrNV");
		func->glWaitVkSemaphoreNV = (PFNGLWAITVKSEMAPHORENVPROC)load("glWaitVkSemaphoreNV");
		func->glSignalVkSemaphoreNV = (PFNGLSIGNALVKSEMAPHORENVPROC)load("glSignalVkSemaphoreNV");
		func->glSignalVkFenceNV = (PFNGLSIGNALVKFENCENVPROC)load("glSignalVkFenceNV");
	}
	static void load_GL_NV_evaluators(LoadProc load, GLExtFunctions* func)
    {
		func->glMapControlPointsNV = (PFNGLMAPCONTROLPOINTSNVPROC)load("glMapControlPointsNV");
		func->glMapParameterivNV = (PFNGLMAPPARAMETERIVNVPROC)load("glMapParameterivNV");
		func->glMapParameterfvNV = (PFNGLMAPPARAMETERFVNVPROC)load("glMapParameterfvNV");
		func->glGetMapControlPointsNV = (PFNGLGETMAPCONTROLPOINTSNVPROC)load("glGetMapControlPointsNV");
		func->glGetMapParameterivNV = (PFNGLGETMAPPARAMETERIVNVPROC)load("glGetMapParameterivNV");
		func->glGetMapParameterfvNV = (PFNGLGETMAPPARAMETERFVNVPROC)load("glGetMapParameterfvNV");
		func->glGetMapAttribParameterivNV = (PFNGLGETMAPATTRIBPARAMETERIVNVPROC)load("glGetMapAttribParameterivNV");
		func->glGetMapAttribParameterfvNV = (PFNGLGETMAPATTRIBPARAMETERFVNVPROC)load("glGetMapAttribParameterfvNV");
		func->glEvalMapsNV = (PFNGLEVALMAPSNVPROC)load("glEvalMapsNV");
	}
	static void load_GL_NV_explicit_multisample(LoadProc load, GLExtFunctions* func)
    {
		func->glGetMultisamplefvNV = (PFNGLGETMULTISAMPLEFVNVPROC)load("glGetMultisamplefvNV");
		func->glSampleMaskIndexedNV = (PFNGLSAMPLEMASKINDEXEDNVPROC)load("glSampleMaskIndexedNV");
		func->glTexRenderbufferNV = (PFNGLTEXRENDERBUFFERNVPROC)load("glTexRenderbufferNV");
	}
	static void load_GL_NV_fence(LoadProc load, GLExtFunctions* func)
    {
		func->glDeleteFencesNV = (PFNGLDELETEFENCESNVPROC)load("glDeleteFencesNV");
		func->glGenFencesNV = (PFNGLGENFENCESNVPROC)load("glGenFencesNV");
		func->glIsFenceNV = (PFNGLISFENCENVPROC)load("glIsFenceNV");
		func->glTestFenceNV = (PFNGLTESTFENCENVPROC)load("glTestFenceNV");
		func->glGetFenceivNV = (PFNGLGETFENCEIVNVPROC)load("glGetFenceivNV");
		func->glFinishFenceNV = (PFNGLFINISHFENCENVPROC)load("glFinishFenceNV");
		func->glSetFenceNV = (PFNGLSETFENCENVPROC)load("glSetFenceNV");
	}
	static void load_GL_NV_fragment_coverage_to_color(LoadProc load, GLExtFunctions* func)
    {
		func->glFragmentCoverageColorNV = (PFNGLFRAGMENTCOVERAGECOLORNVPROC)load("glFragmentCoverageColorNV");
	}
	static void load_GL_NV_fragment_program(LoadProc load, GLExtFunctions* func)
    {
		func->glProgramNamedParameter4fNV = (PFNGLPROGRAMNAMEDPARAMETER4FNVPROC)load("glProgramNamedParameter4fNV");
		func->glProgramNamedParameter4fvNV = (PFNGLPROGRAMNAMEDPARAMETER4FVNVPROC)load("glProgramNamedParameter4fvNV");
		func->glProgramNamedParameter4dNV = (PFNGLPROGRAMNAMEDPARAMETER4DNVPROC)load("glProgramNamedParameter4dNV");
		func->glProgramNamedParameter4dvNV = (PFNGLPROGRAMNAMEDPARAMETER4DVNVPROC)load("glProgramNamedParameter4dvNV");
		func->glGetProgramNamedParameterfvNV = (PFNGLGETPROGRAMNAMEDPARAMETERFVNVPROC)load("glGetProgramNamedParameterfvNV");
		func->glGetProgramNamedParameterdvNV = (PFNGLGETPROGRAMNAMEDPARAMETERDVNVPROC)load("glGetProgramNamedParameterdvNV");
	}
	static void load_GL_NV_framebuffer_mixed_samples(LoadProc load, GLExtFunctions* func)
    {
		func->glRasterSamplesEXT = (PFNGLRASTERSAMPLESEXTPROC)load("glRasterSamplesEXT");
		func->glCoverageModulationTableNV = (PFNGLCOVERAGEMODULATIONTABLENVPROC)load("glCoverageModulationTableNV");
		func->glGetCoverageModulationTableNV = (PFNGLGETCOVERAGEMODULATIONTABLENVPROC)load("glGetCoverageModulationTableNV");
		func->glCoverageModulationNV = (PFNGLCOVERAGEMODULATIONNVPROC)load("glCoverageModulationNV");
	}
	static void load_GL_NV_framebuffer_multisample_coverage(LoadProc load, GLExtFunctions* func)
    {
		func->glRenderbufferStorageMultisampleCoverageNV = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC)load("glRenderbufferStorageMultisampleCoverageNV");
	}
	static void load_GL_NV_geometry_program4(LoadProc load, GLExtFunctions* func)
    {
		func->glProgramVertexLimitNV = (PFNGLPROGRAMVERTEXLIMITNVPROC)load("glProgramVertexLimitNV");
		func->glFramebufferTextureEXT = (PFNGLFRAMEBUFFERTEXTUREEXTPROC)load("glFramebufferTextureEXT");
		func->glFramebufferTextureLayerEXT = (PFNGLFRAMEBUFFERTEXTURELAYEREXTPROC)load("glFramebufferTextureLayerEXT");
		func->glFramebufferTextureFaceEXT = (PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC)load("glFramebufferTextureFaceEXT");
	}
	static void load_GL_NV_gpu_multicast(LoadProc load, GLExtFunctions* func)
    {
		func->glRenderGpuMaskNV = (PFNGLRENDERGPUMASKNVPROC)load("glRenderGpuMaskNV");
		func->glMulticastBufferSubDataNV = (PFNGLMULTICASTBUFFERSUBDATANVPROC)load("glMulticastBufferSubDataNV");
		func->glMulticastCopyBufferSubDataNV = (PFNGLMULTICASTCOPYBUFFERSUBDATANVPROC)load("glMulticastCopyBufferSubDataNV");
		func->glMulticastCopyImageSubDataNV = (PFNGLMULTICASTCOPYIMAGESUBDATANVPROC)load("glMulticastCopyImageSubDataNV");
		func->glMulticastBlitFramebufferNV = (PFNGLMULTICASTBLITFRAMEBUFFERNVPROC)load("glMulticastBlitFramebufferNV");
		func->glMulticastFramebufferSampleLocationsfvNV = (PFNGLMULTICASTFRAMEBUFFERSAMPLELOCATIONSFVNVPROC)load("glMulticastFramebufferSampleLocationsfvNV");
		func->glMulticastBarrierNV = (PFNGLMULTICASTBARRIERNVPROC)load("glMulticastBarrierNV");
		func->glMulticastWaitSyncNV = (PFNGLMULTICASTWAITSYNCNVPROC)load("glMulticastWaitSyncNV");
		func->glMulticastGetQueryObjectivNV = (PFNGLMULTICASTGETQUERYOBJECTIVNVPROC)load("glMulticastGetQueryObjectivNV");
		func->glMulticastGetQueryObjectuivNV = (PFNGLMULTICASTGETQUERYOBJECTUIVNVPROC)load("glMulticastGetQueryObjectuivNV");
		func->glMulticastGetQueryObjecti64vNV = (PFNGLMULTICASTGETQUERYOBJECTI64VNVPROC)load("glMulticastGetQueryObjecti64vNV");
		func->glMulticastGetQueryObjectui64vNV = (PFNGLMULTICASTGETQUERYOBJECTUI64VNVPROC)load("glMulticastGetQueryObjectui64vNV");
	}
	static void load_GL_NV_gpu_program4(LoadProc load, GLExtFunctions* func)
    {
		func->glProgramLocalParameterI4iNV = (PFNGLPROGRAMLOCALPARAMETERI4INVPROC)load("glProgramLocalParameterI4iNV");
		func->glProgramLocalParameterI4ivNV = (PFNGLPROGRAMLOCALPARAMETERI4IVNVPROC)load("glProgramLocalParameterI4ivNV");
		func->glProgramLocalParametersI4ivNV = (PFNGLPROGRAMLOCALPARAMETERSI4IVNVPROC)load("glProgramLocalParametersI4ivNV");
		func->glProgramLocalParameterI4uiNV = (PFNGLPROGRAMLOCALPARAMETERI4UINVPROC)load("glProgramLocalParameterI4uiNV");
		func->glProgramLocalParameterI4uivNV = (PFNGLPROGRAMLOCALPARAMETERI4UIVNVPROC)load("glProgramLocalParameterI4uivNV");
		func->glProgramLocalParametersI4uivNV = (PFNGLPROGRAMLOCALPARAMETERSI4UIVNVPROC)load("glProgramLocalParametersI4uivNV");
		func->glProgramEnvParameterI4iNV = (PFNGLPROGRAMENVPARAMETERI4INVPROC)load("glProgramEnvParameterI4iNV");
		func->glProgramEnvParameterI4ivNV = (PFNGLPROGRAMENVPARAMETERI4IVNVPROC)load("glProgramEnvParameterI4ivNV");
		func->glProgramEnvParametersI4ivNV = (PFNGLPROGRAMENVPARAMETERSI4IVNVPROC)load("glProgramEnvParametersI4ivNV");
		func->glProgramEnvParameterI4uiNV = (PFNGLPROGRAMENVPARAMETERI4UINVPROC)load("glProgramEnvParameterI4uiNV");
		func->glProgramEnvParameterI4uivNV = (PFNGLPROGRAMENVPARAMETERI4UIVNVPROC)load("glProgramEnvParameterI4uivNV");
		func->glProgramEnvParametersI4uivNV = (PFNGLPROGRAMENVPARAMETERSI4UIVNVPROC)load("glProgramEnvParametersI4uivNV");
		func->glGetProgramLocalParameterIivNV = (PFNGLGETPROGRAMLOCALPARAMETERIIVNVPROC)load("glGetProgramLocalParameterIivNV");
		func->glGetProgramLocalParameterIuivNV = (PFNGLGETPROGRAMLOCALPARAMETERIUIVNVPROC)load("glGetProgramLocalParameterIuivNV");
		func->glGetProgramEnvParameterIivNV = (PFNGLGETPROGRAMENVPARAMETERIIVNVPROC)load("glGetProgramEnvParameterIivNV");
		func->glGetProgramEnvParameterIuivNV = (PFNGLGETPROGRAMENVPARAMETERIUIVNVPROC)load("glGetProgramEnvParameterIuivNV");
	}
	static void load_GL_NV_gpu_program5(LoadProc load, GLExtFunctions* func)
    {
		func->glProgramSubroutineParametersuivNV = (PFNGLPROGRAMSUBROUTINEPARAMETERSUIVNVPROC)load("glProgramSubroutineParametersuivNV");
		func->glGetProgramSubroutineParameteruivNV = (PFNGLGETPROGRAMSUBROUTINEPARAMETERUIVNVPROC)load("glGetProgramSubroutineParameteruivNV");
	}
	static void load_GL_NV_gpu_shader5(LoadProc load, GLExtFunctions* func)
    {
		func->glUniform1i64NV = (PFNGLUNIFORM1I64NVPROC)load("glUniform1i64NV");
		func->glUniform2i64NV = (PFNGLUNIFORM2I64NVPROC)load("glUniform2i64NV");
		func->glUniform3i64NV = (PFNGLUNIFORM3I64NVPROC)load("glUniform3i64NV");
		func->glUniform4i64NV = (PFNGLUNIFORM4I64NVPROC)load("glUniform4i64NV");
		func->glUniform1i64vNV = (PFNGLUNIFORM1I64VNVPROC)load("glUniform1i64vNV");
		func->glUniform2i64vNV = (PFNGLUNIFORM2I64VNVPROC)load("glUniform2i64vNV");
		func->glUniform3i64vNV = (PFNGLUNIFORM3I64VNVPROC)load("glUniform3i64vNV");
		func->glUniform4i64vNV = (PFNGLUNIFORM4I64VNVPROC)load("glUniform4i64vNV");
		func->glUniform1ui64NV = (PFNGLUNIFORM1UI64NVPROC)load("glUniform1ui64NV");
		func->glUniform2ui64NV = (PFNGLUNIFORM2UI64NVPROC)load("glUniform2ui64NV");
		func->glUniform3ui64NV = (PFNGLUNIFORM3UI64NVPROC)load("glUniform3ui64NV");
		func->glUniform4ui64NV = (PFNGLUNIFORM4UI64NVPROC)load("glUniform4ui64NV");
		func->glUniform1ui64vNV = (PFNGLUNIFORM1UI64VNVPROC)load("glUniform1ui64vNV");
		func->glUniform2ui64vNV = (PFNGLUNIFORM2UI64VNVPROC)load("glUniform2ui64vNV");
		func->glUniform3ui64vNV = (PFNGLUNIFORM3UI64VNVPROC)load("glUniform3ui64vNV");
		func->glUniform4ui64vNV = (PFNGLUNIFORM4UI64VNVPROC)load("glUniform4ui64vNV");
		func->glGetUniformi64vNV = (PFNGLGETUNIFORMI64VNVPROC)load("glGetUniformi64vNV");
		func->glProgramUniform1i64NV = (PFNGLPROGRAMUNIFORM1I64NVPROC)load("glProgramUniform1i64NV");
		func->glProgramUniform2i64NV = (PFNGLPROGRAMUNIFORM2I64NVPROC)load("glProgramUniform2i64NV");
		func->glProgramUniform3i64NV = (PFNGLPROGRAMUNIFORM3I64NVPROC)load("glProgramUniform3i64NV");
		func->glProgramUniform4i64NV = (PFNGLPROGRAMUNIFORM4I64NVPROC)load("glProgramUniform4i64NV");
		func->glProgramUniform1i64vNV = (PFNGLPROGRAMUNIFORM1I64VNVPROC)load("glProgramUniform1i64vNV");
		func->glProgramUniform2i64vNV = (PFNGLPROGRAMUNIFORM2I64VNVPROC)load("glProgramUniform2i64vNV");
		func->glProgramUniform3i64vNV = (PFNGLPROGRAMUNIFORM3I64VNVPROC)load("glProgramUniform3i64vNV");
		func->glProgramUniform4i64vNV = (PFNGLPROGRAMUNIFORM4I64VNVPROC)load("glProgramUniform4i64vNV");
		func->glProgramUniform1ui64NV = (PFNGLPROGRAMUNIFORM1UI64NVPROC)load("glProgramUniform1ui64NV");
		func->glProgramUniform2ui64NV = (PFNGLPROGRAMUNIFORM2UI64NVPROC)load("glProgramUniform2ui64NV");
		func->glProgramUniform3ui64NV = (PFNGLPROGRAMUNIFORM3UI64NVPROC)load("glProgramUniform3ui64NV");
		func->glProgramUniform4ui64NV = (PFNGLPROGRAMUNIFORM4UI64NVPROC)load("glProgramUniform4ui64NV");
		func->glProgramUniform1ui64vNV = (PFNGLPROGRAMUNIFORM1UI64VNVPROC)load("glProgramUniform1ui64vNV");
		func->glProgramUniform2ui64vNV = (PFNGLPROGRAMUNIFORM2UI64VNVPROC)load("glProgramUniform2ui64vNV");
		func->glProgramUniform3ui64vNV = (PFNGLPROGRAMUNIFORM3UI64VNVPROC)load("glProgramUniform3ui64vNV");
		func->glProgramUniform4ui64vNV = (PFNGLPROGRAMUNIFORM4UI64VNVPROC)load("glProgramUniform4ui64vNV");
	}
	static void load_GL_NV_half_float(LoadProc load, GLExtFunctions* func)
    {
		func->glVertex2hNV = (PFNGLVERTEX2HNVPROC)load("glVertex2hNV");
		func->glVertex2hvNV = (PFNGLVERTEX2HVNVPROC)load("glVertex2hvNV");
		func->glVertex3hNV = (PFNGLVERTEX3HNVPROC)load("glVertex3hNV");
		func->glVertex3hvNV = (PFNGLVERTEX3HVNVPROC)load("glVertex3hvNV");
		func->glVertex4hNV = (PFNGLVERTEX4HNVPROC)load("glVertex4hNV");
		func->glVertex4hvNV = (PFNGLVERTEX4HVNVPROC)load("glVertex4hvNV");
		func->glNormal3hNV = (PFNGLNORMAL3HNVPROC)load("glNormal3hNV");
		func->glNormal3hvNV = (PFNGLNORMAL3HVNVPROC)load("glNormal3hvNV");
		func->glColor3hNV = (PFNGLCOLOR3HNVPROC)load("glColor3hNV");
		func->glColor3hvNV = (PFNGLCOLOR3HVNVPROC)load("glColor3hvNV");
		func->glColor4hNV = (PFNGLCOLOR4HNVPROC)load("glColor4hNV");
		func->glColor4hvNV = (PFNGLCOLOR4HVNVPROC)load("glColor4hvNV");
		func->glTexCoord1hNV = (PFNGLTEXCOORD1HNVPROC)load("glTexCoord1hNV");
		func->glTexCoord1hvNV = (PFNGLTEXCOORD1HVNVPROC)load("glTexCoord1hvNV");
		func->glTexCoord2hNV = (PFNGLTEXCOORD2HNVPROC)load("glTexCoord2hNV");
		func->glTexCoord2hvNV = (PFNGLTEXCOORD2HVNVPROC)load("glTexCoord2hvNV");
		func->glTexCoord3hNV = (PFNGLTEXCOORD3HNVPROC)load("glTexCoord3hNV");
		func->glTexCoord3hvNV = (PFNGLTEXCOORD3HVNVPROC)load("glTexCoord3hvNV");
		func->glTexCoord4hNV = (PFNGLTEXCOORD4HNVPROC)load("glTexCoord4hNV");
		func->glTexCoord4hvNV = (PFNGLTEXCOORD4HVNVPROC)load("glTexCoord4hvNV");
		func->glMultiTexCoord1hNV = (PFNGLMULTITEXCOORD1HNVPROC)load("glMultiTexCoord1hNV");
		func->glMultiTexCoord1hvNV = (PFNGLMULTITEXCOORD1HVNVPROC)load("glMultiTexCoord1hvNV");
		func->glMultiTexCoord2hNV = (PFNGLMULTITEXCOORD2HNVPROC)load("glMultiTexCoord2hNV");
		func->glMultiTexCoord2hvNV = (PFNGLMULTITEXCOORD2HVNVPROC)load("glMultiTexCoord2hvNV");
		func->glMultiTexCoord3hNV = (PFNGLMULTITEXCOORD3HNVPROC)load("glMultiTexCoord3hNV");
		func->glMultiTexCoord3hvNV = (PFNGLMULTITEXCOORD3HVNVPROC)load("glMultiTexCoord3hvNV");
		func->glMultiTexCoord4hNV = (PFNGLMULTITEXCOORD4HNVPROC)load("glMultiTexCoord4hNV");
		func->glMultiTexCoord4hvNV = (PFNGLMULTITEXCOORD4HVNVPROC)load("glMultiTexCoord4hvNV");
		func->glVertexAttrib1hNV = (PFNGLVERTEXATTRIB1HNVPROC)load("glVertexAttrib1hNV");
		func->glVertexAttrib1hvNV = (PFNGLVERTEXATTRIB1HVNVPROC)load("glVertexAttrib1hvNV");
		func->glVertexAttrib2hNV = (PFNGLVERTEXATTRIB2HNVPROC)load("glVertexAttrib2hNV");
		func->glVertexAttrib2hvNV = (PFNGLVERTEXATTRIB2HVNVPROC)load("glVertexAttrib2hvNV");
		func->glVertexAttrib3hNV = (PFNGLVERTEXATTRIB3HNVPROC)load("glVertexAttrib3hNV");
		func->glVertexAttrib3hvNV = (PFNGLVERTEXATTRIB3HVNVPROC)load("glVertexAttrib3hvNV");
		func->glVertexAttrib4hNV = (PFNGLVERTEXATTRIB4HNVPROC)load("glVertexAttrib4hNV");
		func->glVertexAttrib4hvNV = (PFNGLVERTEXATTRIB4HVNVPROC)load("glVertexAttrib4hvNV");
		func->glVertexAttribs1hvNV = (PFNGLVERTEXATTRIBS1HVNVPROC)load("glVertexAttribs1hvNV");
		func->glVertexAttribs2hvNV = (PFNGLVERTEXATTRIBS2HVNVPROC)load("glVertexAttribs2hvNV");
		func->glVertexAttribs3hvNV = (PFNGLVERTEXATTRIBS3HVNVPROC)load("glVertexAttribs3hvNV");
		func->glVertexAttribs4hvNV = (PFNGLVERTEXATTRIBS4HVNVPROC)load("glVertexAttribs4hvNV");
		func->glFogCoordhNV = (PFNGLFOGCOORDHNVPROC)load("glFogCoordhNV");
		func->glFogCoordhvNV = (PFNGLFOGCOORDHVNVPROC)load("glFogCoordhvNV");
		func->glSecondaryColor3hNV = (PFNGLSECONDARYCOLOR3HNVPROC)load("glSecondaryColor3hNV");
		func->glSecondaryColor3hvNV = (PFNGLSECONDARYCOLOR3HVNVPROC)load("glSecondaryColor3hvNV");
		func->glVertexWeighthNV = (PFNGLVERTEXWEIGHTHNVPROC)load("glVertexWeighthNV");
		func->glVertexWeighthvNV = (PFNGLVERTEXWEIGHTHVNVPROC)load("glVertexWeighthvNV");
	}
	static void load_GL_NV_internalformat_sample_query(LoadProc load, GLExtFunctions* func)
    {
		func->glGetInternalformatSampleivNV = (PFNGLGETINTERNALFORMATSAMPLEIVNVPROC)load("glGetInternalformatSampleivNV");
	}
	static void load_GL_NV_memory_attachment(LoadProc load, GLExtFunctions* func)
    {
		func->glGetMemoryObjectDetachedResourcesuivNV = (PFNGLGETMEMORYOBJECTDETACHEDRESOURCESUIVNVPROC)load("glGetMemoryObjectDetachedResourcesuivNV");
		func->glResetMemoryObjectParameterNV = (PFNGLRESETMEMORYOBJECTPARAMETERNVPROC)load("glResetMemoryObjectParameterNV");
		func->glTexAttachMemoryNV = (PFNGLTEXATTACHMEMORYNVPROC)load("glTexAttachMemoryNV");
		func->glBufferAttachMemoryNV = (PFNGLBUFFERATTACHMEMORYNVPROC)load("glBufferAttachMemoryNV");
		func->glTextureAttachMemoryNV = (PFNGLTEXTUREATTACHMEMORYNVPROC)load("glTextureAttachMemoryNV");
		func->glNamedBufferAttachMemoryNV = (PFNGLNAMEDBUFFERATTACHMEMORYNVPROC)load("glNamedBufferAttachMemoryNV");
	}
	static void load_GL_NV_memory_object_sparse(LoadProc load, GLExtFunctions* func)
    {
		func->glBufferPageCommitmentMemNV = (PFNGLBUFFERPAGECOMMITMENTMEMNVPROC)load("glBufferPageCommitmentMemNV");
		func->glTexPageCommitmentMemNV = (PFNGLTEXPAGECOMMITMENTMEMNVPROC)load("glTexPageCommitmentMemNV");
		func->glNamedBufferPageCommitmentMemNV = (PFNGLNAMEDBUFFERPAGECOMMITMENTMEMNVPROC)load("glNamedBufferPageCommitmentMemNV");
		func->glTexturePageCommitmentMemNV = (PFNGLTEXTUREPAGECOMMITMENTMEMNVPROC)load("glTexturePageCommitmentMemNV");
	}
	static void load_GL_NV_mesh_shader(LoadProc load, GLExtFunctions* func)
    {
		func->glDrawMeshTasksNV = (PFNGLDRAWMESHTASKSNVPROC)load("glDrawMeshTasksNV");
		func->glDrawMeshTasksIndirectNV = (PFNGLDRAWMESHTASKSINDIRECTNVPROC)load("glDrawMeshTasksIndirectNV");
		func->glMultiDrawMeshTasksIndirectNV = (PFNGLMULTIDRAWMESHTASKSINDIRECTNVPROC)load("glMultiDrawMeshTasksIndirectNV");
		func->glMultiDrawMeshTasksIndirectCountNV = (PFNGLMULTIDRAWMESHTASKSINDIRECTCOUNTNVPROC)load("glMultiDrawMeshTasksIndirectCountNV");
	}
	static void load_GL_NV_occlusion_query(LoadProc load, GLExtFunctions* func)
    {
		func->glGenOcclusionQueriesNV = (PFNGLGENOCCLUSIONQUERIESNVPROC)load("glGenOcclusionQueriesNV");
		func->glDeleteOcclusionQueriesNV = (PFNGLDELETEOCCLUSIONQUERIESNVPROC)load("glDeleteOcclusionQueriesNV");
		func->glIsOcclusionQueryNV = (PFNGLISOCCLUSIONQUERYNVPROC)load("glIsOcclusionQueryNV");
		func->glBeginOcclusionQueryNV = (PFNGLBEGINOCCLUSIONQUERYNVPROC)load("glBeginOcclusionQueryNV");
		func->glEndOcclusionQueryNV = (PFNGLENDOCCLUSIONQUERYNVPROC)load("glEndOcclusionQueryNV");
		func->glGetOcclusionQueryivNV = (PFNGLGETOCCLUSIONQUERYIVNVPROC)load("glGetOcclusionQueryivNV");
		func->glGetOcclusionQueryuivNV = (PFNGLGETOCCLUSIONQUERYUIVNVPROC)load("glGetOcclusionQueryuivNV");
	}
	static void load_GL_NV_parameter_buffer_object(LoadProc load, GLExtFunctions* func)
    {
		func->glProgramBufferParametersfvNV = (PFNGLPROGRAMBUFFERPARAMETERSFVNVPROC)load("glProgramBufferParametersfvNV");
		func->glProgramBufferParametersIivNV = (PFNGLPROGRAMBUFFERPARAMETERSIIVNVPROC)load("glProgramBufferParametersIivNV");
		func->glProgramBufferParametersIuivNV = (PFNGLPROGRAMBUFFERPARAMETERSIUIVNVPROC)load("glProgramBufferParametersIuivNV");
	}
	static void load_GL_NV_path_rendering(LoadProc load, GLExtFunctions* func)
    {
		func->glGenPathsNV = (PFNGLGENPATHSNVPROC)load("glGenPathsNV");
		func->glDeletePathsNV = (PFNGLDELETEPATHSNVPROC)load("glDeletePathsNV");
		func->glIsPathNV = (PFNGLISPATHNVPROC)load("glIsPathNV");
		func->glPathCommandsNV = (PFNGLPATHCOMMANDSNVPROC)load("glPathCommandsNV");
		func->glPathCoordsNV = (PFNGLPATHCOORDSNVPROC)load("glPathCoordsNV");
		func->glPathSubCommandsNV = (PFNGLPATHSUBCOMMANDSNVPROC)load("glPathSubCommandsNV");
		func->glPathSubCoordsNV = (PFNGLPATHSUBCOORDSNVPROC)load("glPathSubCoordsNV");
		func->glPathStringNV = (PFNGLPATHSTRINGNVPROC)load("glPathStringNV");
		func->glPathGlyphsNV = (PFNGLPATHGLYPHSNVPROC)load("glPathGlyphsNV");
		func->glPathGlyphRangeNV = (PFNGLPATHGLYPHRANGENVPROC)load("glPathGlyphRangeNV");
		func->glWeightPathsNV = (PFNGLWEIGHTPATHSNVPROC)load("glWeightPathsNV");
		func->glCopyPathNV = (PFNGLCOPYPATHNVPROC)load("glCopyPathNV");
		func->glInterpolatePathsNV = (PFNGLINTERPOLATEPATHSNVPROC)load("glInterpolatePathsNV");
		func->glTransformPathNV = (PFNGLTRANSFORMPATHNVPROC)load("glTransformPathNV");
		func->glPathParameterivNV = (PFNGLPATHPARAMETERIVNVPROC)load("glPathParameterivNV");
		func->glPathParameteriNV = (PFNGLPATHPARAMETERINVPROC)load("glPathParameteriNV");
		func->glPathParameterfvNV = (PFNGLPATHPARAMETERFVNVPROC)load("glPathParameterfvNV");
		func->glPathParameterfNV = (PFNGLPATHPARAMETERFNVPROC)load("glPathParameterfNV");
		func->glPathDashArrayNV = (PFNGLPATHDASHARRAYNVPROC)load("glPathDashArrayNV");
		func->glPathStencilFuncNV = (PFNGLPATHSTENCILFUNCNVPROC)load("glPathStencilFuncNV");
		func->glPathStencilDepthOffsetNV = (PFNGLPATHSTENCILDEPTHOFFSETNVPROC)load("glPathStencilDepthOffsetNV");
		func->glStencilFillPathNV = (PFNGLSTENCILFILLPATHNVPROC)load("glStencilFillPathNV");
		func->glStencilStrokePathNV = (PFNGLSTENCILSTROKEPATHNVPROC)load("glStencilStrokePathNV");
		func->glStencilFillPathInstancedNV = (PFNGLSTENCILFILLPATHINSTANCEDNVPROC)load("glStencilFillPathInstancedNV");
		func->glStencilStrokePathInstancedNV = (PFNGLSTENCILSTROKEPATHINSTANCEDNVPROC)load("glStencilStrokePathInstancedNV");
		func->glPathCoverDepthFuncNV = (PFNGLPATHCOVERDEPTHFUNCNVPROC)load("glPathCoverDepthFuncNV");
		func->glCoverFillPathNV = (PFNGLCOVERFILLPATHNVPROC)load("glCoverFillPathNV");
		func->glCoverStrokePathNV = (PFNGLCOVERSTROKEPATHNVPROC)load("glCoverStrokePathNV");
		func->glCoverFillPathInstancedNV = (PFNGLCOVERFILLPATHINSTANCEDNVPROC)load("glCoverFillPathInstancedNV");
		func->glCoverStrokePathInstancedNV = (PFNGLCOVERSTROKEPATHINSTANCEDNVPROC)load("glCoverStrokePathInstancedNV");
		func->glGetPathParameterivNV = (PFNGLGETPATHPARAMETERIVNVPROC)load("glGetPathParameterivNV");
		func->glGetPathParameterfvNV = (PFNGLGETPATHPARAMETERFVNVPROC)load("glGetPathParameterfvNV");
		func->glGetPathCommandsNV = (PFNGLGETPATHCOMMANDSNVPROC)load("glGetPathCommandsNV");
		func->glGetPathCoordsNV = (PFNGLGETPATHCOORDSNVPROC)load("glGetPathCoordsNV");
		func->glGetPathDashArrayNV = (PFNGLGETPATHDASHARRAYNVPROC)load("glGetPathDashArrayNV");
		func->glGetPathMetricsNV = (PFNGLGETPATHMETRICSNVPROC)load("glGetPathMetricsNV");
		func->glGetPathMetricRangeNV = (PFNGLGETPATHMETRICRANGENVPROC)load("glGetPathMetricRangeNV");
		func->glGetPathSpacingNV = (PFNGLGETPATHSPACINGNVPROC)load("glGetPathSpacingNV");
		func->glIsPointInFillPathNV = (PFNGLISPOINTINFILLPATHNVPROC)load("glIsPointInFillPathNV");
		func->glIsPointInStrokePathNV = (PFNGLISPOINTINSTROKEPATHNVPROC)load("glIsPointInStrokePathNV");
		func->glGetPathLengthNV = (PFNGLGETPATHLENGTHNVPROC)load("glGetPathLengthNV");
		func->glPointAlongPathNV = (PFNGLPOINTALONGPATHNVPROC)load("glPointAlongPathNV");
		func->glMatrixLoad3x2fNV = (PFNGLMATRIXLOAD3X2FNVPROC)load("glMatrixLoad3x2fNV");
		func->glMatrixLoad3x3fNV = (PFNGLMATRIXLOAD3X3FNVPROC)load("glMatrixLoad3x3fNV");
		func->glMatrixLoadTranspose3x3fNV = (PFNGLMATRIXLOADTRANSPOSE3X3FNVPROC)load("glMatrixLoadTranspose3x3fNV");
		func->glMatrixMult3x2fNV = (PFNGLMATRIXMULT3X2FNVPROC)load("glMatrixMult3x2fNV");
		func->glMatrixMult3x3fNV = (PFNGLMATRIXMULT3X3FNVPROC)load("glMatrixMult3x3fNV");
		func->glMatrixMultTranspose3x3fNV = (PFNGLMATRIXMULTTRANSPOSE3X3FNVPROC)load("glMatrixMultTranspose3x3fNV");
		func->glStencilThenCoverFillPathNV = (PFNGLSTENCILTHENCOVERFILLPATHNVPROC)load("glStencilThenCoverFillPathNV");
		func->glStencilThenCoverStrokePathNV = (PFNGLSTENCILTHENCOVERSTROKEPATHNVPROC)load("glStencilThenCoverStrokePathNV");
		func->glStencilThenCoverFillPathInstancedNV = (PFNGLSTENCILTHENCOVERFILLPATHINSTANCEDNVPROC)load("glStencilThenCoverFillPathInstancedNV");
		func->glStencilThenCoverStrokePathInstancedNV = (PFNGLSTENCILTHENCOVERSTROKEPATHINSTANCEDNVPROC)load("glStencilThenCoverStrokePathInstancedNV");
		func->glPathGlyphIndexRangeNV = (PFNGLPATHGLYPHINDEXRANGENVPROC)load("glPathGlyphIndexRangeNV");
		func->glPathGlyphIndexArrayNV = (PFNGLPATHGLYPHINDEXARRAYNVPROC)load("glPathGlyphIndexArrayNV");
		func->glPathMemoryGlyphIndexArrayNV = (PFNGLPATHMEMORYGLYPHINDEXARRAYNVPROC)load("glPathMemoryGlyphIndexArrayNV");
		func->glProgramPathFragmentInputGenNV = (PFNGLPROGRAMPATHFRAGMENTINPUTGENNVPROC)load("glProgramPathFragmentInputGenNV");
		func->glGetProgramResourcefvNV = (PFNGLGETPROGRAMRESOURCEFVNVPROC)load("glGetProgramResourcefvNV");
		func->glMatrixFrustumEXT = (PFNGLMATRIXFRUSTUMEXTPROC)load("glMatrixFrustumEXT");
		func->glMatrixLoadIdentityEXT = (PFNGLMATRIXLOADIDENTITYEXTPROC)load("glMatrixLoadIdentityEXT");
		func->glMatrixLoadTransposefEXT = (PFNGLMATRIXLOADTRANSPOSEFEXTPROC)load("glMatrixLoadTransposefEXT");
		func->glMatrixLoadTransposedEXT = (PFNGLMATRIXLOADTRANSPOSEDEXTPROC)load("glMatrixLoadTransposedEXT");
		func->glMatrixLoadfEXT = (PFNGLMATRIXLOADFEXTPROC)load("glMatrixLoadfEXT");
		func->glMatrixLoaddEXT = (PFNGLMATRIXLOADDEXTPROC)load("glMatrixLoaddEXT");
		func->glMatrixMultTransposefEXT = (PFNGLMATRIXMULTTRANSPOSEFEXTPROC)load("glMatrixMultTransposefEXT");
		func->glMatrixMultTransposedEXT = (PFNGLMATRIXMULTTRANSPOSEDEXTPROC)load("glMatrixMultTransposedEXT");
		func->glMatrixMultfEXT = (PFNGLMATRIXMULTFEXTPROC)load("glMatrixMultfEXT");
		func->glMatrixMultdEXT = (PFNGLMATRIXMULTDEXTPROC)load("glMatrixMultdEXT");
		func->glMatrixOrthoEXT = (PFNGLMATRIXORTHOEXTPROC)load("glMatrixOrthoEXT");
		func->glMatrixPopEXT = (PFNGLMATRIXPOPEXTPROC)load("glMatrixPopEXT");
		func->glMatrixPushEXT = (PFNGLMATRIXPUSHEXTPROC)load("glMatrixPushEXT");
		func->glMatrixRotatefEXT = (PFNGLMATRIXROTATEFEXTPROC)load("glMatrixRotatefEXT");
		func->glMatrixRotatedEXT = (PFNGLMATRIXROTATEDEXTPROC)load("glMatrixRotatedEXT");
		func->glMatrixScalefEXT = (PFNGLMATRIXSCALEFEXTPROC)load("glMatrixScalefEXT");
		func->glMatrixScaledEXT = (PFNGLMATRIXSCALEDEXTPROC)load("glMatrixScaledEXT");
		func->glMatrixTranslatefEXT = (PFNGLMATRIXTRANSLATEFEXTPROC)load("glMatrixTranslatefEXT");
		func->glMatrixTranslatedEXT = (PFNGLMATRIXTRANSLATEDEXTPROC)load("glMatrixTranslatedEXT");
	}
	static void load_GL_NV_pixel_data_range(LoadProc load, GLExtFunctions* func)
    {
		func->glPixelDataRangeNV = (PFNGLPIXELDATARANGENVPROC)load("glPixelDataRangeNV");
		func->glFlushPixelDataRangeNV = (PFNGLFLUSHPIXELDATARANGENVPROC)load("glFlushPixelDataRangeNV");
	}
	static void load_GL_NV_point_sprite(LoadProc load, GLExtFunctions* func)
    {
		func->glPointParameteriNV = (PFNGLPOINTPARAMETERINVPROC)load("glPointParameteriNV");
		func->glPointParameterivNV = (PFNGLPOINTPARAMETERIVNVPROC)load("glPointParameterivNV");
	}
	static void load_GL_NV_present_video(LoadProc load, GLExtFunctions* func)
    {
		func->glPresentFrameKeyedNV = (PFNGLPRESENTFRAMEKEYEDNVPROC)load("glPresentFrameKeyedNV");
		func->glPresentFrameDualFillNV = (PFNGLPRESENTFRAMEDUALFILLNVPROC)load("glPresentFrameDualFillNV");
		func->glGetVideoivNV = (PFNGLGETVIDEOIVNVPROC)load("glGetVideoivNV");
		func->glGetVideouivNV = (PFNGLGETVIDEOUIVNVPROC)load("glGetVideouivNV");
		func->glGetVideoi64vNV = (PFNGLGETVIDEOI64VNVPROC)load("glGetVideoi64vNV");
		func->glGetVideoui64vNV = (PFNGLGETVIDEOUI64VNVPROC)load("glGetVideoui64vNV");
	}
	static void load_GL_NV_primitive_restart(LoadProc load, GLExtFunctions* func)
    {
		func->glPrimitiveRestartNV = (PFNGLPRIMITIVERESTARTNVPROC)load("glPrimitiveRestartNV");
		func->glPrimitiveRestartIndexNV = (PFNGLPRIMITIVERESTARTINDEXNVPROC)load("glPrimitiveRestartIndexNV");
	}
	static void load_GL_NV_query_resource(LoadProc load, GLExtFunctions* func)
    {
		func->glQueryResourceNV = (PFNGLQUERYRESOURCENVPROC)load("glQueryResourceNV");
	}
	static void load_GL_NV_query_resource_tag(LoadProc load, GLExtFunctions* func)
    {
		func->glGenQueryResourceTagNV = (PFNGLGENQUERYRESOURCETAGNVPROC)load("glGenQueryResourceTagNV");
		func->glDeleteQueryResourceTagNV = (PFNGLDELETEQUERYRESOURCETAGNVPROC)load("glDeleteQueryResourceTagNV");
		func->glQueryResourceTagNV = (PFNGLQUERYRESOURCETAGNVPROC)load("glQueryResourceTagNV");
	}
	static void load_GL_NV_register_combiners(LoadProc load, GLExtFunctions* func)
    {
		func->glCombinerParameterfvNV = (PFNGLCOMBINERPARAMETERFVNVPROC)load("glCombinerParameterfvNV");
		func->glCombinerParameterfNV = (PFNGLCOMBINERPARAMETERFNVPROC)load("glCombinerParameterfNV");
		func->glCombinerParameterivNV = (PFNGLCOMBINERPARAMETERIVNVPROC)load("glCombinerParameterivNV");
		func->glCombinerParameteriNV = (PFNGLCOMBINERPARAMETERINVPROC)load("glCombinerParameteriNV");
		func->glCombinerInputNV = (PFNGLCOMBINERINPUTNVPROC)load("glCombinerInputNV");
		func->glCombinerOutputNV = (PFNGLCOMBINEROUTPUTNVPROC)load("glCombinerOutputNV");
		func->glFinalCombinerInputNV = (PFNGLFINALCOMBINERINPUTNVPROC)load("glFinalCombinerInputNV");
		func->glGetCombinerInputParameterfvNV = (PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC)load("glGetCombinerInputParameterfvNV");
		func->glGetCombinerInputParameterivNV = (PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC)load("glGetCombinerInputParameterivNV");
		func->glGetCombinerOutputParameterfvNV = (PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC)load("glGetCombinerOutputParameterfvNV");
		func->glGetCombinerOutputParameterivNV = (PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC)load("glGetCombinerOutputParameterivNV");
		func->glGetFinalCombinerInputParameterfvNV = (PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC)load("glGetFinalCombinerInputParameterfvNV");
		func->glGetFinalCombinerInputParameterivNV = (PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC)load("glGetFinalCombinerInputParameterivNV");
	}
	static void load_GL_NV_register_combiners2(LoadProc load, GLExtFunctions* func)
    {
		func->glCombinerStageParameterfvNV = (PFNGLCOMBINERSTAGEPARAMETERFVNVPROC)load("glCombinerStageParameterfvNV");
		func->glGetCombinerStageParameterfvNV = (PFNGLGETCOMBINERSTAGEPARAMETERFVNVPROC)load("glGetCombinerStageParameterfvNV");
	}
	static void load_GL_NV_sample_locations(LoadProc load, GLExtFunctions* func)
    {
		func->glFramebufferSampleLocationsfvNV = (PFNGLFRAMEBUFFERSAMPLELOCATIONSFVNVPROC)load("glFramebufferSampleLocationsfvNV");
		func->glNamedFramebufferSampleLocationsfvNV = (PFNGLNAMEDFRAMEBUFFERSAMPLELOCATIONSFVNVPROC)load("glNamedFramebufferSampleLocationsfvNV");
		func->glResolveDepthValuesNV = (PFNGLRESOLVEDEPTHVALUESNVPROC)load("glResolveDepthValuesNV");
	}
	static void load_GL_NV_scissor_exclusive(LoadProc load, GLExtFunctions* func)
    {
		func->glScissorExclusiveNV = (PFNGLSCISSOREXCLUSIVENVPROC)load("glScissorExclusiveNV");
		func->glScissorExclusiveArrayvNV = (PFNGLSCISSOREXCLUSIVEARRAYVNVPROC)load("glScissorExclusiveArrayvNV");
	}
	static void load_GL_NV_shader_buffer_load(LoadProc load, GLExtFunctions* func)
    {
		func->glMakeBufferResidentNV = (PFNGLMAKEBUFFERRESIDENTNVPROC)load("glMakeBufferResidentNV");
		func->glMakeBufferNonResidentNV = (PFNGLMAKEBUFFERNONRESIDENTNVPROC)load("glMakeBufferNonResidentNV");
		func->glIsBufferResidentNV = (PFNGLISBUFFERRESIDENTNVPROC)load("glIsBufferResidentNV");
		func->glMakeNamedBufferResidentNV = (PFNGLMAKENAMEDBUFFERRESIDENTNVPROC)load("glMakeNamedBufferResidentNV");
		func->glMakeNamedBufferNonResidentNV = (PFNGLMAKENAMEDBUFFERNONRESIDENTNVPROC)load("glMakeNamedBufferNonResidentNV");
		func->glIsNamedBufferResidentNV = (PFNGLISNAMEDBUFFERRESIDENTNVPROC)load("glIsNamedBufferResidentNV");
		func->glGetBufferParameterui64vNV = (PFNGLGETBUFFERPARAMETERUI64VNVPROC)load("glGetBufferParameterui64vNV");
		func->glGetNamedBufferParameterui64vNV = (PFNGLGETNAMEDBUFFERPARAMETERUI64VNVPROC)load("glGetNamedBufferParameterui64vNV");
		func->glGetIntegerui64vNV = (PFNGLGETINTEGERUI64VNVPROC)load("glGetIntegerui64vNV");
		func->glUniformui64NV = (PFNGLUNIFORMUI64NVPROC)load("glUniformui64NV");
		func->glUniformui64vNV = (PFNGLUNIFORMUI64VNVPROC)load("glUniformui64vNV");
		func->glGetUniformui64vNV = (PFNGLGETUNIFORMUI64VNVPROC)load("glGetUniformui64vNV");
		func->glProgramUniformui64NV = (PFNGLPROGRAMUNIFORMUI64NVPROC)load("glProgramUniformui64NV");
		func->glProgramUniformui64vNV = (PFNGLPROGRAMUNIFORMUI64VNVPROC)load("glProgramUniformui64vNV");
	}
	static void load_GL_NV_shading_rate_image(LoadProc load, GLExtFunctions* func)
    {
		func->glBindShadingRateImageNV = (PFNGLBINDSHADINGRATEIMAGENVPROC)load("glBindShadingRateImageNV");
		func->glGetShadingRateImagePaletteNV = (PFNGLGETSHADINGRATEIMAGEPALETTENVPROC)load("glGetShadingRateImagePaletteNV");
		func->glGetShadingRateSampleLocationivNV = (PFNGLGETSHADINGRATESAMPLELOCATIONIVNVPROC)load("glGetShadingRateSampleLocationivNV");
		func->glShadingRateImageBarrierNV = (PFNGLSHADINGRATEIMAGEBARRIERNVPROC)load("glShadingRateImageBarrierNV");
		func->glShadingRateImagePaletteNV = (PFNGLSHADINGRATEIMAGEPALETTENVPROC)load("glShadingRateImagePaletteNV");
		func->glShadingRateSampleOrderNV = (PFNGLSHADINGRATESAMPLEORDERNVPROC)load("glShadingRateSampleOrderNV");
		func->glShadingRateSampleOrderCustomNV = (PFNGLSHADINGRATESAMPLEORDERCUSTOMNVPROC)load("glShadingRateSampleOrderCustomNV");
	}
	static void load_GL_NV_texture_barrier(LoadProc load, GLExtFunctions* func)
    {
		func->glTextureBarrierNV = (PFNGLTEXTUREBARRIERNVPROC)load("glTextureBarrierNV");
	}
	static void load_GL_NV_texture_multisample(LoadProc load, GLExtFunctions* func)
    {
		func->glTexImage2DMultisampleCoverageNV = (PFNGLTEXIMAGE2DMULTISAMPLECOVERAGENVPROC)load("glTexImage2DMultisampleCoverageNV");
		func->glTexImage3DMultisampleCoverageNV = (PFNGLTEXIMAGE3DMULTISAMPLECOVERAGENVPROC)load("glTexImage3DMultisampleCoverageNV");
		func->glTextureImage2DMultisampleNV = (PFNGLTEXTUREIMAGE2DMULTISAMPLENVPROC)load("glTextureImage2DMultisampleNV");
		func->glTextureImage3DMultisampleNV = (PFNGLTEXTUREIMAGE3DMULTISAMPLENVPROC)load("glTextureImage3DMultisampleNV");
		func->glTextureImage2DMultisampleCoverageNV = (PFNGLTEXTUREIMAGE2DMULTISAMPLECOVERAGENVPROC)load("glTextureImage2DMultisampleCoverageNV");
		func->glTextureImage3DMultisampleCoverageNV = (PFNGLTEXTUREIMAGE3DMULTISAMPLECOVERAGENVPROC)load("glTextureImage3DMultisampleCoverageNV");
	}
	static void load_GL_NV_timeline_semaphore(LoadProc load, GLExtFunctions* func)
    {
		func->glCreateSemaphoresNV = (PFNGLCREATESEMAPHORESNVPROC)load("glCreateSemaphoresNV");
		func->glSemaphoreParameterivNV = (PFNGLSEMAPHOREPARAMETERIVNVPROC)load("glSemaphoreParameterivNV");
		func->glGetSemaphoreParameterivNV = (PFNGLGETSEMAPHOREPARAMETERIVNVPROC)load("glGetSemaphoreParameterivNV");
	}
	static void load_GL_NV_transform_feedback(LoadProc load, GLExtFunctions* func)
    {
		func->glBeginTransformFeedbackNV = (PFNGLBEGINTRANSFORMFEEDBACKNVPROC)load("glBeginTransformFeedbackNV");
		func->glEndTransformFeedbackNV = (PFNGLENDTRANSFORMFEEDBACKNVPROC)load("glEndTransformFeedbackNV");
		func->glTransformFeedbackAttribsNV = (PFNGLTRANSFORMFEEDBACKATTRIBSNVPROC)load("glTransformFeedbackAttribsNV");
		func->glBindBufferRangeNV = (PFNGLBINDBUFFERRANGENVPROC)load("glBindBufferRangeNV");
		func->glBindBufferOffsetNV = (PFNGLBINDBUFFEROFFSETNVPROC)load("glBindBufferOffsetNV");
		func->glBindBufferBaseNV = (PFNGLBINDBUFFERBASENVPROC)load("glBindBufferBaseNV");
		func->glTransformFeedbackVaryingsNV = (PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC)load("glTransformFeedbackVaryingsNV");
		func->glActiveVaryingNV = (PFNGLACTIVEVARYINGNVPROC)load("glActiveVaryingNV");
		func->glGetVaryingLocationNV = (PFNGLGETVARYINGLOCATIONNVPROC)load("glGetVaryingLocationNV");
		func->glGetActiveVaryingNV = (PFNGLGETACTIVEVARYINGNVPROC)load("glGetActiveVaryingNV");
		func->glGetTransformFeedbackVaryingNV = (PFNGLGETTRANSFORMFEEDBACKVARYINGNVPROC)load("glGetTransformFeedbackVaryingNV");
		func->glTransformFeedbackStreamAttribsNV = (PFNGLTRANSFORMFEEDBACKSTREAMATTRIBSNVPROC)load("glTransformFeedbackStreamAttribsNV");
	}
	static void load_GL_NV_transform_feedback2(LoadProc load, GLExtFunctions* func)
    {
		func->glBindTransformFeedbackNV = (PFNGLBINDTRANSFORMFEEDBACKNVPROC)load("glBindTransformFeedbackNV");
		func->glDeleteTransformFeedbacksNV = (PFNGLDELETETRANSFORMFEEDBACKSNVPROC)load("glDeleteTransformFeedbacksNV");
		func->glGenTransformFeedbacksNV = (PFNGLGENTRANSFORMFEEDBACKSNVPROC)load("glGenTransformFeedbacksNV");
		func->glIsTransformFeedbackNV = (PFNGLISTRANSFORMFEEDBACKNVPROC)load("glIsTransformFeedbackNV");
		func->glPauseTransformFeedbackNV = (PFNGLPAUSETRANSFORMFEEDBACKNVPROC)load("glPauseTransformFeedbackNV");
		func->glResumeTransformFeedbackNV = (PFNGLRESUMETRANSFORMFEEDBACKNVPROC)load("glResumeTransformFeedbackNV");
		func->glDrawTransformFeedbackNV = (PFNGLDRAWTRANSFORMFEEDBACKNVPROC)load("glDrawTransformFeedbackNV");
	}
	static void load_GL_NV_vdpau_interop(LoadProc load, GLExtFunctions* func)
    {
		func->glVDPAUInitNV = (PFNGLVDPAUINITNVPROC)load("glVDPAUInitNV");
		func->glVDPAUFiniNV = (PFNGLVDPAUFININVPROC)load("glVDPAUFiniNV");
		func->glVDPAURegisterVideoSurfaceNV = (PFNGLVDPAUREGISTERVIDEOSURFACENVPROC)load("glVDPAURegisterVideoSurfaceNV");
		func->glVDPAURegisterOutputSurfaceNV = (PFNGLVDPAUREGISTEROUTPUTSURFACENVPROC)load("glVDPAURegisterOutputSurfaceNV");
		func->glVDPAUIsSurfaceNV = (PFNGLVDPAUISSURFACENVPROC)load("glVDPAUIsSurfaceNV");
		func->glVDPAUUnregisterSurfaceNV = (PFNGLVDPAUUNREGISTERSURFACENVPROC)load("glVDPAUUnregisterSurfaceNV");
		func->glVDPAUGetSurfaceivNV = (PFNGLVDPAUGETSURFACEIVNVPROC)load("glVDPAUGetSurfaceivNV");
		func->glVDPAUSurfaceAccessNV = (PFNGLVDPAUSURFACEACCESSNVPROC)load("glVDPAUSurfaceAccessNV");
		func->glVDPAUMapSurfacesNV = (PFNGLVDPAUMAPSURFACESNVPROC)load("glVDPAUMapSurfacesNV");
		func->glVDPAUUnmapSurfacesNV = (PFNGLVDPAUUNMAPSURFACESNVPROC)load("glVDPAUUnmapSurfacesNV");
	}
	static void load_GL_NV_vdpau_interop2(LoadProc load, GLExtFunctions* func)
    {
		func->glVDPAURegisterVideoSurfaceWithPictureStructureNV = (PFNGLVDPAUREGISTERVIDEOSURFACEWITHPICTURESTRUCTURENVPROC)load("glVDPAURegisterVideoSurfaceWithPictureStructureNV");
	}
	static void load_GL_NV_vertex_array_range(LoadProc load, GLExtFunctions* func)
    {
		func->glFlushVertexArrayRangeNV = (PFNGLFLUSHVERTEXARRAYRANGENVPROC)load("glFlushVertexArrayRangeNV");
		func->glVertexArrayRangeNV = (PFNGLVERTEXARRAYRANGENVPROC)load("glVertexArrayRangeNV");
	}
	static void load_GL_NV_vertex_attrib_integer_64bit(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexAttribL1i64NV = (PFNGLVERTEXATTRIBL1I64NVPROC)load("glVertexAttribL1i64NV");
		func->glVertexAttribL2i64NV = (PFNGLVERTEXATTRIBL2I64NVPROC)load("glVertexAttribL2i64NV");
		func->glVertexAttribL3i64NV = (PFNGLVERTEXATTRIBL3I64NVPROC)load("glVertexAttribL3i64NV");
		func->glVertexAttribL4i64NV = (PFNGLVERTEXATTRIBL4I64NVPROC)load("glVertexAttribL4i64NV");
		func->glVertexAttribL1i64vNV = (PFNGLVERTEXATTRIBL1I64VNVPROC)load("glVertexAttribL1i64vNV");
		func->glVertexAttribL2i64vNV = (PFNGLVERTEXATTRIBL2I64VNVPROC)load("glVertexAttribL2i64vNV");
		func->glVertexAttribL3i64vNV = (PFNGLVERTEXATTRIBL3I64VNVPROC)load("glVertexAttribL3i64vNV");
		func->glVertexAttribL4i64vNV = (PFNGLVERTEXATTRIBL4I64VNVPROC)load("glVertexAttribL4i64vNV");
		func->glVertexAttribL1ui64NV = (PFNGLVERTEXATTRIBL1UI64NVPROC)load("glVertexAttribL1ui64NV");
		func->glVertexAttribL2ui64NV = (PFNGLVERTEXATTRIBL2UI64NVPROC)load("glVertexAttribL2ui64NV");
		func->glVertexAttribL3ui64NV = (PFNGLVERTEXATTRIBL3UI64NVPROC)load("glVertexAttribL3ui64NV");
		func->glVertexAttribL4ui64NV = (PFNGLVERTEXATTRIBL4UI64NVPROC)load("glVertexAttribL4ui64NV");
		func->glVertexAttribL1ui64vNV = (PFNGLVERTEXATTRIBL1UI64VNVPROC)load("glVertexAttribL1ui64vNV");
		func->glVertexAttribL2ui64vNV = (PFNGLVERTEXATTRIBL2UI64VNVPROC)load("glVertexAttribL2ui64vNV");
		func->glVertexAttribL3ui64vNV = (PFNGLVERTEXATTRIBL3UI64VNVPROC)load("glVertexAttribL3ui64vNV");
		func->glVertexAttribL4ui64vNV = (PFNGLVERTEXATTRIBL4UI64VNVPROC)load("glVertexAttribL4ui64vNV");
		func->glGetVertexAttribLi64vNV = (PFNGLGETVERTEXATTRIBLI64VNVPROC)load("glGetVertexAttribLi64vNV");
		func->glGetVertexAttribLui64vNV = (PFNGLGETVERTEXATTRIBLUI64VNVPROC)load("glGetVertexAttribLui64vNV");
		func->glVertexAttribLFormatNV = (PFNGLVERTEXATTRIBLFORMATNVPROC)load("glVertexAttribLFormatNV");
	}
	static void load_GL_NV_vertex_buffer_unified_memory(LoadProc load, GLExtFunctions* func)
    {
		func->glBufferAddressRangeNV = (PFNGLBUFFERADDRESSRANGENVPROC)load("glBufferAddressRangeNV");
		func->glVertexFormatNV = (PFNGLVERTEXFORMATNVPROC)load("glVertexFormatNV");
		func->glNormalFormatNV = (PFNGLNORMALFORMATNVPROC)load("glNormalFormatNV");
		func->glColorFormatNV = (PFNGLCOLORFORMATNVPROC)load("glColorFormatNV");
		func->glIndexFormatNV = (PFNGLINDEXFORMATNVPROC)load("glIndexFormatNV");
		func->glTexCoordFormatNV = (PFNGLTEXCOORDFORMATNVPROC)load("glTexCoordFormatNV");
		func->glEdgeFlagFormatNV = (PFNGLEDGEFLAGFORMATNVPROC)load("glEdgeFlagFormatNV");
		func->glSecondaryColorFormatNV = (PFNGLSECONDARYCOLORFORMATNVPROC)load("glSecondaryColorFormatNV");
		func->glFogCoordFormatNV = (PFNGLFOGCOORDFORMATNVPROC)load("glFogCoordFormatNV");
		func->glVertexAttribFormatNV = (PFNGLVERTEXATTRIBFORMATNVPROC)load("glVertexAttribFormatNV");
		func->glVertexAttribIFormatNV = (PFNGLVERTEXATTRIBIFORMATNVPROC)load("glVertexAttribIFormatNV");
		func->glGetIntegerui64i_vNV = (PFNGLGETINTEGERUI64I_VNVPROC)load("glGetIntegerui64i_vNV");
	}
	static void load_GL_NV_vertex_program(LoadProc load, GLExtFunctions* func)
    {
		func->glAreProgramsResidentNV = (PFNGLAREPROGRAMSRESIDENTNVPROC)load("glAreProgramsResidentNV");
		func->glBindProgramNV = (PFNGLBINDPROGRAMNVPROC)load("glBindProgramNV");
		func->glDeleteProgramsNV = (PFNGLDELETEPROGRAMSNVPROC)load("glDeleteProgramsNV");
		func->glExecuteProgramNV = (PFNGLEXECUTEPROGRAMNVPROC)load("glExecuteProgramNV");
		func->glGenProgramsNV = (PFNGLGENPROGRAMSNVPROC)load("glGenProgramsNV");
		func->glGetProgramParameterdvNV = (PFNGLGETPROGRAMPARAMETERDVNVPROC)load("glGetProgramParameterdvNV");
		func->glGetProgramParameterfvNV = (PFNGLGETPROGRAMPARAMETERFVNVPROC)load("glGetProgramParameterfvNV");
		func->glGetProgramivNV = (PFNGLGETPROGRAMIVNVPROC)load("glGetProgramivNV");
		func->glGetProgramStringNV = (PFNGLGETPROGRAMSTRINGNVPROC)load("glGetProgramStringNV");
		func->glGetTrackMatrixivNV = (PFNGLGETTRACKMATRIXIVNVPROC)load("glGetTrackMatrixivNV");
		func->glGetVertexAttribdvNV = (PFNGLGETVERTEXATTRIBDVNVPROC)load("glGetVertexAttribdvNV");
		func->glGetVertexAttribfvNV = (PFNGLGETVERTEXATTRIBFVNVPROC)load("glGetVertexAttribfvNV");
		func->glGetVertexAttribivNV = (PFNGLGETVERTEXATTRIBIVNVPROC)load("glGetVertexAttribivNV");
		func->glGetVertexAttribPointervNV = (PFNGLGETVERTEXATTRIBPOINTERVNVPROC)load("glGetVertexAttribPointervNV");
		func->glIsProgramNV = (PFNGLISPROGRAMNVPROC)load("glIsProgramNV");
		func->glLoadProgramNV = (PFNGLLOADPROGRAMNVPROC)load("glLoadProgramNV");
		func->glProgramParameter4dNV = (PFNGLPROGRAMPARAMETER4DNVPROC)load("glProgramParameter4dNV");
		func->glProgramParameter4dvNV = (PFNGLPROGRAMPARAMETER4DVNVPROC)load("glProgramParameter4dvNV");
		func->glProgramParameter4fNV = (PFNGLPROGRAMPARAMETER4FNVPROC)load("glProgramParameter4fNV");
		func->glProgramParameter4fvNV = (PFNGLPROGRAMPARAMETER4FVNVPROC)load("glProgramParameter4fvNV");
		func->glProgramParameters4dvNV = (PFNGLPROGRAMPARAMETERS4DVNVPROC)load("glProgramParameters4dvNV");
		func->glProgramParameters4fvNV = (PFNGLPROGRAMPARAMETERS4FVNVPROC)load("glProgramParameters4fvNV");
		func->glRequestResidentProgramsNV = (PFNGLREQUESTRESIDENTPROGRAMSNVPROC)load("glRequestResidentProgramsNV");
		func->glTrackMatrixNV = (PFNGLTRACKMATRIXNVPROC)load("glTrackMatrixNV");
		func->glVertexAttribPointerNV = (PFNGLVERTEXATTRIBPOINTERNVPROC)load("glVertexAttribPointerNV");
		func->glVertexAttrib1dNV = (PFNGLVERTEXATTRIB1DNVPROC)load("glVertexAttrib1dNV");
		func->glVertexAttrib1dvNV = (PFNGLVERTEXATTRIB1DVNVPROC)load("glVertexAttrib1dvNV");
		func->glVertexAttrib1fNV = (PFNGLVERTEXATTRIB1FNVPROC)load("glVertexAttrib1fNV");
		func->glVertexAttrib1fvNV = (PFNGLVERTEXATTRIB1FVNVPROC)load("glVertexAttrib1fvNV");
		func->glVertexAttrib1sNV = (PFNGLVERTEXATTRIB1SNVPROC)load("glVertexAttrib1sNV");
		func->glVertexAttrib1svNV = (PFNGLVERTEXATTRIB1SVNVPROC)load("glVertexAttrib1svNV");
		func->glVertexAttrib2dNV = (PFNGLVERTEXATTRIB2DNVPROC)load("glVertexAttrib2dNV");
		func->glVertexAttrib2dvNV = (PFNGLVERTEXATTRIB2DVNVPROC)load("glVertexAttrib2dvNV");
		func->glVertexAttrib2fNV = (PFNGLVERTEXATTRIB2FNVPROC)load("glVertexAttrib2fNV");
		func->glVertexAttrib2fvNV = (PFNGLVERTEXATTRIB2FVNVPROC)load("glVertexAttrib2fvNV");
		func->glVertexAttrib2sNV = (PFNGLVERTEXATTRIB2SNVPROC)load("glVertexAttrib2sNV");
		func->glVertexAttrib2svNV = (PFNGLVERTEXATTRIB2SVNVPROC)load("glVertexAttrib2svNV");
		func->glVertexAttrib3dNV = (PFNGLVERTEXATTRIB3DNVPROC)load("glVertexAttrib3dNV");
		func->glVertexAttrib3dvNV = (PFNGLVERTEXATTRIB3DVNVPROC)load("glVertexAttrib3dvNV");
		func->glVertexAttrib3fNV = (PFNGLVERTEXATTRIB3FNVPROC)load("glVertexAttrib3fNV");
		func->glVertexAttrib3fvNV = (PFNGLVERTEXATTRIB3FVNVPROC)load("glVertexAttrib3fvNV");
		func->glVertexAttrib3sNV = (PFNGLVERTEXATTRIB3SNVPROC)load("glVertexAttrib3sNV");
		func->glVertexAttrib3svNV = (PFNGLVERTEXATTRIB3SVNVPROC)load("glVertexAttrib3svNV");
		func->glVertexAttrib4dNV = (PFNGLVERTEXATTRIB4DNVPROC)load("glVertexAttrib4dNV");
		func->glVertexAttrib4dvNV = (PFNGLVERTEXATTRIB4DVNVPROC)load("glVertexAttrib4dvNV");
		func->glVertexAttrib4fNV = (PFNGLVERTEXATTRIB4FNVPROC)load("glVertexAttrib4fNV");
		func->glVertexAttrib4fvNV = (PFNGLVERTEXATTRIB4FVNVPROC)load("glVertexAttrib4fvNV");
		func->glVertexAttrib4sNV = (PFNGLVERTEXATTRIB4SNVPROC)load("glVertexAttrib4sNV");
		func->glVertexAttrib4svNV = (PFNGLVERTEXATTRIB4SVNVPROC)load("glVertexAttrib4svNV");
		func->glVertexAttrib4ubNV = (PFNGLVERTEXATTRIB4UBNVPROC)load("glVertexAttrib4ubNV");
		func->glVertexAttrib4ubvNV = (PFNGLVERTEXATTRIB4UBVNVPROC)load("glVertexAttrib4ubvNV");
		func->glVertexAttribs1dvNV = (PFNGLVERTEXATTRIBS1DVNVPROC)load("glVertexAttribs1dvNV");
		func->glVertexAttribs1fvNV = (PFNGLVERTEXATTRIBS1FVNVPROC)load("glVertexAttribs1fvNV");
		func->glVertexAttribs1svNV = (PFNGLVERTEXATTRIBS1SVNVPROC)load("glVertexAttribs1svNV");
		func->glVertexAttribs2dvNV = (PFNGLVERTEXATTRIBS2DVNVPROC)load("glVertexAttribs2dvNV");
		func->glVertexAttribs2fvNV = (PFNGLVERTEXATTRIBS2FVNVPROC)load("glVertexAttribs2fvNV");
		func->glVertexAttribs2svNV = (PFNGLVERTEXATTRIBS2SVNVPROC)load("glVertexAttribs2svNV");
		func->glVertexAttribs3dvNV = (PFNGLVERTEXATTRIBS3DVNVPROC)load("glVertexAttribs3dvNV");
		func->glVertexAttribs3fvNV = (PFNGLVERTEXATTRIBS3FVNVPROC)load("glVertexAttribs3fvNV");
		func->glVertexAttribs3svNV = (PFNGLVERTEXATTRIBS3SVNVPROC)load("glVertexAttribs3svNV");
		func->glVertexAttribs4dvNV = (PFNGLVERTEXATTRIBS4DVNVPROC)load("glVertexAttribs4dvNV");
		func->glVertexAttribs4fvNV = (PFNGLVERTEXATTRIBS4FVNVPROC)load("glVertexAttribs4fvNV");
		func->glVertexAttribs4svNV = (PFNGLVERTEXATTRIBS4SVNVPROC)load("glVertexAttribs4svNV");
		func->glVertexAttribs4ubvNV = (PFNGLVERTEXATTRIBS4UBVNVPROC)load("glVertexAttribs4ubvNV");
	}
	static void load_GL_NV_vertex_program4(LoadProc load, GLExtFunctions* func)
    {
		func->glVertexAttribI1iEXT = (PFNGLVERTEXATTRIBI1IEXTPROC)load("glVertexAttribI1iEXT");
		func->glVertexAttribI2iEXT = (PFNGLVERTEXATTRIBI2IEXTPROC)load("glVertexAttribI2iEXT");
		func->glVertexAttribI3iEXT = (PFNGLVERTEXATTRIBI3IEXTPROC)load("glVertexAttribI3iEXT");
		func->glVertexAttribI4iEXT = (PFNGLVERTEXATTRIBI4IEXTPROC)load("glVertexAttribI4iEXT");
		func->glVertexAttribI1uiEXT = (PFNGLVERTEXATTRIBI1UIEXTPROC)load("glVertexAttribI1uiEXT");
		func->glVertexAttribI2uiEXT = (PFNGLVERTEXATTRIBI2UIEXTPROC)load("glVertexAttribI2uiEXT");
		func->glVertexAttribI3uiEXT = (PFNGLVERTEXATTRIBI3UIEXTPROC)load("glVertexAttribI3uiEXT");
		func->glVertexAttribI4uiEXT = (PFNGLVERTEXATTRIBI4UIEXTPROC)load("glVertexAttribI4uiEXT");
		func->glVertexAttribI1ivEXT = (PFNGLVERTEXATTRIBI1IVEXTPROC)load("glVertexAttribI1ivEXT");
		func->glVertexAttribI2ivEXT = (PFNGLVERTEXATTRIBI2IVEXTPROC)load("glVertexAttribI2ivEXT");
		func->glVertexAttribI3ivEXT = (PFNGLVERTEXATTRIBI3IVEXTPROC)load("glVertexAttribI3ivEXT");
		func->glVertexAttribI4ivEXT = (PFNGLVERTEXATTRIBI4IVEXTPROC)load("glVertexAttribI4ivEXT");
		func->glVertexAttribI1uivEXT = (PFNGLVERTEXATTRIBI1UIVEXTPROC)load("glVertexAttribI1uivEXT");
		func->glVertexAttribI2uivEXT = (PFNGLVERTEXATTRIBI2UIVEXTPROC)load("glVertexAttribI2uivEXT");
		func->glVertexAttribI3uivEXT = (PFNGLVERTEXATTRIBI3UIVEXTPROC)load("glVertexAttribI3uivEXT");
		func->glVertexAttribI4uivEXT = (PFNGLVERTEXATTRIBI4UIVEXTPROC)load("glVertexAttribI4uivEXT");
		func->glVertexAttribI4bvEXT = (PFNGLVERTEXATTRIBI4BVEXTPROC)load("glVertexAttribI4bvEXT");
		func->glVertexAttribI4svEXT = (PFNGLVERTEXATTRIBI4SVEXTPROC)load("glVertexAttribI4svEXT");
		func->glVertexAttribI4ubvEXT = (PFNGLVERTEXATTRIBI4UBVEXTPROC)load("glVertexAttribI4ubvEXT");
		func->glVertexAttribI4usvEXT = (PFNGLVERTEXATTRIBI4USVEXTPROC)load("glVertexAttribI4usvEXT");
		func->glVertexAttribIPointerEXT = (PFNGLVERTEXATTRIBIPOINTEREXTPROC)load("glVertexAttribIPointerEXT");
		func->glGetVertexAttribIivEXT = (PFNGLGETVERTEXATTRIBIIVEXTPROC)load("glGetVertexAttribIivEXT");
		func->glGetVertexAttribIuivEXT = (PFNGLGETVERTEXATTRIBIUIVEXTPROC)load("glGetVertexAttribIuivEXT");
	}
	static void load_GL_NV_video_capture(LoadProc load, GLExtFunctions* func)
    {
		func->glBeginVideoCaptureNV = (PFNGLBEGINVIDEOCAPTURENVPROC)load("glBeginVideoCaptureNV");
		func->glBindVideoCaptureStreamBufferNV = (PFNGLBINDVIDEOCAPTURESTREAMBUFFERNVPROC)load("glBindVideoCaptureStreamBufferNV");
		func->glBindVideoCaptureStreamTextureNV = (PFNGLBINDVIDEOCAPTURESTREAMTEXTURENVPROC)load("glBindVideoCaptureStreamTextureNV");
		func->glEndVideoCaptureNV = (PFNGLENDVIDEOCAPTURENVPROC)load("glEndVideoCaptureNV");
		func->glGetVideoCaptureivNV = (PFNGLGETVIDEOCAPTUREIVNVPROC)load("glGetVideoCaptureivNV");
		func->glGetVideoCaptureStreamivNV = (PFNGLGETVIDEOCAPTURESTREAMIVNVPROC)load("glGetVideoCaptureStreamivNV");
		func->glGetVideoCaptureStreamfvNV = (PFNGLGETVIDEOCAPTURESTREAMFVNVPROC)load("glGetVideoCaptureStreamfvNV");
		func->glGetVideoCaptureStreamdvNV = (PFNGLGETVIDEOCAPTURESTREAMDVNVPROC)load("glGetVideoCaptureStreamdvNV");
		func->glVideoCaptureNV = (PFNGLVIDEOCAPTURENVPROC)load("glVideoCaptureNV");
		func->glVideoCaptureStreamParameterivNV = (PFNGLVIDEOCAPTURESTREAMPARAMETERIVNVPROC)load("glVideoCaptureStreamParameterivNV");
		func->glVideoCaptureStreamParameterfvNV = (PFNGLVIDEOCAPTURESTREAMPARAMETERFVNVPROC)load("glVideoCaptureStreamParameterfvNV");
		func->glVideoCaptureStreamParameterdvNV = (PFNGLVIDEOCAPTURESTREAMPARAMETERDVNVPROC)load("glVideoCaptureStreamParameterdvNV");
	}
	static void load_GL_NV_viewport_swizzle(LoadProc load, GLExtFunctions* func)
    {
		func->glViewportSwizzleNV = (PFNGLVIEWPORTSWIZZLENVPROC)load("glViewportSwizzleNV");
	}
	static void load_GL_OES_byte_coordinates(LoadProc load, GLExtFunctions* func)
    {
		func->glMultiTexCoord1bOES = (PFNGLMULTITEXCOORD1BOESPROC)load("glMultiTexCoord1bOES");
		func->glMultiTexCoord1bvOES = (PFNGLMULTITEXCOORD1BVOESPROC)load("glMultiTexCoord1bvOES");
		func->glMultiTexCoord2bOES = (PFNGLMULTITEXCOORD2BOESPROC)load("glMultiTexCoord2bOES");
		func->glMultiTexCoord2bvOES = (PFNGLMULTITEXCOORD2BVOESPROC)load("glMultiTexCoord2bvOES");
		func->glMultiTexCoord3bOES = (PFNGLMULTITEXCOORD3BOESPROC)load("glMultiTexCoord3bOES");
		func->glMultiTexCoord3bvOES = (PFNGLMULTITEXCOORD3BVOESPROC)load("glMultiTexCoord3bvOES");
		func->glMultiTexCoord4bOES = (PFNGLMULTITEXCOORD4BOESPROC)load("glMultiTexCoord4bOES");
		func->glMultiTexCoord4bvOES = (PFNGLMULTITEXCOORD4BVOESPROC)load("glMultiTexCoord4bvOES");
		func->glTexCoord1bOES = (PFNGLTEXCOORD1BOESPROC)load("glTexCoord1bOES");
		func->glTexCoord1bvOES = (PFNGLTEXCOORD1BVOESPROC)load("glTexCoord1bvOES");
		func->glTexCoord2bOES = (PFNGLTEXCOORD2BOESPROC)load("glTexCoord2bOES");
		func->glTexCoord2bvOES = (PFNGLTEXCOORD2BVOESPROC)load("glTexCoord2bvOES");
		func->glTexCoord3bOES = (PFNGLTEXCOORD3BOESPROC)load("glTexCoord3bOES");
		func->glTexCoord3bvOES = (PFNGLTEXCOORD3BVOESPROC)load("glTexCoord3bvOES");
		func->glTexCoord4bOES = (PFNGLTEXCOORD4BOESPROC)load("glTexCoord4bOES");
		func->glTexCoord4bvOES = (PFNGLTEXCOORD4BVOESPROC)load("glTexCoord4bvOES");
		func->glVertex2bOES = (PFNGLVERTEX2BOESPROC)load("glVertex2bOES");
		func->glVertex2bvOES = (PFNGLVERTEX2BVOESPROC)load("glVertex2bvOES");
		func->glVertex3bOES = (PFNGLVERTEX3BOESPROC)load("glVertex3bOES");
		func->glVertex3bvOES = (PFNGLVERTEX3BVOESPROC)load("glVertex3bvOES");
		func->glVertex4bOES = (PFNGLVERTEX4BOESPROC)load("glVertex4bOES");
		func->glVertex4bvOES = (PFNGLVERTEX4BVOESPROC)load("glVertex4bvOES");
	}
	static void load_GL_OES_fixed_point(LoadProc load, GLExtFunctions* func)
    {
		func->glAlphaFuncxOES = (PFNGLALPHAFUNCXOESPROC)load("glAlphaFuncxOES");
		func->glClearColorxOES = (PFNGLCLEARCOLORXOESPROC)load("glClearColorxOES");
		func->glClearDepthxOES = (PFNGLCLEARDEPTHXOESPROC)load("glClearDepthxOES");
		func->glClipPlanexOES = (PFNGLCLIPPLANEXOESPROC)load("glClipPlanexOES");
		func->glColor4xOES = (PFNGLCOLOR4XOESPROC)load("glColor4xOES");
		func->glDepthRangexOES = (PFNGLDEPTHRANGEXOESPROC)load("glDepthRangexOES");
		func->glFogxOES = (PFNGLFOGXOESPROC)load("glFogxOES");
		func->glFogxvOES = (PFNGLFOGXVOESPROC)load("glFogxvOES");
		func->glFrustumxOES = (PFNGLFRUSTUMXOESPROC)load("glFrustumxOES");
		func->glGetClipPlanexOES = (PFNGLGETCLIPPLANEXOESPROC)load("glGetClipPlanexOES");
		func->glGetFixedvOES = (PFNGLGETFIXEDVOESPROC)load("glGetFixedvOES");
		func->glGetTexEnvxvOES = (PFNGLGETTEXENVXVOESPROC)load("glGetTexEnvxvOES");
		func->glGetTexParameterxvOES = (PFNGLGETTEXPARAMETERXVOESPROC)load("glGetTexParameterxvOES");
		func->glLightModelxOES = (PFNGLLIGHTMODELXOESPROC)load("glLightModelxOES");
		func->glLightModelxvOES = (PFNGLLIGHTMODELXVOESPROC)load("glLightModelxvOES");
		func->glLightxOES = (PFNGLLIGHTXOESPROC)load("glLightxOES");
		func->glLightxvOES = (PFNGLLIGHTXVOESPROC)load("glLightxvOES");
		func->glLineWidthxOES = (PFNGLLINEWIDTHXOESPROC)load("glLineWidthxOES");
		func->glLoadMatrixxOES = (PFNGLLOADMATRIXXOESPROC)load("glLoadMatrixxOES");
		func->glMaterialxOES = (PFNGLMATERIALXOESPROC)load("glMaterialxOES");
		func->glMaterialxvOES = (PFNGLMATERIALXVOESPROC)load("glMaterialxvOES");
		func->glMultMatrixxOES = (PFNGLMULTMATRIXXOESPROC)load("glMultMatrixxOES");
		func->glMultiTexCoord4xOES = (PFNGLMULTITEXCOORD4XOESPROC)load("glMultiTexCoord4xOES");
		func->glNormal3xOES = (PFNGLNORMAL3XOESPROC)load("glNormal3xOES");
		func->glOrthoxOES = (PFNGLORTHOXOESPROC)load("glOrthoxOES");
		func->glPointParameterxvOES = (PFNGLPOINTPARAMETERXVOESPROC)load("glPointParameterxvOES");
		func->glPointSizexOES = (PFNGLPOINTSIZEXOESPROC)load("glPointSizexOES");
		func->glPolygonOffsetxOES = (PFNGLPOLYGONOFFSETXOESPROC)load("glPolygonOffsetxOES");
		func->glRotatexOES = (PFNGLROTATEXOESPROC)load("glRotatexOES");
		func->glScalexOES = (PFNGLSCALEXOESPROC)load("glScalexOES");
		func->glTexEnvxOES = (PFNGLTEXENVXOESPROC)load("glTexEnvxOES");
		func->glTexEnvxvOES = (PFNGLTEXENVXVOESPROC)load("glTexEnvxvOES");
		func->glTexParameterxOES = (PFNGLTEXPARAMETERXOESPROC)load("glTexParameterxOES");
		func->glTexParameterxvOES = (PFNGLTEXPARAMETERXVOESPROC)load("glTexParameterxvOES");
		func->glTranslatexOES = (PFNGLTRANSLATEXOESPROC)load("glTranslatexOES");
		func->glAccumxOES = (PFNGLACCUMXOESPROC)load("glAccumxOES");
		func->glBitmapxOES = (PFNGLBITMAPXOESPROC)load("glBitmapxOES");
		func->glBlendColorxOES = (PFNGLBLENDCOLORXOESPROC)load("glBlendColorxOES");
		func->glClearAccumxOES = (PFNGLCLEARACCUMXOESPROC)load("glClearAccumxOES");
		func->glColor3xOES = (PFNGLCOLOR3XOESPROC)load("glColor3xOES");
		func->glColor3xvOES = (PFNGLCOLOR3XVOESPROC)load("glColor3xvOES");
		func->glColor4xvOES = (PFNGLCOLOR4XVOESPROC)load("glColor4xvOES");
		func->glConvolutionParameterxOES = (PFNGLCONVOLUTIONPARAMETERXOESPROC)load("glConvolutionParameterxOES");
		func->glConvolutionParameterxvOES = (PFNGLCONVOLUTIONPARAMETERXVOESPROC)load("glConvolutionParameterxvOES");
		func->glEvalCoord1xOES = (PFNGLEVALCOORD1XOESPROC)load("glEvalCoord1xOES");
		func->glEvalCoord1xvOES = (PFNGLEVALCOORD1XVOESPROC)load("glEvalCoord1xvOES");
		func->glEvalCoord2xOES = (PFNGLEVALCOORD2XOESPROC)load("glEvalCoord2xOES");
		func->glEvalCoord2xvOES = (PFNGLEVALCOORD2XVOESPROC)load("glEvalCoord2xvOES");
		func->glFeedbackBufferxOES = (PFNGLFEEDBACKBUFFERXOESPROC)load("glFeedbackBufferxOES");
		func->glGetConvolutionParameterxvOES = (PFNGLGETCONVOLUTIONPARAMETERXVOESPROC)load("glGetConvolutionParameterxvOES");
		func->glGetHistogramParameterxvOES = (PFNGLGETHISTOGRAMPARAMETERXVOESPROC)load("glGetHistogramParameterxvOES");
		func->glGetLightxOES = (PFNGLGETLIGHTXOESPROC)load("glGetLightxOES");
		func->glGetMapxvOES = (PFNGLGETMAPXVOESPROC)load("glGetMapxvOES");
		func->glGetMaterialxOES = (PFNGLGETMATERIALXOESPROC)load("glGetMaterialxOES");
		func->glGetPixelMapxv = (PFNGLGETPIXELMAPXVPROC)load("glGetPixelMapxv");
		func->glGetTexGenxvOES = (PFNGLGETTEXGENXVOESPROC)load("glGetTexGenxvOES");
		func->glGetTexLevelParameterxvOES = (PFNGLGETTEXLEVELPARAMETERXVOESPROC)load("glGetTexLevelParameterxvOES");
		func->glIndexxOES = (PFNGLINDEXXOESPROC)load("glIndexxOES");
		func->glIndexxvOES = (PFNGLINDEXXVOESPROC)load("glIndexxvOES");
		func->glLoadTransposeMatrixxOES = (PFNGLLOADTRANSPOSEMATRIXXOESPROC)load("glLoadTransposeMatrixxOES");
		func->glMap1xOES = (PFNGLMAP1XOESPROC)load("glMap1xOES");
		func->glMap2xOES = (PFNGLMAP2XOESPROC)load("glMap2xOES");
		func->glMapGrid1xOES = (PFNGLMAPGRID1XOESPROC)load("glMapGrid1xOES");
		func->glMapGrid2xOES = (PFNGLMAPGRID2XOESPROC)load("glMapGrid2xOES");
		func->glMultTransposeMatrixxOES = (PFNGLMULTTRANSPOSEMATRIXXOESPROC)load("glMultTransposeMatrixxOES");
		func->glMultiTexCoord1xOES = (PFNGLMULTITEXCOORD1XOESPROC)load("glMultiTexCoord1xOES");
		func->glMultiTexCoord1xvOES = (PFNGLMULTITEXCOORD1XVOESPROC)load("glMultiTexCoord1xvOES");
		func->glMultiTexCoord2xOES = (PFNGLMULTITEXCOORD2XOESPROC)load("glMultiTexCoord2xOES");
		func->glMultiTexCoord2xvOES = (PFNGLMULTITEXCOORD2XVOESPROC)load("glMultiTexCoord2xvOES");
		func->glMultiTexCoord3xOES = (PFNGLMULTITEXCOORD3XOESPROC)load("glMultiTexCoord3xOES");
		func->glMultiTexCoord3xvOES = (PFNGLMULTITEXCOORD3XVOESPROC)load("glMultiTexCoord3xvOES");
		func->glMultiTexCoord4xvOES = (PFNGLMULTITEXCOORD4XVOESPROC)load("glMultiTexCoord4xvOES");
		func->glNormal3xvOES = (PFNGLNORMAL3XVOESPROC)load("glNormal3xvOES");
		func->glPassThroughxOES = (PFNGLPASSTHROUGHXOESPROC)load("glPassThroughxOES");
		func->glPixelMapx = (PFNGLPIXELMAPXPROC)load("glPixelMapx");
		func->glPixelStorex = (PFNGLPIXELSTOREXPROC)load("glPixelStorex");
		func->glPixelTransferxOES = (PFNGLPIXELTRANSFERXOESPROC)load("glPixelTransferxOES");
		func->glPixelZoomxOES = (PFNGLPIXELZOOMXOESPROC)load("glPixelZoomxOES");
		func->glPrioritizeTexturesxOES = (PFNGLPRIORITIZETEXTURESXOESPROC)load("glPrioritizeTexturesxOES");
		func->glRasterPos2xOES = (PFNGLRASTERPOS2XOESPROC)load("glRasterPos2xOES");
		func->glRasterPos2xvOES = (PFNGLRASTERPOS2XVOESPROC)load("glRasterPos2xvOES");
		func->glRasterPos3xOES = (PFNGLRASTERPOS3XOESPROC)load("glRasterPos3xOES");
		func->glRasterPos3xvOES = (PFNGLRASTERPOS3XVOESPROC)load("glRasterPos3xvOES");
		func->glRasterPos4xOES = (PFNGLRASTERPOS4XOESPROC)load("glRasterPos4xOES");
		func->glRasterPos4xvOES = (PFNGLRASTERPOS4XVOESPROC)load("glRasterPos4xvOES");
		func->glRectxOES = (PFNGLRECTXOESPROC)load("glRectxOES");
		func->glRectxvOES = (PFNGLRECTXVOESPROC)load("glRectxvOES");
		func->glTexCoord1xOES = (PFNGLTEXCOORD1XOESPROC)load("glTexCoord1xOES");
		func->glTexCoord1xvOES = (PFNGLTEXCOORD1XVOESPROC)load("glTexCoord1xvOES");
		func->glTexCoord2xOES = (PFNGLTEXCOORD2XOESPROC)load("glTexCoord2xOES");
		func->glTexCoord2xvOES = (PFNGLTEXCOORD2XVOESPROC)load("glTexCoord2xvOES");
		func->glTexCoord3xOES = (PFNGLTEXCOORD3XOESPROC)load("glTexCoord3xOES");
		func->glTexCoord3xvOES = (PFNGLTEXCOORD3XVOESPROC)load("glTexCoord3xvOES");
		func->glTexCoord4xOES = (PFNGLTEXCOORD4XOESPROC)load("glTexCoord4xOES");
		func->glTexCoord4xvOES = (PFNGLTEXCOORD4XVOESPROC)load("glTexCoord4xvOES");
		func->glTexGenxOES = (PFNGLTEXGENXOESPROC)load("glTexGenxOES");
		func->glTexGenxvOES = (PFNGLTEXGENXVOESPROC)load("glTexGenxvOES");
		func->glVertex2xOES = (PFNGLVERTEX2XOESPROC)load("glVertex2xOES");
		func->glVertex2xvOES = (PFNGLVERTEX2XVOESPROC)load("glVertex2xvOES");
		func->glVertex3xOES = (PFNGLVERTEX3XOESPROC)load("glVertex3xOES");
		func->glVertex3xvOES = (PFNGLVERTEX3XVOESPROC)load("glVertex3xvOES");
		func->glVertex4xOES = (PFNGLVERTEX4XOESPROC)load("glVertex4xOES");
		func->glVertex4xvOES = (PFNGLVERTEX4XVOESPROC)load("glVertex4xvOES");
	}
	static void load_GL_OES_query_matrix(LoadProc load, GLExtFunctions* func)
    {
		func->glQueryMatrixxOES = (PFNGLQUERYMATRIXXOESPROC)load("glQueryMatrixxOES");
	}
	static void load_GL_OES_single_precision(LoadProc load, GLExtFunctions* func)
    {
		func->glClearDepthfOES = (PFNGLCLEARDEPTHFOESPROC)load("glClearDepthfOES");
		func->glClipPlanefOES = (PFNGLCLIPPLANEFOESPROC)load("glClipPlanefOES");
		func->glDepthRangefOES = (PFNGLDEPTHRANGEFOESPROC)load("glDepthRangefOES");
		func->glFrustumfOES = (PFNGLFRUSTUMFOESPROC)load("glFrustumfOES");
		func->glGetClipPlanefOES = (PFNGLGETCLIPPLANEFOESPROC)load("glGetClipPlanefOES");
		func->glOrthofOES = (PFNGLORTHOFOESPROC)load("glOrthofOES");
	}
	static void load_GL_OVR_multiview(LoadProc load, GLExtFunctions* func)
    {
		func->glFramebufferTextureMultiviewOVR = (PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC)load("glFramebufferTextureMultiviewOVR");
		func->glNamedFramebufferTextureMultiviewOVR = (PFNGLNAMEDFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC)load("glNamedFramebufferTextureMultiviewOVR");
	}
	static void load_GL_PGI_misc_hints(LoadProc load, GLExtFunctions* func)
    {
		func->glHintPGI = (PFNGLHINTPGIPROC)load("glHintPGI");
	}
	static void load_GL_SGIS_detail_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glDetailTexFuncSGIS = (PFNGLDETAILTEXFUNCSGISPROC)load("glDetailTexFuncSGIS");
		func->glGetDetailTexFuncSGIS = (PFNGLGETDETAILTEXFUNCSGISPROC)load("glGetDetailTexFuncSGIS");
	}
	static void load_GL_SGIS_fog_function(LoadProc load, GLExtFunctions* func)
    {
		func->glFogFuncSGIS = (PFNGLFOGFUNCSGISPROC)load("glFogFuncSGIS");
		func->glGetFogFuncSGIS = (PFNGLGETFOGFUNCSGISPROC)load("glGetFogFuncSGIS");
	}
	static void load_GL_SGIS_multisample(LoadProc load, GLExtFunctions* func)
    {
		func->glSampleMaskSGIS = (PFNGLSAMPLEMASKSGISPROC)load("glSampleMaskSGIS");
		func->glSamplePatternSGIS = (PFNGLSAMPLEPATTERNSGISPROC)load("glSamplePatternSGIS");
	}
	static void load_GL_SGIS_pixel_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glPixelTexGenParameteriSGIS = (PFNGLPIXELTEXGENPARAMETERISGISPROC)load("glPixelTexGenParameteriSGIS");
		func->glPixelTexGenParameterivSGIS = (PFNGLPIXELTEXGENPARAMETERIVSGISPROC)load("glPixelTexGenParameterivSGIS");
		func->glPixelTexGenParameterfSGIS = (PFNGLPIXELTEXGENPARAMETERFSGISPROC)load("glPixelTexGenParameterfSGIS");
		func->glPixelTexGenParameterfvSGIS = (PFNGLPIXELTEXGENPARAMETERFVSGISPROC)load("glPixelTexGenParameterfvSGIS");
		func->glGetPixelTexGenParameterivSGIS = (PFNGLGETPIXELTEXGENPARAMETERIVSGISPROC)load("glGetPixelTexGenParameterivSGIS");
		func->glGetPixelTexGenParameterfvSGIS = (PFNGLGETPIXELTEXGENPARAMETERFVSGISPROC)load("glGetPixelTexGenParameterfvSGIS");
	}
	static void load_GL_SGIS_point_parameters(LoadProc load, GLExtFunctions* func)
    {
		func->glPointParameterfSGIS = (PFNGLPOINTPARAMETERFSGISPROC)load("glPointParameterfSGIS");
		func->glPointParameterfvSGIS = (PFNGLPOINTPARAMETERFVSGISPROC)load("glPointParameterfvSGIS");
	}
	static void load_GL_SGIS_sharpen_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glSharpenTexFuncSGIS = (PFNGLSHARPENTEXFUNCSGISPROC)load("glSharpenTexFuncSGIS");
		func->glGetSharpenTexFuncSGIS = (PFNGLGETSHARPENTEXFUNCSGISPROC)load("glGetSharpenTexFuncSGIS");
	}
	static void load_GL_SGIS_texture4D(LoadProc load, GLExtFunctions* func)
    {
		func->glTexImage4DSGIS = (PFNGLTEXIMAGE4DSGISPROC)load("glTexImage4DSGIS");
		func->glTexSubImage4DSGIS = (PFNGLTEXSUBIMAGE4DSGISPROC)load("glTexSubImage4DSGIS");
	}
	static void load_GL_SGIS_texture_color_mask(LoadProc load, GLExtFunctions* func)
    {
		func->glTextureColorMaskSGIS = (PFNGLTEXTURECOLORMASKSGISPROC)load("glTextureColorMaskSGIS");
	}
	static void load_GL_SGIS_texture_filter4(LoadProc load, GLExtFunctions* func)
    {
		func->glGetTexFilterFuncSGIS = (PFNGLGETTEXFILTERFUNCSGISPROC)load("glGetTexFilterFuncSGIS");
		func->glTexFilterFuncSGIS = (PFNGLTEXFILTERFUNCSGISPROC)load("glTexFilterFuncSGIS");
	}
	static void load_GL_SGIX_async(LoadProc load, GLExtFunctions* func)
    {
		func->glAsyncMarkerSGIX = (PFNGLASYNCMARKERSGIXPROC)load("glAsyncMarkerSGIX");
		func->glFinishAsyncSGIX = (PFNGLFINISHASYNCSGIXPROC)load("glFinishAsyncSGIX");
		func->glPollAsyncSGIX = (PFNGLPOLLASYNCSGIXPROC)load("glPollAsyncSGIX");
		func->glGenAsyncMarkersSGIX = (PFNGLGENASYNCMARKERSSGIXPROC)load("glGenAsyncMarkersSGIX");
		func->glDeleteAsyncMarkersSGIX = (PFNGLDELETEASYNCMARKERSSGIXPROC)load("glDeleteAsyncMarkersSGIX");
		func->glIsAsyncMarkerSGIX = (PFNGLISASYNCMARKERSGIXPROC)load("glIsAsyncMarkerSGIX");
	}
	static void load_GL_SGIX_flush_raster(LoadProc load, GLExtFunctions* func)
    {
		func->glFlushRasterSGIX = (PFNGLFLUSHRASTERSGIXPROC)load("glFlushRasterSGIX");
	}
	static void load_GL_SGIX_fragment_lighting(LoadProc load, GLExtFunctions* func)
    {
		func->glFragmentColorMaterialSGIX = (PFNGLFRAGMENTCOLORMATERIALSGIXPROC)load("glFragmentColorMaterialSGIX");
		func->glFragmentLightfSGIX = (PFNGLFRAGMENTLIGHTFSGIXPROC)load("glFragmentLightfSGIX");
		func->glFragmentLightfvSGIX = (PFNGLFRAGMENTLIGHTFVSGIXPROC)load("glFragmentLightfvSGIX");
		func->glFragmentLightiSGIX = (PFNGLFRAGMENTLIGHTISGIXPROC)load("glFragmentLightiSGIX");
		func->glFragmentLightivSGIX = (PFNGLFRAGMENTLIGHTIVSGIXPROC)load("glFragmentLightivSGIX");
		func->glFragmentLightModelfSGIX = (PFNGLFRAGMENTLIGHTMODELFSGIXPROC)load("glFragmentLightModelfSGIX");
		func->glFragmentLightModelfvSGIX = (PFNGLFRAGMENTLIGHTMODELFVSGIXPROC)load("glFragmentLightModelfvSGIX");
		func->glFragmentLightModeliSGIX = (PFNGLFRAGMENTLIGHTMODELISGIXPROC)load("glFragmentLightModeliSGIX");
		func->glFragmentLightModelivSGIX = (PFNGLFRAGMENTLIGHTMODELIVSGIXPROC)load("glFragmentLightModelivSGIX");
		func->glFragmentMaterialfSGIX = (PFNGLFRAGMENTMATERIALFSGIXPROC)load("glFragmentMaterialfSGIX");
		func->glFragmentMaterialfvSGIX = (PFNGLFRAGMENTMATERIALFVSGIXPROC)load("glFragmentMaterialfvSGIX");
		func->glFragmentMaterialiSGIX = (PFNGLFRAGMENTMATERIALISGIXPROC)load("glFragmentMaterialiSGIX");
		func->glFragmentMaterialivSGIX = (PFNGLFRAGMENTMATERIALIVSGIXPROC)load("glFragmentMaterialivSGIX");
		func->glGetFragmentLightfvSGIX = (PFNGLGETFRAGMENTLIGHTFVSGIXPROC)load("glGetFragmentLightfvSGIX");
		func->glGetFragmentLightivSGIX = (PFNGLGETFRAGMENTLIGHTIVSGIXPROC)load("glGetFragmentLightivSGIX");
		func->glGetFragmentMaterialfvSGIX = (PFNGLGETFRAGMENTMATERIALFVSGIXPROC)load("glGetFragmentMaterialfvSGIX");
		func->glGetFragmentMaterialivSGIX = (PFNGLGETFRAGMENTMATERIALIVSGIXPROC)load("glGetFragmentMaterialivSGIX");
		func->glLightEnviSGIX = (PFNGLLIGHTENVISGIXPROC)load("glLightEnviSGIX");
	}
	static void load_GL_SGIX_framezoom(LoadProc load, GLExtFunctions* func)
    {
		func->glFrameZoomSGIX = (PFNGLFRAMEZOOMSGIXPROC)load("glFrameZoomSGIX");
	}
	static void load_GL_SGIX_igloo_interface(LoadProc load, GLExtFunctions* func)
    {
		func->glIglooInterfaceSGIX = (PFNGLIGLOOINTERFACESGIXPROC)load("glIglooInterfaceSGIX");
	}
	static void load_GL_SGIX_instruments(LoadProc load, GLExtFunctions* func)
    {
		func->glGetInstrumentsSGIX = (PFNGLGETINSTRUMENTSSGIXPROC)load("glGetInstrumentsSGIX");
		func->glInstrumentsBufferSGIX = (PFNGLINSTRUMENTSBUFFERSGIXPROC)load("glInstrumentsBufferSGIX");
		func->glPollInstrumentsSGIX = (PFNGLPOLLINSTRUMENTSSGIXPROC)load("glPollInstrumentsSGIX");
		func->glReadInstrumentsSGIX = (PFNGLREADINSTRUMENTSSGIXPROC)load("glReadInstrumentsSGIX");
		func->glStartInstrumentsSGIX = (PFNGLSTARTINSTRUMENTSSGIXPROC)load("glStartInstrumentsSGIX");
		func->glStopInstrumentsSGIX = (PFNGLSTOPINSTRUMENTSSGIXPROC)load("glStopInstrumentsSGIX");
	}
	static void load_GL_SGIX_list_priority(LoadProc load, GLExtFunctions* func)
    {
		func->glGetListParameterfvSGIX = (PFNGLGETLISTPARAMETERFVSGIXPROC)load("glGetListParameterfvSGIX");
		func->glGetListParameterivSGIX = (PFNGLGETLISTPARAMETERIVSGIXPROC)load("glGetListParameterivSGIX");
		func->glListParameterfSGIX = (PFNGLLISTPARAMETERFSGIXPROC)load("glListParameterfSGIX");
		func->glListParameterfvSGIX = (PFNGLLISTPARAMETERFVSGIXPROC)load("glListParameterfvSGIX");
		func->glListParameteriSGIX = (PFNGLLISTPARAMETERISGIXPROC)load("glListParameteriSGIX");
		func->glListParameterivSGIX = (PFNGLLISTPARAMETERIVSGIXPROC)load("glListParameterivSGIX");
	}
	static void load_GL_SGIX_pixel_texture(LoadProc load, GLExtFunctions* func)
    {
		func->glPixelTexGenSGIX = (PFNGLPIXELTEXGENSGIXPROC)load("glPixelTexGenSGIX");
	}
	static void load_GL_SGIX_polynomial_ffd(LoadProc load, GLExtFunctions* func)
    {
		func->glDeformationMap3dSGIX = (PFNGLDEFORMATIONMAP3DSGIXPROC)load("glDeformationMap3dSGIX");
		func->glDeformationMap3fSGIX = (PFNGLDEFORMATIONMAP3FSGIXPROC)load("glDeformationMap3fSGIX");
		func->glDeformSGIX = (PFNGLDEFORMSGIXPROC)load("glDeformSGIX");
		func->glLoadIdentityDeformationMapSGIX = (PFNGLLOADIDENTITYDEFORMATIONMAPSGIXPROC)load("glLoadIdentityDeformationMapSGIX");
	}
	static void load_GL_SGIX_reference_plane(LoadProc load, GLExtFunctions* func)
    {
		func->glReferencePlaneSGIX = (PFNGLREFERENCEPLANESGIXPROC)load("glReferencePlaneSGIX");
	}
	static void load_GL_SGIX_sprite(LoadProc load, GLExtFunctions* func)
    {
		func->glSpriteParameterfSGIX = (PFNGLSPRITEPARAMETERFSGIXPROC)load("glSpriteParameterfSGIX");
		func->glSpriteParameterfvSGIX = (PFNGLSPRITEPARAMETERFVSGIXPROC)load("glSpriteParameterfvSGIX");
		func->glSpriteParameteriSGIX = (PFNGLSPRITEPARAMETERISGIXPROC)load("glSpriteParameteriSGIX");
		func->glSpriteParameterivSGIX = (PFNGLSPRITEPARAMETERIVSGIXPROC)load("glSpriteParameterivSGIX");
	}
	static void load_GL_SGIX_tag_sample_buffer(LoadProc load, GLExtFunctions* func)
    {
		func->glTagSampleBufferSGIX = (PFNGLTAGSAMPLEBUFFERSGIXPROC)load("glTagSampleBufferSGIX");
	}
	static void load_GL_SGI_color_table(LoadProc load, GLExtFunctions* func)
    {
		func->glColorTableSGI = (PFNGLCOLORTABLESGIPROC)load("glColorTableSGI");
		func->glColorTableParameterfvSGI = (PFNGLCOLORTABLEPARAMETERFVSGIPROC)load("glColorTableParameterfvSGI");
		func->glColorTableParameterivSGI = (PFNGLCOLORTABLEPARAMETERIVSGIPROC)load("glColorTableParameterivSGI");
		func->glCopyColorTableSGI = (PFNGLCOPYCOLORTABLESGIPROC)load("glCopyColorTableSGI");
		func->glGetColorTableSGI = (PFNGLGETCOLORTABLESGIPROC)load("glGetColorTableSGI");
		func->glGetColorTableParameterfvSGI = (PFNGLGETCOLORTABLEPARAMETERFVSGIPROC)load("glGetColorTableParameterfvSGI");
		func->glGetColorTableParameterivSGI = (PFNGLGETCOLORTABLEPARAMETERIVSGIPROC)load("glGetColorTableParameterivSGI");
	}
	static void load_GL_SUNX_constant_data(LoadProc load, GLExtFunctions* func)
    {
		func->glFinishTextureSUNX = (PFNGLFINISHTEXTURESUNXPROC)load("glFinishTextureSUNX");
	}
	static void load_GL_SUN_global_alpha(LoadProc load, GLExtFunctions* func)
    {
		func->glGlobalAlphaFactorbSUN = (PFNGLGLOBALALPHAFACTORBSUNPROC)load("glGlobalAlphaFactorbSUN");
		func->glGlobalAlphaFactorsSUN = (PFNGLGLOBALALPHAFACTORSSUNPROC)load("glGlobalAlphaFactorsSUN");
		func->glGlobalAlphaFactoriSUN = (PFNGLGLOBALALPHAFACTORISUNPROC)load("glGlobalAlphaFactoriSUN");
		func->glGlobalAlphaFactorfSUN = (PFNGLGLOBALALPHAFACTORFSUNPROC)load("glGlobalAlphaFactorfSUN");
		func->glGlobalAlphaFactordSUN = (PFNGLGLOBALALPHAFACTORDSUNPROC)load("glGlobalAlphaFactordSUN");
		func->glGlobalAlphaFactorubSUN = (PFNGLGLOBALALPHAFACTORUBSUNPROC)load("glGlobalAlphaFactorubSUN");
		func->glGlobalAlphaFactorusSUN = (PFNGLGLOBALALPHAFACTORUSSUNPROC)load("glGlobalAlphaFactorusSUN");
		func->glGlobalAlphaFactoruiSUN = (PFNGLGLOBALALPHAFACTORUISUNPROC)load("glGlobalAlphaFactoruiSUN");
	}
	static void load_GL_SUN_mesh_array(LoadProc load, GLExtFunctions* func)
    {
		func->glDrawMeshArraysSUN = (PFNGLDRAWMESHARRAYSSUNPROC)load("glDrawMeshArraysSUN");
	}
	static void load_GL_SUN_triangle_list(LoadProc load, GLExtFunctions* func)
    {
		func->glReplacementCodeuiSUN = (PFNGLREPLACEMENTCODEUISUNPROC)load("glReplacementCodeuiSUN");
		func->glReplacementCodeusSUN = (PFNGLREPLACEMENTCODEUSSUNPROC)load("glReplacementCodeusSUN");
		func->glReplacementCodeubSUN = (PFNGLREPLACEMENTCODEUBSUNPROC)load("glReplacementCodeubSUN");
		func->glReplacementCodeuivSUN = (PFNGLREPLACEMENTCODEUIVSUNPROC)load("glReplacementCodeuivSUN");
		func->glReplacementCodeusvSUN = (PFNGLREPLACEMENTCODEUSVSUNPROC)load("glReplacementCodeusvSUN");
		func->glReplacementCodeubvSUN = (PFNGLREPLACEMENTCODEUBVSUNPROC)load("glReplacementCodeubvSUN");
		func->glReplacementCodePointerSUN = (PFNGLREPLACEMENTCODEPOINTERSUNPROC)load("glReplacementCodePointerSUN");
	}
	static void load_GL_SUN_vertex(LoadProc load, GLExtFunctions* func)
    {
		func->glColor4ubVertex2fSUN = (PFNGLCOLOR4UBVERTEX2FSUNPROC)load("glColor4ubVertex2fSUN");
		func->glColor4ubVertex2fvSUN = (PFNGLCOLOR4UBVERTEX2FVSUNPROC)load("glColor4ubVertex2fvSUN");
		func->glColor4ubVertex3fSUN = (PFNGLCOLOR4UBVERTEX3FSUNPROC)load("glColor4ubVertex3fSUN");
		func->glColor4ubVertex3fvSUN = (PFNGLCOLOR4UBVERTEX3FVSUNPROC)load("glColor4ubVertex3fvSUN");
		func->glColor3fVertex3fSUN = (PFNGLCOLOR3FVERTEX3FSUNPROC)load("glColor3fVertex3fSUN");
		func->glColor3fVertex3fvSUN = (PFNGLCOLOR3FVERTEX3FVSUNPROC)load("glColor3fVertex3fvSUN");
		func->glNormal3fVertex3fSUN = (PFNGLNORMAL3FVERTEX3FSUNPROC)load("glNormal3fVertex3fSUN");
		func->glNormal3fVertex3fvSUN = (PFNGLNORMAL3FVERTEX3FVSUNPROC)load("glNormal3fVertex3fvSUN");
		func->glColor4fNormal3fVertex3fSUN = (PFNGLCOLOR4FNORMAL3FVERTEX3FSUNPROC)load("glColor4fNormal3fVertex3fSUN");
		func->glColor4fNormal3fVertex3fvSUN = (PFNGLCOLOR4FNORMAL3FVERTEX3FVSUNPROC)load("glColor4fNormal3fVertex3fvSUN");
		func->glTexCoord2fVertex3fSUN = (PFNGLTEXCOORD2FVERTEX3FSUNPROC)load("glTexCoord2fVertex3fSUN");
		func->glTexCoord2fVertex3fvSUN = (PFNGLTEXCOORD2FVERTEX3FVSUNPROC)load("glTexCoord2fVertex3fvSUN");
		func->glTexCoord4fVertex4fSUN = (PFNGLTEXCOORD4FVERTEX4FSUNPROC)load("glTexCoord4fVertex4fSUN");
		func->glTexCoord4fVertex4fvSUN = (PFNGLTEXCOORD4FVERTEX4FVSUNPROC)load("glTexCoord4fVertex4fvSUN");
		func->glTexCoord2fColor4ubVertex3fSUN = (PFNGLTEXCOORD2FCOLOR4UBVERTEX3FSUNPROC)load("glTexCoord2fColor4ubVertex3fSUN");
		func->glTexCoord2fColor4ubVertex3fvSUN = (PFNGLTEXCOORD2FCOLOR4UBVERTEX3FVSUNPROC)load("glTexCoord2fColor4ubVertex3fvSUN");
		func->glTexCoord2fColor3fVertex3fSUN = (PFNGLTEXCOORD2FCOLOR3FVERTEX3FSUNPROC)load("glTexCoord2fColor3fVertex3fSUN");
		func->glTexCoord2fColor3fVertex3fvSUN = (PFNGLTEXCOORD2FCOLOR3FVERTEX3FVSUNPROC)load("glTexCoord2fColor3fVertex3fvSUN");
		func->glTexCoord2fNormal3fVertex3fSUN = (PFNGLTEXCOORD2FNORMAL3FVERTEX3FSUNPROC)load("glTexCoord2fNormal3fVertex3fSUN");
		func->glTexCoord2fNormal3fVertex3fvSUN = (PFNGLTEXCOORD2FNORMAL3FVERTEX3FVSUNPROC)load("glTexCoord2fNormal3fVertex3fvSUN");
		func->glTexCoord2fColor4fNormal3fVertex3fSUN = (PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC)load("glTexCoord2fColor4fNormal3fVertex3fSUN");
		func->glTexCoord2fColor4fNormal3fVertex3fvSUN = (PFNGLTEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC)load("glTexCoord2fColor4fNormal3fVertex3fvSUN");
		func->glTexCoord4fColor4fNormal3fVertex4fSUN = (PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FSUNPROC)load("glTexCoord4fColor4fNormal3fVertex4fSUN");
		func->glTexCoord4fColor4fNormal3fVertex4fvSUN = (PFNGLTEXCOORD4FCOLOR4FNORMAL3FVERTEX4FVSUNPROC)load("glTexCoord4fColor4fNormal3fVertex4fvSUN");
		func->glReplacementCodeuiVertex3fSUN = (PFNGLREPLACEMENTCODEUIVERTEX3FSUNPROC)load("glReplacementCodeuiVertex3fSUN");
		func->glReplacementCodeuiVertex3fvSUN = (PFNGLREPLACEMENTCODEUIVERTEX3FVSUNPROC)load("glReplacementCodeuiVertex3fvSUN");
		func->glReplacementCodeuiColor4ubVertex3fSUN = (PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FSUNPROC)load("glReplacementCodeuiColor4ubVertex3fSUN");
		func->glReplacementCodeuiColor4ubVertex3fvSUN = (PFNGLREPLACEMENTCODEUICOLOR4UBVERTEX3FVSUNPROC)load("glReplacementCodeuiColor4ubVertex3fvSUN");
		func->glReplacementCodeuiColor3fVertex3fSUN = (PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FSUNPROC)load("glReplacementCodeuiColor3fVertex3fSUN");
		func->glReplacementCodeuiColor3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUICOLOR3FVERTEX3FVSUNPROC)load("glReplacementCodeuiColor3fVertex3fvSUN");
		func->glReplacementCodeuiNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FSUNPROC)load("glReplacementCodeuiNormal3fVertex3fSUN");
		func->glReplacementCodeuiNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUINORMAL3FVERTEX3FVSUNPROC)load("glReplacementCodeuiNormal3fVertex3fvSUN");
		func->glReplacementCodeuiColor4fNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FSUNPROC)load("glReplacementCodeuiColor4fNormal3fVertex3fSUN");
		func->glReplacementCodeuiColor4fNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUICOLOR4FNORMAL3FVERTEX3FVSUNPROC)load("glReplacementCodeuiColor4fNormal3fVertex3fvSUN");
		func->glReplacementCodeuiTexCoord2fVertex3fSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FSUNPROC)load("glReplacementCodeuiTexCoord2fVertex3fSUN");
		func->glReplacementCodeuiTexCoord2fVertex3fvSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FVERTEX3FVSUNPROC)load("glReplacementCodeuiTexCoord2fVertex3fvSUN");
		func->glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FSUNPROC)load("glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN");
		func->glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FNORMAL3FVERTEX3FVSUNPROC)load("glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN");
		func->glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FSUNPROC)load("glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN");
		func->glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN = (PFNGLREPLACEMENTCODEUITEXCOORD2FCOLOR4FNORMAL3FVERTEX3FVSUNPROC)load("glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN");
	}
	static GLExtFunctions* load_GL_EXT_funcs(GLFunctions* func, int major)
	{
		std::vector<const char*> exts;
	    if (major >= 3)
		{
			int num = 0;
			func->glGetIntegerv(GL_NUM_EXTENSIONS, &num);
			exts.resize(num);
			for (int i = 0; i < num; i++)
				exts[i] = (const char*)func->glGetStringi(GL_EXTENSIONS, i);
		}

		if (!exts.empty())
		{
			GLExtFunctions* ext_func = new GLExtFunctions;
#define LOAD_GL_EXT_FUNC(ext, ...) if(has_ext(exts, #ext)) load_##ext(get_proc, __VA_ARGS__)

			LOAD_GL_EXT_FUNC(GL_3DFX_tbuffer, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_debug_output, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_draw_buffers_blend, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_framebuffer_multisample_advanced, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_framebuffer_sample_positions, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_gpu_shader_int64, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_interleaved_elements, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_multi_draw_indirect, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_name_gen_delete, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_occlusion_query_event, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_performance_monitor, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_sample_positions, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_sparse_texture, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_stencil_operation_extended, ext_func);
			LOAD_GL_EXT_FUNC(GL_AMD_vertex_shader_tessellator, ext_func);
			LOAD_GL_EXT_FUNC(GL_APPLE_element_array, ext_func);
			LOAD_GL_EXT_FUNC(GL_APPLE_fence, ext_func);
			// todo: other ext function loader
			/*func->GL_APPLE_float_pixels = has_ext("GL_APPLE_float_pixels");
			func->GL_APPLE_flush_buffer_range = has_ext("GL_APPLE_flush_buffer_range");
			func->GL_APPLE_object_purgeable = has_ext("GL_APPLE_object_purgeable");
			func->GL_APPLE_rgb_422 = has_ext("GL_APPLE_rgb_422");
			func->GL_APPLE_row_bytes = has_ext("GL_APPLE_row_bytes");
			func->GL_APPLE_specular_vector = has_ext("GL_APPLE_specular_vector");
			func->GL_APPLE_texture_range = has_ext("GL_APPLE_texture_range");
			func->GL_APPLE_transform_hint = has_ext("GL_APPLE_transform_hint");
			func->GL_APPLE_vertex_array_object = has_ext("GL_APPLE_vertex_array_object");
			func->GL_APPLE_vertex_array_range = has_ext("GL_APPLE_vertex_array_range");
			func->GL_APPLE_vertex_program_evaluators = has_ext("GL_APPLE_vertex_program_evaluators");
			func->GL_APPLE_ycbcr_422 = has_ext("GL_APPLE_ycbcr_422");
			func->GL_ARB_ES2_compatibility = has_ext("GL_ARB_ES2_compatibility");
			func->GL_ARB_ES3_1_compatibility = has_ext("GL_ARB_ES3_1_compatibility");
			func->GL_ARB_ES3_2_compatibility = has_ext("GL_ARB_ES3_2_compatibility");
			func->GL_ARB_ES3_compatibility = has_ext("GL_ARB_ES3_compatibility");
			func->GL_ARB_arrays_of_arrays = has_ext("GL_ARB_arrays_of_arrays");
			func->GL_ARB_base_instance = has_ext("GL_ARB_base_instance");
			func->GL_ARB_bindless_texture = has_ext("GL_ARB_bindless_texture");
			func->GL_ARB_blend_func_extended = has_ext("GL_ARB_blend_func_extended");
			func->GL_ARB_buffer_storage = has_ext("GL_ARB_buffer_storage");
			func->GL_ARB_cl_event = has_ext("GL_ARB_cl_event");
			func->GL_ARB_clear_buffer_object = has_ext("GL_ARB_clear_buffer_object");
			func->GL_ARB_clear_texture = has_ext("GL_ARB_clear_texture");
			func->GL_ARB_clip_control = has_ext("GL_ARB_clip_control");
			func->GL_ARB_color_buffer_float = has_ext("GL_ARB_color_buffer_float");
			func->GL_ARB_compatibility = has_ext("GL_ARB_compatibility");
			func->GL_ARB_compressed_texture_pixel_storage = has_ext("GL_ARB_compressed_texture_pixel_storage");
			func->GL_ARB_compute_shader = has_ext("GL_ARB_compute_shader");
			func->GL_ARB_compute_variable_group_size = has_ext("GL_ARB_compute_variable_group_size");
			func->GL_ARB_conditional_render_inverted = has_ext("GL_ARB_conditional_render_inverted");
			func->GL_ARB_conservative_depth = has_ext("GL_ARB_conservative_depth");
			func->GL_ARB_copy_buffer = has_ext("GL_ARB_copy_buffer");
			func->GL_ARB_copy_image = has_ext("GL_ARB_copy_image");
			func->GL_ARB_cull_distance = has_ext("GL_ARB_cull_distance");
			func->GL_ARB_debug_output = has_ext("GL_ARB_debug_output");
			func->GL_ARB_depth_buffer_float = has_ext("GL_ARB_depth_buffer_float");
			func->GL_ARB_depth_clamp = has_ext("GL_ARB_depth_clamp");
			func->GL_ARB_depth_texture = has_ext("GL_ARB_depth_texture");
			func->GL_ARB_derivative_control = has_ext("GL_ARB_derivative_control");
			func->GL_ARB_direct_state_access = has_ext("GL_ARB_direct_state_access");
			func->GL_ARB_draw_buffers = has_ext("GL_ARB_draw_buffers");
			func->GL_ARB_draw_buffers_blend = has_ext("GL_ARB_draw_buffers_blend");
			func->GL_ARB_draw_elements_base_vertex = has_ext("GL_ARB_draw_elements_base_vertex");
			func->GL_ARB_draw_indirect = has_ext("GL_ARB_draw_indirect");
			func->GL_ARB_draw_instanced = has_ext("GL_ARB_draw_instanced");
			func->GL_ARB_enhanced_layouts = has_ext("GL_ARB_enhanced_layouts");
			func->GL_ARB_explicit_attrib_location = has_ext("GL_ARB_explicit_attrib_location");
			func->GL_ARB_explicit_uniform_location = has_ext("GL_ARB_explicit_uniform_location");
			func->GL_ARB_fragment_coord_conventions = has_ext("GL_ARB_fragment_coord_conventions");
			func->GL_ARB_fragment_layer_viewport = has_ext("GL_ARB_fragment_layer_viewport");
			func->GL_ARB_fragment_program = has_ext("GL_ARB_fragment_program");
			func->GL_ARB_fragment_program_shadow = has_ext("GL_ARB_fragment_program_shadow");
			func->GL_ARB_fragment_shader = has_ext("GL_ARB_fragment_shader");
			func->GL_ARB_fragment_shader_interlock = has_ext("GL_ARB_fragment_shader_interlock");
			func->GL_ARB_framebuffer_no_attachments = has_ext("GL_ARB_framebuffer_no_attachments");
			func->GL_ARB_framebuffer_object = has_ext("GL_ARB_framebuffer_object");
			func->GL_ARB_framebuffer_sRGB = has_ext("GL_ARB_framebuffer_sRGB");
			func->GL_ARB_geometry_shader4 = has_ext("GL_ARB_geometry_shader4");
			func->GL_ARB_get_program_binary = has_ext("GL_ARB_get_program_binary");
			func->GL_ARB_get_texture_sub_image = has_ext("GL_ARB_get_texture_sub_image");
			func->GL_ARB_gl_spirv = has_ext("GL_ARB_gl_spirv");
			func->GL_ARB_gpu_shader5 = has_ext("GL_ARB_gpu_shader5");
			func->GL_ARB_gpu_shader_fp64 = has_ext("GL_ARB_gpu_shader_fp64");
			func->GL_ARB_gpu_shader_int64 = has_ext("GL_ARB_gpu_shader_int64");
			func->GL_ARB_half_float_pixel = has_ext("GL_ARB_half_float_pixel");
			func->GL_ARB_half_float_vertex = has_ext("GL_ARB_half_float_vertex");
			func->GL_ARB_imaging = has_ext("GL_ARB_imaging");
			func->GL_ARB_indirect_parameters = has_ext("GL_ARB_indirect_parameters");
			func->GL_ARB_instanced_arrays = has_ext("GL_ARB_instanced_arrays");
			func->GL_ARB_internalformat_query = has_ext("GL_ARB_internalformat_query");
			func->GL_ARB_internalformat_query2 = has_ext("GL_ARB_internalformat_query2");
			func->GL_ARB_invalidate_subdata = has_ext("GL_ARB_invalidate_subdata");
			func->GL_ARB_map_buffer_alignment = has_ext("GL_ARB_map_buffer_alignment");
			func->GL_ARB_map_buffer_range = has_ext("GL_ARB_map_buffer_range");
			func->GL_ARB_matrix_palette = has_ext("GL_ARB_matrix_palette");
			func->GL_ARB_multi_bind = has_ext("GL_ARB_multi_bind");
			func->GL_ARB_multi_draw_indirect = has_ext("GL_ARB_multi_draw_indirect");
			func->GL_ARB_multisample = has_ext("GL_ARB_multisample");
			func->GL_ARB_multitexture = has_ext("GL_ARB_multitexture");
			func->GL_ARB_occlusion_query = has_ext("GL_ARB_occlusion_query");
			func->GL_ARB_occlusion_query2 = has_ext("GL_ARB_occlusion_query2");
			func->GL_ARB_parallel_shader_compile = has_ext("GL_ARB_parallel_shader_compile");
			func->GL_ARB_pipeline_statistics_query = has_ext("GL_ARB_pipeline_statistics_query");
			func->GL_ARB_pixel_buffer_object = has_ext("GL_ARB_pixel_buffer_object");
			func->GL_ARB_point_parameters = has_ext("GL_ARB_point_parameters");
			func->GL_ARB_point_sprite = has_ext("GL_ARB_point_sprite");
			func->GL_ARB_polygon_offset_clamp = has_ext("GL_ARB_polygon_offset_clamp");
			func->GL_ARB_post_depth_coverage = has_ext("GL_ARB_post_depth_coverage");
			func->GL_ARB_program_interface_query = has_ext("GL_ARB_program_interface_query");
			func->GL_ARB_provoking_vertex = has_ext("GL_ARB_provoking_vertex");
			func->GL_ARB_query_buffer_object = has_ext("GL_ARB_query_buffer_object");
			func->GL_ARB_robust_buffer_access_behavior = has_ext("GL_ARB_robust_buffer_access_behavior");
			func->GL_ARB_robustness = has_ext("GL_ARB_robustness");
			func->GL_ARB_robustness_isolation = has_ext("GL_ARB_robustness_isolation");
			func->GL_ARB_sample_locations = has_ext("GL_ARB_sample_locations");
			func->GL_ARB_sample_shading = has_ext("GL_ARB_sample_shading");
			func->GL_ARB_sampler_objects = has_ext("GL_ARB_sampler_objects");
			func->GL_ARB_seamless_cube_map = has_ext("GL_ARB_seamless_cube_map");
			func->GL_ARB_seamless_cubemap_per_texture = has_ext("GL_ARB_seamless_cubemap_per_texture");
			func->GL_ARB_separate_shader_objects = has_ext("GL_ARB_separate_shader_objects");
			func->GL_ARB_shader_atomic_counter_ops = has_ext("GL_ARB_shader_atomic_counter_ops");
			func->GL_ARB_shader_atomic_counters = has_ext("GL_ARB_shader_atomic_counters");
			func->GL_ARB_shader_ballot = has_ext("GL_ARB_shader_ballot");
			func->GL_ARB_shader_bit_encoding = has_ext("GL_ARB_shader_bit_encoding");
			func->GL_ARB_shader_clock = has_ext("GL_ARB_shader_clock");
			func->GL_ARB_shader_draw_parameters = has_ext("GL_ARB_shader_draw_parameters");
			func->GL_ARB_shader_group_vote = has_ext("GL_ARB_shader_group_vote");
			func->GL_ARB_shader_image_load_store = has_ext("GL_ARB_shader_image_load_store");
			func->GL_ARB_shader_image_size = has_ext("GL_ARB_shader_image_size");
			func->GL_ARB_shader_objects = has_ext("GL_ARB_shader_objects");
			func->GL_ARB_shader_precision = has_ext("GL_ARB_shader_precision");
			func->GL_ARB_shader_stencil_export = has_ext("GL_ARB_shader_stencil_export");
			func->GL_ARB_shader_storage_buffer_object = has_ext("GL_ARB_shader_storage_buffer_object");
			func->GL_ARB_shader_subroutine = has_ext("GL_ARB_shader_subroutine");
			func->GL_ARB_shader_texture_image_samples = has_ext("GL_ARB_shader_texture_image_samples");
			func->GL_ARB_shader_texture_lod = has_ext("GL_ARB_shader_texture_lod");
			func->GL_ARB_shader_viewport_layer_array = has_ext("GL_ARB_shader_viewport_layer_array");
			func->GL_ARB_shading_language_100 = has_ext("GL_ARB_shading_language_100");
			func->GL_ARB_shading_language_420pack = has_ext("GL_ARB_shading_language_420pack");
			func->GL_ARB_shading_language_include = has_ext("GL_ARB_shading_language_include");
			func->GL_ARB_shading_language_packing = has_ext("GL_ARB_shading_language_packing");
			func->GL_ARB_shadow = has_ext("GL_ARB_shadow");
			func->GL_ARB_shadow_ambient = has_ext("GL_ARB_shadow_ambient");
			func->GL_ARB_sparse_buffer = has_ext("GL_ARB_sparse_buffer");
			func->GL_ARB_sparse_texture = has_ext("GL_ARB_sparse_texture");
			func->GL_ARB_sparse_texture2 = has_ext("GL_ARB_sparse_texture2");
			func->GL_ARB_sparse_texture_clamp = has_ext("GL_ARB_sparse_texture_clamp");
			func->GL_ARB_spirv_extensions = has_ext("GL_ARB_spirv_extensions");
			func->GL_ARB_stencil_texturing = has_ext("GL_ARB_stencil_texturing");
			func->GL_ARB_sync = has_ext("GL_ARB_sync");
			func->GL_ARB_tessellation_shader = has_ext("GL_ARB_tessellation_shader");
			func->GL_ARB_texture_barrier = has_ext("GL_ARB_texture_barrier");
			func->GL_ARB_texture_border_clamp = has_ext("GL_ARB_texture_border_clamp");
			func->GL_ARB_texture_buffer_object = has_ext("GL_ARB_texture_buffer_object");
			func->GL_ARB_texture_buffer_object_rgb32 = has_ext("GL_ARB_texture_buffer_object_rgb32");
			func->GL_ARB_texture_buffer_range = has_ext("GL_ARB_texture_buffer_range");
			func->GL_ARB_texture_compression = has_ext("GL_ARB_texture_compression");
			func->GL_ARB_texture_compression_bptc = has_ext("GL_ARB_texture_compression_bptc");
			func->GL_ARB_texture_compression_rgtc = has_ext("GL_ARB_texture_compression_rgtc");
			func->GL_ARB_texture_cube_map = has_ext("GL_ARB_texture_cube_map");
			func->GL_ARB_texture_cube_map_array = has_ext("GL_ARB_texture_cube_map_array");
			func->GL_ARB_texture_env_add = has_ext("GL_ARB_texture_env_add");
			func->GL_ARB_texture_env_combine = has_ext("GL_ARB_texture_env_combine");
			func->GL_ARB_texture_env_crossbar = has_ext("GL_ARB_texture_env_crossbar");
			func->GL_ARB_texture_env_dot3 = has_ext("GL_ARB_texture_env_dot3");
			func->GL_ARB_texture_filter_anisotropic = has_ext("GL_ARB_texture_filter_anisotropic");
			func->GL_ARB_texture_filter_minmax = has_ext("GL_ARB_texture_filter_minmax");
			func->GL_ARB_texture_float = has_ext("GL_ARB_texture_float");
			func->GL_ARB_texture_gather = has_ext("GL_ARB_texture_gather");
			func->GL_ARB_texture_mirror_clamp_to_edge = has_ext("GL_ARB_texture_mirror_clamp_to_edge");
			func->GL_ARB_texture_mirrored_repeat = has_ext("GL_ARB_texture_mirrored_repeat");
			func->GL_ARB_texture_multisample = has_ext("GL_ARB_texture_multisample");
			func->GL_ARB_texture_non_power_of_two = has_ext("GL_ARB_texture_non_power_of_two");
			func->GL_ARB_texture_query_levels = has_ext("GL_ARB_texture_query_levels");
			func->GL_ARB_texture_query_lod = has_ext("GL_ARB_texture_query_lod");
			func->GL_ARB_texture_rectangle = has_ext("GL_ARB_texture_rectangle");
			func->GL_ARB_texture_rg = has_ext("GL_ARB_texture_rg");
			func->GL_ARB_texture_rgb10_a2ui = has_ext("GL_ARB_texture_rgb10_a2ui");
			func->GL_ARB_texture_stencil8 = has_ext("GL_ARB_texture_stencil8");
			func->GL_ARB_texture_storage = has_ext("GL_ARB_texture_storage");
			func->GL_ARB_texture_storage_multisample = has_ext("GL_ARB_texture_storage_multisample");
			func->GL_ARB_texture_swizzle = has_ext("GL_ARB_texture_swizzle");
			func->GL_ARB_texture_view = has_ext("GL_ARB_texture_view");
			func->GL_ARB_timer_query = has_ext("GL_ARB_timer_query");
			func->GL_ARB_transform_feedback2 = has_ext("GL_ARB_transform_feedback2");
			func->GL_ARB_transform_feedback3 = has_ext("GL_ARB_transform_feedback3");
			func->GL_ARB_transform_feedback_instanced = has_ext("GL_ARB_transform_feedback_instanced");
			func->GL_ARB_transform_feedback_overflow_query = has_ext("GL_ARB_transform_feedback_overflow_query");
			func->GL_ARB_transpose_matrix = has_ext("GL_ARB_transpose_matrix");
			func->GL_ARB_uniform_buffer_object = has_ext("GL_ARB_uniform_buffer_object");
			func->GL_ARB_vertex_array_bgra = has_ext("GL_ARB_vertex_array_bgra");
			func->GL_ARB_vertex_array_object = has_ext("GL_ARB_vertex_array_object");
			func->GL_ARB_vertex_attrib_64bit = has_ext("GL_ARB_vertex_attrib_64bit");
			func->GL_ARB_vertex_attrib_binding = has_ext("GL_ARB_vertex_attrib_binding");
			func->GL_ARB_vertex_blend = has_ext("GL_ARB_vertex_blend");
			func->GL_ARB_vertex_buffer_object = has_ext("GL_ARB_vertex_buffer_object");
			func->GL_ARB_vertex_program = has_ext("GL_ARB_vertex_program");
			func->GL_ARB_vertex_shader = has_ext("GL_ARB_vertex_shader");
			func->GL_ARB_vertex_type_10f_11f_11f_rev = has_ext("GL_ARB_vertex_type_10f_11f_11f_rev");
			func->GL_ARB_vertex_type_2_10_10_10_rev = has_ext("GL_ARB_vertex_type_2_10_10_10_rev");
			func->GL_ARB_viewport_array = has_ext("GL_ARB_viewport_array");
			func->GL_ARB_window_pos = has_ext("GL_ARB_window_pos");
			func->GL_ATI_draw_buffers = has_ext("GL_ATI_draw_buffers");
			func->GL_ATI_element_array = has_ext("GL_ATI_element_array");
			func->GL_ATI_envmap_bumpmap = has_ext("GL_ATI_envmap_bumpmap");
			func->GL_ATI_fragment_shader = has_ext("GL_ATI_fragment_shader");
			func->GL_ATI_map_object_buffer = has_ext("GL_ATI_map_object_buffer");
			func->GL_ATI_meminfo = has_ext("GL_ATI_meminfo");
			func->GL_ATI_pixel_format_float = has_ext("GL_ATI_pixel_format_float");
			func->GL_ATI_pn_triangles = has_ext("GL_ATI_pn_triangles");
			func->GL_ATI_separate_stencil = has_ext("GL_ATI_separate_stencil");
			func->GL_ATI_text_fragment_shader = has_ext("GL_ATI_text_fragment_shader");
			func->GL_ATI_texture_env_combine3 = has_ext("GL_ATI_texture_env_combine3");
			func->GL_ATI_texture_float = has_ext("GL_ATI_texture_float");
			func->GL_ATI_texture_mirror_once = has_ext("GL_ATI_texture_mirror_once");
			func->GL_ATI_vertex_array_object = has_ext("GL_ATI_vertex_array_object");
			func->GL_ATI_vertex_attrib_array_object = has_ext("GL_ATI_vertex_attrib_array_object");
			func->GL_ATI_vertex_streams = has_ext("GL_ATI_vertex_streams");
			func->GL_EXT_422_pixels = has_ext("GL_EXT_422_pixels");
			func->GL_EXT_EGL_image_storage = has_ext("GL_EXT_EGL_image_storage");
			func->GL_EXT_EGL_sync = has_ext("GL_EXT_EGL_sync");
			func->GL_EXT_abgr = has_ext("GL_EXT_abgr");
			func->GL_EXT_bgra = has_ext("GL_EXT_bgra");
			func->GL_EXT_bindable_uniform = has_ext("GL_EXT_bindable_uniform");
			func->GL_EXT_blend_color = has_ext("GL_EXT_blend_color");
			func->GL_EXT_blend_equation_separate = has_ext("GL_EXT_blend_equation_separate");
			func->GL_EXT_blend_func_separate = has_ext("GL_EXT_blend_func_separate");
			func->GL_EXT_blend_logic_op = has_ext("GL_EXT_blend_logic_op");
			func->GL_EXT_blend_minmax = has_ext("GL_EXT_blend_minmax");
			func->GL_EXT_blend_subtract = has_ext("GL_EXT_blend_subtract");
			func->GL_EXT_clip_volume_hint = has_ext("GL_EXT_clip_volume_hint");
			func->GL_EXT_cmyka = has_ext("GL_EXT_cmyka");
			func->GL_EXT_color_subtable = has_ext("GL_EXT_color_subtable");
			func->GL_EXT_compiled_vertex_array = has_ext("GL_EXT_compiled_vertex_array");
			func->GL_EXT_convolution = has_ext("GL_EXT_convolution");
			func->GL_EXT_coordinate_frame = has_ext("GL_EXT_coordinate_frame");
			func->GL_EXT_copy_texture = has_ext("GL_EXT_copy_texture");
			func->GL_EXT_cull_vertex = has_ext("GL_EXT_cull_vertex");
			func->GL_EXT_debug_label = has_ext("GL_EXT_debug_label");
			func->GL_EXT_debug_marker = has_ext("GL_EXT_debug_marker");
			func->GL_EXT_depth_bounds_test = has_ext("GL_EXT_depth_bounds_test");
			func->GL_EXT_direct_state_access = has_ext("GL_EXT_direct_state_access");
			func->GL_EXT_draw_buffers2 = has_ext("GL_EXT_draw_buffers2");
			func->GL_EXT_draw_instanced = has_ext("GL_EXT_draw_instanced");
			func->GL_EXT_draw_range_elements = has_ext("GL_EXT_draw_range_elements");
			func->GL_EXT_external_buffer = has_ext("GL_EXT_external_buffer");
			func->GL_EXT_fog_coord = has_ext("GL_EXT_fog_coord");
			func->GL_EXT_framebuffer_blit = has_ext("GL_EXT_framebuffer_blit");
			func->GL_EXT_framebuffer_blit_layers = has_ext("GL_EXT_framebuffer_blit_layers");
			func->GL_EXT_framebuffer_multisample = has_ext("GL_EXT_framebuffer_multisample");
			func->GL_EXT_framebuffer_multisample_blit_scaled = has_ext("GL_EXT_framebuffer_multisample_blit_scaled");
			func->GL_EXT_framebuffer_object = has_ext("GL_EXT_framebuffer_object");
			func->GL_EXT_framebuffer_sRGB = has_ext("GL_EXT_framebuffer_sRGB");
			func->GL_EXT_geometry_shader4 = has_ext("GL_EXT_geometry_shader4");
			func->GL_EXT_gpu_program_parameters = has_ext("GL_EXT_gpu_program_parameters");
			func->GL_EXT_gpu_shader4 = has_ext("GL_EXT_gpu_shader4");
			func->GL_EXT_histogram = has_ext("GL_EXT_histogram");
			func->GL_EXT_index_array_formats = has_ext("GL_EXT_index_array_formats");
			func->GL_EXT_index_func = has_ext("GL_EXT_index_func");
			func->GL_EXT_index_material = has_ext("GL_EXT_index_material");
			func->GL_EXT_index_texture = has_ext("GL_EXT_index_texture");
			func->GL_EXT_light_texture = has_ext("GL_EXT_light_texture");
			func->GL_EXT_memory_object = has_ext("GL_EXT_memory_object");
			func->GL_EXT_memory_object_fd = has_ext("GL_EXT_memory_object_fd");
			func->GL_EXT_memory_object_win32 = has_ext("GL_EXT_memory_object_win32");
			func->GL_EXT_misc_attribute = has_ext("GL_EXT_misc_attribute");
			func->GL_EXT_multi_draw_arrays = has_ext("GL_EXT_multi_draw_arrays");
			func->GL_EXT_multisample = has_ext("GL_EXT_multisample");
			func->GL_EXT_multiview_tessellation_geometry_shader = has_ext("GL_EXT_multiview_tessellation_geometry_shader");
			func->GL_EXT_multiview_texture_multisample = has_ext("GL_EXT_multiview_texture_multisample");
			func->GL_EXT_multiview_timer_query = has_ext("GL_EXT_multiview_timer_query");
			func->GL_EXT_packed_depth_stencil = has_ext("GL_EXT_packed_depth_stencil");
			func->GL_EXT_packed_float = has_ext("GL_EXT_packed_float");
			func->GL_EXT_packed_pixels = has_ext("GL_EXT_packed_pixels");
			func->GL_EXT_paletted_texture = has_ext("GL_EXT_paletted_texture");
			func->GL_EXT_pixel_buffer_object = has_ext("GL_EXT_pixel_buffer_object");
			func->GL_EXT_pixel_transform = has_ext("GL_EXT_pixel_transform");
			func->GL_EXT_pixel_transform_color_table = has_ext("GL_EXT_pixel_transform_color_table");
			func->GL_EXT_point_parameters = has_ext("GL_EXT_point_parameters");
			func->GL_EXT_polygon_offset = has_ext("GL_EXT_polygon_offset");
			func->GL_EXT_polygon_offset_clamp = has_ext("GL_EXT_polygon_offset_clamp");
			func->GL_EXT_post_depth_coverage = has_ext("GL_EXT_post_depth_coverage");
			func->GL_EXT_provoking_vertex = has_ext("GL_EXT_provoking_vertex");
			func->GL_EXT_raster_multisample = has_ext("GL_EXT_raster_multisample");
			func->GL_EXT_rescale_normal = has_ext("GL_EXT_rescale_normal");
			func->GL_EXT_secondary_color = has_ext("GL_EXT_secondary_color");
			func->GL_EXT_semaphore = has_ext("GL_EXT_semaphore");
			func->GL_EXT_semaphore_fd = has_ext("GL_EXT_semaphore_fd");
			func->GL_EXT_semaphore_win32 = has_ext("GL_EXT_semaphore_win32");
			func->GL_EXT_separate_shader_objects = has_ext("GL_EXT_separate_shader_objects");
			func->GL_EXT_separate_specular_color = has_ext("GL_EXT_separate_specular_color");
			func->GL_EXT_shader_framebuffer_fetch = has_ext("GL_EXT_shader_framebuffer_fetch");
			func->GL_EXT_shader_framebuffer_fetch_non_coherent = has_ext("GL_EXT_shader_framebuffer_fetch_non_coherent");
			func->GL_EXT_shader_image_load_formatted = has_ext("GL_EXT_shader_image_load_formatted");
			func->GL_EXT_shader_image_load_store = has_ext("GL_EXT_shader_image_load_store");
			func->GL_EXT_shader_integer_mix = has_ext("GL_EXT_shader_integer_mix");
			func->GL_EXT_shader_samples_identical = has_ext("GL_EXT_shader_samples_identical");
			func->GL_EXT_shadow_funcs = has_ext("GL_EXT_shadow_funcs");
			func->GL_EXT_shared_texture_palette = has_ext("GL_EXT_shared_texture_palette");
			func->GL_EXT_sparse_texture2 = has_ext("GL_EXT_sparse_texture2");
			func->GL_EXT_stencil_clear_tag = has_ext("GL_EXT_stencil_clear_tag");
			func->GL_EXT_stencil_two_side = has_ext("GL_EXT_stencil_two_side");
			func->GL_EXT_stencil_wrap = has_ext("GL_EXT_stencil_wrap");
			func->GL_EXT_subtexture = has_ext("GL_EXT_subtexture");
			func->GL_EXT_texture = has_ext("GL_EXT_texture");
			func->GL_EXT_texture3D = has_ext("GL_EXT_texture3D");
			func->GL_EXT_texture_array = has_ext("GL_EXT_texture_array");
			func->GL_EXT_texture_buffer_object = has_ext("GL_EXT_texture_buffer_object");
			func->GL_EXT_texture_compression_latc = has_ext("GL_EXT_texture_compression_latc");
			func->GL_EXT_texture_compression_rgtc = has_ext("GL_EXT_texture_compression_rgtc");
			func->GL_EXT_texture_compression_s3tc = has_ext("GL_EXT_texture_compression_s3tc");
			func->GL_EXT_texture_cube_map = has_ext("GL_EXT_texture_cube_map");
			func->GL_EXT_texture_env_add = has_ext("GL_EXT_texture_env_add");
			func->GL_EXT_texture_env_combine = has_ext("GL_EXT_texture_env_combine");
			func->GL_EXT_texture_env_dot3 = has_ext("GL_EXT_texture_env_dot3");
			func->GL_EXT_texture_filter_anisotropic = has_ext("GL_EXT_texture_filter_anisotropic");
			func->GL_EXT_texture_filter_minmax = has_ext("GL_EXT_texture_filter_minmax");
			func->GL_EXT_texture_integer = has_ext("GL_EXT_texture_integer");
			func->GL_EXT_texture_lod_bias = has_ext("GL_EXT_texture_lod_bias");
			func->GL_EXT_texture_mirror_clamp = has_ext("GL_EXT_texture_mirror_clamp");
			func->GL_EXT_texture_object = has_ext("GL_EXT_texture_object");
			func->GL_EXT_texture_perturb_normal = has_ext("GL_EXT_texture_perturb_normal");
			func->GL_EXT_texture_sRGB = has_ext("GL_EXT_texture_sRGB");
			func->GL_EXT_texture_sRGB_R8 = has_ext("GL_EXT_texture_sRGB_R8");
			func->GL_EXT_texture_sRGB_RG8 = has_ext("GL_EXT_texture_sRGB_RG8");
			func->GL_EXT_texture_sRGB_decode = has_ext("GL_EXT_texture_sRGB_decode");
			func->GL_EXT_texture_shadow_lod = has_ext("GL_EXT_texture_shadow_lod");
			func->GL_EXT_texture_shared_exponent = has_ext("GL_EXT_texture_shared_exponent");
			func->GL_EXT_texture_snorm = has_ext("GL_EXT_texture_snorm");
			func->GL_EXT_texture_storage = has_ext("GL_EXT_texture_storage");
			func->GL_EXT_texture_swizzle = has_ext("GL_EXT_texture_swizzle");
			func->GL_EXT_timer_query = has_ext("GL_EXT_timer_query");
			func->GL_EXT_transform_feedback = has_ext("GL_EXT_transform_feedback");
			func->GL_EXT_vertex_array = has_ext("GL_EXT_vertex_array");
			func->GL_EXT_vertex_array_bgra = has_ext("GL_EXT_vertex_array_bgra");
			func->GL_EXT_vertex_attrib_64bit = has_ext("GL_EXT_vertex_attrib_64bit");
			func->GL_EXT_vertex_shader = has_ext("GL_EXT_vertex_shader");
			func->GL_EXT_vertex_weighting = has_ext("GL_EXT_vertex_weighting");
			func->GL_EXT_win32_keyed_mutex = has_ext("GL_EXT_win32_keyed_mutex");
			func->GL_EXT_window_rectangles = has_ext("GL_EXT_window_rectangles");
			func->GL_EXT_x11_sync_object = has_ext("GL_EXT_x11_sync_object");
			func->GL_GREMEDY_frame_terminator = has_ext("GL_GREMEDY_frame_terminator");
			func->GL_GREMEDY_string_marker = has_ext("GL_GREMEDY_string_marker");
			func->GL_HP_convolution_border_modes = has_ext("GL_HP_convolution_border_modes");
			func->GL_HP_image_transform = has_ext("GL_HP_image_transform");
			func->GL_HP_occlusion_test = has_ext("GL_HP_occlusion_test");
			func->GL_HP_texture_lighting = has_ext("GL_HP_texture_lighting");
			func->GL_IBM_cull_vertex = has_ext("GL_IBM_cull_vertex");
			func->GL_IBM_multimode_draw_arrays = has_ext("GL_IBM_multimode_draw_arrays");
			func->GL_IBM_rasterpos_clip = has_ext("GL_IBM_rasterpos_clip");
			func->GL_IBM_static_data = has_ext("GL_IBM_static_data");
			func->GL_IBM_texture_mirrored_repeat = has_ext("GL_IBM_texture_mirrored_repeat");
			func->GL_IBM_vertex_array_lists = has_ext("GL_IBM_vertex_array_lists");
			func->GL_INGR_blend_func_separate = has_ext("GL_INGR_blend_func_separate");
			func->GL_INGR_color_clamp = has_ext("GL_INGR_color_clamp");
			func->GL_INGR_interlace_read = has_ext("GL_INGR_interlace_read");
			func->GL_INTEL_blackhole_render = has_ext("GL_INTEL_blackhole_render");
			func->GL_INTEL_conservative_rasterization = has_ext("GL_INTEL_conservative_rasterization");
			func->GL_INTEL_fragment_shader_ordering = has_ext("GL_INTEL_fragment_shader_ordering");
			func->GL_INTEL_framebuffer_CMAA = has_ext("GL_INTEL_framebuffer_CMAA");
			func->GL_INTEL_map_texture = has_ext("GL_INTEL_map_texture");
			func->GL_INTEL_parallel_arrays = has_ext("GL_INTEL_parallel_arrays");
			func->GL_INTEL_performance_query = has_ext("GL_INTEL_performance_query");
			func->GL_KHR_blend_equation_advanced = has_ext("GL_KHR_blend_equation_advanced");
			func->GL_KHR_blend_equation_advanced_coherent = has_ext("GL_KHR_blend_equation_advanced_coherent");
			func->GL_KHR_context_flush_control = has_ext("GL_KHR_context_flush_control");
			func->GL_KHR_debug = has_ext("GL_KHR_debug");
			func->GL_KHR_no_error = has_ext("GL_KHR_no_error");
			func->GL_KHR_parallel_shader_compile = has_ext("GL_KHR_parallel_shader_compile");
			func->GL_KHR_robust_buffer_access_behavior = has_ext("GL_KHR_robust_buffer_access_behavior");
			func->GL_KHR_robustness = has_ext("GL_KHR_robustness");
			func->GL_KHR_shader_subgroup = has_ext("GL_KHR_shader_subgroup");
			func->GL_KHR_texture_compression_astc_hdr = has_ext("GL_KHR_texture_compression_astc_hdr");
			func->GL_KHR_texture_compression_astc_ldr = has_ext("GL_KHR_texture_compression_astc_ldr");
			func->GL_KHR_texture_compression_astc_sliced_3d = has_ext("GL_KHR_texture_compression_astc_sliced_3d");
			func->GL_MESAX_texture_stack = has_ext("GL_MESAX_texture_stack");
			func->GL_MESA_framebuffer_flip_x = has_ext("GL_MESA_framebuffer_flip_x");
			func->GL_MESA_framebuffer_flip_y = has_ext("GL_MESA_framebuffer_flip_y");
			func->GL_MESA_framebuffer_swap_xy = has_ext("GL_MESA_framebuffer_swap_xy");
			func->GL_MESA_pack_invert = has_ext("GL_MESA_pack_invert");
			func->GL_MESA_program_binary_formats = has_ext("GL_MESA_program_binary_formats");
			func->GL_MESA_resize_buffers = has_ext("GL_MESA_resize_buffers");
			func->GL_MESA_shader_integer_functions = has_ext("GL_MESA_shader_integer_functions");
			func->GL_MESA_tile_raster_order = has_ext("GL_MESA_tile_raster_order");
			func->GL_MESA_window_pos = has_ext("GL_MESA_window_pos");
			func->GL_MESA_ycbcr_texture = has_ext("GL_MESA_ycbcr_texture");
			func->GL_NVX_blend_equation_advanced_multi_draw_buffers = has_ext("GL_NVX_blend_equation_advanced_multi_draw_buffers");
			func->GL_NVX_conditional_render = has_ext("GL_NVX_conditional_render");
			func->GL_NVX_gpu_memory_info = has_ext("GL_NVX_gpu_memory_info");
			func->GL_NVX_gpu_multicast2 = has_ext("GL_NVX_gpu_multicast2");
			func->GL_NVX_linked_gpu_multicast = has_ext("GL_NVX_linked_gpu_multicast");
			func->GL_NVX_progress_fence = has_ext("GL_NVX_progress_fence");
			func->GL_NV_alpha_to_coverage_dither_control = has_ext("GL_NV_alpha_to_coverage_dither_control");
			func->GL_NV_bindless_multi_draw_indirect = has_ext("GL_NV_bindless_multi_draw_indirect");
			func->GL_NV_bindless_multi_draw_indirect_count = has_ext("GL_NV_bindless_multi_draw_indirect_count");
			func->GL_NV_bindless_texture = has_ext("GL_NV_bindless_texture");
			func->GL_NV_blend_equation_advanced = has_ext("GL_NV_blend_equation_advanced");
			func->GL_NV_blend_equation_advanced_coherent = has_ext("GL_NV_blend_equation_advanced_coherent");
			func->GL_NV_blend_minmax_factor = has_ext("GL_NV_blend_minmax_factor");
			func->GL_NV_blend_square = has_ext("GL_NV_blend_square");
			func->GL_NV_clip_space_w_scaling = has_ext("GL_NV_clip_space_w_scaling");
			func->GL_NV_command_list = has_ext("GL_NV_command_list");
			func->GL_NV_compute_program5 = has_ext("GL_NV_compute_program5");
			func->GL_NV_compute_shader_derivatives = has_ext("GL_NV_compute_shader_derivatives");
			func->GL_NV_conditional_render = has_ext("GL_NV_conditional_render");
			func->GL_NV_conservative_raster = has_ext("GL_NV_conservative_raster");
			func->GL_NV_conservative_raster_dilate = has_ext("GL_NV_conservative_raster_dilate");
			func->GL_NV_conservative_raster_pre_snap = has_ext("GL_NV_conservative_raster_pre_snap");
			func->GL_NV_conservative_raster_pre_snap_triangles = has_ext("GL_NV_conservative_raster_pre_snap_triangles");
			func->GL_NV_conservative_raster_underestimation = has_ext("GL_NV_conservative_raster_underestimation");
			func->GL_NV_copy_depth_to_color = has_ext("GL_NV_copy_depth_to_color");
			func->GL_NV_copy_image = has_ext("GL_NV_copy_image");
			func->GL_NV_deep_texture3D = has_ext("GL_NV_deep_texture3D");
			func->GL_NV_depth_buffer_float = has_ext("GL_NV_depth_buffer_float");
			func->GL_NV_depth_clamp = has_ext("GL_NV_depth_clamp");
			func->GL_NV_draw_texture = has_ext("GL_NV_draw_texture");
			func->GL_NV_draw_vulkan_image = has_ext("GL_NV_draw_vulkan_image");
			func->GL_NV_evaluators = has_ext("GL_NV_evaluators");
			func->GL_NV_explicit_multisample = has_ext("GL_NV_explicit_multisample");
			func->GL_NV_fence = has_ext("GL_NV_fence");
			func->GL_NV_fill_rectangle = has_ext("GL_NV_fill_rectangle");
			func->GL_NV_float_buffer = has_ext("GL_NV_float_buffer");
			func->GL_NV_fog_distance = has_ext("GL_NV_fog_distance");
			func->GL_NV_fragment_coverage_to_color = has_ext("GL_NV_fragment_coverage_to_color");
			func->GL_NV_fragment_program = has_ext("GL_NV_fragment_program");
			func->GL_NV_fragment_program2 = has_ext("GL_NV_fragment_program2");
			func->GL_NV_fragment_program4 = has_ext("GL_NV_fragment_program4");
			func->GL_NV_fragment_program_option = has_ext("GL_NV_fragment_program_option");
			func->GL_NV_fragment_shader_barycentric = has_ext("GL_NV_fragment_shader_barycentric");
			func->GL_NV_fragment_shader_interlock = has_ext("GL_NV_fragment_shader_interlock");
			func->GL_NV_framebuffer_mixed_samples = has_ext("GL_NV_framebuffer_mixed_samples");
			func->GL_NV_framebuffer_multisample_coverage = has_ext("GL_NV_framebuffer_multisample_coverage");
			func->GL_NV_geometry_program4 = has_ext("GL_NV_geometry_program4");
			func->GL_NV_geometry_shader4 = has_ext("GL_NV_geometry_shader4");
			func->GL_NV_geometry_shader_passthrough = has_ext("GL_NV_geometry_shader_passthrough");
			func->GL_NV_gpu_multicast = has_ext("GL_NV_gpu_multicast");
			func->GL_NV_gpu_program4 = has_ext("GL_NV_gpu_program4");
			func->GL_NV_gpu_program5 = has_ext("GL_NV_gpu_program5");
			func->GL_NV_gpu_program5_mem_extended = has_ext("GL_NV_gpu_program5_mem_extended");
			func->GL_NV_gpu_shader5 = has_ext("GL_NV_gpu_shader5");
			func->GL_NV_half_float = has_ext("GL_NV_half_float");
			func->GL_NV_internalformat_sample_query = has_ext("GL_NV_internalformat_sample_query");
			func->GL_NV_light_max_exponent = has_ext("GL_NV_light_max_exponent");
			func->GL_NV_memory_attachment = has_ext("GL_NV_memory_attachment");
			func->GL_NV_memory_object_sparse = has_ext("GL_NV_memory_object_sparse");
			func->GL_NV_mesh_shader = has_ext("GL_NV_mesh_shader");
			func->GL_NV_multisample_coverage = has_ext("GL_NV_multisample_coverage");
			func->GL_NV_multisample_filter_hint = has_ext("GL_NV_multisample_filter_hint");
			func->GL_NV_occlusion_query = has_ext("GL_NV_occlusion_query");
			func->GL_NV_packed_depth_stencil = has_ext("GL_NV_packed_depth_stencil");
			func->GL_NV_parameter_buffer_object = has_ext("GL_NV_parameter_buffer_object");
			func->GL_NV_parameter_buffer_object2 = has_ext("GL_NV_parameter_buffer_object2");
			func->GL_NV_path_rendering = has_ext("GL_NV_path_rendering");
			func->GL_NV_path_rendering_shared_edge = has_ext("GL_NV_path_rendering_shared_edge");
			func->GL_NV_pixel_data_range = has_ext("GL_NV_pixel_data_range");
			func->GL_NV_point_sprite = has_ext("GL_NV_point_sprite");
			func->GL_NV_present_video = has_ext("GL_NV_present_video");
			func->GL_NV_primitive_restart = has_ext("GL_NV_primitive_restart");
			func->GL_NV_primitive_shading_rate = has_ext("GL_NV_primitive_shading_rate");
			func->GL_NV_query_resource = has_ext("GL_NV_query_resource");
			func->GL_NV_query_resource_tag = has_ext("GL_NV_query_resource_tag");
			func->GL_NV_register_combiners = has_ext("GL_NV_register_combiners");
			func->GL_NV_register_combiners2 = has_ext("GL_NV_register_combiners2");
			func->GL_NV_representative_fragment_test = has_ext("GL_NV_representative_fragment_test");
			func->GL_NV_robustness_video_memory_purge = has_ext("GL_NV_robustness_video_memory_purge");
			func->GL_NV_sample_locations = has_ext("GL_NV_sample_locations");
			func->GL_NV_sample_mask_override_coverage = has_ext("GL_NV_sample_mask_override_coverage");
			func->GL_NV_scissor_exclusive = has_ext("GL_NV_scissor_exclusive");
			func->GL_NV_shader_atomic_counters = has_ext("GL_NV_shader_atomic_counters");
			func->GL_NV_shader_atomic_float = has_ext("GL_NV_shader_atomic_float");
			func->GL_NV_shader_atomic_float64 = has_ext("GL_NV_shader_atomic_float64");
			func->GL_NV_shader_atomic_fp16_vector = has_ext("GL_NV_shader_atomic_fp16_vector");
			func->GL_NV_shader_atomic_int64 = has_ext("GL_NV_shader_atomic_int64");
			func->GL_NV_shader_buffer_load = has_ext("GL_NV_shader_buffer_load");
			func->GL_NV_shader_buffer_store = has_ext("GL_NV_shader_buffer_store");
			func->GL_NV_shader_storage_buffer_object = has_ext("GL_NV_shader_storage_buffer_object");
			func->GL_NV_shader_subgroup_partitioned = has_ext("GL_NV_shader_subgroup_partitioned");
			func->GL_NV_shader_texture_footprint = has_ext("GL_NV_shader_texture_footprint");
			func->GL_NV_shader_thread_group = has_ext("GL_NV_shader_thread_group");
			func->GL_NV_shader_thread_shuffle = has_ext("GL_NV_shader_thread_shuffle");
			func->GL_NV_shading_rate_image = has_ext("GL_NV_shading_rate_image");
			func->GL_NV_stereo_view_rendering = has_ext("GL_NV_stereo_view_rendering");
			func->GL_NV_tessellation_program5 = has_ext("GL_NV_tessellation_program5");
			func->GL_NV_texgen_emboss = has_ext("GL_NV_texgen_emboss");
			func->GL_NV_texgen_reflection = has_ext("GL_NV_texgen_reflection");
			func->GL_NV_texture_barrier = has_ext("GL_NV_texture_barrier");
			func->GL_NV_texture_compression_vtc = has_ext("GL_NV_texture_compression_vtc");
			func->GL_NV_texture_env_combine4 = has_ext("GL_NV_texture_env_combine4");
			func->GL_NV_texture_expand_normal = has_ext("GL_NV_texture_expand_normal");
			func->GL_NV_texture_multisample = has_ext("GL_NV_texture_multisample");
			func->GL_NV_texture_rectangle = has_ext("GL_NV_texture_rectangle");
			func->GL_NV_texture_rectangle_compressed = has_ext("GL_NV_texture_rectangle_compressed");
			func->GL_NV_texture_shader = has_ext("GL_NV_texture_shader");
			func->GL_NV_texture_shader2 = has_ext("GL_NV_texture_shader2");
			func->GL_NV_texture_shader3 = has_ext("GL_NV_texture_shader3");
			func->GL_NV_timeline_semaphore = has_ext("GL_NV_timeline_semaphore");
			func->GL_NV_transform_feedback = has_ext("GL_NV_transform_feedback");
			func->GL_NV_transform_feedback2 = has_ext("GL_NV_transform_feedback2");
			func->GL_NV_uniform_buffer_std430_layout = has_ext("GL_NV_uniform_buffer_std430_layout");
			func->GL_NV_uniform_buffer_unified_memory = has_ext("GL_NV_uniform_buffer_unified_memory");
			func->GL_NV_vdpau_interop = has_ext("GL_NV_vdpau_interop");
			func->GL_NV_vdpau_interop2 = has_ext("GL_NV_vdpau_interop2");
			func->GL_NV_vertex_array_range = has_ext("GL_NV_vertex_array_range");
			func->GL_NV_vertex_array_range2 = has_ext("GL_NV_vertex_array_range2");
			func->GL_NV_vertex_attrib_integer_64bit = has_ext("GL_NV_vertex_attrib_integer_64bit");
			func->GL_NV_vertex_buffer_unified_memory = has_ext("GL_NV_vertex_buffer_unified_memory");
			func->GL_NV_vertex_program = has_ext("GL_NV_vertex_program");
			func->GL_NV_vertex_program1_1 = has_ext("GL_NV_vertex_program1_1");
			func->GL_NV_vertex_program2 = has_ext("GL_NV_vertex_program2");
			func->GL_NV_vertex_program2_option = has_ext("GL_NV_vertex_program2_option");
			func->GL_NV_vertex_program3 = has_ext("GL_NV_vertex_program3");
			func->GL_NV_vertex_program4 = has_ext("GL_NV_vertex_program4");
			func->GL_NV_video_capture = has_ext("GL_NV_video_capture");
			func->GL_NV_viewport_array2 = has_ext("GL_NV_viewport_array2");
			func->GL_NV_viewport_swizzle = has_ext("GL_NV_viewport_swizzle");
			func->GL_OES_byte_coordinates = has_ext("GL_OES_byte_coordinates");
			func->GL_OES_compressed_paletted_texture = has_ext("GL_OES_compressed_paletted_texture");
			func->GL_OES_fixed_point = has_ext("GL_OES_fixed_point");
			func->GL_OES_query_matrix = has_ext("GL_OES_query_matrix");
			func->GL_OES_read_format = has_ext("GL_OES_read_format");
			func->GL_OES_single_precision = has_ext("GL_OES_single_precision");
			func->GL_OML_interlace = has_ext("GL_OML_interlace");
			func->GL_OML_resample = has_ext("GL_OML_resample");
			func->GL_OML_subsample = has_ext("GL_OML_subsample");
			func->GL_OVR_multiview = has_ext("GL_OVR_multiview");
			func->GL_OVR_multiview2 = has_ext("GL_OVR_multiview2");
			func->GL_PGI_misc_hints = has_ext("GL_PGI_misc_hints");
			func->GL_PGI_vertex_hints = has_ext("GL_PGI_vertex_hints");
			func->GL_REND_screen_coordinates = has_ext("GL_REND_screen_coordinates");
			func->GL_S3_s3tc = has_ext("GL_S3_s3tc");
			func->GL_SGIS_detail_texture = has_ext("GL_SGIS_detail_texture");
			func->GL_SGIS_fog_function = has_ext("GL_SGIS_fog_function");
			func->GL_SGIS_generate_mipmap = has_ext("GL_SGIS_generate_mipmap");
			func->GL_SGIS_multisample = has_ext("GL_SGIS_multisample");
			func->GL_SGIS_pixel_texture = has_ext("GL_SGIS_pixel_texture");
			func->GL_SGIS_point_line_texgen = has_ext("GL_SGIS_point_line_texgen");
			func->GL_SGIS_point_parameters = has_ext("GL_SGIS_point_parameters");
			func->GL_SGIS_sharpen_texture = has_ext("GL_SGIS_sharpen_texture");
			func->GL_SGIS_texture4D = has_ext("GL_SGIS_texture4D");
			func->GL_SGIS_texture_border_clamp = has_ext("GL_SGIS_texture_border_clamp");
			func->GL_SGIS_texture_color_mask = has_ext("GL_SGIS_texture_color_mask");
			func->GL_SGIS_texture_edge_clamp = has_ext("GL_SGIS_texture_edge_clamp");
			func->GL_SGIS_texture_filter4 = has_ext("GL_SGIS_texture_filter4");
			func->GL_SGIS_texture_lod = has_ext("GL_SGIS_texture_lod");
			func->GL_SGIS_texture_select = has_ext("GL_SGIS_texture_select");
			func->GL_SGIX_async = has_ext("GL_SGIX_async");
			func->GL_SGIX_async_histogram = has_ext("GL_SGIX_async_histogram");
			func->GL_SGIX_async_pixel = has_ext("GL_SGIX_async_pixel");
			func->GL_SGIX_blend_alpha_minmax = has_ext("GL_SGIX_blend_alpha_minmax");
			func->GL_SGIX_calligraphic_fragment = has_ext("GL_SGIX_calligraphic_fragment");
			func->GL_SGIX_clipmap = has_ext("GL_SGIX_clipmap");
			func->GL_SGIX_convolution_accuracy = has_ext("GL_SGIX_convolution_accuracy");
			func->GL_SGIX_depth_pass_instrument = has_ext("GL_SGIX_depth_pass_instrument");
			func->GL_SGIX_depth_texture = has_ext("GL_SGIX_depth_texture");
			func->GL_SGIX_flush_raster = has_ext("GL_SGIX_flush_raster");
			func->GL_SGIX_fog_offset = has_ext("GL_SGIX_fog_offset");
			func->GL_SGIX_fragment_lighting = has_ext("GL_SGIX_fragment_lighting");
			func->GL_SGIX_framezoom = has_ext("GL_SGIX_framezoom");
			func->GL_SGIX_igloo_interface = has_ext("GL_SGIX_igloo_interface");
			func->GL_SGIX_instruments = has_ext("GL_SGIX_instruments");
			func->GL_SGIX_interlace = has_ext("GL_SGIX_interlace");
			func->GL_SGIX_ir_instrument1 = has_ext("GL_SGIX_ir_instrument1");
			func->GL_SGIX_list_priority = has_ext("GL_SGIX_list_priority");
			func->GL_SGIX_pixel_texture = has_ext("GL_SGIX_pixel_texture");
			func->GL_SGIX_pixel_tiles = has_ext("GL_SGIX_pixel_tiles");
			func->GL_SGIX_polynomial_ffd = has_ext("GL_SGIX_polynomial_ffd");
			func->GL_SGIX_reference_plane = has_ext("GL_SGIX_reference_plane");
			func->GL_SGIX_resample = has_ext("GL_SGIX_resample");
			func->GL_SGIX_scalebias_hint = has_ext("GL_SGIX_scalebias_hint");
			func->GL_SGIX_shadow = has_ext("GL_SGIX_shadow");
			func->GL_SGIX_shadow_ambient = has_ext("GL_SGIX_shadow_ambient");
			func->GL_SGIX_sprite = has_ext("GL_SGIX_sprite");
			func->GL_SGIX_subsample = has_ext("GL_SGIX_subsample");
			func->GL_SGIX_tag_sample_buffer = has_ext("GL_SGIX_tag_sample_buffer");
			func->GL_SGIX_texture_add_env = has_ext("GL_SGIX_texture_add_env");
			func->GL_SGIX_texture_coordinate_clamp = has_ext("GL_SGIX_texture_coordinate_clamp");
			func->GL_SGIX_texture_lod_bias = has_ext("GL_SGIX_texture_lod_bias");
			func->GL_SGIX_texture_multi_buffer = has_ext("GL_SGIX_texture_multi_buffer");
			func->GL_SGIX_texture_scale_bias = has_ext("GL_SGIX_texture_scale_bias");
			func->GL_SGIX_vertex_preclip = has_ext("GL_SGIX_vertex_preclip");
			func->GL_SGIX_ycrcb = has_ext("GL_SGIX_ycrcb");
			func->GL_SGIX_ycrcb_subsample = has_ext("GL_SGIX_ycrcb_subsample");
			func->GL_SGIX_ycrcba = has_ext("GL_SGIX_ycrcba");
			func->GL_SGI_color_matrix = has_ext("GL_SGI_color_matrix");
			func->GL_SGI_color_table = has_ext("GL_SGI_color_table");
			func->GL_SGI_texture_color_table = has_ext("GL_SGI_texture_color_table");
			func->GL_SUNX_constant_data = has_ext("GL_SUNX_constant_data");
			func->GL_SUN_convolution_border_modes = has_ext("GL_SUN_convolution_border_modes");
			func->GL_SUN_global_alpha = has_ext("GL_SUN_global_alpha");
			func->GL_SUN_mesh_array = has_ext("GL_SUN_mesh_array");
			func->GL_SUN_slice_accum = has_ext("GL_SUN_slice_accum");
			func->GL_SUN_triangle_list = has_ext("GL_SUN_triangle_list");
			func->GL_SUN_vertex = has_ext("GL_SUN_vertex");
			func->GL_WIN_phong_shading = has_ext("GL_WIN_phong_shading");
			func->GL_WIN_specular_fog = has_ext("GL_WIN_specular_fog");*/
#undef LOAD_GL_EXT_FUNC

            return ext_func;
		}

		return nullptr;
	}


	void load_gl_functions(void** func, void** ext_func)
    {
		int major, minor;
		if (open_gl(&major, &minor))
		{
			*func = load_GL_funcs(major, minor);
			*ext_func = load_GL_EXT_funcs((GLFunctions*)(*func), major);
		}

		close_gl();
    }

	void load_gl_es_functions(void** func, void** ext_func)
	{
		int major, minor;
		if (open_gl_es(&major, &minor))
		{
			*func = load_GL_ES_funcs(major, minor);
			*ext_func = load_GL_ES_EXT_funcs((GLFunctions*)*func, major);
		}

		close_gl();
	}

}