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
    """Append text files side by side."""
    usageString = "Usage:\n%s [-t] in_file1 in_file2 ..." % sys.argv[0]

    if len(sys.argv) < 2:
        PrintExit(usageString)
        
    fileNameList = []
    if sys.argv[1] == "-t":
        joinChar = "\t"
        for i in range(2, len(sys.argv)):
            fileNameList.append(sys.argv[i])
    else:
        joinChar = ""
        for i in range(1, len(sys.argv)):
            fileNameList.append(sys.argv[i])
    
    minNLines = -1;   
    inFileLines = []    
    for i in range(0, len(fileNameList)):
        inFile = open(fileNameList[i], 'r')
        inFileLines.append(inFile.readlines())
        inFile.close()
        nLines = len(inFileLines[i])
        if nLines < minNLines or minNLines == -1:
            minNLines = nLines
    
    for i in range(0, minNLines):
        for j in range(0, len(fileNameList)):
            part = inFileLines[j][i]
            if EndsWith(part, "\n"):
                part = part[0: len(part) - 1]
            if j == 0:
                line = part
            else:
                line = line + joinChar + part 
        print line     

# program starts here

main()
   
