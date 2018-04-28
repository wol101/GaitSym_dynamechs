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
    
def WriteLines(filename, lines):
    theOutput = open(filename, 'w')
    for i in range(0, len(lines)):
        line = str(lines[i])
        if EndsWith(line, "\n"):
            theOutput.write(line)
        else:
            theOutput.write(line + "\n")
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
    
def ConvertFileToLines(filename):
    """reads in a file and converts to a list lines"""
    
    input = open(filename, 'r')
    lines = input.readlines()
    input.close()
    
    return lines
    
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

# 3x1 vector routines
def VectorNormalize(v):
    v1 = [0, 0, 0]
    mag = math.sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]) 
    v1[0] = v[0] / mag
    v1[1] = v[1] / mag
    v1[2] = v[2] / mag
    return v1
    
# Quaternion routines
# Note: q[0] = x; q[1] = y; q[2] = z; q[3] = n     
def QuaternionConjugate(q):
    q1 = [0, 0, 0, 0]
    q1[0] = -q[0];
    q1[1] = -q[1];
    q1[2] = -q[2];
    q1[3] = q[3];
    return q1;
    
def QuaternionMultiplyQuaternion(q1, q2):
    q3 = [0, 0, 0, 0]
    q3[0] = q1[3]*q2[0] + q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1]
    q3[1] = q1[3]*q2[1] + q1[1]*q2[3] + q1[2]*q2[0] - q1[0]*q2[2]
    q3[2] = q1[3]*q2[2] + q1[2]*q2[3] + q1[0]*q2[1] - q1[1]*q2[0]							
    q3[3] = q1[3]*q2[3] - q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2]
    return q3
    
def QuaternionMultiplyVector(q, v):
    q2 = [0, 0, 0, 0]
    q2[0] = q[3]*v[0] + q[1]*v[2] - q[2]*v[1]
    q2[1] = q[3]*v[1] + q[2]*v[0] - q[0]*v[2]
    q2[2] = q[3]*v[2] + q[0]*v[1] - q[1]*v[0]
    q2[3] = -(q[0]*v[0] + q[1]*v[1] + q[2]*v[2])
    return q2

def QuaternionRotateVector(q, v):
    v1 = [0, 0, 0]
    q2 = QuaternionMultiplyQuaternion(QuaternionMultiplyVector(q, v), QuaternionConjugate(q))
    v1[0] = q2[0]
    v1[1] = q2[1]
    v1[2] = q2[2]
    return v1
    
def QuaternionCreateFromRotation(v, angle):
    q = [0, 0, 0, 0]
    v1 = VectorNormalize(v)

    sin_a = math.sin( angle / 2 )
    cos_a = math.cos( angle / 2 )

    q[0]    = v1[0] * sin_a
    q[1]    = v1[1] * sin_a
    q[2]    = v1[2] * sin_a
    q[3]    = cos_a

    return q

def CycleTime(tokenList, conversionType):
    if conversionType == "SixMuscle":
        cycleTime = 2 * (abs(float(tokenList[2])) + abs(float(tokenList[30])) + abs(float(tokenList[58])))
    if conversionType == "EightMuscle":
        cycleTime = 2 * (abs(float(tokenList[2])) + abs(float(tokenList[38])) + abs(float(tokenList[74])))
    if conversionType == "TwelveMuscleQuad":
        cycleTime = 2 * (abs(float(tokenList[2])) + abs(float(tokenList[54])) + abs(float(tokenList[106])))
    if conversionType == "SmartSubstitution":
        cycleTime = 2 * float(tokenList[6])
    return cycleTime

