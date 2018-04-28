#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <float.h>
#include <time.h>

#ifdef USE_OPENGL
#include <glut.h>
#include "GLUIRoutines.h"
#endif

#define DEFINING_DM_GLOBALS
#include <dm.h>
#include <dmArticulation.hpp>

#ifdef USE_SOCKETS
#include <pinet.h>
#include "SocketMessages.h"
#endif

#include "Simulation.h"
#include "DataFile.h"
#include "ModifiedContactModel.h"
#include "MAMuscle.h"
#include "Util.h"
#include "PGDMath.h"

#define DEBUG_MAIN
#include "DebugControl.h"

// Simulation global
Simulation *gSimulation;

// window size
int gWindowWidth = 640;
int gWindowHeight = 480;

// control window separate?
bool gUseSeparateWindow = true;

// display frames every...
int gDisplaySkip = 200;

// default field of view (degrees)
float gFOV = 5;

// default distance
float gCameraDistance = -40.0;

// root directory for bones graphics
char *gGraphicsRoot = 0;

// run control
bool gFinishedFlag = true;

// computer runtime limit
long gRunTimeLimit = 0;

#if defined(USE_SOCKETS)
// the server
static pt::ipstream *gHost;
#endif

// command line arguments
static char *gHostlistFilenamePtr = "hosts.txt";
static char *gConfigFilenamePtr = "config.xml";
static char *gScoreFilenamePtr = "score.tmp";
static bool gSummaryFlag = true;
static char *gOutputKinematicsFilenamePtr = 0;
static char *gOutputModelStateFilenamePtr = 0;
static double gOutputModelStateAtTime = 0;

// hostlist globals
struct Hosts
{
    char host[256];
    int port;
};
static vector<Hosts>gHosts;

int ReadModel();
void WriteModel();

static void ParseArguments(int argc, char ** argv);

#if defined(USE_SOCKETS)
static void ParseHostlistFile(void);
#endif

int main(int argc, char ** argv)
{
#if defined(USE_OPENGL)
    // read any arguments relevent to OpenGL and remove them from the list
    // also intialises the windowing system
    glutInit(&argc, argv);
#endif

    // start by parsing the command line arguments
    ParseArguments(argc - 1, &(argv[1]));

#if defined(USE_SOCKETS)
    ParseHostlistFile();
#endif

#if defined(USE_OPENGL)
    // now initialise the OpenGL bits
    StartGlut();

    // and enter the never to be returned loop
    glutMainLoop();
#else
    // another never returned loop (exits are in WriteModel when required)
    long runTime = 0;
    long startTime = time(0);
    while(gRunTimeLimit == 0 || runTime <= gRunTimeLimit)
    {
        runTime = time(0) - startTime;

        if (gFinishedFlag)
        {
            if (ReadModel() == 0)
            {
                gFinishedFlag = false;
            }
            else
            {
                usleep(1000); // slight pause on read failure
            }
        }
        else
        {
            while (gSimulation->ShouldQuit() == false)
            {
                gSimulation->UpdateSimulation();

                if (gSimulation->TestForCatastrophy()) break;
            }

            gFinishedFlag = true;
            WriteModel();
        }
    }
#endif
}

