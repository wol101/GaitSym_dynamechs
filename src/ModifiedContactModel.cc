// wis - had to modify this to allow the right sort of reporting for my
// fitness function

/*****************************************************************************
* Copyright 1999, Scott McMillan
*****************************************************************************
*     File: ModifiedContactModel.cpp
*   Author: Scott McMillan
*  Project: DynaMechs 3.0
*  Summary: Class declaration for contact modelling (contacts with terrain).
*         : Used to be class EndEffector, but now is a component of RigidBody
*         : so that contact forces on each body can be computed.
*****************************************************************************/

#ifdef USE_OPENGL
#include <gl.h>
#endif

#include "MyEnvironment.h"
#include "ModifiedContactModel.h"
#include "DebugControl.h"

extern int gAxisFlag;        // puts axes on each element
extern float gAxisSize;        // size of axis elements
extern int gDrawContactForces;
extern float gForceLineScale; // multiply the force by this before drawing vector

//============================================================================
// class ModifiedContactModel : public dmForce
//============================================================================

//----------------------------------------------------------------------------
ModifiedContactModel::ModifiedContactModel():
dmContactModel()
{
    m_drawValid = false;
    m_world_contact_pos = 0;
    m_world_force = 0;
}

//----------------------------------------------------------------------------
ModifiedContactModel::~ModifiedContactModel()
{
    delete [] m_world_contact_pos;
    delete [] m_world_force;
}

//----------------------------------------------------------------------------
//    Summary: set the list of contact points in local CS
// Parameters: num_contact_points - number of points to be initialized
//             contact_pts - Contact_Locations in local coordinate system
//    Returns: none
//----------------------------------------------------------------------------
void ModifiedContactModel::setContactPoints(unsigned int num_contact_points,
                                      CartesianVector *contact_pos)
{
    // call parent
    dmContactModel::setContactPoints(num_contact_points, contact_pos);
    
    // delete any previous contact points
    if (m_num_contact_points)
    {
        delete [] m_world_contact_pos;
        delete [] m_world_force;
    }

    // add new world storage
    if (num_contact_points)
    {
        m_world_contact_pos = new CartesianVector[m_num_contact_points];
        m_world_force = new SpatialVector[m_num_contact_points];
    }
}

