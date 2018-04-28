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
    usageString = "Usage:\n%s [-c n] directory1 directory2 ..." % sys.argv[0]
    
    defaultCommand = os.environ['GAITSYM'] + "/bin/objective_opengl -S 4000 -f 3 -g " + os.environ['GAITSYM'] + "/unit_bones -c "
    try:
        run_best_command = os.environ['run_best_command']
    except KeyError:
        theCommand = defaultCommand
        print '$run_best_command not set. Using:'
        print theCommand
    else:
        theCommand = run_best_command
        print 'Using $run_best_command:'
        print theCommand

    if len(sys.argv) < 2: PrintExit(usageString)
    
    if sys.argv[1] == "-c":
        count = int(sys.argv[2])
        if count < 2:
            count = 2
        dirList = []
        for i in range(3, len(sys.argv)):
            dirList.append(sys.argv[i])
        if count > len(dirList):
            count = len(dirList)
    else:
        dirList = []
        for i in range(1, len(sys.argv)):
            dirList.append(sys.argv[i])
        count = len(dirList)
    
    bestPathList = []
    for i in range(0, len(dirList)):
        folder = dirList[i]
        fileList = os.listdir(folder)
        fileList.sort()
        fileList.reverse()
        for name in fileList:
            if StartsWith(name, "BestGenome_") and EndsWith(name, ".xml"): break
        if StartsWith(folder, "/"):
            path =  folder + "/" + name
        else:
            path =  os.getcwd() + "/" + folder + "/" + name
        bestPathList.append(path)
    
    i = 0.0
    if count == 1:
        increment = len(bestPathList)
    else:
        increment = float(len(bestPathList) - 1) / float(count - 1)
    # print increment
    while i < len(bestPathList):
        path = bestPathList[int(i + 0.000001)]
        i += increment
        command = theCommand + ' ' + path
        print command
        os.system(command)
    
# program starts here

main()
   

