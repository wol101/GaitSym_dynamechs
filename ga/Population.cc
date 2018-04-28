// Population.cc - storage class for individual genomes

using namespace std;

#include <assert.h>
#include <iostream>
#include <unistd.h>

#include "Genome.h"
#include "Population.h"
#include "Random.h"
#include "Objective.h"
#include "QuickSort.h"
#include "Debug.h"

// constructor
Population::Population()
{
  m_Population = 0;
  m_PopulationSize = 0;
  m_SelectionType = RankBasedSelection;
  m_MaxRunning = 1;
  m_CumulativeFitness = 0;
}

// destructor
Population::~Population()
{
  int i;
  if (m_Population) 
  {
    for (i = 0; i < m_PopulationSize; i++)
    delete m_Population[i];
    delete [] m_Population;
  }
  if (m_CumulativeFitness) delete [] m_CumulativeFitness;
}

// initialise the population 
void Population::InitialisePopulation(int populationSize,
      GenomeType genomeType, 
      int genomeLength,
      double lowBound,
      double highBound,
      bool randomFlag, 
      double value,
      double gaussianSD)
{
  int i;

  m_PopulationSize = populationSize;
  
  m_CumulativeFitness = new double[m_PopulationSize];
  m_Population = new Genome *[m_PopulationSize];
  for (i = 0; i < m_PopulationSize; i++)
  {
    m_Population[i] = new Genome();
    m_Population[i]->InitialiseGenome(genomeType, genomeLength, 
                    lowBound, highBound,
                    randomFlag, value, gaussianSD);
  }
}

// choose a parent from a population
Genome * Population::ChooseParent(int *parentRank)
{
  double r;
  int l, u, p, d;

  // in this version we do uniform selection and just choose a parent
  // at random
  if (m_SelectionType == UniformSelection)
  {
    *parentRank = RandomInt(0, m_PopulationSize - 1);
    return m_Population[*parentRank];
  }

  // this type biases random choice to higher ranked individuals
  // this assumes a sorted genome
  if (m_SelectionType == RankBasedSelection)
  {
    *parentRank = RankBiasedRandomInt(1, m_PopulationSize) - 1;
    return m_Population[*parentRank];
  }
  
  // this type biases random choice to higher ranked individuals
  // this assumes a sorted genome
  if (m_SelectionType == SqrtBasedSelection)
  {
    *parentRank = SqrtBiasedRandomInt(0, m_PopulationSize - 1);
    return m_Population[*parentRank];
  }

  // this type biases random choice to higher ranked individuals
  // again assumes a sorted genome since m_CumulativeFitness is
  // allocated at sort time
  // MUST HAVE POSITIVE FITNESS
  if (m_SelectionType == RouletteWheelSelection)
  {
    // if fitness total is zero fall back on uniform selection
    if (m_CumulativeFitness[m_PopulationSize - 1] == 0)
    {
      *parentRank = RandomInt(0, m_PopulationSize - 1);
      return m_Population[*parentRank];
    }
      
    // first need to get a suitable random number
    r = RandomDouble(0, m_CumulativeFitness[m_PopulationSize - 1]);
    // now find the index where: 
    // m_CumulativeFitness[index - 1] < r <= m_CumulativeFitness[index]
    l = 0;
    u = m_PopulationSize - 1;
    p = (u - l) / 2;
    while (true)
    {
      if (m_CumulativeFitness[p - 1] < r && m_CumulativeFitness[p] >= r) break;
      if (r > m_CumulativeFitness[p]) 
      {
        l = p;
        d = (u - l) / 2;
        if (d == 0) d = 1;
        p += d;
      }
      else 
      {
        u = p;
        d = (u - l) / 2;
        if (d == 0) d = 1;
        p -= d;
      }
    }
    *parentRank = p;
    
    if (gDebug == Debug_Population)
      cerr << "Population::ChooseParent\tRouletteWheelSelection\tr\t" <<
      r << "\tparentRank\t" << *parentRank << "\tm_CumulativeFitness\t" <<
      m_CumulativeFitness[p] << "\n";
      
    return m_Population[*parentRank];
  }
  
  // should never get here
  return 0;
}

// sort by fitness and fill in the m_CumulativeFitness if we
// are using it
void Population::Sort()
{
  double total;
  double fitness;
  int i;
  
  QuickSort(m_Population, m_PopulationSize);
  
  if (m_SelectionType == RouletteWheelSelection)
  {
    // need to fill m_CumulativeFitness
    total = 0;
    for (i = 0; i < m_PopulationSize; i++)
    {
      fitness = m_Population[i]->GetFitness();
      if (fitness < 0) fitness = 0;
      m_CumulativeFitness[i] = total + fitness;
      total += fitness;
      
      if (gDebug == Debug_Population)
        cerr << "Population::Sort\tm_CumulativeFitness[\t" << i <<
        "\t]\t" << m_CumulativeFitness[i] << "\n";
    }
  }
}

// randomise a subset of the population
void Population::Randomise(int from, int to)
{
  int i;
  
  for (i = from; i < to; i++)
    m_Population[i]->Randomise();
}

#if defined (USE_FS)
    
