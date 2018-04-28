// StrapForce.cc

// this routine provides a force that acts on a number of bodies
// via a strap. The strap has an origin and insertion and is tethered
// by a number of Anchors.

#ifdef USE_OPENGL
#include <gl.h>
#endif

#include <assert.h>
#include <dmLink.hpp>
#include <dmArticulation.hpp>

#include "StrapForce.h"
#include "Util.h"
#include "Simulation.h"
#include "DebugControl.h"
#include "Driver.h"
#include "MyRevoluteLink.h"

extern Simulation *gSimulation;

#ifdef USE_OPENGL
extern int gDrawMuscles;
#endif

// constructor
StrapForce::StrapForce()
{
    m_Tension = 0;
    m_Length = 0; // flag
    m_LastLength = 0;
    m_Velocity = 0;
    m_Valid = false;
    m_Driver = 0;
    m_StrapForceType = TwoPoint;
    m_Radius = 1;
    m_Angle1 = 0;
    m_Angle2 = 0;
}

// destructor
StrapForce::~StrapForce()
{
    if (m_Driver) delete m_Driver;
}

// set the number of anchor points (only allowed once)
void
StrapForce::SetNumAnchors(unsigned int numAnchors)
{
    StrapAnchorLocation theAnchorLocation;
    unsigned int theSize = m_AnchorList.size();
    assert(theSize == 0);

    for (unsigned int i = 0; i < numAnchors; i++)
        m_AnchorList.push_back(theAnchorLocation);
}

// assign values to the anchor points
// NB. does not set the world positions
// probably should only be called from StrapForceAnchor
void
StrapForce::SetAnchor(dmLink *link,
                      unsigned int anchorID, CartesianVector location)
{
    unsigned int theSize = m_AnchorList.size();
    assert(anchorID < theSize);

Util::Copy3x1(location, m_AnchorList[anchorID].localPosition);
    m_AnchorList[anchorID].link = link;
}

// calculate the world positions for all the anchor locations
void
StrapForce::UpdateWorldPositions()
{
    unsigned int i;
    unsigned int theSize = m_AnchorList.size();
    assert(theSize > 1);
    const double MIN_ANGLE_RADIUS = 0.1 * M_PI / 180;

    if (m_StrapForceType == Radius)
    {
        assert(theSize == 5);
        MyRevoluteLink *myRevoluteLink = dynamic_cast<MyRevoluteLink *>(m_AnchorList[3].link);
        assert(myRevoluteLink != 0);
        double mdhA, mdhAlpha, mdhD, mdhTheta;
        double originAngle, midpointAngle, insertionAngle, radius;
        myRevoluteLink->getMDHParameters(&mdhA, &mdhAlpha, &mdhD, &mdhTheta);
        insertionAngle = m_Angle1;
        originAngle =  m_Angle2 - mdhTheta;
        midpointAngle = (m_Angle1 + originAngle) / 2;
        radius = ABS(m_Radius);

        // fix up when overlap (or near overlap)
        if (m_Radius >= 0 && (originAngle - m_Angle1) < MIN_ANGLE_RADIUS)
        {
            insertionAngle = midpointAngle - (MIN_ANGLE_RADIUS / 2);
            originAngle = midpointAngle + (MIN_ANGLE_RADIUS / 2);
            m_AnchorList[3].localPosition[0] = radius;
            m_AnchorList[3].localPosition[1] = m_AnchorList[3].localPosition[2] = 0;
Util::ZPlaneRotate(insertionAngle, m_AnchorList[3].localPosition);
        }
        else if (m_Radius < 0 && (m_Angle1 - originAngle) < MIN_ANGLE_RADIUS)
        {
            insertionAngle = midpointAngle + (MIN_ANGLE_RADIUS / 2);
            originAngle = midpointAngle - (MIN_ANGLE_RADIUS / 2);
            m_AnchorList[3].localPosition[0] = radius;
            m_AnchorList[3].localPosition[1] = m_AnchorList[3].localPosition[2] = 0;
Util::ZPlaneRotate(insertionAngle, m_AnchorList[3].localPosition);
        }

        m_AnchorList[1].localPosition[0] = radius;
        m_AnchorList[1].localPosition[1] = m_AnchorList[1].localPosition[2] = 0;
Util::ZPlaneRotate(originAngle, m_AnchorList[1].localPosition);
        m_AnchorList[2].localPosition[0] = radius;
        m_AnchorList[2].localPosition[1] = m_AnchorList[2].localPosition[2] = 0;
Util::ZPlaneRotate(midpointAngle, m_AnchorList[2].localPosition);
        m_AnchorList[3].localPosition[0] = radius;
        m_AnchorList[3].localPosition[1] = m_AnchorList[3].localPosition[2] = 0;
Util::ZPlaneRotate(insertionAngle, m_AnchorList[3].localPosition);
        
        if (gDebug == StrapForceDebug)
        {
            *gDebugStream << "StrapForce::UpdateWorldPositions " << m_name <<
            " m_Angle1 " << m_Angle1 << " m_Angle2 " << m_Angle2 <<
            " midpointAngle " << midpointAngle << " originAngle " << originAngle <<
            " mdhTheta " << mdhTheta <<
            "\n";
        }
    }

    for (i = 0; i < theSize; i++)
    {
        ConvertLocalToWorldP(m_AnchorList[i].link,
                             m_AnchorList[i].localPosition, m_AnchorList[i].worldPosition);

        if (gDebug == StrapForceDebug)
        {
            *gDebugStream << "StrapForce::UpdateWorldPositions " << m_name <<
            " m_AnchorList[" << i << "].localPosition "
            << m_AnchorList[i].localPosition[0] << " "
            << m_AnchorList[i].localPosition[1] << " "
            << m_AnchorList[i].localPosition[2] <<
            " m_AnchorList[" << i << "].worldPosition "
            << m_AnchorList[i].worldPosition[0] << " "
            << m_AnchorList[i].worldPosition[1] << " "
            << m_AnchorList[i].worldPosition[2] << "\n";
        }
    }

    if (m_Valid)
    {
        m_LastLength = m_Length;
        CalculateLength();
        m_Velocity = (m_Length - m_LastLength) / gSimulation->GetTimeIncrement();
    }
    else
    {
        // first time through
        m_Valid = true;
        CalculateLength();
        m_LastLength = m_Length;
        m_Velocity = 0;
    }
}


