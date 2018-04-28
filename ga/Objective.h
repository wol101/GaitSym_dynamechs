// Objective.h - class to hold details of running objectives

#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#if defined (USE_FS)

class Genome;

class Objective
{
  public:
      
    Objective();
    ~Objective();

    bool Valid() { return m_ValidFlag; };
    int GetID() { return m_RunID; };
    double GetScore() { return m_Fitness; };

    void SetSynchronousFlag(bool flag) { m_SynchronousFlag = flag; };
    void SetProgramName(const char * name) { m_ProgramName = name; };
        
    void Run(int id, Genome *genome);
    bool CheckFinished();
    void CleanUp();
    
  protected:

    char m_GenomeName[256];
    char m_ScoreName[256];
    const char *m_ProgramName;
    const char *m_XMLBase;
    
    int m_RunID;
    bool m_ValidFlag;
    double m_Fitness;
    bool m_SynchronousFlag;
    
    Genome *m_Genome;
};  


#elif defined (USE_SOCKETS)

// Objective.h - class to hold details of running objectives

#include <time.h>
#include <pinet.h>

class Genome;

class Objective
{
  public:
      
    Objective();
    ~Objective();

    int GetID() { return m_RunID; };
    double GetScore() { return m_Fitness; };

    void SetProgramName(const char * name) { m_ProgramName = name; };
    void SetID(int id) { m_RunID = id; };
    void SetTimeLimit(time_t timeLimit) { m_TimeLimit = timeLimit; };
    void SetGenome(Genome *genome) { m_Genome = genome; };

    bool Run();
    bool CheckRunning();

    static bool ReportBusy();

  protected:

    const char *m_ProgramName;
    Genome *m_Genome;
    int m_RunID;
    double m_Fitness;
    int m_Child;
    time_t m_StartTime;
    time_t m_TimeLimit;
    bool m_RunningFlag;
    pt::ipstream *m_Client;
    
};  

#endif

#endif // OBJECTIVE_H


