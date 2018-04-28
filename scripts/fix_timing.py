#!/usr/bin/python -u

import sys
import string
import array
import os
import re
import time

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
    usageString = "Usage:\n%s filelist" % sys.argv[0]

    if len(sys.argv) < 2:
        PrintExit(usageString)
        
    i = 1;
    while i < len(sys.argv):
        populationTokenList = ConvertFileToTokens(sys.argv[i])
        populationLength = int(populationTokenList[0])
        genomeLength = int(populationTokenList[2])
        for j in range(0, populationLength):
            populationTokenList[3 + (j * (genomeLength * 4 + 7))] = abs(float(populationTokenList[3 + (j * (genomeLength * 4 + 7))]))
            populationTokenList[31 + (j * (genomeLength * 4 + 7))] = abs(float(populationTokenList[31 + (j * (genomeLength * 4 + 7))]))
            populationTokenList[59 + (j * (genomeLength * 4 + 7))] = abs(float(populationTokenList[59 + (j * (genomeLength * 4 + 7))]))
            populationTokenList[4 + (j * (genomeLength * 4 + 7))] = 0.05
            populationTokenList[32 + (j * (genomeLength * 4 + 7))] = 0.05
            populationTokenList[60 + (j * (genomeLength * 4 + 7))] = 0.05
        WriteTokenList(sys.argv[i] + "_fixed", populationTokenList)    
        i = i + 1    

# program starts here

main()
   
