/*
 *  MyEnvironment.cpp
 *  GaitSym
 *
 *  Created by Bill Sellers on Sat Mar 26 2005.
 *  Copyright (c) 2005 Bill Sellers. All rights reserved.
 *
 */

#ifdef USE_OPENGL
#include <gl.h>
#endif

#include <assert.h>

#include "MyEnvironment.h"

MyEnvironment::MyEnvironment()
{
    m_x_origin = 0;
    m_y_origin = 0;
}

MyEnvironment::~MyEnvironment()
{
}

// ---------------------------------------------------------------------------
// Function : setTerrainData
// Purpose  : Function to set the prismatic terrain data
// Inputs   : filename containing data
// Outputs  :
// ---------------------------------------------------------------------------
void MyEnvironment::setTerrainData(int x_dim, int y_dim, double grid_resolution,
                                   double x_origin, double y_origin,
                                   double *heights, int num_heights)
{
    int i, j, c;

    // set the elevation/depth data in meters.
    m_x_dim = x_dim;
    m_y_dim = y_dim;
    m_grid_resolution = grid_resolution;
    m_x_origin = x_origin;
    m_y_origin = y_origin;

    // allocate space for and read in depth data;
    m_depth = new double *[m_x_dim];

    c = 0;
    for (i = 0; i < m_x_dim; i++)
    {
        m_depth[i] = new double[m_y_dim];

        for (j = 0; j < m_y_dim; j++)
        {
            assert(c < num_heights);
            m_depth[i][j] = *heights;
            heights++;
            c++;
        }
    }

}

// ---------------------------------------------------------------------
// Function : getGroundDepth
// Purpose  : Compute ground location and normal from gridded depth data.
// Inputs   : Current contact position wrt ICS
// Outputs  : Ground depth along z-axis, and normal at this point.
// ---------------------------------------------------------------------
double MyEnvironment::getGroundDepth(CartesianVector contact_pos,
                                     CartesianVector ground_normal) const
{
    // wis - added speedup for flat ground at Z = 0
    if (m_depth == 0)
    {
        ground_normal[0] = 0.0;
        ground_normal[1] = 0.0;
        ground_normal[2] = -1.0;
        return 0;
    }

    CartesianVector new_contact_pos;
    new_contact_pos[0] = contact_pos[0] - m_x_origin;
    new_contact_pos[1] = contact_pos[1] - m_y_origin;
    new_contact_pos[2] = contact_pos[2];
    return dmEnvironment::getGroundDepth(new_contact_pos, ground_normal);
}

// ---------------------------------------------------------------------
// Function : getGroundElevation
// Purpose  : Compute ground location and normal from gridded elevation data.
// Inputs   : Current contact position wrt ICS
// Outputs  : Ground elevation along z-axis, and normal at this point.
// ---------------------------------------------------------------------
double MyEnvironment::getGroundElevation(CartesianVector contact_pos,
                                        CartesianVector ground_normal) const
{
    // wis - added speedup for flat ground at Z = 0
    if (m_depth == 0)
    {
        ground_normal[0] = 0.0;
        ground_normal[1] = 0.0;
        ground_normal[2] = 1.0;
        return 0;
    }

    CartesianVector new_contact_pos;
    new_contact_pos[0] = contact_pos[0] - m_x_origin;
    new_contact_pos[1] = contact_pos[1] - m_y_origin;
    new_contact_pos[2] = contact_pos[2];
    return dmEnvironment::getGroundElevation(new_contact_pos, ground_normal);
}

void
MyEnvironment::draw() const
{
#ifdef USE_OPENGL
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_CULL_FACE);
    if (m_depth) glCallList(m_terrain_model_index);
    glPopAttrib();
#endif
}

#ifdef USE_OPENGL
// routine from gldraw.cpp
inline void compute_face_normal(GLfloat v0[3], GLfloat v1[3], GLfloat v2[3],
                                GLfloat normal[3])
{
   GLfloat a[3], b[3];
   register int i;

   for (i=0; i<3; i++)
   {
      a[i] = v1[i] - v0[i];
      b[i] = v2[i] - v0[i];
   }

   //cross(a, b, normal);
   normal[0] = a[1]*b[2] - a[2]*b[1];
   normal[1] = a[2]*b[0] - a[0]*b[2];
   normal[2] = a[0]*b[1] - a[1]*b[0];

   //normalize(normal);
   GLfloat norm = sqrt(normal[0]*normal[0] +
                     normal[1]*normal[1] +
                     normal[2]*normal[2]);

   if (norm > 0.0)
   {
      normal[0] /= norm;
      normal[1] /= norm;
      normal[2] /= norm;
   }
}
#endif

// dmEnvironment drawInit routine with origins added
void MyEnvironment::drawInit()
{
#ifdef USE_OPENGL
   register int i, j;

   GLfloat vertex[3][3], normal[3];

   // read in and allocate depth data

   m_terrain_model_index = glGenLists(1);

   if (m_terrain_model_index == 0)
   {
      cerr << "loadModel_grid: Error unable to allocate dlist index." << endl;
   }

   glNewList(m_terrain_model_index, GL_COMPILE);
   {
      glPolygonMode(GL_FRONT, GL_LINE); //FILL);
      glPolygonMode(GL_BACK, GL_LINE);

      GLfloat color[4] = {0.5,0.5,1.0,1.0};
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
      glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, color);

      for (j=0; j<m_y_dim-1; j++)
      {
         glBegin(GL_TRIANGLE_STRIP);
         {
            for (i=0; i<m_x_dim; i++)
            {
               vertex[0][0] = ((GLfloat) i)*m_grid_resolution + m_x_origin;
               vertex[0][1] = ((GLfloat) j + 1.0)*m_grid_resolution + m_y_origin;
               vertex[0][2] = m_depth[i][j+1];

               if (i > 0)
               {
                  vertex[1][0] = ((GLfloat) i - 1.0)*m_grid_resolution + m_x_origin;
                  vertex[1][1] = ((GLfloat) j + 1.0)*m_grid_resolution + m_y_origin;
                  vertex[1][2] = m_depth[i-1][j+1];

                  vertex[2][0] = ((GLfloat) i - 1.0)*m_grid_resolution + m_x_origin;
                  vertex[2][1] = ((GLfloat) j)*m_grid_resolution + m_y_origin;
                  vertex[2][2] = m_depth[i-1][j];

                  compute_face_normal(vertex[1], vertex[2], vertex[0], normal);
                  glNormal3fv(normal);
               }
               glVertex3fv(vertex[0]);

               vertex[1][0] = ((GLfloat) i)*m_grid_resolution + m_x_origin;
               vertex[1][1] = ((GLfloat) j)*m_grid_resolution + m_y_origin;
               vertex[1][2] = m_depth[i][j];

               if (i > 0)
               {
                  compute_face_normal(vertex[1], vertex[0], vertex[2], normal);
                  glNormal3fv(normal);
               }
               glVertex3fv(vertex[1]);
            }
         }
         glEnd();
      }
   }
   glEndList();
#endif
}


