/*
 *  MergeXML.cc
 *  MergeXML
 *
 *  Created by Bill Sellers on Wed Dec 17 2003.
 *  Copyright (c) 2003 Bill Sellers. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <fstream>

using namespace std;

#include "SimulationContainer.h"

static const char* gUsage = "MergeXML inFile1 inFile2 proportionInFile2 outFile\n";

int main(int argc, char *argv[])
{
    char *inFile1;
    char *inFile2;
    double proportionInFile2;
    char *outFile;
    SimulationContainer sim1;
    SimulationContainer sim2;
    
    try
    {
        if (argc != 5) throw 0;

        inFile1 = argv[1];
        inFile2 = argv[2];
        proportionInFile2 = strtod(argv[3], 0);
        outFile = argv[4];
        
    }

    catch (int e)
    {
        cerr << gUsage << "\n";
        return 1;
    }

    sim1.LoadXML(inFile1);
    sim2.LoadXML(inFile2);
    sim1.Merge(&sim2, proportionInFile2);

    sim1.WriteXML(outFile);

    return 0;
}



