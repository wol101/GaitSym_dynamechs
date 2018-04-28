/*
 *  MyMobileBaseLink.h
 *  GaitSym
 *
 *  Created by Bill Sellers on Sat Dec 06 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 */

#ifndef MyMobileBaseLink_h
#define MyMobileBaseLink_h

#include <dmMobileBaseLink.hpp>
#include <string>

class dmABForKinStruct;

enum Restriction
{
    None = 0,
    Yeq0
};

class MyMobileBaseLink: public dmMobileBaseLink
{
public:
    MyMobileBaseLink();
    ~MyMobileBaseLink();

    void LoadGraphics(const char *model_filename,
                 double xScale, double yScale, double zScale,
                 double xOffset, double yOffset, double zOffset);

    void SetRanges(double positionRange[6], double velocityRange[6])
    {
        m_XMinP = positionRange[0];
        m_XMaxP = positionRange[1];
        m_YMinP = positionRange[2];
        m_YMaxP = positionRange[3];
        m_ZMinP = positionRange[4];
        m_ZMaxP = positionRange[5];
        m_XMinV = velocityRange[0];
        m_XMaxV = velocityRange[1];
        m_YMinV = velocityRange[2];
        m_YMaxV = velocityRange[3];
        m_ZMinV = velocityRange[4];
        m_ZMaxV = velocityRange[5];
    };
    bool OutsideRange();

    void SetRestriction(Restriction r) { m_Restriction = r; };
    
    // redefined drawing routine
    void draw() const;

    // redefined Kinematics routine
    void ABForwardKinematics(
                             Float q[],  // float[7]
                             Float qd[], // float[6]
                             const dmABForKinStruct &link_val_inboard,
                             dmABForKinStruct &link_val_curr);
        
protected:

    double m_XMinP;
    double m_XMaxP;
    double m_YMinP;
    double m_YMaxP;
    double m_ZMinP;
    double m_ZMaxP;
    double m_XMinV;
    double m_XMaxV;
    double m_YMinV;
    double m_YMaxV;
    double m_ZMinV;
    double m_ZMaxV;

    Restriction m_Restriction;
};

#endif


