// MAMuscle - implementation of an Minetti & Alexander style
// muscle based on the StrapForce class

// Minetti & Alexander, J. theor Biol (1997) 186, 467-476

// Added extra terms to allow a parallel spring element

#include "StrapForce.h"
#include "MAMuscle.h"
#include "DebugControl.h"

// constructor

MAMuscle::MAMuscle()
{
    m_VMax = 0;
    m_F0 = 0;
    m_K = 0;
    m_Alpha = 0;
}

// destructor
MAMuscle::~MAMuscle()
{
}

// set the proportion of muscle fibres that are active
// calculates the tension in the strap

void MAMuscle::SetAlpha(double alpha)
{
    if (alpha < 0) m_Alpha = 0;
    else
    {
        if (alpha > 1.0) m_Alpha = 1.0;
        else m_Alpha = alpha;
    }

    // m_Velocity is negative when muscle shortening
    // we need the sign the other way round
    double v = -m_Velocity;
    double fFull;

    // limit v
    if (v > m_VMax) v = m_VMax;
    else if (v < -m_VMax) v = -m_VMax;
    
    if (v < 0)
    {
        fFull = m_F0 * (1.8 - 0.8 * ((m_VMax + v) / (m_VMax - (7.56 / m_K) * v)));
    }
    else
    {
        fFull = m_F0 * (m_VMax - v) / (m_VMax + (v / m_K));
    }

    // now set the tension as a proportion of fFull

    SetTension(m_Alpha * fFull);

    if (gDebug == MAMuscleDebug)
    {
        *gDebugStream << "MAMuscle::SetAlpha " << m_name <<
        " alpha " << alpha <<
        " m_Alpha " << m_Alpha <<
        " m_F0 " << m_F0 <<
        " m_VMax " << m_VMax <<
        " m_Velocity " << m_Velocity <<
        " fFull " << fFull <<
        " m_Length " << m_Length <<
        " activeTension " << m_Alpha * fFull <<
        "\n";
    }
}

// calculate the metabolic power of the muscle

double MAMuscle::GetMetabolicPower()
{
    // m_Velocity is negative when muscle shortening
    // we need the sign the other way round
    double relV = -m_Velocity / m_VMax;
    
    // limit relV
    if (relV > 1) relV = 1;
    else if (relV < -1) relV = -1;
    
    double relVSquared = relV * relV;
    double relVCubed = relVSquared * relV;

    double sigma = (0.054 + 0.506 * relV + 2.46 * relVSquared) /
        (1 - 1.13 * relV + 12.8 * relVSquared - 1.64 * relVCubed);

    if (gDebug == MAMuscleDebug)
    {
        *gDebugStream << "MAMuscle::GetMetabolicPower " << m_name <<
        " m_Alpha " << m_Alpha <<
        " m_F0 " << m_F0 <<
        " m_VMax " << m_VMax <<
        " m_Velocity " << m_Velocity <<
        " sigma " << sigma <<
        " power " << m_Alpha * m_F0 * m_VMax * sigma << "\n";
    }
    return (m_Alpha * m_F0 * m_VMax * sigma);
}



