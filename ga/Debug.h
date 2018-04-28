// Debug.h - a set of constants used for debug control

#ifndef Debug_h
#define Debug_h

#define Debug_GA 1
#define Debug_Genome 2
#define Debug_HillClimb 3
#define Debug_Mating 4
#define Debug_Objective 5
#define Debug_ParameterFile 6
#define Debug_Population 7
#define Debug_Random 8
#define Debug_StartupScreen 9
#define Debug_Statistics 10
#define Debug_TestObjective 11
#define Debug_TestRandom 12
#define Debug_Utility 13
#define Debug_Sockets 14

#ifdef gDebugDeclare
int gDebug = 0;
#else
extern int gDebug;
#endif

#endif
