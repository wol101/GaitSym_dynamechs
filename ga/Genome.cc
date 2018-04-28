// Genome.cc - basic class containing the genome for an individual

using namespace std;

#include <assert.h>
#include <iostream>
#include <float.h>

#include "Genome.h"
#include "Random.h"

double CalculateFitness(Genome *genome);

// constructor
Genome::Genome()
{
  mGenes = 0;
  mGenomeLength = 0;
  mFitness = 0;
  mLowBounds = 0;
  mHighBounds = 0;
  mGaussianSDs = 0;
  mGenomeType = IndividualRanges;
  mParent1.generationNumber = -1;
  mParent1.rank = -1;
  mParent2.generationNumber = -1;
  mParent2.rank = -1;
  mFitnessValid = false;
}

// destructor - deallocates memory
Genome::~Genome()
{
  if (mGenes) delete [] mGenes;
  if (mLowBounds) delete [] mLowBounds;
  if (mHighBounds) delete [] mHighBounds;
  if (mGaussianSDs) delete [] mGaussianSDs;
}

// initialise genome - lots of options - allocates space for genes
void Genome::InitialiseGenome(GenomeType genomeType, int genomeLength, 
                              double lowBound, double highBound, 
                              bool randomFlag, double value,
                              double gaussianSD)
{
  int i;

  assert(value >= lowBound && value <= highBound);
  assert(genomeType == IndividualRanges);

  if (mGenes) delete [] mGenes;
  if (mLowBounds) delete [] mLowBounds;
  if (mHighBounds) delete [] mHighBounds;
  if (mGaussianSDs) delete [] mGaussianSDs;

  mGenomeLength = genomeLength;
  mGenes = new double[mGenomeLength];

  if (randomFlag)
  {
    for (i = 0; i < mGenomeLength; i++)
      mGenes[i] = RandomDouble(lowBound, highBound);
  }
  else
  {
    for (i = 0; i < mGenomeLength; i++)
      mGenes[i] = value;
  }

  mGenomeType = genomeType;
  if (mGenomeType == IndividualRanges)
  {
    mLowBounds = new double[mGenomeLength];
    mHighBounds = new double[mGenomeLength];
    mGaussianSDs = new double[mGenomeLength];
    for (i = 0; i < mGenomeLength; i++)
    {
      mLowBounds[i] = lowBound;
      mHighBounds[i] = highBound;
      mGaussianSDs[i] = gaussianSD;
    }
  }
}

// randomise the genome
void Genome::Randomise()
{
  int i;

  switch (mGenomeType)
  {
  case IndividualRanges:
    {
      for (i = 0; i < mGenomeLength; i++)
      {
        if (mGaussianSDs[i] != 0)
            mGenes[i] = RandomDouble(mLowBounds[i], mHighBounds[i]);
      }
    }
    break;
  }  

  // and clear the parent
  mParent1.generationNumber = -1;
  mParent1.rank = -1;
  mParent2.generationNumber = -1;
  mParent2.rank = -1;
  
  // and make fitness invalid
  mFitnessValid = false;
}

// get gene from genome
double Genome::GetGene(int i) 
{
  assert(i >= 0 && i < mGenomeLength);
  return mGenes[i]; 
}

// set gene in genome
void Genome::SetGene(int i, double value) 
{
  assert(i >= 0 && i < mGenomeLength);
  if (mGenes[i] != value)
  {
    mGenes[i] = value;  
  
    // and make fitness invalid
    mFitnessValid = false;
  }
}

// get the fitness
double Genome::GetFitness() 
{ 
  assert(mFitnessValid);
  return mFitness;
}

// define = operator
Genome & Genome::operator=(Genome &in)
{
  int i;

  // check for mismatch
  if (mGenomeLength != in.mGenomeLength || mGenomeType != in.mGenomeType)
  {
    if (mGenes) delete [] mGenes;
    if (mLowBounds) delete [] mLowBounds;
    if (mHighBounds) delete [] mHighBounds;
    if (mGaussianSDs) delete [] mGaussianSDs;

    mGenomeLength = in.mGenomeLength;
    mGenomeType = in.mGenomeType;
    mGenes = new double[mGenomeLength];
    if (mGenomeType == IndividualRanges)
    {
      mLowBounds = new double[mGenomeLength];
      mHighBounds = new double[mGenomeLength];
      mGaussianSDs = new double[mGenomeLength];
    }
  }

  
  for (i = 0; i < mGenomeLength; i++) mGenes[i] = in.mGenes[i];
  mFitness = in.mFitness;
  mParent1 = in.mParent1;
  mParent2 = in.mParent2;
  mFitnessValid = in.mFitnessValid;
  
  if (mGenomeType == IndividualRanges)
  {
    for (i = 0; i < mGenomeLength; i++)
    {
      mLowBounds[i] = in.mLowBounds[i];
      mHighBounds[i] = in.mHighBounds[i];
      mGaussianSDs[i] = in.mGaussianSDs[i];
    }
  }
  
  return *this;
}

