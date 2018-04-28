/*
 *  CyclicDriver.cpp
 *  GaitSym
 *
 *  Created by Bill Sellers on Sat Dec 06 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 *  Uses a cyclic stepped function to return a the value at a given time
 *
 */

#include <assert.h>

#include "CyclicDriver.h"
#include "Util.h"

CyclicDriver::CyclicDriver()
{
    m_ValueList = 0;
    m_DurationList = 0;
    m_ListLength = -1;
    m_LastIndex = 0;
    m_PhaseDelay = 0;
}

CyclicDriver::~CyclicDriver()
{
    if (m_DurationList) delete [] m_DurationList;
    if (m_ValueList) delete [] m_ValueList;
}

// Note list is delt0, v0, delt1, v1, delt2, v2 etc
// times are intervals not absolute simulation times
void CyclicDriver::SetValueDurationPairs(int size, double *valueDurationPairs)
{
    int i;
    assert(size > 0);
    if (m_ListLength != size / 2)
    {
        if (m_DurationList) delete [] m_DurationList;
        if (m_ValueList) delete [] m_ValueList;
        m_ListLength = size / 2;
        m_DurationList = new double[m_ListLength + 1];
        m_ValueList = new double[m_ListLength];
    }
    m_DurationList[0] = 0;
    for (i = 0 ; i < m_ListLength; i++)
    {
        m_DurationList[i + 1] = valueDurationPairs[i * 2] + m_DurationList[i];
        m_ValueList[i] = valueDurationPairs[i * 2 + 1];
    }
}

double CyclicDriver::GetValue(double time)
{
    // account for phase
    // m_PhaseDelay is a relative value (0 to 1) but the time offset needs to be positive
    double timeOffset = m_DurationList[m_ListLength] - m_DurationList[m_ListLength] * m_PhaseDelay;
    time = time + timeOffset;
    
    double rem = fmod(time, m_DurationList[m_ListLength]);

    // optimisation because most of the time this is called it just returns the value
    // used previously
    if (m_DurationList[m_LastIndex] <= time && m_DurationList[m_LastIndex + 1] > time)
        return m_ValueList[m_LastIndex];

    m_LastIndex = Util::BinarySearchRange<double>(m_DurationList, m_ListLength, rem);
    
    if (m_LastIndex == -1) m_LastIndex = 0; // fixup for not found errors
    return m_ValueList[m_LastIndex];
}


