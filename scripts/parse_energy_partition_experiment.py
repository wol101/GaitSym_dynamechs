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

    matchList = ["id", "Time:", "Score:", "m_PositiveMechanicalWork", "m_NegativeMechanicalWork", "m_PositiveContractileWork", "m_NegativeContractileWork", "m_PositiveSerialElasticWork", "m_NegativeSerialElasticWork", "m_PositiveParallelElasticWork", "m_NegativeParallelElasticWork"]
        
    for i in range(0, len(matchList) - 1):
        print matchList[i] + " ",
    print matchList[len(matchList) - 1]
    
    reObj = re.compile(".*(Run_.*)/.*")
    
    i = 1;
    while i < len(sys.argv):
        tokenList = ConvertFileToTokens(sys.argv[i])
        i = i + 1
        
        for j in range(0, len(tokenList)):
        
            # find the id string
            m = reObj.match(tokenList[j])
            if m != None:
                print m.group(1) + " ",
                continue
                
            # find the matchlist strings
            
            for k in range(0, len(matchList)):
                if matchList[k] == tokenList[j]:
                    if k != len(matchList) - 1:
                        print tokenList[j + 1] + " ",
                    else:
                        print tokenList[j + 1]
                            

# program starts here

main()
   
