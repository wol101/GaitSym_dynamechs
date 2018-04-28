/*
 *  XMLConverter.cc
 *  GA
 *
 *  Created by Bill Sellers on Fri Dec 12 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 */

using namespace std;

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sstream>
#include <float.h>
#include <math.h>
#include <assert.h>

#include "XMLConverter.h"
#include "Genome.h"
#include "DataFile.h"
#include "ExpressionParser.h"

XMLConverter::XMLConverter()
{
    m_Doc = 0;
    m_DocTxtPtr = 0;
    m_DTDValidateFlag = false;
    m_ConversionType = SixMuscle;
    m_SmartSubstitutionTextBuffer = 0;
}

XMLConverter::~XMLConverter()
{
    unsigned int i;
    if (m_Doc) xmlFreeDoc(m_Doc);
    if (m_DocTxtPtr) xmlFree(m_DocTxtPtr);
    if (m_SmartSubstitutionTextBuffer) delete m_SmartSubstitutionTextBuffer;
    for (i = 0; i < m_SmartSubstitutionTextComponents.size(); i++)
        delete m_SmartSubstitutionTextComponents[i];
    for (i = 0; i < m_SmartSubstitutionParserComponents.size(); i++)
        delete m_SmartSubstitutionParserComponents[i];
}

// load the base file with DTD checking
// or just load up the smart substitution file
int XMLConverter::LoadBaseXMLFile(const char *filename)
{
    if (m_ConversionType == SmartSubstitution)
    {
        DataFile smartSubstitutionBaseXMLFile;
        if (smartSubstitutionBaseXMLFile.ReadFile(filename)) return 1;
        DoSmartSubstitution(smartSubstitutionBaseXMLFile.GetRawData());
        return 0;
    }
    else
    {
        if (m_Doc) xmlFreeDoc(m_Doc);

        xmlParserCtxtPtr ctxt;

        /* create a parser context */
        ctxt = xmlNewParserCtxt();
        if (ctxt == 0)
        {
            fprintf(stderr, "Failed to allocate parser context\n");
            return 1;
        }

        if (m_DTDValidateFlag)
            /* parse the file, activating the DTD validation option */
            m_Doc = xmlCtxtReadFile(ctxt, filename, NULL, XML_PARSE_DTDVALID);
        else
            m_Doc = xmlCtxtReadFile(ctxt, filename, NULL, 0);

        /* check if parsing suceeded */
        if (m_Doc == 0)
        {
            fprintf(stderr, "Failed to parse %s\n", filename);
            xmlFreeParserCtxt(ctxt);
            return 1;
        }

        /* check if validation suceeded */
        if (ctxt->valid == 0)
        {
            fprintf(stderr, "Failed to validate %s\n", filename);
            xmlFreeDoc(m_Doc);
            xmlFreeParserCtxt(ctxt);
            m_Doc = 0;
            return 1;
        }

        // free up the parser context anyway
        xmlFreeParserCtxt(ctxt);
    }
    
    return 0;
}

xmlChar *XMLConverter::GetFormattedXML(int *docTxtLen)
{
    if (m_ConversionType == SmartSubstitution)
    {
        if (m_SmartSubstitutionTextBuffer == 0) return 0;
        char *ptr = m_SmartSubstitutionTextBuffer;
        unsigned int i;
        int chars;
        for (i = 0; i < m_SmartSubstitutionValues.size(); i++)
        {
            chars = sprintf(ptr, "%s", m_SmartSubstitutionTextComponents[i]->c_str());
            ptr += chars;
            chars = sprintf(ptr, "%.17f", m_SmartSubstitutionValues[i]);
            ptr += chars;
        }
        chars = sprintf(ptr, "%s", m_SmartSubstitutionTextComponents[i]->c_str());
        ptr += chars;
        *docTxtLen = (int)(ptr - m_SmartSubstitutionTextBuffer);
        return (xmlChar *)m_SmartSubstitutionTextBuffer;
    }
    else
    {
        if (m_Doc == 0) return 0;
        if (m_DocTxtPtr == 0) return 0;

        *docTxtLen = m_DocTxtLen;
        return m_DocTxtPtr;
    }
}

