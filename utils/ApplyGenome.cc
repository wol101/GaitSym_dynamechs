// ApplyGenome.cc - produce a XML file from a genome in just the 

using namespace std;

#include <iostream>
#include <fstream>

#include "Genome.h"
#include "XMLConverter.h"

char *g_Usage = "Usage: ApplyGenome genome modelXML outputXML\n";

int main(int argc, char *argv[])
{

    if (argc != 4)
    {
        cerr << g_Usage;
        return 1;
    }
    
    char *genomeFileName = argv[1];
    char *modelXMLFileName = argv[2];
    char *outputXMLFileName = argv[3];
    
    Genome genome;
    ifstream genomeStream;
    genomeStream.open(genomeFileName);
    genomeStream >> genome;
    genomeStream.close();
    
    XMLConverter xmlConverter;
    xmlConverter.SetConversionType(SmartSubstitution);
    xmlConverter.LoadBaseXMLFile(modelXMLFileName);
    xmlConverter.ApplyGenome(&genome);
    int docTxtLen;
    xmlChar* ptr = xmlConverter.GetFormattedXML(&docTxtLen);
    
    ofstream outputXMLStream;
    outputXMLStream.open(outputXMLFileName);
    outputXMLStream.write((const char *)ptr, docTxtLen);
    outputXMLStream.close();
}
            
