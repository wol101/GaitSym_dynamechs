/*
 *  SimulationContainer.cc
 *  MergeXML
 *
 *  Created by Bill Sellers on Wed Dec 17 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <map>
#include <string>
#include <sstream>
#include <float.h>


using namespace std;

#include "SimulationContainer.h"
#include "Util.h"
#include "DataFile.h"

SimulationContainer::SimulationContainer()
{
    m_Doc = 0;
    m_DTDValidateFlag = false;
    m_Global = 0;
    m_Robot = 0;
}

SimulationContainer::~SimulationContainer()
{
    if (m_Doc) xmlFreeDoc(m_Doc);
}

int SimulationContainer::LoadXML(const char *filename)
{
    xmlNodePtr cur;
    xmlParserCtxtPtr ctxt = 0;

    DataFile theXMLFile;
    if (theXMLFile.ReadFile(filename)) return 1;

    xmlChar *data = (xmlChar *)theXMLFile.GetRawData();
    CleanExpressions((char *)data);
    
    try
    {
        // do the basic XML parsing
    
        /* create a parser context */
        ctxt = xmlNewParserCtxt();
        if (ctxt == 0)
        {
            fprintf(stderr, "Failed to allocate parser context\n");
            throw 1;
        }

        if (m_DTDValidateFlag)
            /* parse the file, activating the DTD validation option */
            m_Doc = xmlCtxtReadDoc(ctxt, data, "", NULL, XML_PARSE_DTDVALID);
        else
            m_Doc = xmlCtxtReadDoc(ctxt, data, "", NULL, 0);
    
        /* check if parsing suceeded */
        if (m_Doc == 0)
        {
            fprintf(stderr, "Failed to parse %s\n", filename);
            throw 1;
        }
    
        /* check if validation suceeded */
        if (ctxt->valid == 0)
        {
            fprintf(stderr, "Failed to validate %s\n", filename);
            throw 1;
        }
    
        cur = xmlDocGetRootElement(m_Doc);
    
        if (cur == NULL)
        {
            fprintf(stderr,"Empty document\n");
            throw 1;
        }
    
        if (xmlStrcmp(cur->name, (const xmlChar *) "GAITSYM"))
        {
            fprintf(stderr,"Document of the wrong type, root node != GAITSYM");
            throw 1;
        }
    
        // now parse the elements in the file

        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"GLOBAL"))) m_Global = cur;
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"MOBILE_BASE_LINK")))
                m_MobileBaseLinkList[(char *)xmlGetProp(cur, (const xmlChar *)"Name")] = cur;
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"REVOLUTE_LINK")))
                m_RevoluteLinkList[(char *)xmlGetProp(cur, (const xmlChar *)"Name")] = cur;
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"ROBOT"))) m_Robot = cur;
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"MUSCLE")))
                m_MuscleList[(char *)xmlGetProp(cur, (const xmlChar *)"Name")] = cur;
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"CONTACT")))
                m_ContactList[(char *)xmlGetProp(cur, (const xmlChar *)"Name")] = cur;
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"CYCLIC_DRIVER")))
                m_CyclicDriverList[(char *)xmlGetProp(cur, (const xmlChar *)"Name")] = cur;
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"STEP_DRIVER")))
                m_StepDriverList[(char *)xmlGetProp(cur, (const xmlChar *)"Name")] = cur;
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"DATA_TARGET")))
                m_DataTargetList[(char *)xmlGetProp(cur, (const xmlChar *)"Name")] = cur;
            cur = cur->next;
        }
    }

    catch(...)
    {
        if (ctxt) xmlFreeParserCtxt(ctxt);
        if (m_Doc) xmlFreeDoc(m_Doc);
        m_Doc = 0;
        return 1;
    }

    xmlFreeParserCtxt(ctxt);
    return 0;
}