// this needs to be customised depending on how the genome interacts with
// the XML file specifying the simulation
int XMLConverter::ApplyGenome(Genome *g)
{
    if (m_ConversionType == SmartSubstitution)
    {
        for (unsigned int i = 0; i < m_SmartSubstitutionParserComponents.size(); i++)
        {
            m_SmartSubstitutionValues[i] = m_SmartSubstitutionParserComponents[i]->Evaluate(g);
        }
        return 0;
    }
    else
    {
        xmlNodePtr root;
        xmlNodePtr cur;
        xmlNodePtr next;

        if (m_Doc == 0) return 1;

        root = xmlDocGetRootElement(m_Doc);

        // some basic checks

        if (root == NULL) return 1;
        if (xmlStrcmp(root->name, (const xmlChar *) "GAITSYM")) return 1;

        // now delete the Cyclic and Step Drivers

        cur = root->xmlChildrenNode;
        while (cur != NULL)
        {
            next = cur->next;
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"CYCLIC_DRIVER")))
            {
                xmlUnlinkNode(cur);
                xmlFreeNode(cur);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"STEP_DRIVER")))
            {
                xmlUnlinkNode(cur);
                xmlFreeNode(cur);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"text")))
            {
                xmlUnlinkNode(cur);
                xmlFreeNode(cur);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"comment")))
            {
                xmlUnlinkNode(cur);
                xmlFreeNode(cur);
            }
            cur = next;
        }

        if (m_ConversionType == SixMuscle)
        {
            // and create some new ones based on the genome
            double d[6], v[6], v2[6];
            int i;
            d[0] = d[3] = fabs(g->GetGene(0));
            d[1] = d[4] = fabs(g->GetGene(7));
            d[2] = d[5] = fabs(g->GetGene(14));

            i = 0;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 8); v[2] = g->GetGene(i + 15);
            v[3] = g->GetGene(i + 4); v[4] = g->GetGene(i + 11); v[5] = g->GetGene(i + 18);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightHipFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightHipExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 8); v[2] = g->GetGene(i + 15);
            v[3] = g->GetGene(i + 4); v[4] = g->GetGene(i + 11); v[5] = g->GetGene(i + 18);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightKneeFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightKneeExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 8); v[2] = g->GetGene(i + 15);
            v[3] = g->GetGene(i + 4); v[4] = g->GetGene(i + 11); v[5] = g->GetGene(i + 18);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightAnkleFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightAnkleExtensor", 6, d, v2);

            i = 0;
            v[0] = g->GetGene(i + 4); v[1] = g->GetGene(i + 11); v[2] = g->GetGene(i + 18);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 8); v[5] = g->GetGene(i + 15);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftHipFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftHipExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 4); v[1] = g->GetGene(i + 11); v[2] = g->GetGene(i + 18);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 8); v[5] = g->GetGene(i + 15);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftKneeFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftKneeExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 4); v[1] = g->GetGene(i + 11); v[2] = g->GetGene(i + 18);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 8); v[5] = g->GetGene(i + 15);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftAnkleFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftAnkleExtensor", 6, d, v2);
        }

        if (m_ConversionType == EightMuscle)
        {
            // and create some new ones based on the genome
            double d[6], v[6], v2[6];
            int i;
            d[0] = d[3] = fabs(g->GetGene(0));
            d[1] = d[4] = fabs(g->GetGene(9));
            d[2] = d[5] = fabs(g->GetGene(18));

            i = 0;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 10); v[2] = g->GetGene(i + 19);
            v[3] = g->GetGene(i + 5); v[4] = g->GetGene(i + 14); v[5] = g->GetGene(i + 23);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightHipFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightHipExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 10); v[2] = g->GetGene(i + 19);
            v[3] = g->GetGene(i + 5); v[4] = g->GetGene(i + 14); v[5] = g->GetGene(i + 23);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightKneeFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightKneeExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 10); v[2] = g->GetGene(i + 19);
            v[3] = g->GetGene(i + 5); v[4] = g->GetGene(i + 14); v[5] = g->GetGene(i + 23);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightAnkleFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightAnkleExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 10); v[2] = g->GetGene(i + 19);
            v[3] = g->GetGene(i + 5); v[4] = g->GetGene(i + 14); v[5] = g->GetGene(i + 23);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightHamstring", 6, d, v2);

            i = 0;
            v[0] = g->GetGene(i + 5); v[1] = g->GetGene(i + 14); v[2] = g->GetGene(i + 23);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 10); v[5] = g->GetGene(i + 19);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftHipFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftHipExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 5); v[1] = g->GetGene(i + 14); v[2] = g->GetGene(i + 23);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 10); v[5] = g->GetGene(i + 19);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftKneeFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftKneeExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 5); v[1] = g->GetGene(i + 14); v[2] = g->GetGene(i + 23);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 10); v[5] = g->GetGene(i + 19);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftAnkleFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftAnkleExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 5); v[1] = g->GetGene(i + 14); v[2] = g->GetGene(i + 23);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 10); v[5] = g->GetGene(i + 19);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftHamstring", 6, d, v2);
        }

        if (m_ConversionType == TwelveMuscleQuad)
        {
            // and create some new ones based on the genome
            double d[6], v[6], v2[6];
            double phase;
            int i;
            d[0] = d[3] = fabs(g->GetGene(0));
            d[1] = d[4] = fabs(g->GetGene(13));
            d[2] = d[5] = fabs(g->GetGene(26));
            phase = fabs(g->GetGene(39));

            i = 0;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 14); v[2] = g->GetGene(i + 27);
            v[3] = g->GetGene(i + 7); v[4] = g->GetGene(i + 20); v[5] = g->GetGene(i + 33);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightHipFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightHipExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 14); v[2] = g->GetGene(i + 27);
            v[3] = g->GetGene(i + 7); v[4] = g->GetGene(i + 20); v[5] = g->GetGene(i + 33);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightKneeFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightKneeExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 14); v[2] = g->GetGene(i + 27);
            v[3] = g->GetGene(i + 7); v[4] = g->GetGene(i + 20); v[5] = g->GetGene(i + 33);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightAnkleFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightAnkleExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 14); v[2] = g->GetGene(i + 27);
            v[3] = g->GetGene(i + 7); v[4] = g->GetGene(i + 20); v[5] = g->GetGene(i + 33);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightShoulderFlexor", 6, d, v2, phase);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightShoulderExtensor", 6, d, v2, phase);

            i++;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 14); v[2] = g->GetGene(i + 27);
            v[3] = g->GetGene(i + 7); v[4] = g->GetGene(i + 20); v[5] = g->GetGene(i + 33);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightElbowFlexor", 6, d, v2, phase);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightElbowExtensor", 6, d, v2, phase);

            i++;
            v[0] = g->GetGene(i + 1); v[1] = g->GetGene(i + 14); v[2] = g->GetGene(i + 27);
            v[3] = g->GetGene(i + 7); v[4] = g->GetGene(i + 20); v[5] = g->GetGene(i + 33);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightWristFlexor", 6, d, v2, phase);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "RightWristExtensor", 6, d, v2, phase);

            i = 0;
            v[0] = g->GetGene(i + 7); v[1] = g->GetGene(i + 20); v[2] = g->GetGene(i + 33);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 14); v[5] = g->GetGene(i + 27);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftHipFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftHipExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 7); v[1] = g->GetGene(i + 20); v[2] = g->GetGene(i + 33);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 14); v[5] = g->GetGene(i + 27);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftKneeFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftKneeExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 7); v[1] = g->GetGene(i + 20); v[2] = g->GetGene(i + 33);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 14); v[5] = g->GetGene(i + 27);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftAnkleFlexor", 6, d, v2);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftAnkleExtensor", 6, d, v2);

            i++;
            v[0] = g->GetGene(i + 7); v[1] = g->GetGene(i + 20); v[2] = g->GetGene(i + 33);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 14); v[5] = g->GetGene(i + 27);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftShoulderFlexor", 6, d, v2, phase);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftShoulderExtensor", 6, d, v2, phase);

            i++;
            v[0] = g->GetGene(i + 7); v[1] = g->GetGene(i + 20); v[2] = g->GetGene(i + 33);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 14); v[5] = g->GetGene(i + 27);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftElbowFlexor", 6, d, v2, phase);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftElbowExtensor", 6, d, v2, phase);

            i++;
            v[0] = g->GetGene(i + 7); v[1] = g->GetGene(i + 20); v[2] = g->GetGene(i + 33);
            v[3] = g->GetGene(i + 1); v[4] = g->GetGene(i + 14); v[5] = g->GetGene(i + 27);
            FlexorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftWristFlexor", 6, d, v2, phase);
            ExtensorEncode(v, 6, v2);
            NewCyclicDriver(root, "LeftWristExtensor", 6, d, v2, phase);
        }

        if (m_ConversionType == TwentyFourMuscleQuadKinematic)
        {
            int i, j;
            int len = g->GetGenomeLength() / 25;
            double *d = new double[len];
            double *rightHipFlexor = new double[len];
            double *rightKneeFlexor = new double[len];
            double *rightAnkleFlexor = new double[len];
            double *rightShoulderFlexor = new double[len];
            double *rightElbowFlexor = new double[len];
            double *rightWristFlexor = new double[len];
            double *rightHipExtensor = new double[len];
            double *rightKneeExtensor = new double[len];
            double *rightAnkleExtensor = new double[len];
            double *rightShoulderExtensor = new double[len];
            double *rightElbowExtensor = new double[len];
            double *rightWristExtensor = new double[len];
            double *leftHipFlexor = new double[len];
            double *leftKneeFlexor = new double[len];
            double *leftAnkleFlexor = new double[len];
            double *leftShoulderFlexor = new double[len];
            double *leftElbowFlexor = new double[len];
            double *leftWristFlexor = new double[len];
            double *leftHipExtensor = new double[len];
            double *leftKneeExtensor = new double[len];
            double *leftAnkleExtensor = new double[len];
            double *leftShoulderExtensor = new double[len];
            double *leftElbowExtensor = new double[len];
            double *leftWristExtensor = new double[len];

            for (i = 0; i < len; i++)
            {
                if (i == 0) d[i] = 0;
                else d[i] = d[i - 1] + g->GetGene((i - 1) * 25);
                j = 1;
                rightHipFlexor[i] = g->GetGene(i * 25 + j++);
                rightKneeFlexor[i] = g->GetGene(i * 25 + j++);
                rightAnkleFlexor[i] = g->GetGene(i * 25 + j++);
                rightShoulderFlexor[i] = g->GetGene(i * 25 + j++);
                rightElbowFlexor[i] = g->GetGene(i * 25 + j++);
                rightWristFlexor[i] = g->GetGene(i * 25 + j++);
                rightHipExtensor[i] = g->GetGene(i * 25 + j++);
                rightKneeExtensor[i] = g->GetGene(i * 25 + j++);
                rightAnkleExtensor[i] = g->GetGene(i * 25 + j++);
                rightShoulderExtensor[i] = g->GetGene(i * 25 + j++);
                rightElbowExtensor[i] = g->GetGene(i * 25 + j++);
                rightWristExtensor[i] = g->GetGene(i * 25 + j++);
                leftHipFlexor[i] = g->GetGene(i * 25 + j++);
                leftKneeFlexor[i] = g->GetGene(i * 25 + j++);
                leftAnkleFlexor[i] = g->GetGene(i * 25 + j++);
                leftShoulderFlexor[i] = g->GetGene(i * 25 + j++);
                leftElbowFlexor[i] = g->GetGene(i * 25 + j++);
                leftWristFlexor[i] = g->GetGene(i * 25 + j++);
                leftHipExtensor[i] = g->GetGene(i * 25 + j++);
                leftKneeExtensor[i] = g->GetGene(i * 25 + j++);
                leftAnkleExtensor[i] = g->GetGene(i * 25 + j++);
                leftShoulderExtensor[i] = g->GetGene(i * 25 + j++);
                leftElbowExtensor[i] = g->GetGene(i * 25 + j++);
                leftWristExtensor[i] = g->GetGene(i * 25 + j++);
            }

            NewStepDriver(root, "RightHipFlexor", len, d, rightHipFlexor);
            NewStepDriver(root, "RightKneeFlexor", len, d, rightKneeFlexor);
            NewStepDriver(root, "RightAnkleFlexor", len, d, rightAnkleFlexor);
            NewStepDriver(root, "RightShoulderFlexor", len, d, rightShoulderFlexor);
            NewStepDriver(root, "RightElbowFlexor", len, d, rightElbowFlexor);
            NewStepDriver(root, "RightWristFlexor", len, d, rightWristFlexor);
            NewStepDriver(root, "RightHipExtensor", len, d, rightHipExtensor);
            NewStepDriver(root, "RightKneeExtensor", len, d, rightKneeExtensor);
            NewStepDriver(root, "RightAnkleExtensor", len, d, rightAnkleExtensor);
            NewStepDriver(root, "RightShoulderExtensor", len, d, rightShoulderExtensor);
            NewStepDriver(root, "RightElbowExtensor", len, d, rightElbowExtensor);
            NewStepDriver(root, "RightWristExtensor", len, d, rightWristExtensor);
            NewStepDriver(root, "LeftHipFlexor", len, d, leftHipFlexor);
            NewStepDriver(root, "LeftKneeFlexor", len, d, leftKneeFlexor);
            NewStepDriver(root, "LeftAnkleFlexor", len, d, leftAnkleFlexor);
            NewStepDriver(root, "LeftShoulderFlexor", len, d, leftShoulderFlexor);
            NewStepDriver(root, "LeftElbowFlexor", len, d, leftElbowFlexor);
            NewStepDriver(root, "LeftWristFlexor", len, d, leftWristFlexor);
            NewStepDriver(root, "LeftHipExtensor", len, d, leftHipExtensor);
            NewStepDriver(root, "LeftKneeExtensor", len, d, leftKneeExtensor);
            NewStepDriver(root, "LeftAnkleExtensor", len, d, leftAnkleExtensor);
            NewStepDriver(root, "LeftShoulderExtensor", len, d, leftShoulderExtensor);
            NewStepDriver(root, "LeftElbowExtensor", len, d, leftElbowExtensor);
            NewStepDriver(root, "LeftWristExtensor", len, d, leftWristExtensor);

            delete [] rightHipFlexor;
            delete [] rightKneeFlexor;
            delete [] rightAnkleFlexor;
            delete [] rightShoulderFlexor;
            delete [] rightElbowFlexor;
            delete [] rightWristFlexor;
            delete [] rightHipExtensor;
            delete [] rightKneeExtensor;
            delete [] rightAnkleExtensor;
            delete [] rightShoulderExtensor;
            delete [] rightElbowExtensor;
            delete [] rightWristExtensor;
            delete [] leftHipFlexor;
            delete [] leftKneeFlexor;
            delete [] leftAnkleFlexor;
            delete [] leftShoulderFlexor;
            delete [] leftElbowFlexor;
            delete [] leftWristFlexor;
            delete [] leftHipExtensor;
            delete [] leftKneeExtensor;
            delete [] leftAnkleExtensor;
            delete [] leftShoulderExtensor;
            delete [] leftElbowExtensor;
            delete [] leftWristExtensor;
        }

        if (m_ConversionType == TwelveMuscleQuadKinematic)
        {
            int i, j;
            int len = g->GetGenomeLength() / 13;
            double *d = new double[len];
            double *rightHipFlexor = new double[len];
            double *rightKneeFlexor = new double[len];
            double *rightAnkleFlexor = new double[len];
            double *rightShoulderFlexor = new double[len];
            double *rightElbowFlexor = new double[len];
            double *rightWristFlexor = new double[len];
            double *rightHipExtensor = new double[len];
            double *rightKneeExtensor = new double[len];
            double *rightAnkleExtensor = new double[len];
            double *rightShoulderExtensor = new double[len];
            double *rightElbowExtensor = new double[len];
            double *rightWristExtensor = new double[len];
            double *leftHipFlexor = new double[len];
            double *leftKneeFlexor = new double[len];
            double *leftAnkleFlexor = new double[len];
            double *leftShoulderFlexor = new double[len];
            double *leftElbowFlexor = new double[len];
            double *leftWristFlexor = new double[len];
            double *leftHipExtensor = new double[len];
            double *leftKneeExtensor = new double[len];
            double *leftAnkleExtensor = new double[len];
            double *leftShoulderExtensor = new double[len];
            double *leftElbowExtensor = new double[len];
            double *leftWristExtensor = new double[len];

            for (i = 0; i < len; i++)
            {
                if (i == 0) d[i] = 0;
                else d[i] = d[i - 1] + g->GetGene((i - 1) * 13);
                j = 1;
                rightHipFlexor[i] = g->GetGene(i * 13 + j++);
                rightKneeFlexor[i] = g->GetGene(i * 13 + j++);
                rightAnkleFlexor[i] = g->GetGene(i * 13 + j++);
                rightShoulderFlexor[i] = g->GetGene(i * 13 + j++);
                rightElbowFlexor[i] = g->GetGene(i * 13 + j++);
                rightWristFlexor[i] = g->GetGene(i * 13 + j++);
                leftHipFlexor[i] = g->GetGene(i * 13 + j++);
                leftKneeFlexor[i] = g->GetGene(i * 13 + j++);
                leftAnkleFlexor[i] = g->GetGene(i * 13 + j++);
                leftShoulderFlexor[i] = g->GetGene(i * 13 + j++);
                leftElbowFlexor[i] = g->GetGene(i * 13 + j++);
                leftWristFlexor[i] = g->GetGene(i * 13 + j++);
            }

            ExtensorEncode(rightHipFlexor, len, rightHipExtensor);
            ExtensorEncode(rightKneeFlexor, len, rightKneeExtensor);
            ExtensorEncode(rightAnkleFlexor, len, rightAnkleExtensor);
            ExtensorEncode(rightShoulderFlexor, len, rightShoulderExtensor);
            ExtensorEncode(rightElbowFlexor, len, rightElbowExtensor);
            ExtensorEncode(rightWristFlexor, len, rightWristExtensor);
            ExtensorEncode(leftHipFlexor, len, leftHipExtensor);
            ExtensorEncode(leftKneeFlexor, len, leftKneeExtensor);
            ExtensorEncode(leftAnkleFlexor, len, leftAnkleExtensor);
            ExtensorEncode(leftShoulderFlexor, len, leftShoulderExtensor);
            ExtensorEncode(leftElbowFlexor, len, leftElbowExtensor);
            ExtensorEncode(leftWristFlexor, len, leftWristExtensor);

            FlexorEncode(rightHipFlexor, len, rightHipFlexor);
            FlexorEncode(rightKneeFlexor, len, rightKneeFlexor);
            FlexorEncode(rightAnkleFlexor, len, rightAnkleFlexor);
            FlexorEncode(rightShoulderFlexor, len, rightShoulderFlexor);
            FlexorEncode(rightElbowFlexor, len, rightElbowFlexor);
            FlexorEncode(rightWristFlexor, len, rightWristFlexor);
            FlexorEncode(leftHipFlexor, len, leftHipFlexor);
            FlexorEncode(leftKneeFlexor, len, leftKneeFlexor);
            FlexorEncode(leftAnkleFlexor, len, leftAnkleFlexor);
            FlexorEncode(leftShoulderFlexor, len, leftShoulderFlexor);
            FlexorEncode(leftElbowFlexor, len, leftElbowFlexor);
            FlexorEncode(leftWristFlexor, len, leftWristFlexor);

            NewStepDriver(root, "RightHipFlexor", len, d, rightHipFlexor);
            NewStepDriver(root, "RightKneeFlexor", len, d, rightKneeFlexor);
            NewStepDriver(root, "RightAnkleFlexor", len, d, rightAnkleFlexor);
            NewStepDriver(root, "RightShoulderFlexor", len, d, rightShoulderFlexor);
            NewStepDriver(root, "RightElbowFlexor", len, d, rightElbowFlexor);
            NewStepDriver(root, "RightWristFlexor", len, d, rightWristFlexor);
            NewStepDriver(root, "RightHipExtensor", len, d, rightHipExtensor);
            NewStepDriver(root, "RightKneeExtensor", len, d, rightKneeExtensor);
            NewStepDriver(root, "RightAnkleExtensor", len, d, rightAnkleExtensor);
            NewStepDriver(root, "RightShoulderExtensor", len, d, rightShoulderExtensor);
            NewStepDriver(root, "RightElbowExtensor", len, d, rightElbowExtensor);
            NewStepDriver(root, "RightWristExtensor", len, d, rightWristExtensor);
            NewStepDriver(root, "LeftHipFlexor", len, d, leftHipFlexor);
            NewStepDriver(root, "LeftKneeFlexor", len, d, leftKneeFlexor);
            NewStepDriver(root, "LeftAnkleFlexor", len, d, leftAnkleFlexor);
            NewStepDriver(root, "LeftShoulderFlexor", len, d, leftShoulderFlexor);
            NewStepDriver(root, "LeftElbowFlexor", len, d, leftElbowFlexor);
            NewStepDriver(root, "LeftWristFlexor", len, d, leftWristFlexor);
            NewStepDriver(root, "LeftHipExtensor", len, d, leftHipExtensor);
            NewStepDriver(root, "LeftKneeExtensor", len, d, leftKneeExtensor);
            NewStepDriver(root, "LeftAnkleExtensor", len, d, leftAnkleExtensor);
            NewStepDriver(root, "LeftShoulderExtensor", len, d, leftShoulderExtensor);
            NewStepDriver(root, "LeftElbowExtensor", len, d, leftElbowExtensor);
            NewStepDriver(root, "LeftWristExtensor", len, d, leftWristExtensor);

            delete [] rightHipFlexor;
            delete [] rightKneeFlexor;
            delete [] rightAnkleFlexor;
            delete [] rightShoulderFlexor;
            delete [] rightElbowFlexor;
            delete [] rightWristFlexor;
            delete [] rightHipExtensor;
            delete [] rightKneeExtensor;
            delete [] rightAnkleExtensor;
            delete [] rightShoulderExtensor;
            delete [] rightElbowExtensor;
            delete [] rightWristExtensor;
            delete [] leftHipFlexor;
            delete [] leftKneeFlexor;
            delete [] leftAnkleFlexor;
            delete [] leftShoulderFlexor;
            delete [] leftElbowFlexor;
            delete [] leftWristFlexor;
            delete [] leftHipExtensor;
            delete [] leftKneeExtensor;
            delete [] leftAnkleExtensor;
            delete [] leftShoulderExtensor;
            delete [] leftElbowExtensor;
            delete [] leftWristExtensor;
        }

        if (m_DocTxtPtr) xmlFree(m_DocTxtPtr);
        xmlDocDumpFormatMemory(m_Doc, &m_DocTxtPtr, &m_DocTxtLen, 1);
    }
    return 0;    
}

