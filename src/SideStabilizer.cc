// SideStabilizer.cc - force to stop the torso falling over sideways

#include "SideStabilizer.h"
#include "DebugControl.h"

//============================================================================
// class SideStabilizer : public ExtendedForce
//============================================================================

//----------------------------------------------------------------------------
SideStabilizer::SideStabilizer()
{
    for (int j = 0; j < 3; j++)
    {
        mLocalStabilizerPosition[j] = 0.0;
    }
    mDesiredYPosition = 0.0;
    mDesiredYVelocity = 0.0;
    mSpringConstant = 10000;
    mDampingConstant = 1000;
}

//----------------------------------------------------------------------------
SideStabilizer::~SideStabilizer()
{
}


//----------------------------------------------------------------------------
//    Summary: Compute the external force to correct for lateral movement
//
// Parameters: val - struct containing position and orientation of the rigid
//                   body wrt the inertial CS, and the spatial velocity
//                   with respect to the body's CS.
//    Returns: f_contact - spatial contact force exerted on the body wrt to the
//                   body's CS
//----------------------------------------------------------------------------
void SideStabilizer::computeForce(const dmABForKinStruct & val, SpatialVector f_contact)
{
    CartesianVector worldStabilizerPosition;
    CartesianVector localVelocity;
    CartesianVector worldVelocity;
    CartesianVector worldForce;
    CartesianVector localForce;
    CartesianVector localTorque;
    double ySpatialError;
    double yVelocityError;
    int j;

    // calculate the world position of the stabilizerPosition
    for (j = 0; j < 3; j++)
    {
        worldStabilizerPosition[j] = val.p_ICS[j] +
        val.R_ICS[j][0] * mLocalStabilizerPosition[0] +
        val.R_ICS[j][1] * mLocalStabilizerPosition[1] +
        val.R_ICS[j][2] * mLocalStabilizerPosition[2];
    }

    // calculate spatial error
    ySpatialError = worldStabilizerPosition[1] - mDesiredYPosition;

    // now calculate velocity of stabilizer by combining linear and rotational components
    crossproduct(&val.v[0], mLocalStabilizerPosition, localVelocity);
    localVelocity[0] += val.v[3];
    localVelocity[1] += val.v[4];
    localVelocity[2] += val.v[5];

    // convert velocity to world coordinates
    for (j = 0; j < 3; j++)
    {
        worldVelocity[j] = val.R_ICS[j][0] * localVelocity[0] +
        val.R_ICS[j][1] * localVelocity[1] +
        val.R_ICS[j][2] * localVelocity[2];
    }

    // calculate the velocity error
    yVelocityError = worldVelocity[1] - mDesiredYVelocity;

    // calculate the world force

    worldForce[0] = 0.0;
    worldForce[1] = - mSpringConstant * ySpatialError - mDampingConstant * yVelocityError;
    worldForce[2] = 0;

    if (gDebug == SideStabilizerDebug)
        *gDebugStream << "SideStabilizer::computeForce ySpatialError " << ySpatialError <<
            " yVelocityError " << yVelocityError <<
            " worldForce[1] " << worldForce[1] << "\n";

    // convert force to local coordinate syste,
    for (j = 0; j < 3; j++)
    {
        localForce[j] = val.R_ICS[0][j] * worldForce[0] +
        val.R_ICS[1][j] * worldForce[1] + val.R_ICS[2][j] * worldForce[2];
    }
    crossproduct(mLocalStabilizerPosition, localForce, localTorque);

    // Accumulate for multiple contact points.
    for (j = 0; j < 3; j++)
    {
        f_contact[j] = localTorque[j];
        f_contact[j + 3] = localForce[j];
    }
}

// draw thyself!
// this isn't optimised yet - array calculations are done locally which is
// potentially very slow.

void SideStabilizer::draw() const
{
#ifdef USE_OPENGL

#endif
}
