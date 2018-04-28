#!/usr/bin/python -u

import sys
import string
import array
import os
import re
import time
import math
import random

def CopyFile(theInputFileName, theOutputFileName):
    """Copies the contents of a file"""
    theInput = open(theInputFileName, 'r')
    theData = theInput.read()
    theInput.close()
    theOutput = open(theOutputFileName, 'w')
    theOutput.write(theData)
    theOutput.close()

def StartsWith(theString, thePrefix):
    if theString[0: len(thePrefix)] == thePrefix:
        return 1
    return 0

def EndsWith(theString, theSuffix):
    if theString[len(theString) - len(theSuffix):] == theSuffix:
        return 1
    return 0

def PrintExit(value):
    """exits with error message"""
    sys.stderr.write(str(value) + "\n");
    sys.exit(1)

def WriteTokenList(filename, tokenList):
    theOutput = open(filename, 'w')
    for i in range(0, len(tokenList)):
        theOutput.write(str(tokenList[i]) + "\n")
    theOutput.close()
    

def ConvertToListOfTokens(theBuffer):
    """converts a string into a list of tokens delimited by whitespace"""
    
    charList = list(theBuffer)
    i = 0
    tokenList = []
    while i < len(charList):
        byte = charList[i]
        if byte in string.whitespace: 
            i = i + 1
            continue
        word = ""
        if byte == '"':
            i = i + 1
            byte = charList[i]
            while byte != '"':
                word = word + byte
                i = i + 1
                if i >= len(charList): PrintExit("Unmatched \" error")
                byte = charList[i]
        else:
            while (byte in string.whitespace) == 0:
                word = word + byte
                i = i + 1
                if i >= len(charList): break
                byte = charList[i]
        
        if len(word) > 0: tokenList.append(word)
        i = i + 1
            
    return tokenList        

def ConvertFileToTokens(filename):
    """reads in a file and converts to a list of whitespace delimited tokens"""
    
    input = open(filename, 'r')
    theBuffer = input.read()
    input.close()
    
    return ConvertToListOfTokens(theBuffer)
    
def ConvertToListOfTokensIgnoreQuotes(theBuffer):
    """converts a string into a list of tokens delimited by whitespace"""
    
    charList = list(theBuffer)
    i = 0
    tokenList = []
    while i < len(charList):
        byte = charList[i]
        if byte in string.whitespace: 
            i = i + 1
            continue
        word = ""
        while (byte in string.whitespace) == 0:
            word = word + byte
            i = i + 1
            if i >= len(charList): break
            byte = charList[i]
        
        if len(word) > 0: tokenList.append(word)
        i = i + 1
            
    return tokenList        

def ConvertFileToTokensIgnoreQuotes(filename):
    """reads in a file and converts to a list of whitespace delimited tokens"""
    
    input = open(filename, 'r')
    theBuffer = input.read()
    input.close()
    
    return ConvertToListOfTokensIgnoreQuotes(theBuffer)  
     
def GetIndexFromTokenList(tokenList, match):
    """returns the index pointing to the value in the tokenList (match + 1)
       raises NoMatch on error"""
    for i in range(0, len(tokenList) - 1):
        if tokenList[i] == match: return i + 1
        
    raise "NoMatch", match

def GetValueFromTokenList(tokenList, match):
    """returns value after the matching token in the tokenList (match + 1)
       raises NoMatch on error"""
    for i in range(0, len(tokenList) - 1):
        if tokenList[i] == match: return tokenList[i + 1]
        
    raise "NoMatch", match

def IsANumber(theString):
    """checks to see whether a string is a valid number"""
    if re.match('([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?', theString) == None: return 0
    return 1    