int SimulationContainer::WriteXML(const char *filename)
{
    xmlDocPtr doc;
    xmlNodePtr rootNode;
    xmlNodePtr newNode;

    doc = xmlNewDoc((xmlChar *)"1.0");
    if (doc == 0) return 1;

    rootNode = xmlNewDocNode(doc, 0, (xmlChar *)"GAITSYM", 0);
    xmlDocSetRootElement(doc, rootNode);

    // GLOBAL
    newNode = xmlCopyNode(m_Global, 1);
    xmlAddChild(rootNode, newNode);

    // MOBILE_BASE_LINK
    {
        map<string, xmlNodePtr>::const_iterator iter;
        for (iter=m_MobileBaseLinkList.begin(); iter != m_MobileBaseLinkList.end(); iter++)
        {
            newNode = xmlCopyNode(iter->second, 1);
            xmlAddChild(rootNode, newNode);
        }
    }

    // REVOLUTE_LINK
    {
        map<string, xmlNodePtr>::const_iterator iter;
        for (iter=m_RevoluteLinkList.begin(); iter != m_RevoluteLinkList.end(); iter++)
        {
            newNode = xmlCopyNode(iter->second, 1);
            xmlAddChild(rootNode, newNode);
        }
    }
    
    // ROBOT
    newNode = xmlCopyNode(m_Robot, 1);
    xmlAddChild(rootNode, newNode);

    // MUSCLE
    {
        map<string, xmlNodePtr>::const_iterator iter;
        for (iter=m_MuscleList.begin(); iter != m_MuscleList.end(); iter++)
        {
            newNode = xmlCopyNode(iter->second, 1);
            xmlAddChild(rootNode, newNode);
        }
    }

    // CONTACT
    {
        map<string, xmlNodePtr>::const_iterator iter;
        for (iter=m_ContactList.begin(); iter != m_ContactList.end(); iter++)
        {
            newNode = xmlCopyNode(iter->second, 1);
            xmlAddChild(rootNode, newNode);
        }
    }

    // CYCLIC_DRIVER
    {
        map<string, xmlNodePtr>::const_iterator iter;
        for (iter=m_CyclicDriverList.begin(); iter != m_CyclicDriverList.end(); iter++)
        {
            newNode = xmlCopyNode(iter->second, 1);
            xmlAddChild(rootNode, newNode);
        }
    }

    // STEP_DRIVER
    {
        map<string, xmlNodePtr>::const_iterator iter;
        for (iter=m_StepDriverList.begin(); iter != m_StepDriverList.end(); iter++)
        {
            newNode = xmlCopyNode(iter->second, 1);
            xmlAddChild(rootNode, newNode);
        }
    }

    // DATA_TARGET
    {
        map<string, xmlNodePtr>::const_iterator iter;
        for (iter=m_DataTargetList.begin(); iter != m_DataTargetList.end(); iter++)
        {
            newNode = xmlCopyNode(iter->second, 1);
            xmlAddChild(rootNode, newNode);
        }
    }

    xmlChar *docTxtPtr;
    int docTxtLen;
    xmlDocDumpFormatMemory(doc, &docTxtPtr, &docTxtLen, 1);
    RecoverExpressions((char *)docTxtPtr);
    
    DataFile saveFile;
    saveFile.SetRawData((char *)docTxtPtr);
    saveFile.WriteFile(filename, false);
    
    xmlFreeDoc(doc);
    return 0;
}

