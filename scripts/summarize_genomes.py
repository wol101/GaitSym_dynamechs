#!/usr/bin/python -u

import sys
import string
import array
import os
import re
import time

def PrintExit(value):
    """exits with error message"""
    sys.stderr.write(str(value) + "\n");
    sys.exit(1)

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
    
    return ConvertToListOfTokens(theBuffer)
   
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

    usageString = "Usage:\n%s [-v] list_of_genomes" % sys.argv[0]
    verboseFlag = 0
    
    if len(sys.argv) < 2: PrintExit(usageString)
    
    # get reverse sorted list of genomes
    i = 1
    listOfGenomes = []
    while i < len(sys.argv):
        if i == 1:
            if sys.argv[i] == "-v":
                verboseFlag = 1
                i = i + 1
                continue
        listOfGenomes.append(sys.argv[i])
        i = i + 1
        
    listOfGenomes.sort()
    listOfGenomes.reverse()
    
    i = 0
    lastParent = ""
    listOfBestGenomes = []
    while i < len(listOfGenomes):
        theFile = os.path.basename(listOfGenomes[i])
        theParent = os.path.dirname(listOfGenomes[i])
        if theParent != lastParent:
            listOfBestGenomes.append(listOfGenomes[i])
            lastParent = theParent
        i = i + 1 
	   
    listOfBestGenomes.sort()
    i = 0
    while i < len(listOfBestGenomes):
        tokenList = ConvertFileToTokens(listOfBestGenomes[i])
        spaceSeparatedPath = string.replace(listOfBestGenomes[i], "/", " ")
        duration = abs(float(tokenList[2])) + abs(float(tokenList[30])) + abs(float(tokenList[58]))
        fitness = float(tokenList[86])
        if verboseFlag:
            print "%s %f %f" % (spaceSeparatedPath, fitness, duration) 
        else:
            print "%f" % (duration) 
        i = i + 1
    
# program starts here

main()
   
    
    
