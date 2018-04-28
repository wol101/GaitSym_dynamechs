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
 *     File: dmZScrewTxLink.hpp
 *   Author: Scott McMillan
 *  Created: 22 May 1998
 *  Summary: Class definition for a constant z-axial screw transformation (for
 *           branching).
 *****************************************************************************/

#ifndef _DM_ZSCREW_TX_LINK_HPP
#define _DM_ZSCREW_TX_LINK_HPP

#include <dm.h>
#include <dmLink.hpp>

//======================================================================
/**

This is an unusual concrete link class because while it inherits the AB
simulation algorithm functionality from the {\tt dmLink} class, it does not
represent a rigid body so it does not contain any dynamic parameters (that is,
it does not inherit from the {\tt dmRigidBody} class).  The purpose of this
class is to implement efficient branching within tree stuctures by modelling a
constant $z$ axial screw transformation between coordinate systems as
recommended in the kinematics literature by Khalil and Kleinfinger.

The $z$ axial screw is defined by two parameters: a translation along and a
rotation about the $z$ axis.  Since they are constant, these parameters can
only be set with arguments to the constructor where {\tt d} is the translation
amount and {\tt theta} is the rotation in radians.

The remainder of the functions are described in the {\tt dmLink} reference
pages and are implemented in this class for the specific case of $z$ axial
screws.  Note that {\tt getNumDOFs} returns 0.  As such the joint variables
{\tt q}, {\tt qd}, {\tt qdd}, and {\tt joint\_input} are empty arrays.

See also {\tt dmLink}.

 */


class DM_DLL_API dmZScrewTxLink : public dmLink
{
   enum {NUM_DOFS = 0};

public:
   ///
   dmZScrewTxLink(Float d, Float theta);
   ///
   virtual ~dmZScrewTxLink();

   ///
   inline int getNumDOFs() const {return NUM_DOFS;}
   ///
   inline void setState(Float [], Float []) {};
   ///
   inline void getState(Float [], Float []) const {};
   ///
   void getPose(RotationMatrix R, CartesianVector p) const;

   ///
   inline void setJointInput(Float []) {};

   // link-to-link transformation functions:
   ///
   void   rtxToInboard(const CartesianVector p_0,
                       CartesianVector p_inboard) const;
   ///
   void rtxFromInboard(const CartesianVector p_inboard,
                       CartesianVector p_0) const;

   ///
   void   stxToInboard(const SpatialVector p_0,
                       SpatialVector p_inboard) const;
   ///
   void stxFromInboard(const SpatialVector p_inboard,
                       SpatialVector p_0) const;

   ///
   void rcongtxToInboardSym(const CartesianTensor M_0,
                            CartesianTensor M_inboard) const;
   ///
   void rcongtxToInboardGen(const CartesianTensor M_0,
                            CartesianTensor M_inboard) const;
   ///
   void scongtxToInboardIrefl(const SpatialTensor M_0,
                              SpatialTensor M_inboard) const;
   ///
   void XikToInboard(Float **Xik_curr,
                     Float **Xik_prev,
                     int columns_Xik) const;
   ///
   void BToInboard(Float **Bkn,
                   Float **Xik, int cols_Xik,
                   Float **Xin, int cols_Xin) const;
   ///
   void xformZetak(Float *zetak,
                   Float **Xik, int cols_Xik) const;

// AB algorithm functions:
   ///
   void ABForwardKinematics(Float [],
                            Float [],
                            const dmABForKinStruct &link_val_inboard,
                            dmABForKinStruct &link_val_curr);

   ///
   void ABBackwardDynamics(const dmABForKinStruct &link_val_curr,
                           SpatialVector f_star_curr,
                           SpatialTensor I_refl_curr,
                           SpatialVector f_star_inboard,
                           SpatialTensor I_refl_inboard);
   ///
   void ABBackwardDynamicsN(const dmABForKinStruct &link_val_curr,
                            SpatialVector f_star_inboard,
                            SpatialTensor I_refl_inboard);

   ///
   void ABForwardAccelerations(SpatialVector a_inboard,
                               SpatialVector a_curr,
                               Float [],
                               Float []);
   ///
   void ABForwardAccelerations(SpatialVector a_inboard,
                               unsigned int *LB,
                               unsigned int num_elements_LB,
                               Float ***Xik,
                               Float **constraint_forces,
                               unsigned int *num_constraints,
                               SpatialVector a_curr,
                               Float qd[],
                               Float qdd[]);
   ///
   virtual Float getPotentialEnergy(const dmABForKinStruct &,
                                    CartesianVector) const {return 0;}
   ///
   virtual Float getKineticEnergy(const dmABForKinStruct &) const {return 0;}

// rendering function:
   ///
   virtual void draw() const;

private:
   dmZScrewTxLink();                // default construction prohibited
   dmZScrewTxLink(const dmZScrewTxLink &);
   dmZScrewTxLink &operator=(const dmZScrewTxLink &);

protected:
   const Float m_dMDH, m_thetaMDH;     // defines tx from ref to base of
                                       // articulation (angle in radians)
   Float m_stheta, m_ctheta;
   Float m_stst, m_stct, m_ctctmstst, m_stct2;
};

#endif
