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

def GetValuesFromTokenList(tokenList, match, numberOfValues):
    """returns value after the matching token in the tokenList (match + 1)
       raises NoMatch on error"""
    for i in range(0, len(tokenList) - 1):
        if tokenList[i] == match: 
            values = []
            for j in range(0, numberOfValues):
                values[j] = tokenList[i + 1 + j]
            return values
        
    raise "NoMatch", match
    
def GetNumbersFromTokenList(tokenList, match):
    """returns value after the matching token in the tokenList (match + 1)
       raises NoMatch on error"""
    for i in range(0, len(tokenList) - 1):
        if tokenList[i] == match: 
            values = []
            c = 1
            while IsANumber(tokenList[i + c]):
                values.append(float(tokenList[i + c]))
                c = c + 1
            return values
        
    raise "NoMatch", match
    
def IsANumber(theString):
    """checks to see whether a string is a valid number"""
    if re.match('([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?', theString) == None: return 0
    return 1    

def main():
    usageString = "Usage:\n%s [--controlFile file]" % sys.argv[0]

    controlFile = "kmcontrol.txt";

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
        timesWanted = GetNumbersFromTokenList(tokenList, "timesWanted")
        sdForCurrent = float(GetValueFromTokenList(tokenList, "sdForCurrent"))
        sdForOld = float(GetValueFromTokenList(tokenList, "sdForOld"))
        timeStringToReplace = GetValueFromTokenList(tokenList, "timeStringToReplace")
        
    
        rootConfigFile = GetValueFromTokenList(tokenList, "rootConfigFile")
        rootPopulationFile = GetValueFromTokenList(tokenList, "rootPopulationFile")
        genesPerTime = int(GetValueFromTokenList(tokenList, "genesPerTime"))
        
        parameterFile = GetValueFromTokenList(tokenList, "parameterFile")
        
        workingConfigFile = GetValueFromTokenList(tokenList, "workingConfigFile")
        workingPopulationFile = GetValueFromTokenList(tokenList, "workingPopulationFile")
        
        gaCommand = GetValueFromTokenList(tokenList, "gaCommand")
        
    except "NoMatch", data:
        PrintExit("Error: %s undefined" % data)
    
    lastDirectory = ""
    for i in range(0, len(timesWanted)):
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
        
        # set the sd limits for the population
	print "Modifying " + startPopulation + " to produce " + workingPopulationFile
        populationTokenList = ConvertFileToTokens(startPopulation)
	populationLength = int(populationTokenList[0])
	genomeLength = int(populationTokenList[2])
        index = 1
        for j in range(0, populationLength):
            # parse the genome
            if int(populationTokenList[index]) != -1: PrintExit("Error: parsing %s" % startPopulation)
            index = index + 1
            startPopulationGenomeLength = int(populationTokenList[index])
            index = index + 1
            for k in range(0, startPopulationGenomeLength):
                # populationTokenList[index] =  # gene
                index = index + 1
                # populationTokenList[index] =  # low bound
                index = index + 1
                # populationTokenList[index] =  # high bound
                index = index + 1

                theSD = sdForCurrent
                if k < genesPerTime * i: theSD = sdForOld
                if k >= genesPerTime * (i + 1): theSD = 0
                populationTokenList[index] = theSD # SD 
                index = index + 1

            for k in range(0, 5):
                #populationTokenList[index] =  # fitness + parents
                index = index + 1

        WriteTokenList(workingPopulationFile, populationTokenList)
        
        # set the time limit for the simulation
	print "Modifying " + rootConfigFile + " to produce " + workingConfigFile
        theInput = open(rootConfigFile, 'r')
        theData = theInput.read()
        theInput.close()
        
        if string.find(theData, timeStringToReplace) == -1: PrintExit(timeStringToReplace + " not found in " + rootConfigFile)
        theData = string.replace(theData, timeStringToReplace, "%f" % (timesWanted[i]))
        theOutput = open(workingConfigFile, 'w')
        theOutput.write(theData)
        theOutput.close()
 
         # create the directory name
        theTime = time.localtime(time.time())
        lastDirectory = "%07.4f_Run_%04d-%02d-%02d_%02d.%02d.%02d" % (timesWanted[i], theTime[0], theTime[1], theTime[2], theTime[3], theTime[4], theTime[5])
        os.makedirs(lastDirectory)
        
        # run the GA program
        command = "%s -p %s -o %s" % (gaCommand, parameterFile, lastDirectory)
	print command
        os.system(command)  
       

# program starts here

main()
   

