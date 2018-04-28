// SideStabilizer.h - force to stop the torso falling over sideways

#ifndef SideStabilizer_h
#define SideStabilizer_h

#include <dm.h>
#include <dmForce.hpp>

class SideStabilizer:public dmForce
{
public:
  SideStabilizer();
  ~SideStabilizer();

  // required functions
  void computeForce (const dmABForKinStruct & val, SpatialVector force);
  void reset() {};
    
  // rendering functions
  void draw() const;
  
private:
  CartesianVector mLocalStabilizerPosition;
  double mDesiredYPosition;
  double mDesiredYVelocity;
  double mSpringConstant;
  double mDampingConstant;
};

#endif
