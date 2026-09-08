/*
 * Geometry and viewer code originally provided by Didier Smets,
 * Professor at Sorbonne University.
 * Adapted and integrated into the PLNS-Solver project.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gl_utils.h"
#include "shaders.h"


GLuint create_shader(const char *vs_path, const char *fs_path) {

    int success;
    GLchar infoLog[512];


    // Create vertex shader.

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);

    FILE *f = fopen(vs_path, "rb");

    if (!f) {
        printf("ERROR: Vertex shader %s not found!\n", vs_path);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *source = (char *)malloc(fsize + 1);

    if (!source) {
        fclose(f);
        printf("ERROR: unable to allocate vertex shader source.\n");
        return 0;
    }

    fread(source, 1, fsize, f);
    fclose(f);

    source[fsize] = '\0';

    const char *source_ptr = source;

    glShaderSource(vert, 1, &source_ptr, NULL);
    glCompileShader(vert);

    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);

    free(source);

    if (!success) {
        glGetShaderInfoLog(vert, 512, NULL, infoLog);

        printf(
            "ERROR: compiling vertex shader %s failed!\n%s\n",
            vs_path,
            infoLog
        );

        glDeleteShader(vert);

        return 0;
    }


    // Create fragment shader.

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

    FILE *g = fopen(fs_path, "rb");

    if (!g) {
        printf("ERROR: Fragment shader %s not found!\n", fs_path);

        glDeleteShader(vert);

        return 0;
    }

    fseek(g, 0, SEEK_END);
    int gsize = ftell(g);
    fseek(g, 0, SEEK_SET);

    char *source2 = (char *)malloc(gsize + 1);

    if (!source2) {
        fclose(g);
        glDeleteShader(vert);

        printf("ERROR: unable to allocate fragment shader source.\n");

        return 0;
    }

    fread(source2, 1, gsize, g);
    fclose(g);

    source2[gsize] = '\0';

    const char *source2_ptr = source2;

    glShaderSource(frag, 1, &source2_ptr, NULL);
    glCompileShader(frag);

    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);

    free(source2);

    if (!success) {
        glGetShaderInfoLog(frag, 512, NULL, infoLog);

        printf(
            "ERROR: compiling fragment shader %s failed!\n%s\n",
            fs_path,
            infoLog
        );

        glDeleteShader(vert);
        glDeleteShader(frag);

        return 0;
    }


    // Create shader program.

    GLuint prg = glCreateProgram();

    glAttachShader(prg, vert);
    glAttachShader(prg, frag);

    glLinkProgram(prg);

    glGetProgramiv(prg, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(prg, 512, NULL, infoLog);

        printf(
            "ERROR: linking shader program failed!\n%s\n",
            infoLog
        );

        glDeleteProgram(prg);
        glDeleteShader(vert);
        glDeleteShader(frag);

        return 0;
    }


    // Shader objects are no longer needed after linking.

    glDetachShader(prg, vert);
    glDetachShader(prg, frag);

    glDeleteShader(vert);
    glDeleteShader(frag);

    return prg;
}