// LoadObj.cc - routine to load a Wavefront .obj model for the
// geometry of a shape

#ifdef USE_OPENGL

// based on glLoadModels.cpp

// NB. Only works with obj files with triangular meshes, absolute
// references and no extra normal information.
// Any colours or material properties in the file are also ignored.

// This is quite restrictive but it's the only sort of obj file that
// I use.

#include <gl.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <map>
#include <string>

// Use the std namespace. To do this we must first guarantee that it exists.
#if defined(__sgi) || defined(__WIN32_) || defined(WIN32) || (defined(__GNUC__) && (__GNUC__>=3))
namespace std {}
using namespace std;
#endif

#include "LoadObj.h"

// root directory for bones graphics
extern char *gGraphicsRoot;

// do we have to draw triangle twice?
extern int gDrawTrianglesTwice;

struct MyVertex
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct MyFace
{
    int v1;
    int v2;
    int v3;
};

static void ExecuteObj(ifstream &data_ptr);
static inline void ComputeFaceNormal(const MyVertex &v1,
                                     const MyVertex &v2, const MyVertex &v3,
                                     GLfloat normal[3]);

inline void ComputeFaceNormal(const MyVertex &v1,
                              const MyVertex &v2, const MyVertex &v3,
                              GLfloat normal[3])
{
    GLfloat a[3], b[3];

    // calculate in plane vectors
    a[0] = v2.x - v1.x;
    a[1] = v2.y - v1.y;
    a[2] = v2.z - v1.z;
    b[0] = v3.x - v1.x;
    b[1] = v3.y - v1.y;
    b[2] = v3.z - v1.z;

    // cross(a, b, normal);
    normal[0] = a[1]*b[2] - a[2]*b[1];
    normal[1] = a[2]*b[0] - a[0]*b[2];
    normal[2] = a[0]*b[1] - a[1]*b[0];

    // normalize(normal);
    GLfloat norm = (GLfloat)sqrt((double)(normal[0]*normal[0] +
                                          normal[1]*normal[1] +
                                          normal[2]*normal[2]));

    if (norm > 0.0)
    {
        normal[0] /= norm;
        normal[1] /= norm;
        normal[2] /= norm;
    }
}

// optimised LoadObj routine
// previous shapes are stored and reused
GLuint LoadObj(const char *model_filename,
               GLfloat xScale, GLfloat yScale, GLfloat zScale,
               GLfloat xOffset, GLfloat yOffset, GLfloat zOffset)
{
    ifstream data_ptr;
    static map<string, GLuint> listOfLoadedFiles;
    map<string, GLuint>::iterator myIterator;
    string fileName;
    GLuint dlist_index;

    if (gGraphicsRoot) // need to prepend the extra path specification
    {
        fileName = gGraphicsRoot;
        fileName += "/";
        fileName += model_filename;
    }
    else
    {
        fileName = model_filename;
    }

    // have we already loaded the file?
    myIterator = listOfLoadedFiles.find(fileName);
    if (myIterator != listOfLoadedFiles.end())
    {
        return listOfLoadedFiles[fileName];
    }
    else
    {
        data_ptr.open(fileName.c_str());
        if (!data_ptr)
        {
            cerr << "LoadObj: Error unable to open data file: "
            << fileName << endl;
            return 0;
        }

        dlist_index = glGenLists(1);
        if (dlist_index == 0)
        {
            cerr << "LoadObj: Error unable to allocate dlist index." << endl;
            return 0;
        }

        glNewList(dlist_index, GL_COMPILE);
        glPushMatrix();
        glTranslatef(xOffset, yOffset, zOffset);
        glScalef(xScale, yScale, zScale);
        ExecuteObj(data_ptr);
        glPopMatrix();
        glEndList();

        listOfLoadedFiles[fileName] = dlist_index;
    }

    return dlist_index;
}

void ExecuteObj(ifstream &data_ptr)
{
    int i;
    MyVertex theVertex;
    MyFace theFace;
    char buffer[256];
std::vector<MyVertex> vertexList;
std::vector<MyFace> faceList;
    GLfloat normal[3];

    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_FILL);

    // hardwired material properties
    SetGLColour(233.0/255.0, 218.0/255.0, 201.0/255.0,
                0.2, 0.9, 0.3,
                20);

    while (data_ptr.good())
    {
        data_ptr.getline(buffer, sizeof(buffer));
        if (buffer[0] == 'v')
        {
            sscanf(&(buffer[1]), "%f%f%f",
                   &theVertex.x, &theVertex.y, &theVertex.z);
            vertexList.push_back(theVertex);
        }
        if (buffer[0] == 'f')
        {
            sscanf(&(buffer[1]), "%d%d%d",
                   &theFace.v1, &theFace.v2, &theFace.v3);
            // correct for zero offset rather than 1 offset
            theFace.v1--;
            theFace.v2--;
            theFace.v3--;
            faceList.push_back(theFace);
        }
    }

    int numFaces = (int)faceList.size();
    for (i = 0; i < numFaces; i++)
    {
        glBegin(GL_TRIANGLE_STRIP);

        ComputeFaceNormal(vertexList[faceList[i].v1],
                          vertexList[faceList[i].v2], vertexList[faceList[i].v3], normal);
        glNormal3fv(normal);

        glVertex3f(vertexList[faceList[i].v1].x,
                   vertexList[faceList[i].v1].y, vertexList[faceList[i].v1].z);
        glVertex3f(vertexList[faceList[i].v2].x,
                   vertexList[faceList[i].v2].y, vertexList[faceList[i].v2].z);
        glVertex3f(vertexList[faceList[i].v3].x,
                   vertexList[faceList[i].v3].y, vertexList[faceList[i].v3].z);

        glEnd();

        // kludge (draw each triangle twice) to get over backfacing polygon problem
        // inefficient so should be avoided if possible

        if (gDrawTrianglesTwice)
        {
            glBegin(GL_TRIANGLE_STRIP);

            ComputeFaceNormal(vertexList[faceList[i].v1],
                              vertexList[faceList[i].v3], vertexList[faceList[i].v2], normal);
            glNormal3fv(normal);

            glVertex3f(vertexList[faceList[i].v1].x,
                       vertexList[faceList[i].v1].y, vertexList[faceList[i].v1].z);
            glVertex3f(vertexList[faceList[i].v3].x,
                       vertexList[faceList[i].v3].y, vertexList[faceList[i].v3].z);
            glVertex3f(vertexList[faceList[i].v2].x,
                       vertexList[faceList[i].v2].y, vertexList[faceList[i].v2].z);

            glEnd();
        }
    }
}

// set some standard material colours
void SetGLColour(GLfloat red, GLfloat green, GLfloat blue,
                 GLfloat ambient, GLfloat diffuse, GLfloat specular,
                 GLfloat specularPower)
{
    int i;
    GLfloat color[4];
    GLfloat color2[4];

    color[0] = red;
    color[1] = green;
    color[2] = blue;
    color[3] = 1.0;

    // for (i = 0; i < 4; i++) color2[i] = color[i] * emission;
    //glMaterialfv(GL_FRONT, GL_EMISSION, color2);

    for (i = 0; i < 4; i++) color2[i] = color[i] * ambient;
    glMaterialfv(GL_FRONT, GL_AMBIENT, color2);

    for (i = 0; i < 4; i++) color2[i] = color[i] * diffuse;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, color2);

    for (i = 0; i < 4; i++) color2[i] = color[i] * specular;
    glMaterialfv(GL_FRONT, GL_SPECULAR, color2);

    glMaterialf(GL_FRONT, GL_SHININESS, specularPower);
}

#endif
