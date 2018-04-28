/*
 *  SimulationContainer.h
 *  MergeXML
 *
 *  Created by Bill Sellers on Wed Dec 17 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 */

#ifndef SimulationContainer_h
#define SimulationContainer_h

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <map>
#include <string>
#include <vector>

class SimulationContainer
{
public:
    
    SimulationContainer();
    ~SimulationContainer();

    int LoadXML(const char *file);
    int WriteXML(const char *file);
    int Merge(SimulationContainer *sim, double proportion);

    int Merge(xmlNodePtr node1, xmlNodePtr node2, double proportion, char *name);

protected:
    void CleanExpressions(char *dataPtr);
    void RecoverExpressions(char *dataPtr);
    static void ReplaceChar(char *p1, int len, char c1, char c2);
    
    xmlDocPtr m_Doc;
    xmlNodePtr m_Global;
    xmlNodePtr m_Robot;
    map<string, xmlNodePtr>m_MobileBaseLinkList;
    map<string, xmlNodePtr>m_RevoluteLinkList;
    map<string, xmlNodePtr>m_MuscleList;
    map<string, xmlNodePtr>m_ContactList;
    map<string, xmlNodePtr>m_CyclicDriverList;
    map<string, xmlNodePtr>m_StepDriverList;
    map<string, xmlNodePtr>m_DataTargetList;

    bool m_DTDValidateFlag;

};




#endif