// calculate the length
void
StrapForce::CalculateLength()
{
    unsigned int i;
    unsigned int theSize = m_AnchorList.size();

    assert(theSize >= 2);
    assert(m_Valid);

    m_Length = 0;
    for (i = 1; i < theSize; i++)
    {
        m_Length += Util::Distance3x1(m_AnchorList[i - 1].worldPosition,
                                      m_AnchorList[i].worldPosition);
    }
    
    if (gDebug == StrapForceDebug)
    {
        *gDebugStream << "StrapForce::CalculateLength " << m_name << " m_Length " <<
        m_Length << "\n";
    }

}

// convert a link local coordinate to a world coordinate

void
StrapForce::ConvertLocalToWorldP(dmLink *link,
                                 const CartesianVector local, CartesianVector world)
{
    // get positional data for the link
    unsigned int linkIndex = gSimulation->GetRobot()->getLinkIndex(link);
    const dmABForKinStruct *m = gSimulation->GetRobot()->getForKinStruct(linkIndex);

    for (int j = 0; j < 3; j++)
    {

        world[j] = m->p_ICS[j] +
        m->R_ICS[j][0] * local[0] +
        m->R_ICS[j][1] * local[1] +
        m->R_ICS[j][2] * local[2];
    }
}

// set the strap tension
void
StrapForce::SetTension(double tension)
{
    if (tension < 0)
    {
        tension = 0;
        if (gDebug == StrapForceDebug)
        {
            *gDebugStream << "StrapForce::SetTension " << m_name
            << "Warning tension < 0\n";
        }
    }
    
    m_Tension = tension;
    if (gDebug == StrapForceDebug)
    {
        *gDebugStream << "StrapForce::SetTension " << m_name
        << " m_Tension " << m_Tension << "\n";
    }
}

// get the anchor location
void
StrapForce::GetAnchorLocation(unsigned int anchor,
                              CartesianVector localPosition)
{
    unsigned int theSize = m_AnchorList.size();
    assert(anchor < theSize);

Util::Copy3x1(m_AnchorList[anchor].localPosition, localPosition);
}

