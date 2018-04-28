/*
 *  MyRevoluteLink.cpp
 *  GaitSym
 *
 *  Created by Bill Sellers on Sat Dec 06 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 */

#ifdef USE_OPENGL
#include <gl.h>
#include "LoadObj.h"
#endif

#include "MyRevoluteLink.h"
#include "Util.h"

MyRevoluteLink::MyRevoluteLink()
{
}

MyRevoluteLink::~MyRevoluteLink()
{
#ifdef USE_OPENGL
    delete (GLuint *)m_user_data;
#endif
}

void
MyRevoluteLink::LoadGraphics(const char *model_filename,
                             double xScale, double yScale, double zScale,
                             double xOffset, double yOffset, double zOffset)
{
#ifdef USE_OPENGL
    GLuint *dlist = new GLuint;
    *dlist = LoadObj(model_filename,
                     xScale, yScale, zScale,
                     xOffset, yOffset, zOffset);
    setUserData((void *) dlist);
#endif
}

void
MyRevoluteLink::draw() const
{
#ifdef USE_OPENGL
    dmRevoluteLink::draw();
    Util::DrawAxes();
    double mass;
    CartesianTensor inertia;
    CartesianVector cg_pos;
    getInertiaParameters(mass, inertia, cg_pos);
    Util::DrawCOM(cg_pos[0], cg_pos[1], cg_pos[2]);
#endif
}


