#pragma once

/******************************************************************************
 * OpenGL utilities.
 *
 * GLFW is used to:
 *
 *   - create and manage OpenGL contexts,
 *   - create windows,
 *   - handle input,
 *   - retrieve addresses of modern OpenGL functions.
 *
 * On Windows, the system OpenGL headers expose only the legacy OpenGL API.
 * Modern functions such as:
 *
 *   - glGenBuffers
 *   - glBindBuffer
 *   - glBufferData
 *   - glGenVertexArrays
 *   - glBindVertexArray
 *
 * must therefore be loaded dynamically after an OpenGL context has been created.
 * The function init_gl() must be called after a valid OpenGL context has been created.
 *
 *****************************************************************************/

#include <stddef.h>

#include <GLFW/glfw3.h>


/******************************************************************************
 * Modern OpenGL types.
 *
 * These types are normally provided by modern OpenGL headers/loaders.
 * They are defined here because the Windows system OpenGL headers may only
 * expose an older OpenGL API.
 *****************************************************************************/

#ifndef GLsizeiptr
typedef ptrdiff_t GLsizeiptr;
#endif

#ifndef GLchar
typedef char GLchar;
#endif

// OpenGL constants that may be missing from old Windows headers.

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif

#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif

#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif

#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#endif

#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif

#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif

#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif

#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif

#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif

/******************************************************************************
 * OpenGL function pointer types.
 *****************************************************************************/

/* Buffer objects */

typedef void (*PFNGLGENBUFFERSPROC)(
    GLsizei n,
    GLuint *buffers
);

typedef void (*PFNGLBINDBUFFERPROC)(
    GLenum target,
    GLuint buffer
);

typedef void (*PFNGLBUFFERDATAPROC)(
    GLenum target,
    GLsizeiptr size,
    const void *data,
    GLenum usage
);

typedef void (*PFNGLDELETEBUFFERSPROC)(
    GLsizei n,
    const GLuint *buffers
);


/* Vertex Array Objects */

typedef void (*PFNGLGENVERTEXARRAYSPROC)(
    GLsizei n,
    GLuint *arrays
);

typedef void (*PFNGLBINDVERTEXARRAYPROC)(
    GLuint array
);

typedef void (*PFNGLDELETEVERTEXARRAYSPROC)(
    GLsizei n,
    const GLuint *arrays
);

/* Vertex attributes */

typedef void (*PFNGLVERTEXATTRIBPOINTERPROC)(
    GLuint index,
    GLint size,
    GLenum type,
    GLboolean normalized,
    GLsizei stride,
    const void *pointer
);

typedef void (*PFNGLENABLEVERTEXATTRIBARRAYPROC)(
    GLuint index
);

typedef void (*PFNGLDISABLEVERTEXATTRIBARRAYPROC)(
    GLuint index
);

// Shader objects.

typedef GLuint (*PFNGLCREATESHADERPROC)(
    GLenum type
);

typedef void (*PFNGLSHADERSOURCEPROC)(
    GLuint shader,
    GLsizei count,
    const GLchar *const *string,
    const GLint *length
);

typedef void (*PFNGLCOMPILESHADERPROC)(
    GLuint shader
);

typedef void (*PFNGLGETSHADERIVPROC)(
    GLuint shader,
    GLenum pname,
    GLint *params
);

typedef void (*PFNGLGETSHADERINFOLOGPROC)(
    GLuint shader,
    GLsizei maxLength,
    GLsizei *length,
    GLchar *infoLog
);

typedef void (*PFNGLDELETESHADERPROC)(
    GLuint shader
);


// Shader programs.

typedef GLuint (*PFNGLCREATEPROGRAMPROC)(
    void
);

typedef void (*PFNGLATTACHSHADERPROC)(
    GLuint program,
    GLuint shader
);

typedef void (*PFNGLLINKPROGRAMPROC)(
    GLuint program
);

typedef void (*PFNGLGETPROGRAMIVPROC)(
    GLuint program,
    GLenum pname,
    GLint *params
);

typedef void (*PFNGLGETPROGRAMINFOLOGPROC)(
    GLuint program,
    GLsizei maxLength,
    GLsizei *length,
    GLchar *infoLog
);

typedef void (*PFNGLDETACHSHADERPROC)(
    GLuint program,
    GLuint shader
);

typedef void (*PFNGLDELETEPROGRAMPROC)(
    GLuint program
);

// Program use and uniforms.

typedef void (*PFNGLUSEPROGRAMPROC)(
    GLuint program
);

typedef void (*PFNGLUNIFORMMATRIX4FVPROC)(
    GLint location,
    GLsizei count,
    GLboolean transpose,
    const GLfloat *value
);

typedef void (*PFNGLUNIFORM1FPROC)(
    GLint location,
    GLfloat v0
);

// Retrieve the location of a uniform variable in a shader program.

