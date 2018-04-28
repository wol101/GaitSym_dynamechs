// LoadObj.h - routine to load a Wavefront .obj model for the
// geometry of a shape

// based on glLoadModels.cpp

// NB. Only works with obj files with triangular meshes, absolute
// references and no extra normal information. 

// This is quite restrictive but it's the only sort of obj file that
// I use.

#ifndef LoadObj_h
#define LoadObj_h

#include <gl.h>

GLuint LoadObj(const char *model_filename, 
      GLfloat xScale = 1.0, GLfloat yScale = 1.0, GLfloat zScale = 1.0,
      GLfloat xOffset = 0.0, GLfloat yOffset = 0.0, GLfloat zOffset = 0.0);
void SetGLColour(GLfloat red, GLfloat green, GLfloat blue,
      GLfloat ambient, GLfloat diffuse, GLfloat specular,
      GLfloat specularPower);

#endif // LoadObj_h