void ParseArguments(int argc, char ** argv)
{
    int i;

    if (gDebug == MainDebug)
    {
        for (i = 0; i < argc; i++)
        {
            *gDebugStream <<  "ParseArguments " << i << argv[i] << "\n";
        }
    }
        
    // do some simple stuff with command line arguments

    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--score") == 0 ||
                strcmp(argv[i], "-s") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing score filename\n";
                    exit(1);
                }
                gScoreFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--config") == 0 ||
                strcmp(argv[i], "-c") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing config filename\n";
                    exit(1);
                }
                gConfigFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--graphicsRoot") == 0 ||
                strcmp(argv[i], "-g") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing graphics root directory\n";
                    exit(1);
                }
                gGraphicsRoot = argv[i];
            }
        else
            if (strcmp(argv[i], "--debug") == 0 ||
                strcmp(argv[i], "-d") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing debug level\n";
                    exit(1);
                }
                gDebug = strtol(argv[i], 0, 10);
            }
        else
            if (strcmp(argv[i], "--windowSize") == 0 ||
                strcmp(argv[i], "-w") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing window size x\n";
                    exit(1);
                }
                gWindowWidth = strtol(argv[i], 0, 10);
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing window size y\n";
                    exit(1);
                }
                gWindowHeight = strtol(argv[i], 0, 10);
            }
        else
            if (strcmp(argv[i], "--displaySkip") == 0 ||
                strcmp(argv[i], "-S") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing --displaySkip\n";
                    exit(1);
                }
                gDisplaySkip = strtol(argv[i], 0, 10);
            }
        else 
            if (strcmp(argv[i], "--fieldOfView") == 0 ||
                strcmp(argv[i], "-f") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing --fieldOfView\n";
                    exit(1);
                }
                gFOV = strtod(argv[i], 0);
            }
        else 
            if (strcmp(argv[i], "--cameraDistance") == 0 ||
                strcmp(argv[i], "-D") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing --cameraDistance\n";
                    exit(1);
                }
                gCameraDistance = strtod(argv[i], 0);
            }
        else
            if (strcmp(argv[i], "--hostList") == 0 ||
                strcmp(argv[i], "-L") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing --hostList\n";
                    exit(1);
                }
                gHostlistFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--runTimeLimit") == 0 ||
                strcmp(argv[i], "-r") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing --runTimeLimit\n";
                    exit(1);
                }
                gRunTimeLimit = strtol(argv[i], 0, 10);
            }
        else
            if (strcmp(argv[i], "--outputKinematics") == 0 ||
                strcmp(argv[i], "-K") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing output kinematics filename\n";
                    exit(1);
                }
                gOutputKinematicsFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--outputModelStateFile") == 0 ||
                strcmp(argv[i], "-M") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing model state filename\n";
                    exit(1);
                }
                gOutputModelStateFilenamePtr = argv[i];
            }
        else
            if (strcmp(argv[i], "--outputModelStateAtTime") == 0 ||
                strcmp(argv[i], "-t") == 0)
            {
                i++;
                if (i >= argc)
                {
                    cerr << "Error parsing --runTimeLimit\n";
                    exit(1);
                }
                gOutputModelStateAtTime = strtod(argv[i], 0);
            }
        else
            if (strcmp(argv[i], "--help") == 0 ||
                strcmp(argv[i], "-h") == 0 ||
                strcmp(argv[i], "-?") == 0)
            {
                cerr << "\nObjective build " << __DATE__ << " " << __TIME__ << "\n\n";
                cerr << "-c filename, --config filename\n";
                cerr << "Reads filename rather than the default config.xml as the config data\n\n";
                cerr << "-s filename, --score filename\n";
                cerr << "Writes filename rather than the default score.tmp as the fitness data\n\n";
                cerr << "-g path, --graphicsRoot path\n";
                cerr << "Prepends path to the graphics filenames\n\n";
                cerr << "-w x y, --windowSize x y\n";
                cerr << "Sets the initial display window size - requires separate x and y values.\n\n";
                cerr << "-S n, --displaySkip n\n";
                cerr << "Displays a new frame every n calculations\n\n";
                cerr << "-f x, --fieldOfView x\n";
                cerr << "Set the field of view angle (degrees)\n\n";
                cerr << "-D x, --cameraDistance x\n";
                cerr << "Set the camera distance (metres)\n\n";
                cerr << "-L filename, --hostList filename\n";
                cerr << "Set the filename for the list of servers for the socket version\n\n";
                cerr << "-r n, --runTimeLimit n\n";
                cerr << "Quits the program (approximately) after it has run n seconds\n\n";
                cerr << "-K filename, --outputKinematics filename\n";
                cerr << "Writes tab-delimited kinematic data to filename\n\n";
                cerr << "-M filename, --outputModelStateFile filename\n";
                cerr << "Sets the model state filename\n\n";
                cerr << "-t x, --outputModelStateAtTime x\n";
                cerr << "Writes the model state to model state file at time x\n\n";
                cerr << "-h, -?, --help\n";
                cerr << "Prints this message!\n\n";

                int nDebugLabels = sizeof(gDebugLabels) / sizeof(gDebugLabels[0]);
                cerr << "-d n, --debug n\n";
                cerr << "Prints out a lot of extra debugging information on stderr if n is higher than 0.\n";
                cerr << "Debug numbers:\n";
                for (i = 0; i < nDebugLabels; i++)
                    fprintf(stderr, "%3d %s\n", i, gDebugLabels[i]);
                cerr << "\n";
                exit(1);
            }
        else
        {
            cerr << "Unrecognised option. Try 'objective --help' for more info\n";
            exit(1);
        }
    }
}