//----------------------------------------------------------------------------
//    Summary: Compute the external force due to contact with terrain stored in
//             a MyEnvironment object
// Parameters: val - struct containing position and orientation of the rigid
//                   body wrt the inertial CS, and the spatial velocity
//                   with respect to the body's CS.
//    Returns: f_contact - spatial contact force exerted on the body wrt to the
//                   body's CS
//----------------------------------------------------------------------------
void ModifiedContactModel::computeForce(const dmABForKinStruct &val,
                                        SpatialVector f_contact)
{
    int j;

    // this section copied from dmContactModel except where indicated
    double ground_elevation;

    for (j = 0; j < 6; j++)
    {
        f_contact[j] = 0.0;
    }
    if (MyEnvironment::getEnvironment() == NULL)
    {
        return;
    }

    for (unsigned int i = 0; i < m_num_contact_points; i++)
    {
        for (j = 0; j < 3; j++)  // compute the contact pos.
        {                        // wrt ICS.

            current_pos[j] = val.p_ICS[j] +
            val.R_ICS[j][0]*m_contact_pos[i][0] +
            val.R_ICS[j][1]*m_contact_pos[i][1] +
            val.R_ICS[j][2]*m_contact_pos[i][2];

            // wis - store the global contact position
            m_world_contact_pos[i][j] = current_pos[j];
        }

        ground_elevation =
        (MyEnvironment::getEnvironment())->getGroundElevation(current_pos,
                                                              normal);

        if (current_pos[2] > ground_elevation)  // No contact
        {
            // Reset flags.
            if (m_contact_flag[i] == true)
            {
                m_contact_flag[i] = false;
                m_boundary_flag = true;
            }
            m_sliding_flag[i] = false;

            // Store last position
            m_initial_contact_pos[i][0] = current_pos[0];
            m_initial_contact_pos[i][1] = current_pos[1];
            m_initial_contact_pos[i][2] = current_pos[2];
        }
        else                      // Contact!
        {
            if (!m_contact_flag[i] || m_reset_flag)  // set initial contact Pos.
            {
#ifdef DEBUG
                cout << "Contact " << flush;
#endif
                m_initial_contact_pos[i][0] = current_pos[0];
                m_initial_contact_pos[i][1] = current_pos[1];
                m_initial_contact_pos[i][2] = ground_elevation;
            }

            if (m_contact_flag[i] == false)
            {
                m_contact_flag[i] = true;
                m_boundary_flag = true;
            }

            // End-effector linear velocity and "spring" displacement wrt ICS.
            crossproduct(&val.v[0], m_contact_pos[i], vnC_pos);
            vnC_pos[0] += val.v[3];
            vnC_pos[1] += val.v[4];
            vnC_pos[2] += val.v[5];

            for (j = 0; j < 3; j++)
            {
                veC_pos[j] = val.R_ICS[j][0]*vnC_pos[0] +
                val.R_ICS[j][1]*vnC_pos[1] +
                val.R_ICS[j][2]*vnC_pos[2];
                peC_pos[j] = current_pos[j] - m_initial_contact_pos[i][j];
            }

            // Magnitudes of normal components of velocity and delta position.
            vtemp = veC_pos[0]*normal[0] +
                veC_pos[1]*normal[1] +
                veC_pos[2]*normal[2];
            ptemp = peC_pos[0]*normal[0] +
                peC_pos[1]*normal[1] +
                peC_pos[2]*normal[2];

            // Magnitude of Normal force.
            fe_normal_mag = -(MyEnvironment::getEnvironment())->
                getGroundNormalDamperConstant()*vtemp -
                (MyEnvironment::getEnvironment())->
                getGroundNormalSpringConstant()*ptemp;

            if (fe_normal_mag < 0.0)   // Invalid...trying to suck into ground.
            {
                fe[0] = fe[1] = fe[2] = 0.0;
            }
            else
            {
                for (j = 0; j < 3; j++)
                {
                    fe_normal[j] = normal[j]*fe_normal_mag;
                }

                // Planar forces for sticking contact.
                for (j = 0; j < 3; j++)
                {
                    v_planar[j] = veC_pos[j] - normal[j]*vtemp;
                    p_planar[j] = peC_pos[j] - normal[j]*ptemp;
                    fe_planar[j] = -(MyEnvironment::getEnvironment())->
                        getGroundPlanarDamperConstant()*v_planar[j] -
                        (MyEnvironment::getEnvironment())->
                        getGroundPlanarSpringConstant()*p_planar[j];
                }
                fe_planar_mag = sqrt(fe_planar[0]*fe_planar[0] +
                                     fe_planar[1]*fe_planar[1] +
                                     fe_planar[2]*fe_planar[2]);

                // Check to see whether it should start sticking.
                if (m_sliding_flag[i])
                {
                    // if sliding, is it going slow enough to stick.
                    //Float speedSquared = (v_planar[0]*v_planar[0] +
                    //                      v_planar[1]*v_planar[1] +
                    //                      v_planar[2]*v_planar[2]);

                    // Stick if speed less than a threshhold or
                    // static planar force is less than the kinetic one.
                    if (             //(speedSquared < 0.005) ||
                                     (fe_planar_mag < (fe_normal_mag*
                                                       (MyEnvironment::getEnvironment())->
                                                       getGroundKineticFrictionCoeff())))
                    {
                        m_sliding_flag[i] = false;
                        m_boundary_flag = true;
                    }
                }
                // Check to see whether it should start sliding.
                else
                {
                    if (fe_planar_mag > (fe_normal_mag*
                                         (MyEnvironment::getEnvironment())->
                                         getGroundStaticFrictionCoeff()))
                    {
                        //slippage!
                        m_sliding_flag[i] = true;
                        m_boundary_flag = true;
                    }
                }

                // if sliding recompute a smaller planar force
                if (m_sliding_flag[i])
                {
                    temp = (fe_normal_mag/fe_planar_mag)*
                    (MyEnvironment::getEnvironment())->
                    getGroundKineticFrictionCoeff();
                    fe_planar[0] *= temp;
                    fe_planar[1] *= temp;
                    fe_planar[2] *= temp;

                    m_initial_contact_pos[i][0] = current_pos[0];
                    m_initial_contact_pos[i][1] = current_pos[1];
                    m_initial_contact_pos[i][2] = ground_elevation;
                }

                // Add normal and planar forces.
                for (j = 0; j < 3; j++)
                {
                    fe[j] = fe_normal[j] + fe_planar[j];
                }
            }

            // Compute Contact Force at link CS
            for (j = 0; j < 3; j++)
            {
                fn[j] = val.R_ICS[0][j]*fe[0] +
                val.R_ICS[1][j]*fe[1] +
                val.R_ICS[2][j]*fe[2];
            }
            crossproduct(m_contact_pos[i], fn, nn);

            // Accumulate for multiple contact points.
            for (j = 0; j < 3; j++)
            {
                f_contact[j] += nn[j];
                f_contact[j + 3] += fn[j];
            }

            // wis - store the global force at each contact point
            for (j = 0; j < 3; j++)
            {
                m_world_force[i][j] = // not sure this part is correct
                val.R_ICS[j][0] * nn[0] +
                val.R_ICS[j][1] * nn[1] +
                val.R_ICS[j][2] * nn[2];

                m_world_force[i][j + 3] =
                    val.R_ICS[j][0] * fn[0] +
                    val.R_ICS[j][1] * fn[1] +
                    val.R_ICS[j][2] * fn[2];
            }

        }
    }
    m_reset_flag = false;
    // end of section copied from dmContactModel
    
    m_drawValid = true;

    if (gDebug == ContactDebug)
    {
        SpatialVector world;
        for (j = 0; j < 3; j++)
        {
            world[j] = // not sure this part is correct
            val.R_ICS[j][0] * f_contact[0] +
            val.R_ICS[j][1] * f_contact[1] +
            val.R_ICS[j][2] * f_contact[2];

            world[j + 3] =
                val.R_ICS[j][0] * f_contact[0 + 3] +
                val.R_ICS[j][1] * f_contact[1 + 3] +
                val.R_ICS[j][2] * f_contact[2 + 3];
        }
        *gDebugStream << "ModifiedContactModel::computeForce" << " ";
        *gDebugStream << m_name << " World ";
        for (j = 0; j < 6; j++)
            *gDebugStream << world[j] << " ";
        *gDebugStream << "Local ";
        for (j = 0; j < 6; j++)
            *gDebugStream << f_contact[j] << " ";
        *gDebugStream << "\n";
    }
}

