using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <iostream>

#define MAX_VALUES 100000
#define MAX_ONOFF 1000
#define LINE_BUFFER_SIZE 100000
#define MAX_VALUES_PER_LINE 1000

int ReturnTokens(char *string, char *ptrs[], int size);
void FindOnOff(double *data, int len, 
        double interval, double smoothInterval, double threshold,
        double *switchOn, int *switchOnLen, double *switchOff, int *switchOffLen);

int main(int argc, char *argv[])
{
    FILE *in;
    char buffer[LINE_BUFFER_SIZE];
    char *ptrs[MAX_VALUES_PER_LINE];
    double *LeftFootContactX = new double[MAX_VALUES];
    double *RightFootContactX = new double[MAX_VALUES];
    double *LeftFootContactXPtr = LeftFootContactX;
    double *RightFootContactXPtr = RightFootContactX;
    double *LeftFootContactY = new double[MAX_VALUES];
    double *RightFootContactY = new double[MAX_VALUES];
    double *LeftFootContactYPtr = LeftFootContactY;
    double *RightFootContactYPtr = RightFootContactY;
    double *LeftFootContactZ = new double[MAX_VALUES];
    double *RightFootContactZ = new double[MAX_VALUES];
    double *LeftFootContactZPtr = LeftFootContactZ;
    double *RightFootContactZPtr = RightFootContactZ;
    double *LeftHandContactX = new double[MAX_VALUES];
    double *RightHandContactX = new double[MAX_VALUES];
    double *LeftHandContactXPtr = LeftHandContactX;
    double *RightHandContactXPtr = RightHandContactX;
    double *LeftHandContactY = new double[MAX_VALUES];
    double *RightHandContactY = new double[MAX_VALUES];
    double *LeftHandContactYPtr = LeftHandContactY;
    double *RightHandContactYPtr = RightHandContactY;
    double *LeftHandContactZ = new double[MAX_VALUES];
    double *RightHandContactZ = new double[MAX_VALUES];
    double *LeftHandContactZPtr = LeftHandContactZ;
    double *RightHandContactZPtr = RightHandContactZ;
    int count = 0;
    double interval = 5e-5;
    double smoothInterval = 0.01;
    double threshold = 0.001;
    int i, j;
    char *filename = 0;
    
    // parse the command line
    try
    {
        for (i = 1; i < argc; i++)
        {
        // do something with arguments

        if (strcmp(argv[i], "--file") == 0 ||
              strcmp(argv[i], "-f") == 0)
            {
                i++;
                if (i >= argc) throw(1);
                filename = argv[i];
            }

            else if (strcmp(argv[i], "--threshold") == 0 ||
              strcmp(argv[i], "-t") == 0)
            {
                i++;
                if (i >= argc) throw(1);
                threshold = strtod(argv[i], 0);
            }

            else if (strcmp(argv[i], "--interval") == 0 ||
              strcmp(argv[i], "-i") == 0)
            {
                i++;
                if (i >= argc) throw(1);
                interval = strtod(argv[i], 0);
            }

            else if (strcmp(argv[i], "--smoothInterval") == 0 ||
              strcmp(argv[i], "-s") == 0)
            {
                i++;
                if (i >= argc) throw(1);
                smoothInterval = strtod(argv[i], 0);
            }


            else
            {   
                throw(1);
            }
        }
    }
  
    // catch argument errors
    catch (int e)
    {
        cerr << "\nCalculate Phase from ContactDebug file\n";
        cerr << "Build " << __DATE__ << " " << __TIME__ << "\n";
        cerr << "Supported options:\n\n";
        cerr << "(-f) --file filename\n\n";
        cerr << "(-t) --threshold x\n\n";
        cerr << "(-i) --interval x\n\n";
        cerr << "(-s) --smoothInterval x\n\n";
        cerr << "\nNote: if filename not specified reads from stdin\n";
        cerr << "Short form is shown in brackets.\n";
        return (e);
    }
    
    if (filename) in = fopen(filename, "r");
    else in = stdin;
    
    // read in the array values 
    while(fgets(buffer, sizeof(buffer), in) != NULL)
    {
        if (strstr(buffer, "ModifiedContactModel::computeForce"))
        {
            ReturnTokens(buffer, ptrs, MAX_VALUES_PER_LINE);
            if (strcmp(ptrs[1], "LeftFootContact") == 0)
            {
                count++;
                *LeftFootContactXPtr++ = strtod(ptrs[6], NULL);
                *LeftFootContactYPtr++ = strtod(ptrs[7], NULL);
                *LeftFootContactZPtr++ = strtod(ptrs[8], NULL);
            }
            else if (strcmp(ptrs[1], "RightFootContact") == 0)
            {
                *RightFootContactXPtr++ = strtod(ptrs[6], NULL);
                *RightFootContactYPtr++ = strtod(ptrs[7], NULL);
                *RightFootContactZPtr++ = strtod(ptrs[8], NULL);
            }
            else if (strcmp(ptrs[1], "LeftHandContact") == 0)
            {
                *LeftHandContactXPtr++ = strtod(ptrs[6], NULL);
                *LeftHandContactYPtr++ = strtod(ptrs[7], NULL);
                *LeftHandContactZPtr++ = strtod(ptrs[8], NULL);
            }
            else if (strcmp(ptrs[1], "RightHandContact") == 0)
            {
                *RightHandContactXPtr++ = strtod(ptrs[6], NULL);
                *RightHandContactYPtr++ = strtod(ptrs[7], NULL);
                *RightHandContactZPtr++ = strtod(ptrs[8], NULL);
            }
        }
    }
    
    if (filename) fclose(in);
    
    double *LeftFootOn = new double[MAX_ONOFF];
    double *LeftFootOff = new double[MAX_ONOFF];
    double *RightFootOn = new double[MAX_ONOFF];
    double *RightFootOff = new double[MAX_ONOFF];
    double *LeftHandOn = new double[MAX_ONOFF];
    double *LeftHandOff = new double[MAX_ONOFF];
    double *RightHandOn = new double[MAX_ONOFF];
    double *RightHandOff = new double[MAX_ONOFF];
    int LeftFootOnLen;
    int LeftFootOffLen;
    int RightFootOnLen;
    int RightFootOffLen;
    int LeftHandOnLen;
    int LeftHandOffLen;
    int RightHandOnLen;
    int RightHandOffLen;
    
    FindOnOff(LeftFootContactZ, count, 
        interval, smoothInterval, threshold,
        LeftFootOn, &LeftFootOnLen, LeftFootOff, &LeftFootOffLen);
    FindOnOff(RightFootContactZ, count, 
        interval, smoothInterval, threshold,
        RightFootOn, &RightFootOnLen, RightFootOff, &RightFootOffLen);
    FindOnOff(LeftHandContactZ, count, 
        interval, smoothInterval, threshold,
        LeftHandOn, &LeftHandOnLen, LeftHandOff, &LeftHandOffLen);
    FindOnOff(RightHandContactZ, count, 
        interval, smoothInterval, threshold,
        RightHandOn, &RightHandOnLen, RightHandOff, &RightHandOffLen);
    
    int lastJ = 0;
    double cycleDuration, delay, phase;
    printf("cycleDuration\tdelay\tphase\n");
    for (i = 0; i < LeftFootOnLen - 1; i++)
    {
        cycleDuration = LeftFootOn[i + 1] - LeftFootOn[i];
        for (j = lastJ; j < LeftHandOnLen; j++)
        {
            if (LeftHandOn[j] > LeftFootOn[i])
            {
                lastJ = j;
                delay = LeftHandOn[j] - LeftFootOn[i];
                phase = delay / cycleDuration;
                printf("%f\t%f\t%f\n", cycleDuration, delay, phase);
                break;
            }
        }
    }
}