void XMLConverter::NewCyclicDriver(xmlNodePtr parent, char *target,
                     int n, double *d, double *v, double phase)
{
    xmlNodePtr newNode;
    xmlAttrPtr newAttr;
    char buffer[256];
    ostringstream out;
    int i;
    out.precision(DBL_DIG + 2);
    assert(phase >= 0 && phase <= 1);

    newNode = xmlNewTextChild(parent, 0, (xmlChar *)"CYCLIC_DRIVER", 0);
    strncpy(buffer, target, 200);
    strcat(buffer, "Driver");
    newAttr = xmlNewProp(newNode, (xmlChar *)"Name", (xmlChar *)buffer);
    newAttr = xmlNewProp(newNode, (xmlChar *)"Target", (xmlChar *)target);

    if (phase != 0)
    {
        double *d2 = new double[n + 1];
        double *v2 = new double[n + 1];
        double totalDuration = 0;
        for (i = 0; i < n; i++) totalDuration += d[i];
        double durationOffset = totalDuration * phase;
        totalDuration = 0;
        for (i = 0; i < n; i++)
        {
            totalDuration += d[i];
            if (totalDuration >= durationOffset) break;
        }
        int split = i;
        int index = 0;
        d2[index] = totalDuration - durationOffset;
        v2[index] = v[split];
        index++;
        for (i = split + 1; i < n; i++)
        {
            d2[index] = d[i];
            v2[index] = v[i];
            index++;
        }
        for (i = 0; i < split; i++)
        {
            d2[index] = d[i];
            v2[index] = v[i];
            index++;
        }
        d2[index] = d[split] - d2[0];
        v2[index] = v[split];

        for (i = 0; i < n + 1; i++) out << d2[i] << " " << v2[i] << " ";
        
        delete [] d2;
        delete [] v2;
    }
    else
    {
        for (i = 0; i < n; i++) out << d[i] << " " << v[i] << " ";
    }
    
    newAttr = xmlNewProp(newNode, (xmlChar *)"DurationValuePairs", (xmlChar *)out.str().c_str());
}

