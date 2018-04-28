/*
 *  DataTarget.cpp
 *  GaitSym
 *
 *  Created by Bill Sellers on Sat May 22 2004.
 *  Copyright (c) 2004 Bill Sellers. All rights reserved.
 *
 */

#include <assert.h>

#include "DataTarget.h"
#include "Util.h"

DataTarget::DataTarget()
{
    m_ValueList = 0;
    m_DurationList = 0;
    m_ListLength = -1;
    m_LastIndex = 0;
    m_Weight = 1;
}

DataTarget::~DataTarget()
{
    if (m_DurationList) delete [] m_DurationList;
    if (m_ValueList) delete [] m_ValueList;
}

// Note list is t0, v0, t1, v1, t2, v2 etc
// times are absolute simulation times not intervals
void DataTarget::SetValueDurationPairs(int size, double *valueDurationPairs)
{
    int i;
    assert(size > 0);
    if (m_ListLength != size / 2)
    {
        if (m_DurationList) delete [] m_DurationList;
        if (m_ValueList) delete [] m_ValueList;
        m_ListLength = size / 2;
        m_DurationList = new double[m_ListLength];
        m_ValueList = new double[m_ListLength];
    }
    for (i = 0 ; i < m_ListLength; i++)
    {
        m_DurationList[i] = valueDurationPairs[i * 2];
        m_ValueList[i] = valueDurationPairs[i * 2 + 1];
    }
    m_LastIndex = 0;
}

// optimised assuming the integration step is smaler than the time
// interval (which it always should be)
// returns true when the target value has changed otherwise false
bool DataTarget::GetTargetValue(double time, double *target, double *weight)
{
    if (m_LastIndex >= m_ListLength)
    {
        return false;
    }
    if (time < m_DurationList[m_LastIndex])
    {
        return false;
    }

    while (true)
    {
        *target = m_ValueList[m_LastIndex];
        *weight = m_Weight;
        m_LastIndex++;
        if (m_LastIndex >= m_ListLength)
        {
            return true;
        }
        if (time < m_DurationList[m_LastIndex])
        {
            return true;
        }
    }
}

