/*
 *  DataTarget.h
 *  GaitSym
 *
 *  Created by Bill Sellers on Sat May 22 2004.
 *  Copyright (c) 2004 Bill Sellers. All rights reserved.
 *
 */

#ifndef DataTarget_h
#define DataTarget_h

#include <dmObject.hpp>

class dmRigidBody;

class DataTarget: public dmObject
{
public:
    DataTarget();
    ~DataTarget();

    enum DataType
    {
        XP,
        YP,
        ZP,
        Q0,
        Q1,
        Q2,
        Q3,
        THETA,
        XV,
        YV,
        ZV,
        XRV,
        YRV,
        ZRV,
        THETAV
    };
    
    void SetValueDurationPairs(int size, double *valueDurationPairs);
    void SetWeight(double w) { m_Weight = w; };
    void SetTarget(dmRigidBody *target) { m_Target = target; };
    void SetDataType(DataType dataType) { m_DataType = dataType; };
    
    bool GetTargetValue(double time, double *target, double *weight);
    dmRigidBody *GetTarget() { return m_Target; };
    DataType GetDataType() { return m_DataType; };

protected:
    double *m_ValueList;
    double *m_DurationList;
    int m_ListLength;
    int m_LastIndex;
    double m_Weight;
    dmRigidBody *m_Target;
    DataType m_DataType;
};

#endif

