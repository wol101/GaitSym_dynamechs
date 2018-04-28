/*
 *  MyEnvironment.h
 *  GaitSym
 *
 *  Created by Bill Sellers on Sat Mar 26 2005.
 *  Copyright (c) 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef MyEnvironment_h
#define MyEnvironment_h

#include <dmEnvironment.hpp>

class MyEnvironment: public dmEnvironment
{
public:
    MyEnvironment();
    ~MyEnvironment();

    void setTerrainData(int x_dim, int y_dim, double grid_resolution,
                        double x_origin, double y_origin,
                        double *heights, int num_heights);

    double getGroundDepth(CartesianVector contact_pos,
                          CartesianVector ground_normal) const;
    double getGroundElevation(CartesianVector contact_pos,
                          CartesianVector ground_normal) const;

    // redefined drawing routine
    void draw() const;
    void drawInit();
    
protected:

    double m_x_origin;
    double m_y_origin;
};


#endif