typedef GLint (*PFNGLGETUNIFORMLOCATIONPROC)(
    GLuint program,
    const GLchar *name
);

// Set a vec3 uniform variable.

typedef void (*PFNGLUNIFORM3FVPROC)(
    GLint location,
    GLsizei count,
    const GLfloat *value
);

// Set an integer uniform variable.

typedef void (*PFNGLUNIFORM1IPROC)(
    GLint location,
    GLint v0
);

/******************************************************************************
 * Modern OpenGL constants.
 *
 * These constants may not be present in the legacy Windows OpenGL headers.
 *****************************************************************************/

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif

#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif

#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif

#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#endif


/******************************************************************************
 * OpenGL function pointers.
 *
 * These variables are initialized by init_gl().
 *****************************************************************************/

/* Buffer objects */
extern PFNGLGENBUFFERSPROC glGenBuffers_ptr;
extern PFNGLBINDBUFFERPROC glBindBuffer_ptr;
extern PFNGLBUFFERDATAPROC glBufferData_ptr;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers_ptr;


/* Vertex Array Objects */
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays_ptr;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray_ptr;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays_ptr;

/* Vertex attributes */
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_ptr;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ptr;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray_ptr;

// Shader objects.
extern PFNGLCREATESHADERPROC glCreateShader_ptr;
extern PFNGLSHADERSOURCEPROC glShaderSource_ptr;
extern PFNGLCOMPILESHADERPROC glCompileShader_ptr;
extern PFNGLGETSHADERIVPROC glGetShaderiv_ptr;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_ptr;
extern PFNGLDELETESHADERPROC glDeleteShader_ptr;

// Shader programs.
extern PFNGLCREATEPROGRAMPROC glCreateProgram_ptr;
extern PFNGLATTACHSHADERPROC glAttachShader_ptr;
extern PFNGLLINKPROGRAMPROC glLinkProgram_ptr;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv_ptr;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_ptr;
extern PFNGLDETACHSHADERPROC glDetachShader_ptr;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram_ptr;

// Program use and uniforms.

extern PFNGLUSEPROGRAMPROC glUseProgram_ptr;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_ptr;
extern PFNGLUNIFORM1FPROC glUniform1f_ptr;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ptr;
extern PFNGLUNIFORM3FVPROC glUniform3fv_ptr;
extern PFNGLUNIFORM1IPROC glUniform1i_ptr;


/******************************************************************************
 * Initialize modern OpenGL functions.
 *
 * This function must be called after:
 *
 *     glfwMakeContextCurrent(window);
 *
 * Returns:
 *
 *     true  : all required OpenGL functions were loaded.
 *
 *     false : at least one required function could not be loaded.
 *****************************************************************************/

bool init_gl();


/******************************************************************************
 * OpenGL function aliases.
 *
 * These aliases allow the rest of the program to use the standard OpenGL
 * function names:
 *
 *     glGenBuffers(...)
 *     glBindBuffer(...)
 *     glBufferData(...)
 *
 * while the actual calls are performed through dynamically loaded function
 * pointers.
 *****************************************************************************/

/* Buffer objects */

#define glGenBuffers glGenBuffers_ptr
#define glBindBuffer glBindBuffer_ptr
#define glBufferData glBufferData_ptr
#define glDeleteBuffers glDeleteBuffers_ptr

/* Vertex Array Objects */
#define glGenVertexArrays glGenVertexArrays_ptr
#define glBindVertexArray glBindVertexArray_ptr
#define glDeleteVertexArrays glDeleteVertexArrays_ptr

/* Vertex attributes */
#define glVertexAttribPointer glVertexAttribPointer_ptr
#define glEnableVertexAttribArray glEnableVertexAttribArray_ptr
#define glDisableVertexAttribArray glDisableVertexAttribArray_ptr

// Shader objects.
#define glCreateShader glCreateShader_ptr
#define glShaderSource glShaderSource_ptr
#define glCompileShader glCompileShader_ptr
#define glGetShaderiv glGetShaderiv_ptr
#define glGetShaderInfoLog glGetShaderInfoLog_ptr
#define glDeleteShader glDeleteShader_ptr

// Shader programs.
#define glCreateProgram glCreateProgram_ptr
#define glAttachShader glAttachShader_ptr
#define glLinkProgram glLinkProgram_ptr
#define glGetProgramiv glGetProgramiv_ptr
#define glGetProgramInfoLog glGetProgramInfoLog_ptr
#define glDetachShader glDetachShader_ptr
#define glDeleteProgram glDeleteProgram_ptr

// Program use and uniforms.
#define glUseProgram glUseProgram_ptr
#define glUniformMatrix4fv glUniformMatrix4fv_ptr
#define glUniform1f glUniform1f_ptr
#define glGetUniformLocation glGetUniformLocation_ptr
#define glUniform3fv glUniform3fv_ptr
#define glUniform1i glUniform1i_ptr