int SimulationContainer::Merge(SimulationContainer *sim, double proportion)
{
    string buf;

    try
    {
        // GLOBAL
        if (sim->m_Global)
        {
            Merge(m_Global, sim->m_Global, proportion, "IntegrationStepsize");
            Merge(m_Global, sim->m_Global, proportion, "GravityVector");
            Merge(m_Global, sim->m_Global, proportion, "GroundPlanarSpringConstant");
            Merge(m_Global, sim->m_Global, proportion, "GroundNormalSpringConstant");
            Merge(m_Global, sim->m_Global, proportion, "GroundPlanarDamperConstant");
            Merge(m_Global, sim->m_Global, proportion, "GroundNormalDamperConstant");
            Merge(m_Global, sim->m_Global, proportion, "GroundStaticFrictionCoeff");
            Merge(m_Global, sim->m_Global, proportion, "GroundKineticFrictionCoeff");
            Merge(m_Global, sim->m_Global, proportion, "BMR");
            Merge(m_Global, sim->m_Global, proportion, "DefaultJointLimitSpringConstant");
            Merge(m_Global, sim->m_Global, proportion, "DefaultJointLimitDamperConstant");
            Merge(m_Global, sim->m_Global, proportion, "DefaultJointFriction");
            Merge(m_Global, sim->m_Global, proportion, "DefaultMuscleActivationK");
            Merge(m_Global, sim->m_Global, proportion, "DefaultMuscleForcePerUnitArea");
            Merge(m_Global, sim->m_Global, proportion, "DefaultMuscleVMaxFactor");
            Merge(m_Global, sim->m_Global, proportion, "DefaultMuscleWidth");
            Merge(m_Global, sim->m_Global, proportion, "DefaultMuscleFastTwitchProportion");
            Merge(m_Global, sim->m_Global, proportion, "DefaultMuscleDensity");
            Merge(m_Global, sim->m_Global, proportion, "TimeLimit");
            Merge(m_Global, sim->m_Global, proportion, "MechanicalEnergyLimit");
            Merge(m_Global, sim->m_Global, proportion, "MetabolicEnergyLimit");
            Merge(m_Global, sim->m_Global, proportion, "TerrainHeights");
        }
        // MOBILE_BASE_LINK
        {
            map<string, xmlNodePtr>::const_iterator iter;
            for (iter=sim->m_MobileBaseLinkList.begin(); iter != sim->m_MobileBaseLinkList.end(); iter++)
            {
                if (m_MobileBaseLinkList.find(iter->first) == m_MobileBaseLinkList.end()) // not found
                {
                    m_MobileBaseLinkList[iter->first] = iter->second;
                }
                else
                {
                    Merge(m_MobileBaseLinkList[iter->first], iter->second, proportion, "Mass");
                    Merge(m_MobileBaseLinkList[iter->first], iter->second, proportion, "CM");
                    Merge(m_MobileBaseLinkList[iter->first], iter->second, proportion, "MOI");
                    Merge(m_MobileBaseLinkList[iter->first], iter->second, proportion, "Position");
                    Merge(m_MobileBaseLinkList[iter->first], iter->second, proportion, "Velocity");
                    Merge(m_MobileBaseLinkList[iter->first], iter->second, proportion, "VelocityRange");
                    Merge(m_MobileBaseLinkList[iter->first], iter->second, proportion, "PositionRange");
                    Merge(m_MobileBaseLinkList[iter->first], iter->second, proportion, "Scale");
                }
            }
        }

        // REVOLUTE_LINK
        {
            map<string, xmlNodePtr>::const_iterator iter;
            for (iter=sim->m_RevoluteLinkList.begin(); iter != sim->m_RevoluteLinkList.end(); iter++)
            {
                if (m_RevoluteLinkList.find(iter->first) == m_RevoluteLinkList.end()) // not found
                {
                    m_RevoluteLinkList[iter->first] = iter->second;
                }
                else
                {
                    Merge(m_RevoluteLinkList[iter->first], iter->second, proportion, "Mass");
                    Merge(m_RevoluteLinkList[iter->first], iter->second, proportion, "CM");
                    Merge(m_RevoluteLinkList[iter->first], iter->second, proportion, "MOI");
                    Merge(m_RevoluteLinkList[iter->first], iter->second, proportion, "MDHA");
                    Merge(m_RevoluteLinkList[iter->first], iter->second, proportion, "MDHAlpha");
                    Merge(m_RevoluteLinkList[iter->first], iter->second, proportion, "MDHD");
                    Merge(m_RevoluteLinkList[iter->first], iter->second, proportion, "MDHTheta");
                    Merge(m_RevoluteLinkList[iter->first], iter->second, proportion, "InitialJointVelocity");
                    Merge(m_RevoluteLinkList[iter->first], iter->second, proportion, "JointMin");
                    Merge(m_RevoluteLinkList[iter->first], iter->second, proportion, "JointMax");
                    Merge(m_RevoluteLinkList[iter->first], iter->second, proportion, "Scale");
                }
            }
        }
        
        // ROBOT - nothing to do...

        // MUSCLE
        {
            map<string, xmlNodePtr>::const_iterator iter;
            for (iter=sim->m_MuscleList.begin(); iter != sim->m_MuscleList.end(); iter++)
            {
                if (m_MuscleList.find(iter->first) == m_MuscleList.end()) // not found
                {
                    m_MuscleList[iter->first] = iter->second;
                }
                else
                {
                    Merge(m_MuscleList[iter->first], iter->second, proportion, "PCA");
                    Merge(m_MuscleList[iter->first], iter->second, proportion, "FibreLength");
                    Merge(m_MuscleList[iter->first], iter->second, proportion, "TendonLength");
                    Merge(m_MuscleList[iter->first], iter->second, proportion, "Origin");
                    Merge(m_MuscleList[iter->first], iter->second, proportion, "Insertion");
                    Merge(m_MuscleList[iter->first], iter->second, proportion, "MidPoint");
                    Merge(m_MuscleList[iter->first], iter->second, proportion, "Radius");
                    Merge(m_MuscleList[iter->first], iter->second, proportion, "SerialStrainAtFmax");
                    Merge(m_MuscleList[iter->first], iter->second, proportion, "ParallelStrainAtFmax");
                }
            }
        }
        
        // CONTACT
        {
            map<string, xmlNodePtr>::const_iterator iter;
            for (iter=sim->m_ContactList.begin(); iter != sim->m_ContactList.end(); iter++)
            {
                if (m_ContactList.find(iter->first) == m_ContactList.end()) // not found
                {
                    m_ContactList[iter->first] = iter->second;
                }
                else
                {
                    Merge(m_ContactList[iter->first], iter->second, proportion, "ContactPositions");
                }
            }
        }

        // CYCLIC_DRIVER
        {
            map<string, xmlNodePtr>::const_iterator iter;
            for (iter=sim->m_CyclicDriverList.begin(); iter != sim->m_CyclicDriverList.end(); iter++)
            {
                if (m_CyclicDriverList.find(iter->first) == m_CyclicDriverList.end()) // not found
                {
                    m_CyclicDriverList[iter->first] = iter->second;
                }
                else
                {
                    Merge(m_CyclicDriverList[iter->first], iter->second, proportion, "DurationValuePairs");
                    Merge(m_CyclicDriverList[iter->first], iter->second, proportion, "PhaseDelay");
                }
            }
        }

        // STEP_DRIVER
        {
            map<string, xmlNodePtr>::const_iterator iter;
            for (iter=sim->m_StepDriverList.begin(); iter != sim->m_StepDriverList.end(); iter++)
            {
                if (m_StepDriverList.find(iter->first) == m_StepDriverList.end()) // not found
                {
                    m_StepDriverList[iter->first] = iter->second;
                }
                else
                {
                    Merge(m_StepDriverList[iter->first], iter->second, proportion, "DurationValuePairs");
                }
            }
        }

        // DATA_TARGET
        {
            map<string, xmlNodePtr>::const_iterator iter;
            for (iter=sim->m_DataTargetList.begin(); iter != sim->m_DataTargetList.end(); iter++)
            {
                if (m_DataTargetList.find(iter->first) == m_DataTargetList.end()) // not found
                {
                    m_DataTargetList[iter->first] = iter->second;
                }
                else
                {
                    Merge(m_DataTargetList[iter->first], iter->second, proportion, "DurationValuePairs");
                }
            }
        }

    }
    catch (...)
    {
        cerr << "Error merging file\n";
        return 1;
    }

    return 0;
}