// get the anchor location (world coordinates)
void
StrapForce::GetAnchorWorldLocation(unsigned int anchor,
                                   CartesianVector worldPosition)
{
    unsigned int theSize = m_AnchorList.size();
    assert(anchor < theSize);

Util::Copy3x1(m_AnchorList[anchor].worldPosition, worldPosition);
}

// set the anchor location
void
StrapForce::SetAnchorLocation(unsigned int anchor,
                              CartesianVector localPosition)
{
    unsigned int theSize = m_AnchorList.size();
    assert(anchor < theSize);

Util::Copy3x1(localPosition, m_AnchorList[anchor].localPosition);
}

// calculate the force vector at an Anchor (world coordinates)

void
StrapForce::GetAnchorForce(unsigned int anchor, CartesianVector force)
{
    unsigned int theSize = m_AnchorList.size();
    CartesianVector v;
    CartesianVector v1, v2;

    assert(anchor < theSize);

    if (!m_Valid) UpdateWorldPositions();


    // check for origin
    if (anchor == 0)
    {
        // calculate the direction
Util::Subtract3x1(m_AnchorList[anchor + 1].worldPosition, m_AnchorList[anchor].worldPosition, v);
Util::Unit3x1(v);
        // multiply the unit direction vector by the tension
Util::ScalarMultiply3x1(m_Tension, v, force);
        if (gDebug == StrapForceDebug)
        {
            *gDebugStream << "StrapForce::GetAnchorForce " << m_name
            << " anchor " << anchor
            << " force " << force[0]
            << " " << force[1]
            << " " << force[2] << "\n";
        }
        return;
    }

    // check for insertion
    if (anchor == theSize - 1)
    {
        // calculate the direction
Util::Subtract3x1(m_AnchorList[anchor - 1].worldPosition, m_AnchorList[anchor].worldPosition, v);
Util::Unit3x1(v);
        // multiply the unit direction vector by the tension
Util::ScalarMultiply3x1(m_Tension, v, force);
        if (gDebug == StrapForceDebug)
        {
            *gDebugStream << "StrapForce::GetAnchorForce " << m_name
            << " anchor " << anchor
            << " force " << force[0]
            << " " << force[1]
            << " " << force[2] << "\n";
        }
        return;
    }

    // normal mid anchor
    // calculate the two pull vectors v1 and v2

Util::Subtract3x1(m_AnchorList[anchor - 1].worldPosition, m_AnchorList[anchor].worldPosition, v1);
Util::Unit3x1(v1);
Util::Subtract3x1(m_AnchorList[anchor + 1].worldPosition, m_AnchorList[anchor].worldPosition, v2);
Util::Unit3x1(v2);

    // and calculate actual force
Util::Add3x1(v1, v2, v);
Util::ScalarMultiply3x1(m_Tension, v, force);
    if (gDebug == StrapForceDebug)
    {
        *gDebugStream << "StrapForce::GetAnchorForce " << m_name
        << " anchor " << anchor
        << " force " << force[0]
        << " " << force[1]
        << " " << force[2] << "\n";
    }
    return;
}

// draw the strap force
void
StrapForce::Draw()
{
#ifdef USE_OPENGL
    // draw strap force as a single line

    unsigned int theSize = m_AnchorList.size();
    if (theSize < 2) return;

    if (gDrawMuscles)
    {
        glDisable(GL_LIGHTING);

        // FIX ME
        // this should be implemented as polylines not single line segments
        // but it's a fairly minor inefficiency

        for (unsigned int i = 1; i < theSize; i++)
        {
            glBegin(GL_LINES);
            glColor3f(1.0, 0.0, 0.0);
            glVertex3f(m_AnchorList[i - 1].worldPosition[0],
                       m_AnchorList[i - 1].worldPosition[1], m_AnchorList[i - 1].worldPosition[2]);
            glVertex3f(m_AnchorList[i].worldPosition[0],
                       m_AnchorList[i].worldPosition[1], m_AnchorList[i].worldPosition[2]);
            glEnd();
        }
        
        glEnable(GL_LIGHTING);
    }
#endif
}