// draw thyself!
// this isn't optimised yet - array calculations are done locally which is
// potentially very slow.

void ModifiedContactModel::draw() const
{
#ifdef USE_OPENGL
    if (m_drawValid)
    {
        for (unsigned int i = 0; i < m_num_contact_points; i++)
        {
            if (gAxisFlag)
            {
                glDisable(GL_LIGHTING);

                glBegin(GL_LINES);
                glColor3f(1.0, 0.0, 0.0);
                glVertex3f(gAxisSize + m_world_contact_pos[i][0],
                           m_world_contact_pos[i][1], m_world_contact_pos[i][2]);
                glVertex3f(m_world_contact_pos[i][0],
                           m_world_contact_pos[i][1], m_world_contact_pos[i][2]);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(0.0, 1.0, 0.0);
                glVertex3f(m_world_contact_pos[i][0], gAxisSize +
                           m_world_contact_pos[i][1], m_world_contact_pos[i][2]);
                glVertex3f(m_world_contact_pos[i][0],
                           m_world_contact_pos[i][1], m_world_contact_pos[i][2]);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(0.0, 0.0, 1.0);
                glVertex3f(m_world_contact_pos[i][0],
                           m_world_contact_pos[i][1], gAxisSize + m_world_contact_pos[i][2]);
                glVertex3f(m_world_contact_pos[i][0],
                           m_world_contact_pos[i][1], m_world_contact_pos[i][2]);
                glEnd();

                glEnable(GL_LIGHTING);
            }

            if (gDrawContactForces)
            {
                glDisable(GL_LIGHTING);

                glBegin(GL_LINES);
                glColor3f(0.8, 0.8, 0.8);
                glVertex3f(m_world_contact_pos[i][0],
                           m_world_contact_pos[i][1], m_world_contact_pos[i][2]);
                glVertex3f(m_world_contact_pos[i][0] + m_world_force[i][3] * gForceLineScale,
                           m_world_contact_pos[i][1] + m_world_force[i][4] * gForceLineScale,
                           m_world_contact_pos[i][2] + m_world_force[i][5] * gForceLineScale);
                glEnd();

                glEnable(GL_LIGHTING);
            }
        }
    }
#endif
}
