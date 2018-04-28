/*****************************************************************************
 * DynaMechs: A Multibody Dynamic Simulation Library
 *
 * Copyright (C) 1994-2001  Scott McMillan   All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************
 *     File: dmContactModel.hpp
 *   Author: Scott McMillan
 *  Summary: Class definition for contact modelling (contacts with Env).
 *         : Used to be class EndEffector, but now is a component of RigidBody
 *         : as a dmForce component, so that contact forces on each body can be
 *         : computed.
 *****************************************************************************/

#ifndef _DM_CONTACT_MODEL_HPP
#define _DM_CONTACT_MODEL_HPP

#include <dm.h>
#include <dmEnvironment.hpp>
#include <dmForce.hpp>

//============================================================================

/**

A {\tt dmContactModel} object is subclassed from the {\tt dmForce} and computes
the compliant forces of contact with a prismatic terrain.  It is instantiated
and ``assigned'' to a {\tt dmRigidBody} object using the {\tt
dmRigidBody::addForce()} function.  This class queries the static {\tt
dmEnvironment} object about terrain and computes forces of contact between it
and the contact points defined in this class for the particular {\tt
dmRigidBody} object.  {\em This class is one of those that could use a bit of
work.  The whole interaction between robot (more specifically, rigid bodies)
and environment has not been completely thought through yet, with the dmForce
class it has taken one major step forward, though.}

The default constructor instantiates an empty object.  A call to the
{\tt setContactPoints} function initializes the contact points that
will be set.  Its first parameter, {\tt num\_contact\_points}, is the
number of points (3D vertices) that are to be allocated, and the
second is an array of {\tt num\_contact\_points} {\tt
CartesianVector}s that specify the constant locations of the contact
points relative to the rigid body's coordinate system.  The function
{\tt getNumContactPoints} can be used to query the size of the array,
and {\tt getContactPoint} can get the values of a particular vertex by
index (0 for the first, 1 for the second, etc.).  The latter returns
true if the index is within range; otherwise, it returns false.  The
{\tt getContactState} function can be used to query whether an
individual contact point is currently in contact.

The {\tt computeForce} is the main part of this class and computes the
compliant contact force exerted on the rigid body.  It is called by the {\tt
ABForwardAcceleration} functions of all {\tt dmRefMember} objects and the {\tt
dmABBackwardDynamics} functions of the {\tt dmLink} objects that are derived
from the {\tt dmRigidBody} class.  The {\tt ABForwardKinematics} functions
computes the position and orientation of the rigid body with respect to the
inertial coordinate system, and the spatial velocity with respect to the body's
coordinate system and sets the {\tt p\_ICS}, {\tt R\_ICS}, and {\tt v} elements
of the {\tt dmABForKinStruct} struct ({\tt val}).  This struct is used at the
beginning of the subsequent pass of the AB algorthm when it calls this class's
{\tt computeForce} to compute the resultant spatial force exerted at all of the
contact points and returns it in {\tt force} parameter (with respect to the
body's coordinate system).

In the current implementation of the {\tt computeForce} function, an initial
contact location is set to maintain information from iteration to iteration and
compute spring-like forces from the initial contact point.  If the state of the
system is manually changed while some of these points are in contact with the
terrain, large forces can result on the next iteration of the simulation.  The
{\tt reset} function is used to overcome this potential problem by resetting
all contact points.  {\em Also note that this iteration to iteration dependency
is really only valid assuming a simple single step integration method like
Euler.  I do not know how valid this is with multistep methods like
Runge-Kutte, but it seems to work so far.}

See also {\tt dmEnvironment}, {\tt dmRigidBody}, {\tt dmLoadFile\_dm}*/

//============================================================================

class DM_DLL_API dmContactModel : public dmForce
{
public:
   ///
   dmContactModel();
   ///
   virtual ~dmContactModel();

   ///
   void setContactPoints(unsigned int num_contact_points,
                         CartesianVector *contact_pos);
   ///
   unsigned int getNumContactPoints() const {return m_num_contact_points;}
   ///
   bool getContactPoint(unsigned int index, CartesianVector pos) const;
   ///
   bool getContactState(unsigned int index) const;

   ///
   void computeForce(const dmABForKinStruct &val, SpatialVector force);
   ///
   inline void reset() { m_reset_flag = true;}

   ///
   void pushState();
   ///
   void popState();

// rendering functions (for future expansion/visualization):
   ///
   void draw() const;

private:
   // not implemented
   dmContactModel(const dmContactModel &);
   dmContactModel &operator=(const dmContactModel &);

// wis - changed from private to protected
protected:
   bool m_reset_flag;

   //dmEnvironment *m_env;

   unsigned int m_num_contact_points;
   bool *m_contact_flag;
   bool *m_sliding_flag;
   CartesianVector *m_contact_pos;  // list of point locations to be checked
   CartesianVector *m_initial_contact_pos;  // ICS 1st pnt of contact for each

   // variables to store contact state for backing out of a step
   // when using variable stepsize integrators.
   bool *m_contact_flag_stored;
   bool *m_sliding_flag_stored;
   CartesianVector *m_initial_contact_pos_stored;


   // temporary variables only used by computeContactForce
   Float ptemp, vtemp, temp;
   Float fe_normal_mag, fe_planar_mag;
   CartesianVector normal, current_pos;
   CartesianVector peC_pos, veC_pos, vnC_pos, fe, fn, nn;
   CartesianVector p_planar, v_planar;
   CartesianVector fe_normal, fe_planar;
   
};

#endif
