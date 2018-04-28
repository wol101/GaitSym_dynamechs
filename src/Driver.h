/*
 *  Driver.h
 *  GaitSym
 *
 *  Created by Bill Sellers on Sat May 22 2004.
 *  Copyright (c) 2004 Bill Sellers. All rights reserved.
 *
 * Virtual class that all drivers descend from
 */

#ifndef Driver_h
#define Driver_h

#include <dmObject.hpp>

class Driver: public dmObject
{
public:
    Driver() {};
    virtual ~Driver() {};

    virtual double GetValue(double time) = 0;

};

#endif