int SimulationContainer::Merge(xmlNodePtr node1, xmlNodePtr node2, double proportion, char *name)
{
    xmlChar *prop1 = 0;
    xmlChar *prop2 = 0;
    int i, n1, n2;
    double *list1 = 0;
    double *list2 = 0;
    xmlAttrPtr attrPtr = 0;
    double v;
        
    ostringstream out;
    out.precision(DBL_DIG + 2);

    try
    {
        prop1 = xmlGetProp(node1, (const xmlChar *)name);
        if (prop1 == 0) throw 1;
        if (strstr((char *)prop1, "[[")) throw 1; // don't merge if [[expressions]]
        
        prop2 = xmlGetProp(node2, (const xmlChar *)name);
        if (prop2 == 0) throw 1;
        if (strstr((char *)prop1, "[[")) throw 1; // don't merge if [[expressions]]
        
        n1 = DataFile::CountTokens((char *)prop1);
        n2 = DataFile::CountTokens((char *)prop2);

        if (n1 != n2) throw 1;

        list1 = new double[n1];
        list2 = new double[n2];

        Util::Double(prop1, n1, list1);
        Util::Double(prop2, n2, list2);

        for (i = 0; i < n2; i++)
        {
            v = list1[i] + proportion * (list2[i] - list1[i]);
            out << v << " ";
        }

        attrPtr = xmlSetProp(node1, (xmlChar *)name, (xmlChar *)out.str().c_str());
        if (attrPtr == 0) throw 1;
    }

    catch (...)
    {
        if (prop1) xmlFree(prop1);
        if (prop2) xmlFree(prop2);
        if (list1) delete [] list1;
        if (list2) delete [] list2;
        return 1;
    }

    xmlFree(prop1);
    xmlFree(prop2);
    delete [] list1;
    delete [] list2;
    return 0;
}

void SimulationContainer::CleanExpressions(char *dataPtr)
{
    char *ptr1 = dataPtr;

    char *ptr2 = strstr(ptr1, "[[");
    while (ptr2)
    {
        ptr2 += 2;
        ptr1 = strstr(ptr2, "]]");
        if (ptr2 == 0)
        {
            cerr << "Error: could not find matching ]]\n";
            exit(1);
        }

        ReplaceChar(ptr2, ptr1 - ptr2, '<', '{');
        ReplaceChar(ptr2, ptr1 - ptr2, '>', '}');
        ReplaceChar(ptr2, ptr1 - ptr2, '&', '#');

        ptr1 += 2;
        ptr2 = strstr(ptr1, "[[");
    }
}

void SimulationContainer::RecoverExpressions(char *dataPtr)
{
    char *ptr1 = dataPtr;

    char *ptr2 = strstr(ptr1, "[[");
    while (ptr2)
    {
        ptr2 += 2;
        ptr1 = strstr(ptr2, "]]");
        if (ptr2 == 0)
        {
            cerr << "Error: could not find matching ]]\n";
            exit(1);
        }

        ReplaceChar(ptr2, ptr1 - ptr2, '{', '<');
        ReplaceChar(ptr2, ptr1 - ptr2, '}', '>');
        ReplaceChar(ptr2, ptr1 - ptr2, '#', '&');

        ptr1 += 2;
        ptr2 = strstr(ptr1, "[[");
    }
}

void SimulationContainer::ReplaceChar(char *p1, int len, char c1, char c2)
{
    for (int i = 0; i < len; i++)
        if (p1[i] == c1) p1[i] = c2;
}
