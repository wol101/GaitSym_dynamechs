// StrapForceAnchor.h - an anchor point for a strap force

#ifndef StrapForceAnchor_h
#define StrapForceAnchor_h

#include <dmForce.hpp>

class StrapForce;
class dmLink;

class StrapForceAnchor: public dmForce
{
   public:
   
      StrapForceAnchor();
      ~StrapForceAnchor();
      
      void SetStrapForce(dmLink *link, CartesianVector location,
         StrapForce *strapForce, unsigned int anchorID);
      
      // required funcitons
      void reset() {};
      void computeForce(const dmABForKinStruct & val, SpatialVector force);
      void draw() const;
      
   protected:
   
      dmLink            *m_Link;
      StrapForce        *m_StrapForce;
      unsigned int      m_Anchor_ID;
      
      // drawing variables
      bool m_drawValid;
      SpatialVector     m_world_force;
      CartesianVector   m_world_position;

};


#endif // StrapForceAnchor_h