def main():
    usageString = "Usage:\n%s [--controlFile file]" % sys.argv[0]

    controlFile = "control.txt";

    i = 1;
    while i < len(sys.argv):
        if sys.argv[i] == "--controlFile":
            i = i + 1
            if i >= len(sys.argv): PrintExit(usageString)
            controlFile = sys.argv[i] 
            i = i + 1
            continue 

        PrintExit(usageString)

    # read and parse the control file
    tokenList = ConvertFileToTokens(controlFile)

    try:
        timeIncrement = float(GetValueFromTokenList(tokenList, "timeIncrement"))
        sdReductionFactor = float(GetValueFromTokenList(tokenList, "sdReductionFactor"))
        numberOfRepeats = int(GetValueFromTokenList(tokenList, "numberOfRepeats"))
    
        rootConfigFile = GetValueFromTokenList(tokenList, "rootConfigFile")
        rootParameterFile = GetValueFromTokenList(tokenList, "rootParameterFile")
        rootGenomeFile = GetValueFromTokenList(tokenList, "rootGenomeFile")
        rootPopulationFile = GetValueFromTokenList(tokenList, "rootPopulationFile")
        
        workingConfigFile = GetValueFromTokenList(tokenList, "workingConfigFile")
        workingParameterFile = GetValueFromTokenList(tokenList, "workingParameterFile")
        workingPopulation = GetValueFromTokenList(tokenList, "workingPopulation")
        
        gaCommand = GetValueFromTokenList(tokenList, "gaCommand")
        objectiveCommand = GetValueFromTokenList(tokenList, "objectiveCommand")
        mergeCommand = GetValueFromTokenList(tokenList, "mergeCommand")
        
        populateFromBestN = int(GetValueFromTokenList(tokenList, "populateFromBestN"))

    except "NoMatch", data:
        PrintExit("Error: %s undefined" % data)

    modelGenomeTokens = ConvertFileToTokens(rootGenomeFile)
    if int(modelGenomeTokens[0]) != -1: PrintExit("Error: parsing %s" % rootGenomeFile)
    modelGenomeLength = int(modelGenomeTokens[1])

    timeLimit = 0
    lastDirectory = ""
    for i in range(0, numberOfRepeats):
        timeLimit = timeLimit + timeIncrement
        
        # get name of start population
        if len(lastDirectory) > 0:
            fileList = os.listdir(lastDirectory)
            fileList.sort()
            fileList.reverse()
            for name in fileList:
                if StartsWith(name, "Population_") and EndsWith(name, ".txt"):
                    startPopulation =  lastDirectory + "/" + name
                    break
        # special first case
        else:
            startPopulation = rootPopulationFile
        
        # add extra section to each genome in the population
        populationTokenList = ConvertFileToTokens(startPopulation)
	populationLength = int(populationTokenList[0])
	genomeLength = int(populationTokenList[2])
        workingPopulationTokenList = [populationLength]
        if populateFromBestN > 0:
            index = 1
            startBestIndex = 1 + (populationLength - populateFromBestN) * (genomeLength * 4 + 7)
            bestIndex = startBestIndex
            for j in range(0, populationLength):
                # parse the genome
                if int(populationTokenList[index]) != -1: PrintExit("Error: parsing %s" % startPopulation)
                if int(populationTokenList[bestIndex]) != -1: PrintExit("Error: parsing %s" % startPopulation)
                index = index + 1
                bestIndex = bestIndex + 1
                startPopulationGenomeLength = int(populationTokenList[index])
                index = index + 1
                bestIndex = bestIndex + 1
                for k in range(0, startPopulationGenomeLength):
                    populationTokenList[index] = populationTokenList[bestIndex] # gene
                    index = index + 1
                    bestIndex = bestIndex + 1
                    populationTokenList[index] = populationTokenList[bestIndex] # low bound
                    index = index + 1
                    bestIndex = bestIndex + 1
                    populationTokenList[index] = populationTokenList[bestIndex] # high bound
                    index = index + 1
                    bestIndex = bestIndex + 1
                    populationTokenList[index] = populationTokenList[bestIndex] # SD 
                    index = index + 1
                    bestIndex = bestIndex + 1
                for k in range(0, 5):
                    populationTokenList[index]  = populationTokenList[bestIndex] # fitness + parents
                    index = index + 1
                    bestIndex = bestIndex + 1            
            	if bestIndex >= 1 + populationLength * (genomeLength * 4 + 7):
                    bestIndex = startBestIndex
            
        index = 1
        for j in range(0, populationLength):
            # parse the genome
            if int(populationTokenList[index]) != -1: PrintExit("Error: parsing %s" % startPopulation)
            workingPopulationTokenList.append(int(populationTokenList[index]))
            index = index + 1
            startPopulationGenomeLength = int(populationTokenList[index])
            index = index + 1
	    newPopulationGenomeLength = startPopulationGenomeLength + modelGenomeLength
            workingPopulationTokenList.append(newPopulationGenomeLength)
            for k in range(0, startPopulationGenomeLength):
                workingPopulationTokenList.append(populationTokenList[index]) # gene
                index = index + 1
                workingPopulationTokenList.append(populationTokenList[index]) # low bound
                index = index + 1
                workingPopulationTokenList.append(populationTokenList[index]) # high bound
                index = index + 1
                workingPopulationTokenList.append(float(populationTokenList[index]) * sdReductionFactor) # SD 
                index = index + 1
            for k in range(0, 5):
                # workingPopulationTokenList.append(populationTokenList[index]) # fitness + parents
                index = index + 1
            # and add the model
            modelIndex = 2
            for k in range(0, modelGenomeLength):
                if (float(modelGenomeTokens[modelIndex + 3]) == 0): 
                    workingPopulationTokenList.append(modelGenomeTokens[modelIndex]) # gene
                else:
                    workingPopulationTokenList.append(random.uniform(float(modelGenomeTokens[modelIndex + 1]), float(modelGenomeTokens[modelIndex + 2])))
                modelIndex = modelIndex + 1
                workingPopulationTokenList.append(modelGenomeTokens[modelIndex]) # low bound
                modelIndex = modelIndex + 1
                workingPopulationTokenList.append(modelGenomeTokens[modelIndex]) # high bound
                modelIndex = modelIndex + 1
                workingPopulationTokenList.append(modelGenomeTokens[modelIndex]) # SD 
                modelIndex = modelIndex + 1
            for k in range(0, 5):
                workingPopulationTokenList.append(modelGenomeTokens[modelIndex]) # fitness + parents
                modelIndex = modelIndex + 1
        
        # write the working population        
        WriteTokenList(workingPopulation, workingPopulationTokenList)
        
        # write the working parameter file
        theOutput = open(workingParameterFile, 'w')
        theOutput.write("startingPopulation %s\n" % workingPopulation)
        theOutput.write("extraDataFile %s\n" % workingConfigFile)
        theOutput.write("genomeLength %d\n" % newPopulationGenomeLength)
	theOutput.write("\n")
        theInput = open(rootParameterFile, 'r');
        theData = theInput.read();
        theInput.close();
        theOutput.write(theData);
        theOutput.close();
        
        # write a dummy XML file containing the required time limit
        tempFile = "dummy.xml"
        output = open(tempFile, 'w')
        output.write('<?xml version="1.0"?>\n<GAITSYM>\n<GLOBAL TimeLimit="%f"/>\n</GAITSYM>\n' % (timeLimit))
        output.close()
        
        # write the working config file
        command = "%s %s %s %s %s" % (mergeCommand, rootConfigFile, tempFile, 1, workingConfigFile)
        print command
        os.system(command)
        
        # create the directory name
        theTime = time.localtime(time.time())
        lastDirectory = "%04d_Run_%04d-%02d-%02d_%02d.%02d.%02d" % (i, theTime[0], theTime[1], theTime[2], theTime[3], theTime[4], theTime[5])
        os.makedirs(lastDirectory)
        
        # run the GA program
        command = "%s -p %s -o %s" % (gaCommand, workingParameterFile, lastDirectory)
        print command
        os.system(command)  

# program starts here

main()
   

