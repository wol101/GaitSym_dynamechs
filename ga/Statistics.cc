// Statistics.cc - calculate various statistics from a population

using namespace std;

#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <float.h>

#include "Statistics.h"
#include "Genome.h"
#include "Population.h"

// calculate standard statistics
void CalculateStatistics(Population *thePopulation, Statistics *stats)
{
  int i;
  double min = DBL_MAX;
  double max = -DBL_MAX;
  double sum = 0;
  double sumSquareDiff = 0;
  double variance;
  double mean;
  double fitness;
  double diff;
  double sd;
  int populationSize = thePopulation->GetPopulationSize();

  for (i = 0; i < populationSize; i++)
  {
    fitness = thePopulation->GetGenome(i)->GetFitness();
    if (fitness > max) max = fitness;
    if (fitness < min) min = fitness;
    sum += fitness;
  }

  mean = sum / populationSize;
  for (i = 0; i < populationSize; i++)
  {
    fitness = thePopulation->GetGenome(i)->GetFitness();
    diff = fitness - mean;
    sumSquareDiff += diff * diff;
  }
  variance = sumSquareDiff / populationSize;
  sd = sqrt(variance);

  stats->min = min;
  stats->max = max;
  stats->mean = mean;
  stats->sd = sd;
}

// output to a stream
ostream& operator<<(ostream &out, Statistics &s)
{
  out << s.max << "\t" << s.mean << "\t"
	  << s.min << "\t" << s.sd;
  
  return out;
}

// calculate percentiles at 10% intervals
// ASSUMES POPULATION IS SORTED!
void CalculateTenPercentiles(Population *thePopulation, TenPercentiles *perc)
{
  int lastGenome = thePopulation->GetPopulationSize() - 1;
  int i;
  
  for (i = 0; i < 11; i ++)
  {
    perc->values[i] = 
        thePopulation->GetGenome((i * lastGenome) / 10)->GetFitness();
  }
}

// output to a stream
ostream& operator<<(ostream &out, TenPercentiles &s)
{
  int i;
  char buffer[32];
    
  for (i = 0; i < 11; i++)
  {
    sprintf(buffer, "%.5f", s.values[i]);
    buffer[7] = 0;
    if (i == 10) out << buffer;
    else out << buffer << "\t";
  }
  
  return out;
}
