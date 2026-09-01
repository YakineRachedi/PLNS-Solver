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
 * must therefore be loaded dynamically after an OpenGL context has been
 * created.
 *
 * The function:
 *
 *     init_gl()
 *
 * must be called after:
 *
 *     glfwMakeContextCurrent(window);
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