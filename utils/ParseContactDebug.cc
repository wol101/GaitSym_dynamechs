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
    double *LeftFootContactX = (double *)malloc(8 * 100000);
    double *RightFootContactX = (double *)malloc(8 * 100000);
    double *LeftFootContactXPtr = LeftFootContactX;
    double *RightFootContactXPtr = RightFootContactX;
    double *LeftFootContactZ = (double *)malloc(8 * 100000);
    double *RightFootContactZ = (double *)malloc(8 * 100000);
    double *LeftFootContactZPtr = LeftFootContactZ;
    double *RightFootContactZPtr = RightFootContactZ;
    int count = 0;
    double interval = 0.0002;
#ifdef SMOOTH   
    double wantedInterval = 0.01;
    int skip = (int)(0.5 + wantedInterval/interval);
    int skipCount = 0;
#endif
    int k;
    
    in = fopen("ContactDebug.txt", "r");
    
    while(fgets(buffer, sizeof(buffer), in) != NULL)
    {
        if (strstr(buffer, "ModifiedContactModel::computeForce"))
        {
            ReturnTokens(buffer, ptrs, 1000);
            if (strcmp(ptrs[1], "LeftFootContact") == 0)
            {
                count++;
                *LeftFootContactXPtr++ = strtod(ptrs[6], NULL);
                *LeftFootContactZPtr++ = strtod(ptrs[8], NULL);
            }
            else if (strcmp(ptrs[1], "RightFootContact") == 0)
            {
                *RightFootContactXPtr++ = strtod(ptrs[6], NULL);
                *RightFootContactZPtr++ = strtod(ptrs[8], NULL);
            }
        }
    }
    
    fclose(in);
    
    out = fopen("output.txt", "w");
    fprintf(out,"%s\t%s\t%s\t%s\t%s\n", 
        "time", 
        "LeftFootContactX", "LeftFootContactZ", 
        "RightFootContactX", "RightFootContactZ");

#ifdef SMOOTH   
    for (k = skip; k < count - skip; k += skip)
    {
  	fprintf(out,"%f\t%f\t%f\t%f\t%f\n", 
            (double)k * interval, 
            Mean(&(LeftFootContactX[k - skip / 2]), skip),
            Mean(&(LeftFootContactZ[k - skip / 2]), skip),
            Mean(&(RightFootContactX[k - skip / 2]), skip),
            Mean(&(RightFootContactZ[k - skip / 2]), skip));
    }
#else
    for (k = 0; k < count; k++)
    {
  	fprintf(out,"%f\t%f\t%f\t%f\t%f\n", 
            (double)k * interval, 
            LeftFootContactX[k],
            LeftFootContactZ[k],
            RightFootContactX[k],
            RightFootContactZ[k]);
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


            
            