// calculate the fitness for a subset of the population
void Population::CalculateFitness(int from, int to, char *programName, time_t timeLimit,
                      int restartOnError)
{
  int i, j;
  int nRunning = 0;
  
  if (m_MaxRunning == 1)
  {
    Objective objective;
    objective.SetSynchronousFlag(true);
    if (programName) objective.SetProgramName(programName);
    // loop through the population
    for (i = from; i < to; i++)
    {
      if (m_Population[i]->GetFitnessValid() == false)
      {
        objective.Run(i, m_Population[i]);
        objective.CheckFinished(); // needed anyway
        m_Population[i]->SetFitness(objective.GetScore());
        objective.CleanUp();
      }
    }
  }
  else
  {
    Objective *runList = new Objective[m_MaxRunning];
    if (programName)
    {
      for (i = 0; i < m_MaxRunning; i++)
      {
        runList[i].SetProgramName(programName);
      }
    }
   // loop through the population
    for (i = from; i < to; i++)
    {
      if (m_Population[i]->GetFitnessValid() == false)
      {
        while (nRunning >= m_MaxRunning) // need to look for some finished processes
        {
          usleep(10000); // sleep for 10ms - might need to tweak
          // look for a finished process
          for (j = 0; j < m_MaxRunning; j++)
          {
            if (runList[j].CheckFinished())
            {
              m_Population[runList[j].GetID()]->SetFitness(runList[j].GetScore());
              runList[j].CleanUp();
              nRunning--;
            }
          }
        }

        // must now be space to start up a new process
        for (j = 0; j < m_MaxRunning; j++)
        {
          if (runList[j].Valid() == false) // find gap in list (there *must* be one)
          {
            runList[j].Run(i, m_Population[i]);
            nRunning++;
            break;
          }
        }
      }
    }

    // there are probably still a few processes running
    while (nRunning)
    {
      for (j = 0; j < m_MaxRunning; j++)
      {
        if (runList[j].CheckFinished())
        {
          m_Population[runList[j].GetID()]->SetFitness(runList[j].GetScore());
          runList[j].CleanUp();
          nRunning--;
        }
      }
    }      
    delete [] runList;
  }
}


#elif defined(USE_SOCKETS)

// calculate the fitness for a subset of the population    
void Population::CalculateFitness(int from, int to, 
    char *programName, time_t timeLimit, 
    int restartOnError)
{
  int i, j;
  bool stillWaiting;
  
  if (gDebug == Debug_Population)
    cerr << "Population::CalculateFitness\tfrom\t" << from << "\tto\t" << to
    << "\ttimeLimit\t" << timeLimit << "\n";
  
  
  int runListSize = to - from;
  Objective *runList = new Objective[runListSize];
  
  // loop through runList starting processes
  for (i = 0; i < runListSize; i++)
  {
    // set up the data
    runList[i].SetID(from + i);
    
    //if (m_Population[from + i]->GetFitnessValid() == true)
    //  cerr << "m_Population[" << from + i << "]->GetFitnessValid() is true\n";

    if (m_Population[from + i]->GetFitnessValid() == false)
    {
      runList[i].SetGenome(m_Population[from + i]);
      runList[i].SetTimeLimit(timeLimit);
      if (programName) runList[i].SetProgramName(programName);
      
      // and wait until the process starts up
      while (runList[i].Run() == false) 
      {
        // cerr << "runList[i].Run() " << i << "\n";
        for (j = 0; j < i; j++) runList[j].CheckRunning();
        usleep(10000);
      }
    }
  }
  
  // now loop until all the processes have finished
  do
  {
    stillWaiting = false;
    for (j = 0; j < runListSize; j++) 
    {
      if (runList[j].CheckRunning()) stillWaiting = true;
      // cerr << "CalculateFitness Objective::CheckRunning() stillWaiting " << stillWaiting << "\n";
    }
    Objective::ReportBusy();
    //cerr << "CalculateFitness Objective::ReportBusy() runListSize " << runListSize << " stillWaiting " << stillWaiting << "\n";
  } while (stillWaiting);

  // all finished now so read off the fitness values
  for (i = 0; i < runListSize; i++)
  {
    //if (m_Population[runList[i].GetID()]->GetFitnessValid() == true)
    //  cerr << "m_Population[" << runList[i].GetID() << "]->GetFitnessValid() is true\n";
    
    if (m_Population[runList[i].GetID()]->GetFitnessValid() == false)
      m_Population[runList[i].GetID()]->SetFitness(runList[i].GetScore());
  }
  
  // and delete the run list  
  delete [] runList;
}


#endif

// Resort a previously sorted genome so that only unique fitness elements
// are represented (duplicates are put to the end of the list)
void Population::UniqueResort()
{
  int i, j;
  Genome *t;
  
  for (i = m_PopulationSize - 1; i >= 1; i--)
  {
    if (m_Population[i]->GetFitness() <= m_Population[i - 1]->GetFitness())
    {
      // got a match so search down list for first non-match
      for (j = i - 2; j >= 0; j--)
      {
        if (m_Population[i]->GetFitness() > m_Population[j]->GetFitness()) break;
      }
      if (j < 0) break; // we've done the best we can
      t = m_Population[i - 1];
      m_Population[i - 1] = m_Population[j];
      m_Population[j] = t;
    }
  }
}
          
ostream& operator<<(ostream &out, Population &g)
{
  int i;
  out << g.m_PopulationSize << "\n";
  for (i = 0; i < g.m_PopulationSize; i++)
  out << *(g.m_Population[i]);
  return out;
}

istream& operator>>(istream &in, Population &g)
{
  int i;
  int populationSize;

  in >> populationSize;
  if (populationSize !=g.m_PopulationSize)
  {
    if (g.m_Population) 
    {
      for (i = 0; i < g.m_PopulationSize; i++)
        delete g.m_Population[i];
      delete [] g.m_Population;
    }
    if (g.m_CumulativeFitness) delete [] g.m_CumulativeFitness;
    
    g.m_PopulationSize = populationSize;
    g.m_CumulativeFitness = new double[g.m_PopulationSize];
    g.m_Population = new Genome *[g.m_PopulationSize];
    for (i = 0; i < g.m_PopulationSize; i++)
      g.m_Population[i] = new Genome();
  }

  for (i = 0; i < g.m_PopulationSize; i++)
    in >> *(g.m_Population[i]);
  return in;
}  


