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
    """Replaces [[tokens]] with values from a list contained in a control file
    An optional number and operator can be added e.g. [[3 * token]]
    Also 3d coordinate values can use the operators MirrorX, MirrorY, MirrorZ
    [[MirrorX x y z]] or [[MirrorX x1 y1 z1 x2 y2 z2 etc]]
    Note: order must be number operator token and there must be whitespace
    between the 3 parts."""
    usageString = "Usage:\n%s control_file in_file out_file" % sys.argv[0]

    if len(sys.argv) != 4:
        PrintExit(usageString)
    
    controlFile = sys.argv[1]
    inFileName = sys.argv[2]
    outFileName = sys.argv[3]
    
    controlTokenList = ConvertFileToTokens(controlFile)
    
    inFile = open(inFileName, 'r')
    inData = inFile.read()
    inFile.close()
    
    outData = ""
    searchData = inData
    regex = "\\[\\[[^\\]]*\\]\\]"
    # print regex
    match = re.search(regex, searchData)
    while (match != None):
        outData = outData + searchData[0:match.start()]
        searchData = searchData[match.end():]
        tokenList = ConvertToListOfTokens(match.group()[2:-2])
        if (len(tokenList) < 1 and len(tokenList) > 3):
            PrintExit("Error parsing token %s" % (match.group()))
        if (len(tokenList) == 1):
            try:
                swap = GetValueFromTokenList(controlTokenList, tokenList[0])
                outData = outData + swap
            except "NoMatch", data:
                print "Warning: %s undefined" % (data)
                outData = outData + match.group()
        if (len(tokenList) == 2):
            try:
                op = tokenList[0]
                coordsList = ConvertToListOfTokens(GetValueFromTokenList(controlTokenList, tokenList[1]))
                if (len(coordsList) % 3 != 0):
                    PrintExit("Error parsing token %s" % (match.group()))
                for i in range(0, len(coordsList), 3):
                    x = float(coordsList[0 + i])
                    y = float(coordsList[1 + i])
                    z = float(coordsList[2 + i])
                    if (op == "MirrorX"): x = x * -1
                    elif (op == "MirrorY"): y = y * -1
                    elif (op == "MirrorZ"): z = z * -1
                    else:
                        PrintExit("Unrecognised operator in token %s" % (match.group()))
                    outData = outData + ("%f %f %f " % (x, y, z))
            except "NoMatch", data:
                print "Warning: %s undefined" % (data)
                outData = outData + match.group()
        if (len(tokenList) == 3):
            try:
                val = float(tokenList[0])
                swap = float(GetValueFromTokenList(controlTokenList, tokenList[2]))
                if (tokenList[1] == "+"): result = val + swap
                elif (tokenList[1] == "-"): result = val - swap
                elif (tokenList[1] == "*"): result = val * swap
                elif (tokenList[1] == "/"): result = val / swap
                else:
                    PrintExit("Unrecognised operator in token %s" % (match.group()))
                outData = outData + ("%f" % (result))
            except "NoMatch", data:
                print "Warning: %s undefined" % (data)
                outData = outData + match.group()
        match = re.search(regex, searchData)
            
    outData = outData + searchData  
        
    outFile = open(outFileName, 'w')
    outFile.write(outData)
    outFile.close()

# program starts here

main()
   
