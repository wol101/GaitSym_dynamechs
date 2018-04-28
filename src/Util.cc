/*
 *  Util.h
 *  GaitSym
 *
 *  Created by Bill Sellers on Sat Dec 06 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 *  All the routines I can't think of a better place for
 *
 */

#ifdef USE_OPENGL
#include <gl.h>
#include <glut.h>
#endif

#include "Util.h"

// calculate the tangent intercept of a line from point (x, y) to a circle
// of radius r and centred on the origin. The anticlockwise return value is the
// angle of the radius pointing to the intercept when the vector from the point to
// the intercept acts in an anticlockwise direction.
// returns zero on success, non-zero on failure.
int Util::PointToCircleTangentIntercept(double x, double y, double r,
                                   double *anticlockwise, double *clockwise)
{
    if (x == 0 && y == 0) return 1;
    double theta = atan2(y, x);
    double h = sqrt(SQUARE(x) + SQUARE(y));
    r = ABS(r);
    if (r > h) return 2;
    double alpha = acos(r / h);
    *anticlockwise = theta + alpha;
    *clockwise = theta - alpha;
    return 0;
}

#ifdef USE_OPENGL
// axes
extern int gAxisFlag;
extern float gAxisSize;

void
Util::DrawAxes()
{
    if (gAxisFlag)
    {
        glDisable(GL_LIGHTING);

        glBegin(GL_LINES);
        glColor3f(1.0, 0.0, 0.0);
        glVertex3f(gAxisSize, 0.0, 0.0);
        glVertex3f(0.0, 0.0, 0.0);
        glEnd();

        glBegin(GL_LINES);
        glColor3f(0.0, 1.0, 0.0);
        glVertex3f(0.0, gAxisSize, 0.0);
        glVertex3f(0.0, 0.0, 0.0);
        glEnd();

        glBegin(GL_LINES);
        glColor3f(0.0, 0.0, 1.0);
        glVertex3f(0.0, 0.0, gAxisSize);
        glVertex3f(0.0, 0.0, 0.0);
        glEnd();

        glEnable(GL_LIGHTING);
    }
}

void
Util::DrawCOM(double x, double y, double z)
{
    if (gAxisFlag)
    {
        glPushMatrix();
        glTranslatef(x, y, z);
        glutSolidSphere(gAxisSize / 10, 16, 16);
        glPopMatrix();
    }
}

#endif