void XMLConverter::NewStepDriver(xmlNodePtr parent, char *target,
                                   int n, double *d, double *v)
{
    xmlNodePtr newNode;
    xmlAttrPtr newAttr;
    char buffer[256];
ostringstream out;
    int i;
    out.precision(DBL_DIG + 2);

    newNode = xmlNewTextChild(parent, 0, (xmlChar *)"STEP_DRIVER", 0);
    strncpy(buffer, target, 200);
    strcat(buffer, "Driver");
    newAttr = xmlNewProp(newNode, (xmlChar *)"Name", (xmlChar *)buffer);
    newAttr = xmlNewProp(newNode, (xmlChar *)"Target", (xmlChar *)target);

    for (i = 0; i < n; i++) out << d[i] << " " << v[i] << " ";

    newAttr = xmlNewProp(newNode, (xmlChar *)"DurationValuePairs", (xmlChar *)out.str().c_str());
}

void XMLConverter::FlexorEncode(double *in, int n, double *out)
{
    int i;
    for (i = 0; i < n; i++) out[i] = in[i] > 0 ? in[i]: 0;
}

void XMLConverter::ExtensorEncode(double *in, int n, double *out)
{
    int i;
    for (i = 0; i < n; i++) out[i] = in[i] < 0 ? -in[i]: 0;
}

// perform the smart substitution
// returns 0 on no error
void XMLConverter::DoSmartSubstitution(char *dataPtr)
{
    char *ptr1 = dataPtr;
    string *s;
    ExpressionParser *expressionParser;
    int length = strlen(dataPtr);

    char *ptr2 = strstr(ptr1, "[[");
    while (ptr2)
    {
        *ptr2 = 0;
        s = new string(ptr1);
        m_SmartSubstitutionTextComponents.push_back(s);

        ptr2 += 2;
        ptr1 = strstr(ptr2, "]]");
        if (ptr2 == 0)
        {
            cerr << "Error: could not find matching ]]\n";
            exit(1);
        }
        expressionParser = new ExpressionParser();
        expressionParser->CreateFromString(ptr2, ptr1 - ptr2);
        m_SmartSubstitutionParserComponents.push_back(expressionParser);
        m_SmartSubstitutionValues.push_back(0); // dummy values
        ptr1 += 2;
        ptr2 = strstr(ptr1, "[[");
    }
    s = new string(ptr1);
    m_SmartSubstitutionTextComponents.push_back(s);

    // make the text buffer plenty big enough
    m_SmartSubstitutionTextBuffer = new char[length + m_SmartSubstitutionValues.size() * 64];
}

    



