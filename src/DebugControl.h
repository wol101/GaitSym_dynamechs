// DebugControl.h - holds some values useful for debugging

#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>

const int NoDebug = 0;
const int ContactDebug = 1;
const int FitnessDebug = 2;
const int MainDebug = 3;
const int StrapForceDebug = 4;
const int StrapForceAnchorDebug = 5;
const int MAMuscleDebug = 6;
const int MemoryDebug = 7;
const int SideStabilizerDebug = 8;
const int SocketDebug = 9;
const int SimulationDebug = 10;
const int GLUIDebug = 11;
const int DampedSpringDebug = 12;
const int MyMobileBaseLinkDebug = 13;
const int UGMMuscleDebug = 14;

const char * const gDebugLabels[] =
{
    "NoDebug", 
    "ContactDebug",
    "FitnessDebug", 
    "MainDebug", 
    "StrapForceDebug",
    "StrapForceAnchorDebug", 
    "MAMuscleDebug", 
    "MemoryDebug", 
    "SideStabilizerDebug",
    "SocketDebug",
    "SimulationDebug",
    "GLUIDebug",
    "DampedSpringDebug",
    "MyMobileBaseLinkDebug",
    "UGMMuscleDebug"
};

#ifndef DEBUG_MAIN
// cope with declaring globals extern
extern int gDebug;
extern ostream *gDebugStream;
#else
int gDebug = NoDebug;
ostream *gDebugStream = &cerr;
#endif

#endif // DEBUG_H