// this routine attemps to read the model specification and initialise the simulation
// it returns zero on success
int ReadModel()
{
#if defined(USE_SOCKETS)
    char *buf = 0;
    int l;
    char buffer[kSocketMaxMessageLength + 1];
    buffer[kSocketMaxMessageLength] = 0;
    static unsigned int useHost = 0;
#endif

    // create the simulation object
    gSimulation = new Simulation();
    if (gOutputKinematicsFilenamePtr) gSimulation->SetOutputKinematicsFile(gOutputKinematicsFilenamePtr);
    if (gOutputModelStateFilenamePtr) gSimulation->SetOutputModelStateFile(gOutputModelStateFilenamePtr);
    if (gOutputModelStateAtTime > 0) gSimulation->SetOutputModelStateAtTime(gOutputModelStateAtTime);

    DataFile myFile;
#if !defined(USE_SOCKETS)
    myFile.SetExitOnError(true);
#endif

    // load the config file
#if defined(USE_SOCKETS)
    gHost = 0;
    
    if (gDebug == SocketDebug) *gDebugStream <<  "ReadModel useHost " << useHost 
    	<< " host " << gHosts[useHost].host 
    	<< " port " << gHosts[useHost].port 
	<< "\n";
    
    // get model config file from server
    try
    {
        gHost = new pt::ipstream(gHosts[useHost].host, gHosts[useHost].port);
        gHost->open();
        strcpy(buffer, kSocketRequestTaskMessage);
        gHost->write(buffer, kSocketMaxMessageLength);
        gHost->flush();
        if (gDebug == SocketDebug) *gDebugStream <<  "ReadModel Start String\n" << buffer << "\n";
        
	// wait up to 1000ms for data
	if (gHost->waitfor(1000) == false) throw (0);
	
        gHost->read(buffer, kSocketMaxMessageLength);
        if (gDebug == SocketDebug) *gDebugStream <<  "ReadModel Reply String\n" << buffer << "\n";
        if (strcmp(buffer, kSocketSendingTask) != 0) throw (0);
        gHost->read(buffer, 16);
        l = (int)strtol(buffer, 0, 10);
        buf = new char[l + 1];
        buf[l] = 0;
        gHost->read(buf, l);

        if (gDebug == SocketDebug) *gDebugStream << "ReadModel Config File\n" <<  buf << "\n";

        myFile.SetRawData(buf);
        delete [] buf;
        buf = 0;
    }
    catch (int e)
    {
        if (buf) delete [] buf;
        delete gSimulation;
        if (gHost) delete gHost;
        useHost++;
        if (useHost >= gHosts.size()) useHost = 0;
        return 1;
    }
    catch (pt::estream* e)
    {
        delete e;
        if (buf) delete [] buf;
        delete gSimulation;
        delete gHost;
        useHost++;
        if (useHost >= gHosts.size()) useHost = 0;
        return 1;
    }
#else
    myFile.ReadFile(gConfigFilenamePtr);
#endif

    if (gSimulation->LoadModel(myFile.GetRawData())) return 1;

    return 0;
}

void WriteModel()
{
    double score;
#if defined(USE_SOCKETS)
    char buffer[kSocketMaxMessageLength];
#endif

    score = gSimulation->CalculateInstantaneousFitness();
    // if (gSimulation->TestForCatastrophy())
    //  score -= 100000;

    if (gSummaryFlag)
        cerr << "Simulation Time: " << gSimulation->GetTime() <<
            " Steps: " << gSimulation->GetStepCount() <<
            " Score: " << score <<
            " Mechanical Energy: " << gSimulation->GetMechanicalEnergy() <<
            " Metabolic Energy: " << gSimulation->GetMetabolicEnergy() << "\n";

#if defined(USE_SOCKETS)
    try
    {
        strcpy(buffer, kSocketSendingScore);
        gHost->write(buffer, kSocketMaxMessageLength);
        sprintf(buffer, "%.17f", score);
        gHost->write(buffer, 64);
        gHost->flush();
        if (gDebug == SocketDebug) *gDebugStream << "ObjectiveMain Score\n" <<  buffer << "\n";
    }
    catch (pt::estream* e)
    {
        delete e;
    }
    delete gHost;
#else
    FILE *out;
    out = fopen(gScoreFilenamePtr, "wb");
    fwrite(&score, sizeof(double), 1, out);
    fclose(out);
#endif

    if (gDebug == MemoryDebug)
        *gDebugStream << "main About to delete gSimulation\n";
    delete gSimulation;

#if ! defined(USE_SOCKETS) && ! defined (USE_OPENGL)
    cerr << "exiting\n";
    exit(0);
#endif
}

#if defined(USE_SOCKETS)
// read the file containing a list of hosts and ports
void ParseHostlistFile(void)
{
    char buffer[256];
    char *tokens[256];
    bool result;
    int count;
    Hosts host;
    DataFile hostListFile;
    hostListFile.SetExitOnError(true);

    hostListFile.ReadFile(gHostlistFilenamePtr);

    do
    {
        result = hostListFile.ReadNextLine(buffer, 256, true, '#');
        count = DataFile::ReturnTokens(buffer, tokens, 256);
        if (count >= 2)
        {

            strcpy(host.host, tokens[0]);
            host.port = strtol(tokens[1], 0, 10);
            gHosts.push_back(host);
        }
    } while (result == false);

    if (gHosts.size() == 0)
    {
        cerr << "Could not get a list of hosts from " << gHostlistFilenamePtr << "\n";
        exit(-1);
    }
}

#endif



