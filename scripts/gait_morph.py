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
    usageString = "Usage:\n%s [--controlFile file] [--jumpPercent x] [--dirPrefix dir]" % sys.argv[0]

    controlFile = "control.txt"
    dirPrefix = ""
    jumpFlag = 0;

    i = 1;
    while i < len(sys.argv):
        if sys.argv[i] == "--controlFile":
            i = i + 1
            if i >= len(sys.argv): PrintExit(usageString)
            controlFile = sys.argv[i] 
            i = i + 1
            continue 

        if sys.argv[i] == "--dirPrefix":
            i = i + 1
            if i >= len(sys.argv): PrintExit(usageString)
            dirPrefix = sys.argv[i] 
            i = i + 1
            continue 

        if sys.argv[i] == "--jumpPercent":
            i = i + 1
            if i >= len(sys.argv): PrintExit(usageString)
            jumpFlag = 1
            jumpPercent = float(sys.argv[i]) 
            i = i + 1
            continue 

        PrintExit(usageString)


    tokenList = ConvertFileToTokens(controlFile)

    # compulsory values
    try:
        kRangeControlStartPercent = float(GetValueFromTokenList(tokenList, "startPercent"))
        kRangeControlEndPercent = float(GetValueFromTokenList(tokenList, "endPercent"))
        kRangeControlIncrementPercent = float(GetValueFromTokenList(tokenList, "incrementPercent"))
        rootConfigFile1 = GetValueFromTokenList(tokenList, "rootConfigFile1")
        rootConfigFile2 = GetValueFromTokenList(tokenList, "rootConfigFile2")
        rootParameterFile = GetValueFromTokenList(tokenList, "rootParameterFile")
        rootGenomeFile = GetValueFromTokenList(tokenList, "rootGenomeFile")
        rootPopulationFile = GetValueFromTokenList(tokenList, "rootPopulationFile")
        rootModelStateFile = GetValueFromTokenList(tokenList, "rootModelStateFile")
        workingConfigFile = GetValueFromTokenList(tokenList, "workingConfigFile")
        workingParameterFile = GetValueFromTokenList(tokenList, "workingParameterFile")
        workingModelState = GetValueFromTokenList(tokenList, "workingModelState")
        gaCommand = GetValueFromTokenList(tokenList, "gaCommand")
        objectiveCommand = GetValueFromTokenList(tokenList, "objectiveCommand")
        mergeCommand = GetValueFromTokenList(tokenList, "mergeCommand")
        workingGenome = GetValueFromTokenList(tokenList, "workingGenome")
        workingPopulation = GetValueFromTokenList(tokenList, "workingPopulation")
        conversionType = GetValueFromTokenList(tokenList, "conversionType")
        usePopulationFlag = int(GetValueFromTokenList(tokenList, "usePopulationFlag"))
        correctRotationFlag = int(GetValueFromTokenList(tokenList, "correctRotationFlag"))
        repeatsAtEachValue = int(GetValueFromTokenList(tokenList, "repeatsAtEachValue"))
        newStartCycleCount = int(GetValueFromTokenList(tokenList, "newStartCycleCount"))
        newStartCycleCountAfterMorph = int(GetValueFromTokenList(tokenList, "newStartCycleCountAfterMorph"))

    except "NoMatch", data:
        PrintExit("Error: %s undefined" % data)
        
    # optional values
    try:
        startGradient = float(GetValueFromTokenList(tokenList, "startGradient")) 
        endGradient = float(GetValueFromTokenList(tokenList, "endGradient")) 
        doGradientCorrectionFlag = 1
    except "NoMatch", data:
        doGradientCorrectionFlag = 0         

    try:
        startMaxXVelocity = float(GetValueFromTokenList(tokenList, "startMaxXVelocity")) 
        endMaxXVelocity = float(GetValueFromTokenList(tokenList, "endMaxXVelocity")) 
        doXVelocityCorrectionFlag = 1
    except "NoMatch", data:
        doXVelocityCorrectionFlag = 0         

    if jumpFlag:
        kRangeControlStartPercent = jumpPercent
    else:
        if usePopulationFlag:
            print "Copying %s to %s" % (rootPopulationFile, workingPopulation)
            CopyFile(rootPopulationFile, workingPopulation)
        else:
            print "Copying %s to %s" % (rootGenomeFile, workingGenome)
            CopyFile(rootGenomeFile, workingGenome)

        print "Copying %s to %s" % (rootModelStateFile, workingModelState)
        CopyFile(rootModelStateFile, workingModelState)

    dirName = range(0, repeatsAtEachValue)
    repeatCount = 0
    kRangeControlPercent = kRangeControlStartPercent
    while kRangeControlPercent <= kRangeControlEndPercent:
        kRangeControl = kRangeControlPercent / 100.0 

        # build the working config file
        # merge the relevant files
        tempFile = "tempConfigFile.xml"
        command = "%s %s %s %s %s" % (mergeCommand, rootConfigFile1, rootConfigFile2, kRangeControl, tempFile)
        print command
        os.system(command)

        # read the torso orientation
        # Note, this is very fragile since it relies on
        # the exact format of the ModelState file not the XML content
        # this version REQUIRES the new format ModelState with extra spaces
        # in the content strings
        tokenList = ConvertFileToTokensIgnoreQuotes(workingModelState)
        index = GetIndexFromTokenList(tokenList, 'Name="Torso"')
        # zero the X  & Y displacement in the model state
        tokenList[index + 5] = 0 # X
        tokenList[index + 6] = 0 # Y
        
        # get the orientation
        if correctRotationFlag:
            q = [float(tokenList[index + 1]), float(tokenList[index + 2]), float(tokenList[index + 3]), float(tokenList[index + 4])]
            print "Torso Quaternion %f %f %f %f" % (q[0], q[1], q[2], q[3])
	    # rotate a suitable vector to find Z rotation
	    v = [1, 0, 0]
            v = QuaternionRotateVector(q, v)
	    print "X Vector %f %f %f" % (v[0], v[1], v[2])
            zRotation = math.atan(v[1]/v[0])
            print "Z Rotation is %f degrees" % (zRotation * 180 / math.pi)
            # create a quaternion to perform the rotation
	    v = [0, 0, 1]
            q1 = QuaternionCreateFromRotation(v, -zRotation)
            # and correct the orginal orientation
            q2 = QuaternionMultiplyQuaternion(q1, q)
            print "Corrected Torso Quaternion %f %f %f %f" % (q2[0], q2[1], q2[2], q2[3])
	    v = [1, 0, 0]
            v = QuaternionRotateVector(q2, v)
	    print "Corrected X Vector %f %f %f" % (v[0], v[1], v[2])
	    zRotation = math.atan(v[1]/v[0])
	    print "Corrected Z Rotation is %f" % (zRotation * 180 / math.pi)
            # set the quaternion
            tokenList[index + 1] = "%.16f" % (q2[0])
            tokenList[index + 2] = "%.16f" % (q2[1])
            tokenList[index + 3] = "%.16f" % (q2[2])
            tokenList[index + 4] = "%.16f" % (q2[3])
            # now fix the velocities
            avel = [float(tokenList[index + 10]), float(tokenList[index + 11]), float(tokenList[index + 12])]
            vel = [float(tokenList[index + 13]), float(tokenList[index + 14]), float(tokenList[index + 15])]
            newAvel = QuaternionRotateVector(q1, avel)
            newVel = QuaternionRotateVector(q1, vel)
            print "Old angular velocity %f %f %f" % (avel[0], avel[1], avel[2])
            print "New angular velocity %f %f %f" % (newAvel[0], newAvel[1], newAvel[2])
            print "Old velocity %f %f %f" % (vel[0], vel[1], vel[2])
            print "New velocity %f %f %f" % (newVel[0], newVel[1], newVel[2])
            tokenList[index + 10] = "%.16f" % (newAvel[0])
            tokenList[index + 11] = "%.16f" % (newAvel[1])
            tokenList[index + 12] = "%.16f" % (newAvel[2])
            tokenList[index + 13] = "%.16f" % (newVel[0])
            tokenList[index + 14] = "%.16f" % (newVel[1])
            tokenList[index + 15] = "%.16f" % (newVel[2])
        
        # get the forward velogity
        if doXVelocityCorrectionFlag:
            index = GetIndexFromTokenList(tokenList, 'Name="Torso"')
            # set the X velocity in the model state
            maxXVelocity = startMaxXVelocity + kRangeControl * (endMaxXVelocity - startMaxXVelocity)
            print "Old Start X Velocity %f" % (tokenList[index + 13])        
            print "Maximum X Velocity %f" % (maxXVelocity)
            if float(tokenList[index + 13]) >  maxXVelocity:
                tokenList[index + 13] = maxXVelocity    
            print "New Start X Velocity %f" % (tokenList[index + 13])        
        
        WriteTokenList(workingModelState, tokenList)

        # merge in the model state
        command = "%s %s %s %s %s" % (mergeCommand, tempFile, workingModelState, 1, workingConfigFile)
        print command
        os.system(command)
        os.unlink(tempFile)

        # build the working parameter file

        theOutput = open(workingParameterFile, 'w')
        if usePopulationFlag: theOutput.write("startingPopulation %s\n\n" % workingPopulation)
        else: theOutput.write("modelGenome %s\n\n" % workingGenome)
        theOutput.write("extraDataFile %s\n\n" % workingConfigFile)
        theInput = open(rootParameterFile, 'r');
        theData = theInput.read();
        theInput.close();
        theOutput.write(theData);
        theOutput.close();

        # create the directory name
        theTime = time.localtime(time.time())
        dirName[repeatCount] = "%s%.4f/Run_%04d-%02d-%02d_%02d.%02d.%02d" % (dirPrefix, kRangeControl, theTime[0], theTime[1], theTime[2], theTime[3], theTime[4], theTime[5])
        os.makedirs(dirName[repeatCount])
        command = "%s -p %s -b %s -x %s -l %s -o %s" % (gaCommand, workingParameterFile, workingGenome, workingConfigFile, workingPopulation, dirName[repeatCount])
        print command
        os.system(command)  

        # find the cycle time from the genome
        tokenList = ConvertFileToTokens(workingGenome)
	if (newStartCycleCount == 0):
	    cycleTime = 1e-10;
	else:
            cycleTime = CycleTime(tokenList, conversionType)
            cycleTime = cycleTime * newStartCycleCount
	    
        # run the objective

        command = "%s -c %s --outputModelStateAtTime %s" % (objectiveCommand, workingConfigFile, cycleTime)
        print command
        os.system(command) 
         
        # correct for gradient based height change
        if doGradientCorrectionFlag:
            tokenList = ConvertFileToTokensIgnoreQuotes(workingModelState)
            index = GetIndexFromTokenList(tokenList, 'Name="Torso"')
            gradientBasedHeight = float(tokenList[index + 5]) * (startGradient + kRangeControl * (endGradient - startGradient))
	    print "Gradient based height %f" % (gradientBasedHeight)
	    print "Height before %f" % (float(tokenList[index + 7]))
            # modify Z displacement to account for substrate height change
            tokenList[index + 7] = float(tokenList[index + 7]) - gradientBasedHeight
	    print "Height after %f" % (tokenList[index + 7])
            WriteTokenList(workingModelState, tokenList)

        repeatCount = repeatCount + 1;
        if repeatCount >= repeatsAtEachValue:
            repeatCount = 0;
            kRangeControlPercent = kRangeControlPercent + kRangeControlIncrementPercent

            # find the best score from the previour block
            maxScore = -1e30
            bestDir = 0
            for i in range(0, repeatsAtEachValue):
                fileList = os.listdir(dirName[i])
                fileList.sort()
                fileList.reverse()
                for name in fileList:
                    if StartsWith(name, "BestGenome_") and EndsWith(name, ".txt"): break
                tokenList = ConvertFileToTokens(dirName[i] + "/" + name)
                if int(tokenList[0]) != -1:
                    print "Error reading genome: %s" % (dirName[i] + "/" + name)
                    continue
                genomeLength = int(tokenList[1])
                score = float(tokenList[genomeLength * 4 + 2])
                print "Score", score
                if score > maxScore: 
                    maxScore = score
                    bestDir = i
            
            print "Best score %f found in %s" % (maxScore, dirName[bestDir])
            
            # copy the files over
            fileList = os.listdir(dirName[bestDir])
            fileList.sort()
            fileList.reverse()
            for name in fileList:
                if StartsWith(name, "BestGenome_") and EndsWith(name, ".txt"): 
                    print "Copying %s to %s" % (dirName[bestDir] + "/" + name, workingGenome)
                    CopyFile(dirName[bestDir] + "/" + name, workingGenome)
                    break
            
            for name in fileList:
                if StartsWith(name, "BestGenome_") and EndsWith(name, ".xml"): 
                    print "Copying %s to %s" % (dirName[bestDir] + "/" + name, workingConfigFile)
                    CopyFile(dirName[bestDir] + "/" + name, workingConfigFile)
                    break
            
            for name in fileList:
                if StartsWith(name, "Population_") and EndsWith(name, ".txt"): 
                    print "Copying %s to %s" % (dirName[bestDir] + "/" + name, workingPopulation)
                    CopyFile(dirName[bestDir] + "/" + name, workingPopulation)
                    break
                    
            # find the cycle time from the genome
            tokenList = ConvertFileToTokens(workingGenome)
	    if (newStartCycleCountAfterMorph == 0):
	    	cycleTime = 1e-10;
	    else:
                cycleTime = CycleTime(tokenList, conversionType)
           	cycleTime = cycleTime * newStartCycleCountAfterMorph;
		
            # run the objective

            command = "%s -c %s --outputModelStateAtTime %s" % (objectiveCommand, workingConfigFile, cycleTime)
            print command
            os.system(command)  

            # correct for gradient based height change
            if doGradientCorrectionFlag:
                tokenList = ConvertFileToTokensIgnoreQuotes(workingModelState)
                index = GetIndexFromTokenList(tokenList, 'Name="Torso"')
                gradientBasedHeight = float(tokenList[index + 5]) * (startGradient + kRangeControl * (endGradient - startGradient))
	        print "Gradient based height %f" % (gradientBasedHeight)
	        print "Height before %f" % (float(tokenList[index + 7]))
                # modify Z displacement to account for substrate height change
                tokenList[index + 7] = float(tokenList[index + 7]) - gradientBasedHeight
	        print "Height after %f" % (tokenList[index + 7])
                WriteTokenList(workingModelState, tokenList)
            
# program starts here

main()
   

