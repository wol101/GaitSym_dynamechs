// Genome.h - basic class containing the genome for an individual

#ifndef GENOME_H
#define GENOME_H

#include <iostream>

struct Parent
{
  int generationNumber;
  int rank;
};

enum GenomeType
{
  IndividualRanges = -1
};

class Genome
{
public:

  Genome();
  ~Genome();

  void InitialiseGenome(GenomeType genomeType,
                        int genomeLength,
                        double lowBound,
                        double highBound,
                        bool randomFlag, 
                        double value,
                        double gaussianSD);
  double GetGene(int i);
  int GetGenomeLength() { return mGenomeLength; };
  double GetHighBound(int i) { return mHighBounds[i]; };
  double GetLowBound(int i) { return mLowBounds[i]; };
  double GetGaussianSD(int i) { return mGaussianSDs[i]; };
  bool GetFitnessValid() { return mFitnessValid; };
  double GetFitness(); 
  Parent *GetParent1() { return &mParent1; };
  Parent *GetParent2() { return &mParent2; };
  GenomeType GetGenomeType() { return mGenomeType; };

  void Randomise();
  void SetGene(int i, double value);
  void SetFitness(double fitness) 
      { 
        mFitness = fitness; 
        mFitnessValid = true;
      };
  void SetParent1(int generationNumber, int parentRank) 
      { 
        mParent1.generationNumber = generationNumber;
        mParent1.rank = parentRank;
      };
  void SetParent2(int generationNumber, int parentRank) 
      { 
        mParent2.generationNumber = generationNumber;
        mParent2.rank = parentRank;
      };

  Genome& operator=(Genome &in);

  friend ostream& operator<<(ostream &out, Genome &g);
  friend istream& operator>>(istream &in, Genome &g);
  
  friend bool GreaterThan(Genome *g1, Genome *g2);

protected:

  int mGenomeLength;
  double *mGenes;
  double mFitness;
  Parent mParent1;
  Parent mParent2;
  GenomeType mGenomeType;
  double *mLowBounds;
  double *mHighBounds;
  double *mGaussianSDs;
  bool mFitnessValid;
};

#endif // GENOME_H
