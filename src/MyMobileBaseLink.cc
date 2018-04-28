/*
 *  MyMobileBaseLink.cpp
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

#include <float.h>

#include "MyMobileBaseLink.h"
#include "DebugControl.h"
#include "Util.h"
#include "PGDMath.h"

MyMobileBaseLink::MyMobileBaseLink()
{
    m_XMinP = -DBL_MAX;
    m_XMaxP = DBL_MAX;
    m_YMinP = -DBL_MAX;
    m_YMaxP = DBL_MAX;
    m_ZMinP = -DBL_MAX;
    m_ZMaxP = DBL_MAX;
    m_XMinV = -DBL_MAX;
    m_XMaxV = DBL_MAX;
    m_YMinV = -DBL_MAX;
    m_YMaxV = DBL_MAX;
    m_ZMinV = -DBL_MAX;
    m_ZMaxV = DBL_MAX;

    m_Restriction = Yeq0;
}

MyMobileBaseLink::~MyMobileBaseLink()
{
#ifdef USE_OPENGL
    delete (GLuint *)m_user_data;
#endif
}

void
MyMobileBaseLink::LoadGraphics(const char *model_filename,
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
MyMobileBaseLink::draw() const
{
#ifdef USE_OPENGL
    dmMobileBaseLink::draw();
    Util::DrawAxes();
    double mass;
    CartesianTensor inertia;
    CartesianVector cg_pos;
    getInertiaParameters(mass, inertia, cg_pos);
    Util::DrawCOM(cg_pos[0], cg_pos[1], cg_pos[2]);
#endif
}

// returns true if position and speed are outside specified ranges
bool
MyMobileBaseLink::OutsideRange()
{
    double position[7];
    double velocity[7];
    // there is no inboard link so these coordinates should be world coordinates
    getState(position, velocity);
    if (position[4] <= m_XMinP || position[4] >= m_XMaxP) 
    {
        cerr << "XP = " << position[4] << "\n";
        return true;
    }
    if (position[5] <= m_YMinP || position[5] >= m_YMaxP)
    {
        cerr << "YP = " << position[5] << "\n";
        return true;
    }
    if (position[6] <= m_ZMinP || position[6] >= m_ZMaxP)
    {
        cerr << "ZP = " << position[6] << "\n";
        return true;
    }
    if (velocity[3] <= m_XMinV || velocity[3] >= m_XMaxV)
    {
        cerr << "XV = " << velocity[3] << "\n";
        return true;
    }
    if (velocity[4] <= m_YMinV || velocity[4] >= m_YMaxV)
    {
        cerr << "YV = " << velocity[4] << "\n";
        return true;
    }
    if (velocity[5] <= m_ZMinV || velocity[5] >= m_ZMaxV)
    {
        cerr << "ZV = " << velocity[5] << "\n";
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
//    Summary: beginning of AB's Forward Kinematics recursion, called by
//             dmSystem::ABDynamics function only, also computes any contact
//             forces and then m_beta (resultant bias force).
// Parameters: q, qd   - new state of body (provided by numerical integrator)
//             q[0..3] - normalized quaternion
//             q[4..6] - link position wrt inboard link (^{i-1}p_i)
//             qd - relative spatial velocity in inboard link coordinates
//             link_val_inboard - kinematic parameters of inboard link
//    Returns: link_val_curr - various kinematic parameters for the link.
//----------------------------------------------------------------------------
void MyMobileBaseLink::ABForwardKinematics(
                                           Float q[],  // float[7]
                                           Float qd[], // float[6]
                                           const dmABForKinStruct &link_val_inboard,
                                           dmABForKinStruct &link_val_curr)
{
    if (m_Restriction == Yeq0)
    {
        // fix the q & qd values to there is no movement in y = 0 plane
        
        // zero the Y position and linear velocity; X and Z angular velocity;
        q[5] = qd[4] = qd[0] = qd[2]  = 0;
        
        // work out the angle from the y=0 plane
        pgd::Quaternion rotation(q[3], q[0], q[1], q[2]);
        // find out where this rotation moves a y=0 vector of the MobileBaseLink to
        // note this has to be in MobileBaseLink coordinates so should be defined in the data file
        pgd::Vector v = pgd::QVRotate(rotation, pgd::Vector(1, 0, 0));
        if (ABS(v.y) > 1e-10)
        {
            // now find angle between the vector and the y=0 plane (by using the normal vector)
            double theta = (M_PI/2) - acos(pgd::Vector(0, 1, 0) * v);
            // now find the axis of rotation
            pgd::Vector axis = pgd::Vector(v.x, 0, v.z) ^ v;
            // and rotate back that amount
            pgd::Quaternion rotationNeeded = pgd::MakeQFromAxis(axis.x, axis.y, axis.z, -theta);
            pgd::Quaternion correctedRotation = rotationNeeded * rotation; // should this be QRotate?
            if (gDebug == MyMobileBaseLinkDebug)
            {
                cerr << "MyMobileBaseLink::ABForwardKinematics q " <<
                    q[0] << " " << q[1] << " " << q[2] << " " << q[3] << " " <<
                    "v " << v.x << " " << v.y << " " << v.z << " " <<
                    "theta " << theta << " " <<
                    "axis " << axis.x << " " << axis.y << " " << axis.z << " " <<
                "correctedRotation " << correctedRotation.v.x << " " << correctedRotation.v.y << " " << correctedRotation.v.z << " " << correctedRotation.n << " ";
                v = pgd::QVRotate(correctedRotation, pgd::Vector(1, 0, 0));
                theta = (M_PI/2) - acos(pgd::Vector(0, 1, 0) * v);
                cerr << "v_prime " << v.x << " " << v.y << " " << v.z << " " <<
                    "theta_prime " << theta << "\n";
            }
            q[0] = correctedRotation.v.x;
            q[1] = correctedRotation.v.y;
            q[2] = correctedRotation.v.z;
            q[3] = correctedRotation.n;
        }
    }
    
    // and call the parent routine
    dmMobileBaseLink::ABForwardKinematics(q, qd, link_val_inboard, link_val_curr);
}    

