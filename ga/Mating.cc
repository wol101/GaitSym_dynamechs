// Mating.cc - functions associated with mating + mutation

using namespace std;

#include <iostream>

#include "Mating.h"
#include "Random.h"
#include "Genome.h"
#include "Debug.h"

// mate two parents producing an offspring
// some crossover *ALWAYS* occurs
void Mate(Genome *parent1, Genome *parent2, Genome *offspring, CrossoverType type)
{

  int i;
  int genomeLength = offspring->GetGenomeLength();
  
  switch (type)
  {      
      case OnePoint:  
      {   
         int crossoverPoint = RandomInt(1, genomeLength - 1);

         for (i = 0; i < crossoverPoint; i++)
           offspring->SetGene(i, parent1->GetGene(i));

         for (i = crossoverPoint; i < genomeLength; i++)
           offspring->SetGene(i, parent2->GetGene(i));
         
         break;
      }
      
      case Average:
      {
         for (i = 0; i < genomeLength; i++)
            offspring->SetGene(i, (parent1->GetGene(i) + parent2->GetGene(i)) / 2.0);
         break;
      }
   }         
}

// mutate an individual by adding a gaussian distributed double
// if gaussianSD < 0 then use value times current value as SD
void GaussianMutate(Genome *genome, double mutationChance)
{
  double v = 0;
  int genomeLength = genome->GetGenomeLength();
  int location;
  double r;
  
  if (mutationChance == 0) return;
  if (mutationChance < 1.0)
  {
    if (!CoinFlip(mutationChance)) return;
  }
  
  // gaussian mutator
  r = RandomUnitGaussian();
  location = RandomInt(0, genomeLength - 1);
  if (genome->GetGaussianSD(location) > 0)
  {
    v = genome->GetGene(location) + r * genome->GetGaussianSD(location);
    if (v < genome->GetLowBound(location)) v = genome->GetLowBound(location);
    if (v > genome->GetHighBound(location)) v = genome->GetHighBound(location);
    genome->SetGene(location, v);
  }
  if (gDebug == Debug_Mating)
    cerr << "GaussianMutate r = " << r << " v = " << v << "\n";
}

// mutate an individual by adding a gaussian distributed double
// with a finite chance per gene
// if gaussianSD <= 0 then don't mutate
void MultipleGaussianMutate(Genome *genome, double mutationChance)
{
  // use the mutation chance on each gene

  int i;
  double v = 0;
  int genomeLength = genome->GetGenomeLength();
  double r;
  
  if (mutationChance == 0) return;
  
  // gaussian mutator
  for (i = 0; i < genomeLength; i++)
  {
    if (CoinFlip(mutationChance)) 
    {
      r = RandomUnitGaussian();
      if (genome->GetGaussianSD(i) > 0)
      {
        v = genome->GetGene(i) + r * genome->GetGaussianSD(i);
        if (v < genome->GetLowBound(i)) v = genome->GetLowBound(i);
        if (v > genome->GetHighBound(i)) v = genome->GetHighBound(i);
        genome->SetGene(i, v);
      }
      if (gDebug == Debug_Mating)
        cerr << "MultipleGaussianMutate r = " << r << " v = " << v << "\n";
    }
  }
}
 
// mutate an individual by inserting or deleting a gene
void FrameShiftMutate(Genome *genome, double mutationChance)
{
  int i;
  int location;
  int genomeLength = genome->GetGenomeLength();
  
  if (mutationChance == 0) return;
  if (mutationChance < 1.0)
  {
    if (!CoinFlip(mutationChance)) return;
  }
  
  // insertion/deletion
  location = RandomInt(0, genomeLength - 1);
  if (CoinFlip(0.5))
  {
    // deletion
    for (i = location; i < genomeLength - 1; i++)
      genome->SetGene(i, genome->GetGene(i + 1));
  }
  else
  {
    // insertion
    for (i = genomeLength - 2; i >= location; i--)
      genome->SetGene(i + 1, genome->GetGene(i));
  }
}

// mutate an individual by duplicating a block and inserting it in a 
// random location
void DuplicationMutate(Genome *genome, double mutationChance)
{
  int i;
  int genomeLength = genome->GetGenomeLength();
  int origin;
  int length;
  int insertion;
  double *store;
  
  if (mutationChance == 0) return;
  if (mutationChance < 1.0)
  {
    if (!CoinFlip(mutationChance)) return;
  }

  // calculate segment to copy
  origin = RandomInt(0, genomeLength - 1);
  if (genomeLength - origin == 1) length = 1;
  else length = RandomInt(1, genomeLength - origin);
  
  // make a copy
  store = new double[length];
  for (i = 0; i < length; i++)
    store[i] = genome->GetGene(origin + i);
  
  // and write the copy into the genome
  insertion = RandomInt(0, genomeLength - 1);
  for (i = 0; i < length; i++)
  {
    genome->SetGene(insertion, store[i]);
    insertion++;
    if (insertion >= genomeLength) break;
  }
  
  delete [] store;
}

  
  


