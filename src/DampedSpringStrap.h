// DampedSpringStrap - implementation of a damped spring strap force

#ifndef DampedSpringStrap_h
#define DampedSpringStrap_h

#include "StrapForce.h"

class DampedSpringStrap : public StrapForce
{
   public:
   
      DampedSpringStrap();
      ~DampedSpringStrap();
      
      void SetDamping(double d) { m_Damping = d; };
      void SetSpringConstant(double k) { m_SpringConstant = k; };
      void SetUnloadedLength(double l) { m_UnloadedLength = l; };

      void UpdateTension();

   protected:

   double m_Damping;
   double m_SpringConstant;
   double m_UnloadedLength;
};








#endif // MAMuscle_h