// output to a stream
ostream& operator<<(ostream &out, Genome &g)
{
  int i;

  // set precision (could also use manipulator)
  out.precision(DBL_DIG + 2); // added the extra digits to help with rounding error
  
  switch (g.mGenomeType)
  {
    case IndividualRanges:
      {
        out << g.mGenomeType << "\n";
        out << g.mGenomeLength << "\n";
        for (i = 0; i < g.mGenomeLength; i++)
          out << g.mGenes[i] << "\t" << g.mLowBounds[i] << "\t" << g.mHighBounds[i] 
            << "\t" << g.mGaussianSDs[i] << "\n";
        out << g.mFitness << "\t" 
            << g.mParent1.generationNumber << "\t" << g.mParent1.rank << "\t"
            << g.mParent2.generationNumber << "\t" << g.mParent2.rank
            << "\n";
      }
      break;
  }
  
  return out;
}

// input from a stream
istream& operator>>(istream &in, Genome &g)
{
  int i;
  int genomeLength;
  GenomeType genomeType;
  bool oldStyleGenomeFlag = false;
  double lowBound, highBound;
  double gaussianSD = 0;

  in >> i;
  if (i > 0) // old style genome
  {
    genomeType = IndividualRanges;
    genomeLength = i;
    oldStyleGenomeFlag = true;
    if (g.mGaussianSDs) gaussianSD = g.mGaussianSDs[0];
  }
  else
  {
    genomeType = (GenomeType)i;
    in >> genomeLength;
  }

  // check for mismatch
  if (genomeLength != g.mGenomeLength || genomeType != g.mGenomeType)
  {
    if (g.mGenes) delete [] g.mGenes;
    if (g.mLowBounds) delete [] g.mLowBounds;
    if (g.mHighBounds) delete [] g.mHighBounds;
    if (g.mGaussianSDs) delete [] g.mGaussianSDs;

    g.mGenomeLength = genomeLength;
    g.mGenomeType = genomeType;
    g.mGenes = new double[g.mGenomeLength];
    if (g.mGenomeType == IndividualRanges)
    {
      g.mLowBounds = new double[g.mGenomeLength];
      g.mHighBounds = new double[g.mGenomeLength];
      g.mGaussianSDs = new double[g.mGenomeLength];
    }
  }


  switch (g.mGenomeType)
  {
    case IndividualRanges:
      {
        if (oldStyleGenomeFlag)
        {
          for (i = 0; i < g.mGenomeLength; i++) in >> g.mGenes[i];
          in >> lowBound >> highBound 
            >> g.mFitness >> g.mParent1.generationNumber >> g.mParent1.rank
            >> g.mParent2.generationNumber >> g.mParent2.rank;
          for (i = 0; i < g.mGenomeLength; i++)
          {
            g.mLowBounds[i] = lowBound;
            g.mHighBounds[i] = highBound;
            g.mGaussianSDs[i] = gaussianSD;
          }
        }
        else
        {
          for (i = 0; i < g.mGenomeLength; i++)
            in >> g.mGenes[i] >> g.mLowBounds[i] >> g.mHighBounds[i] >> g.mGaussianSDs[i];
          in >> g.mFitness >> g.mParent1.generationNumber >> g.mParent1.rank
             >> g.mParent2.generationNumber >> g.mParent2.rank;
        }
      }
      break;
   }
  
   // always load up with fitness NOT valid
   g.mFitnessValid = false;
   return in;
}

// boolean < for pointer to Genome
bool GreaterThan(Genome *g1, Genome *g2)
{
  if (g1->mFitness > g2->mFitness) return true;
  return false;
}