def main():
    usageString = "Usage:\n%s [--controlFile file]" % sys.argv[0]

    controlFile = "phaseControl.txt";

    i = 1;
    while i < len(sys.argv):
        if sys.argv[i] == "--controlFile":
            i = i + 1
            if i >= len(sys.argv): PrintExit(usageString)
            controlFile = sys.argv[i] 
            i = i + 1
            continue 

        PrintExit(usageString)


    tokenList = ConvertFileToTokens(controlFile)

    try:
        startPhase = float(GetValueFromTokenList(tokenList, "startPhase"))
        endPhase = float(GetValueFromTokenList(tokenList, "endPhase"))
        incrementPhase = float(GetValueFromTokenList(tokenList, "incrementPhase"))
        
        rootGenomeXML = GetValueFromTokenList(tokenList, "rootGenomeXML")
        rootGenomeTXT = GetValueFromTokenList(tokenList, "rootGenomeTXT")
        rootPopulation = GetValueFromTokenList(tokenList, "rootPopulation")
        newModelStateFile = GetValueFromTokenList(tokenList, "newModelStateFile")
        newRootGenome = GetValueFromTokenList(tokenList, "newRootGenome")
        newRootPopulation = GetValueFromTokenList(tokenList, "newRootPopulation")
        
        conversionType = GetValueFromTokenList(tokenList, "conversionType")
        objectiveCommand = GetValueFromTokenList(tokenList, "objectiveCommand")
        runCommand = GetValueFromTokenList(tokenList, "runCommand")

    except "NoMatch", data:
        PrintExit("Error: %s undefined" % data)

    # calculate cycle time
    rootGenomeTXTTokens = ConvertFileToTokensIgnoreQuotes(rootGenomeTXT)
    cycleTime = CycleTime(rootGenomeTXTTokens, conversionType)
    
    # loop through the phases
    phase = startPhase
    # produce the zero phase ModelState.xml
    offsetTime = 0.00000000001
    modelState1 = "ModelStateTeq%.11f.xml" % (offsetTime)
    command = "%s -t %.17f -c %s -l %s" % (objectiveCommand, offsetTime, rootGenomeXML, modelState1)
    print command
    os.system(command)
    modelState1Lines = ConvertFileToLines(modelState1)
    
    while phase < endPhase + 0.0000001:
        # produce the offset phase ModelState.xml
	offsetTime = cycleTime * (1 - phase)
	if (offsetTime <= 0): offsetTime = 0.00000000002
        modelState2 = "ModelStateTeq%.11f.xml" % (offsetTime)
        command = "%s -t %.17f -c %s -l %s" % (objectiveCommand, offsetTime, rootGenomeXML, modelState2)
        print command
        os.system(command)
        modelState2Lines = ConvertFileToLines(modelState2)

        # now replace the forelimb parts of modelState1Lines
        # this is very fragile and relies on the model state having eack link on a separate line
        # and the link order being the same in modelState1Lines and modelState2Lines
        for i in range(0, len(modelState1Lines)):
	    l = modelState1Lines[i]
	    if (string.find(l, "Arm") != -1) or (string.find(l, "Forearm") != -1) or (string.find(l, "Hand") != -1):
                modelState1Lines[i] = modelState2Lines[i]

        WriteLines(newModelStateFile, modelState1Lines)
	
	# now change the phase for the genome
	rootGenomeTXTTokens[2] = phase
	WriteTokenList(newRootGenome, rootGenomeTXTTokens)
	
	# and for the population
	rootPopulationTokens = ConvertFileToTokensIgnoreQuotes(rootPopulation)
	popLen = int(rootPopulationTokens[0])
	genomeStart = 1
	for i in range(0, popLen):
	    genomeLen = int(rootPopulationTokens[genomeStart + 1])
	    rootPopulationTokens[genomeStart + 2] = phase
	    genomeStart = genomeStart + genomeLen * 4 + 7
	WriteTokenList(newRootPopulation, rootPopulationTokens)
	
        if len(runCommand) > 0:
            command = "%s --dirPrefix %.4f_" % (runCommand, phase)
            print command
            os.system(command)
        
        phase = phase + incrementPhase
    

# program starts here

main()
   

