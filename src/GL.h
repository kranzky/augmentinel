#pragma once

// OpenGL 3.3 core access for the renderer.
//
// macOS exports the whole core profile from its framework. Elsewhere the system
// library only guarantees OpenGL 1.1, so the newer entry points the renderer uses
// are loaded through SDL once a context exists (replacing GLEW).

#ifdef __APPLE__
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
#include <OpenGL/gl3.h>
#else
// Defining APIENTRY stops SDL_opengl.h pulling in <windows.h> and its macros.
#if defined(_WIN32) && !defined(APIENTRY)
#define APIENTRY __stdcall
#endif
#include <SDL_opengl.h>

// Entry points beyond OpenGL 1.1, as X(type, name).
#define AUGMENTINEL_GL_FUNCTIONS(X) \
    X(PFNGLACTIVETEXTUREPROC, glActiveTexture) \
    X(PFNGLATTACHSHADERPROC, glAttachShader) \
    X(PFNGLBINDBUFFERPROC, glBindBuffer) \
    X(PFNGLBINDBUFFERBASEPROC, glBindBufferBase) \
    X(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer) \
    X(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer) \
    X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray) \
    X(PFNGLBLITFRAMEBUFFERPROC, glBlitFramebuffer) \
    X(PFNGLBUFFERDATAPROC, glBufferData) \
    X(PFNGLBUFFERSUBDATAPROC, glBufferSubData) \
    X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus) \
    X(PFNGLCOMPILESHADERPROC, glCompileShader) \
    X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
    X(PFNGLCREATESHADERPROC, glCreateShader) \
    X(PFNGLDELETEBUFFERSPROC, glDeleteBuffers) \
    X(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers) \
    X(PFNGLDELETEPROGRAMPROC, glDeleteProgram) \
    X(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers) \
    X(PFNGLDELETESHADERPROC, glDeleteShader) \
    X(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays) \
    X(PFNGLDETACHSHADERPROC, glDetachShader) \
    X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
    X(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer) \
    X(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D) \
    X(PFNGLGENBUFFERSPROC, glGenBuffers) \
    X(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers) \
    X(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers) \
    X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays) \
    X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
    X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
    X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
    X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
    X(PFNGLGETUNIFORMBLOCKINDEXPROC, glGetUniformBlockIndex) \
    X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation) \
    X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
    X(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC, glRenderbufferStorageMultisample) \
    X(PFNGLSHADERSOURCEPROC, glShaderSource) \
    X(PFNGLUNIFORM1IPROC, glUniform1i) \
    X(PFNGLUNIFORM4FPROC, glUniform4f) \
    X(PFNGLUNIFORMBLOCKBINDINGPROC, glUniformBlockBinding) \
    X(PFNGLUSEPROGRAMPROC, glUseProgram) \
    X(PFNGLVERTEXATTRIBIPOINTERPROC, glVertexAttribIPointer) \
    X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer)

#define AUGMENTINEL_GL_DECLARE(type, name) extern type p_##name;
AUGMENTINEL_GL_FUNCTIONS(AUGMENTINEL_GL_DECLARE)
#undef AUGMENTINEL_GL_DECLARE

// Route calls through the loaded pointers. Defined after SDL_opengl.h so its
// prototypes (glActiveTexture is declared there) are left untouched.
#define glActiveTexture p_glActiveTexture
#define glAttachShader p_glAttachShader
#define glBindBuffer p_glBindBuffer
#define glBindBufferBase p_glBindBufferBase
#define glBindFramebuffer p_glBindFramebuffer
#define glBindRenderbuffer p_glBindRenderbuffer
#define glBindVertexArray p_glBindVertexArray
#define glBlitFramebuffer p_glBlitFramebuffer
#define glBufferData p_glBufferData
#define glBufferSubData p_glBufferSubData
#define glCheckFramebufferStatus p_glCheckFramebufferStatus
#define glCompileShader p_glCompileShader
#define glCreateProgram p_glCreateProgram
#define glCreateShader p_glCreateShader
#define glDeleteBuffers p_glDeleteBuffers
#define glDeleteFramebuffers p_glDeleteFramebuffers
#define glDeleteProgram p_glDeleteProgram
#define glDeleteRenderbuffers p_glDeleteRenderbuffers
#define glDeleteShader p_glDeleteShader
#define glDeleteVertexArrays p_glDeleteVertexArrays
#define glDetachShader p_glDetachShader
#define glEnableVertexAttribArray p_glEnableVertexAttribArray
#define glFramebufferRenderbuffer p_glFramebufferRenderbuffer
#define glFramebufferTexture2D p_glFramebufferTexture2D
#define glGenBuffers p_glGenBuffers
#define glGenFramebuffers p_glGenFramebuffers
#define glGenRenderbuffers p_glGenRenderbuffers
#define glGenVertexArrays p_glGenVertexArrays
#define glGetProgramInfoLog p_glGetProgramInfoLog
#define glGetProgramiv p_glGetProgramiv
#define glGetShaderInfoLog p_glGetShaderInfoLog
#define glGetShaderiv p_glGetShaderiv
#define glGetUniformBlockIndex p_glGetUniformBlockIndex
#define glGetUniformLocation p_glGetUniformLocation
#define glLinkProgram p_glLinkProgram
#define glRenderbufferStorageMultisample p_glRenderbufferStorageMultisample
#define glShaderSource p_glShaderSource
#define glUniform1i p_glUniform1i
#define glUniform4f p_glUniform4f
#define glUniformBlockBinding p_glUniformBlockBinding
#define glUseProgram p_glUseProgram
#define glVertexAttribIPointer p_glVertexAttribIPointer
#define glVertexAttribPointer p_glVertexAttribPointer
#endif

// Resolve the entry points above for the current context. Returns false, and
// logs the first missing function, if the driver lacks OpenGL 3.3.
bool LoadGL();