// Return tokens utility
// note string is altered by this routine
// if returned count is >= size then there are still tokens
// (this is probably an error status)
// recommend that tokens are counted first
int ReturnTokens(char *string, char *ptrs[], int size)
{
    char *p = string;
    bool inToken = false;
    int count = 0;

    while (*p != 0)
    {
        if (inToken == false && *p > 32)
        {
            inToken = true;
            if (count >= size) return count;
            ptrs[count] = p;
            count++;
            if (*p == '"')
            {
                p++;
                ptrs[count - 1] = p;
                while (*p != '"')
                {
                    p++;
                    if (*p == 0) return count;
                }
                *p = 0;
            }
        }
        else if (inToken == true && *p <= 32)
        {
            inToken = false;
            *p = 0;
        }
        p++;
    }
    return count;
}

// find On/Off change times

void FindOnOff(double *data, int len, 
        double interval, double smoothInterval, double threshold,
        double *switchOn, int *switchOnLen, double *switchOff, int *switchOffLen)
{
    int i, j;
    int oldState, newState;
    char *buffer;
    int smoothLen = (int)(smoothInterval / interval);
    int timeAtState = 0;
    int lastTimeAtState = 0;
    int lastStateChange = 0;
    
    assert(len > 0);
    
    buffer = new char[len];
    for (i = 0; i < len; i++)
    {
        if (data[i] > threshold) buffer[i] = 1;
        else buffer[i] = 0;
    }
    
    // get rid of short state changes
    if (buffer[0]) oldState = 1;
    else oldState = 0;
    for (i = 1; i < len; i++)
    {
        if (buffer[i]) newState = 1;
        else newState = 0;
        if (newState == oldState) timeAtState++;
        else
        {
            if (timeAtState < smoothLen)
            {
                for (j = lastStateChange; j < i; j++) buffer[j] = newState;
                timeAtState += lastTimeAtState;
                lastStateChange -= lastTimeAtState;
                oldState = newState;
            }
            else
            {
                lastTimeAtState = timeAtState;
                timeAtState = 0;
                oldState = newState;
                lastStateChange = i;
            }
        }
    }
    
    if (buffer[0]) oldState = 1;
    else oldState = 0;
    
    *switchOnLen = 0;
    *switchOffLen = 0;
    
    for (i = 1; i < len; i++)
    {
        if (buffer[i]) newState = 1;
        else newState = 0;
        
        if (newState != oldState)
        {
            if (oldState == 0)
            {
                switchOn[*switchOnLen] = (double)i * interval;
                (*switchOnLen)++;
            }
            else
            {
                switchOff[*switchOffLen] = (double)i * interval;
                (*switchOffLen)++;
            }
            oldState = newState;
        }
    }
    
    delete [] buffer;
}
            
            
