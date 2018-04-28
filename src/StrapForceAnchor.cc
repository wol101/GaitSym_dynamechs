// StrapForceAnchor.cc - an anchor point for a strap force

#ifdef USE_OPENGL
#include <gl.h>
#endif

#include <dmLink.hpp>

#include "StrapForceAnchor.h"
#include "StrapForce.h"
#include "Util.h"
#include "DebugControl.h"

#ifdef USE_OPENGL
extern int gDrawMuscleForces;
extern float gForceLineScale; // multiply the force by this before drawing vector
#endif

// default constructor
StrapForceAnchor::StrapForceAnchor()
{
    m_StrapForce = 0;
    m_Anchor_ID = 0;
    m_drawValid = false;
}

// default destructor
// doesn't do anything since m_StrapForce not owned by this object
StrapForceAnchor::~StrapForceAnchor()
{
}

// set the location of the anchor point in local link coordinates
// set the strap force and register this anchor point
void StrapForceAnchor::SetStrapForce(dmLink *link,
                                     CartesianVector location,
                                     StrapForce *strapForce,
                                     unsigned int anchorID)
{
    m_Link = link;
    m_StrapForce = strapForce;
    m_Anchor_ID = anchorID;

    m_StrapForce->SetAnchor(m_Link, m_Anchor_ID, location);

    if (gDebug == StrapForceAnchorDebug)
    {
        *gDebugStream << "StrapForceAnchor::SetStrapForce " << m_name;
        *gDebugStream << " link->getName() " << link->getName();
        *gDebugStream << " m_Anchor_ID " << m_Anchor_ID;
        *gDebugStream << " m_Location " << location[0];
        *gDebugStream << " " << location[1];
        *gDebugStream << " " << location[2] << "\n";
    }
}

// calculate the force as needed by the link this force is attached to
void StrapForceAnchor::computeForce(const dmABForKinStruct & val, SpatialVector f_contact)
{
    CartesianVector worldForce;
    CartesianVector localForce;
    CartesianVector localTorque;
    CartesianVector location;
    int j;

    // get the force from the StrapForce
    m_StrapForce->GetAnchorForce(m_Anchor_ID, worldForce);

    // convert to local coordinates
    for (j = 0; j < 3; j++)
    {
        localForce[j] = val.R_ICS[0][j] * worldForce[0] +
        val.R_ICS[1][j] * worldForce[1] + val.R_ICS[2][j] * worldForce[2];
    }

    // and local torque
    m_StrapForce->GetAnchorLocation(m_Anchor_ID, location);
Util::CrossProduct3x1(location, localForce, localTorque);

    // and build the return force array
    for (j = 0; j < 3; j++)
    {
        f_contact[j] = localTorque[j];
        f_contact[j + 3] = localForce[j];
    }

    // calculate world values for drawing
    m_drawValid = true;
    m_StrapForce->GetAnchorWorldLocation(m_Anchor_ID, m_world_position);
    for (j = 0; j < 3; j++)
    {
        m_world_force[j] = // not sure this part is correct
        val.R_ICS[j][0] * localTorque[0] +
        val.R_ICS[j][1] * localTorque[1] +
        val.R_ICS[j][2] * localTorque[2];

        m_world_force[j + 3] =
            val.R_ICS[j][0] * localForce[0] +
            val.R_ICS[j][1] * localForce[1] +
            val.R_ICS[j][2] * localForce[2];
    }

    if (gDebug == StrapForceAnchorDebug)
    {
        *gDebugStream << "StrapForceAnchor::computeForce " << m_name;
        *gDebugStream << " localForce " << localForce[0];
        *gDebugStream << " " << localForce[1];
        *gDebugStream << " " << localForce[2];
        *gDebugStream << " worldForce " << worldForce[0];
        *gDebugStream << " " << worldForce[1];
        *gDebugStream << " " << worldForce[2] << "\n";
    }
}

// draw thyself!
// this isn't optimised yet - array calculations are done locally which is
// potentially very slow.

void StrapForceAnchor::draw() const
{
#ifdef USE_OPENGL
    if (m_drawValid)
    {
        if (gDrawMuscleForces)
        {
            glDisable(GL_LIGHTING);

            glBegin(GL_LINES);
            glColor3f(0.8, 0.8, 0.0);
            glVertex3f(m_world_position[0],
                       m_world_position[1], m_world_position[2]);
            glVertex3f(m_world_position[0] + m_world_force[3] * gForceLineScale,
                       m_world_position[1] + m_world_force[4] * gForceLineScale,
                       m_world_position[2] + m_world_force[5] * gForceLineScale);
            glEnd();

            glEnable(GL_LIGHTING);
        }
    }
#endif
}

