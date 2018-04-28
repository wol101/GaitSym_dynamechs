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
    
def ReturnNumber(theString):
    """checks to see whether a string is a valid number"""
    matchObj = re.search('([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?', theString)
    if matchObj == None: return 0
    return float(matchObj.group())
    
def main():
    usageString = "Usage:\n%s [--inputFile file]" % sys.argv[0]

    inputFile = "output.log"

    i = 1
    while i < len(sys.argv):
        if sys.argv[i] == "--inputFile":
            i = i + 1
            if i >= len(sys.argv): PrintExit(usageString)
            inputFile = sys.argv[i] 
            i = i + 1
            continue 

        PrintExit(usageString)
            
    theTokenList = ConvertFileToTokens(inputFile)

    count = 0
    scaleList = []
    idList = []
    timeList = []
    scoreList = []
    mechEnergyList = []
    metEnergyList = []
    while count < len(theTokenList):
        if theTokenList[count] == "-o":
            count = count + 1
            parts = string.split(theTokenList[count], "/")
            scaleList.append(ReturnNumber(parts[0]))
            idList.append(parts[1])
            count = count + 1
            
            while count < len(theTokenList):
                if theTokenList[count] == "Simulation":
                    count = count + 2
                    timeList.append(float(theTokenList[count]))
                    count = count + 1
                    continue

                if theTokenList[count] == "Score:":
                    count = count + 1
                    scoreList.append(float(theTokenList[count]))
                    count = count + 1
                    continue

                if theTokenList[count] == "Mechanical":
                    count = count + 2
                    mechEnergyList.append(float(theTokenList[count]))
                    count = count + 1
                    continue

                if theTokenList[count] == "Metabolic":
                    count = count + 2
                    metEnergyList.append(float(theTokenList[count]))
                    count = count + 1
                    break
                    
                count = count + 1
            continue
               
        count = count + 1
    
    if len(idList) != len(scaleList) or \
        len(idList) != len(timeList) or \
        len(idList) != len(scoreList) or \
        len(idList) != len(mechEnergyList) or \
        len(idList) != len(metEnergyList):
            print "Warning: list length mismatch"
            
    count = 0
    print "ID\tScale\tTime\tScore\tMechEn\tMetEn"
    while count < len(idList):
        print "%s\t%f\t%f\t%f\t%f\t%f" % (idList[count], scaleList[count], \
            timeList[count], scoreList[count], mechEnergyList[count], \
            metEnergyList[count])
        count = count + 1    


# program starts here

main()
   

