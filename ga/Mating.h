// Mating.h - functions associated with mating + mutation

#ifndef MATING_H
#define MATING_H

class Genome;
class Population;

enum CrossoverType
{
   OnePoint = 0,
   Average = 1
};

void Mate(Genome *parent1, Genome *parent2, Genome *offspring, CrossoverType type);
void GaussianMutate(Genome *genome, double mutationChance);
void MultipleGaussianMutate(Genome *genome, double mutationChance);
void FrameShiftMutate(Genome *genome, double mutationChance);
void DuplicationMutate(Genome *genome, double mutationChance);

#endif // MATING_H
