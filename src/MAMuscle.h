// MAMuscle - implementation of an Minetti & Alexander style 
// muscle based on the StrapForce class

// Minetti & Alexander, J. theor Biol (1997) 186, 467-476

// Added extra terms to allow a parallel spring element

#ifndef MAMuscle_h
#define MAMuscle_h

#include "StrapForce.h"

class MAMuscle : public StrapForce
{
   public:
   
      MAMuscle();
      ~MAMuscle();
      
      void SetVMax(double vMax) { m_VMax = vMax; };
      void SetF0(double f0) { m_F0 = f0; };
      void SetK(double k) { m_K = k; };
      void SetAlpha(double alpha);
      double GetMetabolicPower();

   protected:
   
   double m_VMax;
   double m_F0;
   double m_K; 
   double m_Alpha;
};








#endif // MAMuscle_h
