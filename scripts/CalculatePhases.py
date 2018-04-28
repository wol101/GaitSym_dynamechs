#!/usr/bin/python -u

import sys
import string
import array
import os
import re
import time
import math

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
    usageString = "Usage:\n%s list_of_directories" % sys.argv[0]

    if len(sys.argv) <= 1:
        PrintExit(usageString)
        
    controlFile = "CalculatePhases.txt"
    tokenList = ConvertFileToTokens(controlFile)

    try:
        threshold = float(GetValueFromTokenList(tokenList, "threshold"))
        interval = float(GetValueFromTokenList(tokenList, "interval"))
        smoothInterval = float(GetValueFromTokenList(tokenList, "smoothInterval"))
    
    except "NoMatch", data:
        PrintExit("Error: %s undefined" % data)

    for i in range(1, len(sys.argv)):
        folder = sys.argv[i]
        
        fileList = os.listdir(folder)
        fileList.sort()
        fileList.reverse()
        
        foundFlag = 0
        for name in fileList:
            if StartsWith(name, "BestGenome_") and EndsWith(name, ".xml"):
                foundFlag = 1 
                break
        
        if foundFlag == 0:
            PrintExit("Could not find BestGenome_*.xml")
        
        if StartsWith(folder, "/"):
            path =  folder + "/" + name
        else:
            path =  os.getcwd() + "/" + folder + "/" + name
        print "Analysis of %s" % (path)
        
        command = os.environ['GAITSYM'] + "/bin/objective -c " + path
        print command
        os.system(command)
        
        command1 = command + " -d 1"
        command2 = "%s/bin/CalcPhase --threshold %.17f --interval %.17f --smoothInterval %.17f" % (os.environ['GAITSYM'], threshold, interval, smoothInterval)
        
        command = 'tcsh -c "' + command1 + " |& " + command2 + '"'
        print command
        os.system(command)

# program starts here

main()
   

