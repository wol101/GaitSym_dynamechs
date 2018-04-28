#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bool int
#define false 0
#define true (!0)

int ReturnTokens(char *string, char *ptrs[], int size);
double Mean(double *list, int n);

int main(int argc, char *argv[])
{
    FILE *in, *out;
    char buffer[10000];
    char *ptrs[1000];
    double *LeftHipFlexorVelocity = (double *)malloc(8 * 100000);
    double *LeftHipFlexorTension = (double *)malloc(8 * 100000);
    double *LeftHipExtensorVelocity = (double *)malloc(8 * 100000);
    double *LeftHipExtensorTension = (double *)malloc(8 * 100000);
    double *LeftKneeFlexorVelocity = (double *)malloc(8 * 100000);
    double *LeftKneeFlexorTension = (double *)malloc(8 * 100000);
    double *LeftKneeExtensorVelocity = (double *)malloc(8 * 100000);
    double *LeftKneeExtensorTension = (double *)malloc(8 * 100000);
    double *LeftAnkleFlexorVelocity = (double *)malloc(8 * 100000);
    double *LeftAnkleFlexorTension = (double *)malloc(8 * 100000);
    double *LeftAnkleExtensorVelocity = (double *)malloc(8 * 100000);
    double *LeftAnkleExtensorTension = (double *)malloc(8 * 100000);
    double *RightHipFlexorVelocity = (double *)malloc(8 * 100000);
    double *RightHipFlexorTension = (double *)malloc(8 * 100000);
    double *RightHipExtensorVelocity = (double *)malloc(8 * 100000);
    double *RightHipExtensorTension = (double *)malloc(8 * 100000);
    double *RightKneeFlexorVelocity = (double *)malloc(8 * 100000);
    double *RightKneeFlexorTension = (double *)malloc(8 * 100000);
    double *RightKneeExtensorVelocity = (double *)malloc(8 * 100000);
    double *RightKneeExtensorTension = (double *)malloc(8 * 100000);
    double *RightAnkleFlexorVelocity = (double *)malloc(8 * 100000);
    double *RightAnkleFlexorTension = (double *)malloc(8 * 100000);
    double *RightAnkleExtensorVelocity = (double *)malloc(8 * 100000);
    double *RightAnkleExtensorTension = (double *)malloc(8 * 100000);
    double *LeftHipFlexorVelocityPtr = LeftHipFlexorVelocity;
    double *LeftHipFlexorTensionPtr = LeftHipFlexorTension;
    double *LeftHipExtensorVelocityPtr = LeftHipExtensorVelocity;
    double *LeftHipExtensorTensionPtr = LeftHipExtensorTension;
    double *LeftKneeFlexorVelocityPtr = LeftKneeFlexorVelocity;
    double *LeftKneeFlexorTensionPtr = LeftKneeFlexorTension;
    double *LeftKneeExtensorVelocityPtr = LeftKneeExtensorVelocity;
    double *LeftKneeExtensorTensionPtr = LeftKneeExtensorTension;
    double *LeftAnkleFlexorVelocityPtr = LeftAnkleFlexorVelocity;
    double *LeftAnkleFlexorTensionPtr = LeftAnkleFlexorTension;
    double *LeftAnkleExtensorVelocityPtr = LeftAnkleExtensorVelocity;
    double *LeftAnkleExtensorTensionPtr = LeftAnkleExtensorTension;
    double *RightHipFlexorVelocityPtr = RightHipFlexorVelocity;
    double *RightHipFlexorTensionPtr = RightHipFlexorTension;
    double *RightHipExtensorVelocityPtr = RightHipExtensorVelocity;
    double *RightHipExtensorTensionPtr = RightHipExtensorTension;
    double *RightKneeFlexorVelocityPtr = RightKneeFlexorVelocity;
    double *RightKneeFlexorTensionPtr = RightKneeFlexorTension;
    double *RightKneeExtensorVelocityPtr = RightKneeExtensorVelocity;
    double *RightKneeExtensorTensionPtr = RightKneeExtensorTension;
    double *RightAnkleFlexorVelocityPtr = RightAnkleFlexorVelocity;
    double *RightAnkleFlexorTensionPtr = RightAnkleFlexorTension;
    double *RightAnkleExtensorVelocityPtr = RightAnkleExtensorVelocity;
    double *RightAnkleExtensorTensionPtr = RightAnkleExtensorTension;
    int count = 0;
    double interval = 0.0002;
#ifdef SMOOTH   
    double wantedInterval = 0.01;
    int skip = (int)(0.5 + wantedInterval/interval);
    int skipCount = 0;
#endif
    int k;
    
    in = fopen("MAMuscleDebug.txt", "r");
    
    while(fgets(buffer, sizeof(buffer), in) != NULL)
    {
        if (strstr(buffer, "MAMuscle::SetAlpha"))
        {
            ReturnTokens(buffer, ptrs, 1000);
            if (strcmp(ptrs[1], "RightHipFlexor") == 0)
            {
                count++;
                *RightHipFlexorVelocityPtr++ = strtod(ptrs[11], NULL);
                *RightHipFlexorTensionPtr++ = strtod(ptrs[21], NULL);
            }
            else if (strcmp(ptrs[1], "RightHipExtensor") == 0)
            {
                *RightHipExtensorVelocityPtr++ = strtod(ptrs[11], NULL);
                *RightHipExtensorTensionPtr++ = strtod(ptrs[21], NULL);
            }
            else if (strcmp(ptrs[1], "RightKneeFlexor") == 0)
            {
                *RightKneeFlexorVelocityPtr++ = strtod(ptrs[11], NULL);
                *RightKneeFlexorTensionPtr++ = strtod(ptrs[21], NULL);
            }
            else if (strcmp(ptrs[1], "RightKneeExtensor") == 0)
            {
                *RightKneeExtensorVelocityPtr++ = strtod(ptrs[11], NULL);
                *RightKneeExtensorTensionPtr++ = strtod(ptrs[21], NULL);
            }
            else if (strcmp(ptrs[1], "RightAnkleFlexor") == 0)
            {
                *RightAnkleFlexorVelocityPtr++ = strtod(ptrs[11], NULL);
                *RightAnkleFlexorTensionPtr++ = strtod(ptrs[21], NULL);
            }
            else if (strcmp(ptrs[1], "RightAnkleExtensor") == 0)
            {
                *RightAnkleExtensorVelocityPtr++ = strtod(ptrs[11], NULL);
                *RightAnkleExtensorTensionPtr++ = strtod(ptrs[21], NULL);
            }
            else if (strcmp(ptrs[1], "LeftHipFlexor") == 0)
            {
                *LeftHipFlexorVelocityPtr++ = strtod(ptrs[11], NULL);
                *LeftHipFlexorTensionPtr++ = strtod(ptrs[21], NULL);
            }
            else if (strcmp(ptrs[1], "LeftHipExtensor") == 0)
            {
                *LeftHipExtensorVelocityPtr++ = strtod(ptrs[11], NULL);
                *LeftHipExtensorTensionPtr++ = strtod(ptrs[21], NULL);
            }
            else if (strcmp(ptrs[1], "LeftKneeFlexor") == 0)
            {
                *LeftKneeFlexorVelocityPtr++ = strtod(ptrs[11], NULL);
                *LeftKneeFlexorTensionPtr++ = strtod(ptrs[21], NULL);
            }
            else if (strcmp(ptrs[1], "LeftKneeExtensor") == 0)
            {
                *LeftKneeExtensorVelocityPtr++ = strtod(ptrs[11], NULL);
                *LeftKneeExtensorTensionPtr++ = strtod(ptrs[21], NULL);
            }
            else if (strcmp(ptrs[1], "LeftAnkleFlexor") == 0)
            {
                *LeftAnkleFlexorVelocityPtr++ = strtod(ptrs[11], NULL);
                *LeftAnkleFlexorTensionPtr++ = strtod(ptrs[21], NULL);
            }
            else if (strcmp(ptrs[1], "LeftAnkleExtensor") == 0)
            {
                *LeftAnkleExtensorVelocityPtr++ = strtod(ptrs[11], NULL);
                *LeftAnkleExtensorTensionPtr++ = strtod(ptrs[21], NULL);
            }
        }
    }
    
    fclose(in);
    
    out = fopen("output.txt", "w");
    fprintf(out,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", 
        "time", 
        "LeftHipFlexorVelocity", "LeftHipFlexorTension", 
        "LeftHipExtensorVelocity", "LeftHipExtensorTension", 
        "LeftKneeFlexorVelocity", "LeftKneeFlexorTension", 
        "LeftKneeExtensorVelocity", "LeftKneeExtensorTension", 
        "LeftAnkleFlexorVelocity", "LeftAnkleFlexorTension", 
        "LeftAnkleExtensorVelocity", "LeftAnkleExtensorTension", 
        "RightHipFlexorVelocity", "RightHipFlexorTension", 
        "RightHipExtensorVelocity", "RightHipExtensorTension", 
        "RightKneeFlexorVelocity", "RightKneeFlexorTension", 
        "RightKneeExtensorVelocity", "RightKneeExtensorTension", 
        "RightAnkleFlexorVelocity", "RightAnkleFlexorTension", 
        "RightAnkleExtensorVelocity", "RightAnkleExtensorTension");

#ifdef SMOOTH   
    for (k = skip; k < count - skip; k += skip)
    {
  	fprintf(out,"%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", 
            (double)k * interval, 
            Mean(&(LeftHipFlexorVelocity[k - skip / 2]), skip),
            Mean(&(LeftHipFlexorTension[k - skip / 2]), skip),
            Mean(&(LeftHipExtensorVelocity[k - skip / 2]), skip),
            Mean(&(LeftHipExtensorTension[k - skip / 2]), skip),
            Mean(&(LeftKneeFlexorVelocity[k - skip / 2]), skip),
            Mean(&(LeftKneeFlexorTension[k - skip / 2]), skip),
            Mean(&(LeftKneeExtensorVelocity[k - skip / 2]), skip),
            Mean(&(LeftKneeExtensorTension[k - skip / 2]), skip),
            Mean(&(LeftAnkleFlexorVelocity[k - skip / 2]), skip),
            Mean(&(LeftAnkleFlexorTension[k - skip / 2]), skip),
            Mean(&(LeftAnkleExtensorVelocity[k - skip / 2]), skip),
            Mean(&(LeftAnkleExtensorTension[k - skip / 2]), skip),
            Mean(&(RightHipFlexorVelocity[k - skip / 2]), skip),
            Mean(&(RightHipFlexorTension[k - skip / 2]), skip),
            Mean(&(RightHipExtensorVelocity[k - skip / 2]), skip),
            Mean(&(RightHipExtensorTension[k - skip / 2]), skip),
            Mean(&(RightKneeFlexorVelocity[k - skip / 2]), skip),
            Mean(&(RightKneeFlexorTension[k - skip / 2]), skip),
            Mean(&(RightKneeExtensorVelocity[k - skip / 2]), skip),
            Mean(&(RightKneeExtensorTension[k - skip / 2]), skip),
            Mean(&(RightAnkleFlexorVelocity[k - skip / 2]), skip),
            Mean(&(RightAnkleFlexorTension[k - skip / 2]), skip),
            Mean(&(RightAnkleExtensorVelocity[k - skip / 2]), skip),
            Mean(&(RightAnkleExtensorTension[k - skip / 2]), skip));
    }
#else
    for (k = 0; k < count; k++)
    {
  	fprintf(out,"%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", 
            (double)k * interval, 
            LeftHipFlexorVelocity[k],
            LeftHipFlexorTension[k],
            LeftHipExtensorVelocity[k],
            LeftHipExtensorTension[k],
            LeftKneeFlexorVelocity[k],
            LeftKneeFlexorTension[k],
            LeftKneeExtensorVelocity[k],
            LeftKneeExtensorTension[k],
            LeftAnkleFlexorVelocity[k],
            LeftAnkleFlexorTension[k],
            LeftAnkleExtensorVelocity[k],
            LeftAnkleExtensorTension[k],
            RightHipFlexorVelocity[k],
            RightHipFlexorTension[k],
            RightHipExtensorVelocity[k],
            RightHipExtensorTension[k],
            RightKneeFlexorVelocity[k],
            RightKneeFlexorTension[k],
            RightKneeExtensorVelocity[k],
            RightKneeExtensorTension[k],
            RightAnkleFlexorVelocity[k],
            RightAnkleFlexorTension[k],
            RightAnkleExtensorVelocity[k],
            RightAnkleExtensorTension[k]);
    }
#endif
    fclose(out);
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

// calculate the mean of a list of numbers
double Mean(double *list, int n)
{
    int i;
    double total = 0;
    double *ptr = list;
    for (i = 0; i < n; i++)
    {
        total += *ptr;
        ptr++;
    }
    
    return (total / (double)n);
}


            
            
