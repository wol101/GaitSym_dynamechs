/*
 *  XMLConverter.h
 *  GA
 *
 *  Created by Bill Sellers on Fri Dec 12 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 */

#ifndef XMLConverter_h
#define XMLConverter_h

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <vector>
#include <string>

class Genome;
class DataFile;
class ExpressionParser;

enum ConversionType
{
    SixMuscle = 0,
    EightMuscle = 1,
    TwelveMuscleQuad = 2,
    TwentyFourMuscleQuadKinematic = 3,
    TwelveMuscleQuadKinematic = 4,
    SmartSubstitution = 5
};

class XMLConverter
{
public:
    XMLConverter();
    ~XMLConverter();

    int LoadBaseXMLFile(const char *filename);
    int ApplyGenome(Genome *genome);
    xmlChar* GetFormattedXML(int * docTxtLen);

    void SetConversionType(ConversionType c) { m_ConversionType = c; };
    
protected:

    void NewCyclicDriver(xmlNodePtr parent, char *target,
                         int n, double *d, double *v,
                         double phase = 0);
    void NewStepDriver(xmlNodePtr parent, char *target,
                         int n, double *d, double *v);
    
    void FlexorEncode(double *in, int n, double *out);
    void ExtensorEncode(double *in, int n, double *out);

    void DoSmartSubstitution(char *dataPtr);
    
    xmlDocPtr m_Doc;
    xmlChar *m_DocTxtPtr;
    int m_DocTxtLen;
    bool m_DTDValidateFlag;

    vector<string *> m_SmartSubstitutionTextComponents;
    vector<ExpressionParser *> m_SmartSubstitutionParserComponents;
    vector<double> m_SmartSubstitutionValues;
    char *m_SmartSubstitutionTextBuffer;

    ConversionType m_ConversionType;
};


#endif
