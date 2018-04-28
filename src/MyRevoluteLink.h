/*
 *  MyRevoluteLink.h
 *  GaitSym
 *
 *  Created by Bill Sellers on Sat Dec 06 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 */

#ifndef MyRevoluteLink_h
#define MyRevoluteLink_h

#include <dmRevoluteLink.hpp>
#include <string>

class MyRevoluteLink: public dmRevoluteLink
{
public:
    MyRevoluteLink();
    ~MyRevoluteLink();

    void LoadGraphics(const char *model_filename,
                 double xScale, double yScale, double zScale,
                 double xOffset, double yOffset, double zOffset);
    
    // redefined drawing routine
    void draw() const;
};

#endif
