#include "gl_utils.h"


/******************************************************************************
 * Modern OpenGL function pointers.
 *
 * These pointers are initialized by init_gl().
 *****************************************************************************/

/* Buffer objects */

PFNGLGENBUFFERSPROC
    glGenBuffers_ptr = nullptr;

PFNGLBINDBUFFERPROC
    glBindBuffer_ptr = nullptr;

PFNGLBUFFERDATAPROC
    glBufferData_ptr = nullptr;

PFNGLDELETEBUFFERSPROC
    glDeleteBuffers_ptr = nullptr;


/* Vertex Array Objects */

PFNGLGENVERTEXARRAYSPROC
    glGenVertexArrays_ptr = nullptr;

PFNGLBINDVERTEXARRAYPROC
    glBindVertexArray_ptr = nullptr;

PFNGLDELETEVERTEXARRAYSPROC
    glDeleteVertexArrays_ptr = nullptr;


/* Vertex attributes */

PFNGLVERTEXATTRIBPOINTERPROC
    glVertexAttribPointer_ptr = nullptr;

PFNGLENABLEVERTEXATTRIBARRAYPROC
    glEnableVertexAttribArray_ptr = nullptr;

PFNGLDISABLEVERTEXATTRIBARRAYPROC
    glDisableVertexAttribArray_ptr = nullptr;


/******************************************************************************
 * Initialize modern OpenGL functions.
 *
 * GLFW provides glfwGetProcAddress(), which retrieves the address of an
 * OpenGL function for the currently active OpenGL context.
 *
 * This function must therefore be called only after:
 *
 *     glfwMakeContextCurrent(window);
 *****************************************************************************/

bool init_gl() {
    /**************************************************************************
     * Buffer functions.
     *************************************************************************/

    glGenBuffers_ptr =
        reinterpret_cast<PFNGLGENBUFFERSPROC>(
            glfwGetProcAddress("glGenBuffers")
        );

    glBindBuffer_ptr =
        reinterpret_cast<PFNGLBINDBUFFERPROC>(
            glfwGetProcAddress("glBindBuffer")
        );

    glBufferData_ptr =
        reinterpret_cast<PFNGLBUFFERDATAPROC>(
            glfwGetProcAddress("glBufferData")
        );

    glDeleteBuffers_ptr =
        reinterpret_cast<PFNGLDELETEBUFFERSPROC>(
            glfwGetProcAddress("glDeleteBuffers")
        );


    /**************************************************************************
     * Vertex Array Object functions.
     *************************************************************************/

    glGenVertexArrays_ptr =
        reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(
            glfwGetProcAddress("glGenVertexArrays")
        );

    glBindVertexArray_ptr =
        reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(
            glfwGetProcAddress("glBindVertexArray")
        );

    glDeleteVertexArrays_ptr =
        reinterpret_cast<PFNGLDELETEVERTEXARRAYSPROC>(
            glfwGetProcAddress("glDeleteVertexArrays")
        );


    /**************************************************************************
     * Vertex attribute functions.
     *************************************************************************/

    glVertexAttribPointer_ptr =
        reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(
            glfwGetProcAddress("glVertexAttribPointer")
        );

    glEnableVertexAttribArray_ptr =
        reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(
            glfwGetProcAddress("glEnableVertexAttribArray")
        );

    glDisableVertexAttribArray_ptr =
        reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAYPROC>(
            glfwGetProcAddress("glDisableVertexAttribArray")
        );


    /**************************************************************************
     * Verify that all required functions were loaded.
     *************************************************************************/

    return

        /* Buffers */

        glGenBuffers_ptr != nullptr &&
        glBindBuffer_ptr != nullptr &&
        glBufferData_ptr != nullptr &&
        glDeleteBuffers_ptr != nullptr &&


        /* Vertex Array Objects */

        glGenVertexArrays_ptr != nullptr &&
        glBindVertexArray_ptr != nullptr &&
        glDeleteVertexArrays_ptr != nullptr &&


        /* Vertex attributes */

        glVertexAttribPointer_ptr != nullptr &&
        glEnableVertexAttribArray_ptr != nullptr &&
        glDisableVertexAttribArray_ptr != nullptr;
}