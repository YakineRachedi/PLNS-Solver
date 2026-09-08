#include "gl_utils.h"


/******************************************************************************
 * Modern OpenGL function pointers.
 *
 * These pointers are initialized by init_gl().
 *****************************************************************************/


// Buffer objects.

PFNGLGENBUFFERSPROC glGenBuffers_ptr = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer_ptr = nullptr;
PFNGLBUFFERDATAPROC glBufferData_ptr = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers_ptr = nullptr;


// Vertex Array Objects.

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays_ptr = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray_ptr = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays_ptr = nullptr;


// Vertex attributes.

PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_ptr = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ptr = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray_ptr = nullptr;


// Shader objects.

PFNGLCREATESHADERPROC glCreateShader_ptr = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource_ptr = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader_ptr = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv_ptr = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_ptr = nullptr;
PFNGLDELETESHADERPROC glDeleteShader_ptr = nullptr;


// Shader programs.

PFNGLCREATEPROGRAMPROC glCreateProgram_ptr = nullptr;
PFNGLATTACHSHADERPROC glAttachShader_ptr = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram_ptr = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv_ptr = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_ptr = nullptr;
PFNGLDETACHSHADERPROC glDetachShader_ptr = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram_ptr = nullptr;


// Program use and uniforms.

PFNGLUSEPROGRAMPROC glUseProgram_ptr = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_ptr = nullptr;
PFNGLUNIFORM1FPROC glUniform1f_ptr = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ptr = nullptr;
PFNGLUNIFORM3FVPROC glUniform3fv_ptr = nullptr;
PFNGLUNIFORM1IPROC glUniform1i_ptr = nullptr;


bool init_gl() {
    
    // Buffer objects.

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


    // Vertex Array Objects.

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


    // Vertex attributes.

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


    // Shader objects.

    glCreateShader_ptr =
        reinterpret_cast<PFNGLCREATESHADERPROC>(
            glfwGetProcAddress("glCreateShader")
        );

    glShaderSource_ptr =
        reinterpret_cast<PFNGLSHADERSOURCEPROC>(
            glfwGetProcAddress("glShaderSource")
        );

    glCompileShader_ptr =
        reinterpret_cast<PFNGLCOMPILESHADERPROC>(
            glfwGetProcAddress("glCompileShader")
        );

    glGetShaderiv_ptr =
        reinterpret_cast<PFNGLGETSHADERIVPROC>(
            glfwGetProcAddress("glGetShaderiv")
        );

    glGetShaderInfoLog_ptr =
        reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(
            glfwGetProcAddress("glGetShaderInfoLog")
        );

    glDeleteShader_ptr =
        reinterpret_cast<PFNGLDELETESHADERPROC>(
            glfwGetProcAddress("glDeleteShader")
        );


    // Shader programs.

    glCreateProgram_ptr =
        reinterpret_cast<PFNGLCREATEPROGRAMPROC>(
            glfwGetProcAddress("glCreateProgram")
        );

    glAttachShader_ptr =
        reinterpret_cast<PFNGLATTACHSHADERPROC>(
            glfwGetProcAddress("glAttachShader")
        );

    glLinkProgram_ptr =
        reinterpret_cast<PFNGLLINKPROGRAMPROC>(
            glfwGetProcAddress("glLinkProgram")
        );

    glGetProgramiv_ptr =
        reinterpret_cast<PFNGLGETPROGRAMIVPROC>(
            glfwGetProcAddress("glGetProgramiv")
        );

    glGetProgramInfoLog_ptr =
        reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(
            glfwGetProcAddress("glGetProgramInfoLog")
        );

    glDetachShader_ptr =
        reinterpret_cast<PFNGLDETACHSHADERPROC>(
            glfwGetProcAddress("glDetachShader")
        );

    glDeleteProgram_ptr =
        reinterpret_cast<PFNGLDELETEPROGRAMPROC>(
            glfwGetProcAddress("glDeleteProgram")
        );


    // Program use and uniforms.

    glUseProgram_ptr =
        reinterpret_cast<PFNGLUSEPROGRAMPROC>(
            glfwGetProcAddress("glUseProgram")
        );

    glUniformMatrix4fv_ptr =
        reinterpret_cast<PFNGLUNIFORMMATRIX4FVPROC>(
            glfwGetProcAddress("glUniformMatrix4fv")
        );

    glUniform1f_ptr =
        reinterpret_cast<PFNGLUNIFORM1FPROC>(
            glfwGetProcAddress("glUniform1f")
        );

    glGetUniformLocation_ptr =
        reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(
            glfwGetProcAddress("glGetUniformLocation")
        );

    glUniform3fv_ptr =
        reinterpret_cast<PFNGLUNIFORM3FVPROC>(
            glfwGetProcAddress("glUniform3fv")
        );

    glUniform1i_ptr =
        reinterpret_cast<PFNGLUNIFORM1IPROC>(
            glfwGetProcAddress("glUniform1i")
        );


    // Verify that all required functions were loaded.

    return
        glGenBuffers_ptr != nullptr &&
        glBindBuffer_ptr != nullptr &&
        glBufferData_ptr != nullptr &&
        glDeleteBuffers_ptr != nullptr &&

        glGenVertexArrays_ptr != nullptr &&
        glBindVertexArray_ptr != nullptr &&
        glDeleteVertexArrays_ptr != nullptr &&

        glVertexAttribPointer_ptr != nullptr &&
        glEnableVertexAttribArray_ptr != nullptr &&
        glDisableVertexAttribArray_ptr != nullptr &&

        glCreateShader_ptr != nullptr &&
        glShaderSource_ptr != nullptr &&
        glCompileShader_ptr != nullptr &&
        glGetShaderiv_ptr != nullptr &&
        glGetShaderInfoLog_ptr != nullptr &&
        glDeleteShader_ptr != nullptr &&

        glCreateProgram_ptr != nullptr &&
        glAttachShader_ptr != nullptr &&
        glLinkProgram_ptr != nullptr &&
        glGetProgramiv_ptr != nullptr &&
        glGetProgramInfoLog_ptr != nullptr &&
        glDetachShader_ptr != nullptr &&
        glDeleteProgram_ptr != nullptr &&

        glUseProgram_ptr != nullptr &&
        glUniformMatrix4fv_ptr != nullptr &&
        glUniform1f_ptr != nullptr &&
        glGetUniformLocation_ptr != nullptr &&
        glUniform3fv_ptr != nullptr &&
        glUniform1i_ptr != nullptr;
}