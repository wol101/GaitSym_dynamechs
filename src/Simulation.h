// Simulation.h - this simulation object is used to encapsulate
// a dynamechs simulation

#ifndef __SIMULATION_H__
#define __SIMULATION_H__

#include <map>
#include <string>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

class dmArticulation;
class dmIntegrator;
class MyEnvironment;
class dmForce;
class StrapForce;
class dmRigidBody;
class MyMobileBaseLink;
class DataTarget;

enum FitnessType
{
    DistanceTravelled = 0,
    KinematicMatch = 1
};

const int kBufferSize = 10000;

class Simulation 
{
public:

  Simulation(void);
  ~Simulation(void);

  int LoadModel(char *buffer);  // load parameters from a dynamechs configuration file
  void UpdateSimulation(void);     // called at each iteration through simulation

  // get hold of various variables

  dmArticulation *GetRobot(void) { return m_Robot; };
  dmIntegrator *GetIntegrator(void) { return m_Integrator; };
  double GetTime(void) { return m_SimulationTime; };
  double GetTimeIncrement(void) { return m_IDT; };
  long long GetStepCount(void) { return m_StepCount; };
  double GetMechanicalEnergy(void) { return m_MechanicalEnergy; };
  double GetMetabolicEnergy(void) { return m_MetabolicEnergy; };
  double GetTimeLimit(void) { return m_TimeLimit; };
  double GetMetabolicEnergyLimit(void) { return m_MetabolicEnergyLimit; };
  double GetMechanicalEnergyLimit(void) { return m_MechanicalEnergyLimit; };

  void SetTimeLimit(double timeLimit) { m_TimeLimit = timeLimit; };
  void SetMetabolicEnergyLimit(double energyLimit) { m_MetabolicEnergyLimit = energyLimit; };
  void SetMechanicalEnergyLimit(double energyLimit) { m_MechanicalEnergyLimit = energyLimit; };
  void SetOutputModelStateAtTime(double outputModelStateAtTime) { m_OutputModelStateAtTime = outputModelStateAtTime; };
  void SetOutputKinematicsFile(const char *filename);
  void SetOutputModelStateFile(const char *filename);
  
  // fitness related values
  
  bool TestForCatastrophy();
  double CalculateInstantaneousFitness();
  bool ShouldQuit();

  // draw the simulation
  void Draw();  
 
protected:

  void ParseGlobal(xmlNodePtr cur);
  void ParseMobileBaseLink(xmlNodePtr cur);
  void ParseRevoluteLink(xmlNodePtr cur);
  void ParseRobot(xmlNodePtr cur);
  void ParseMuscle(xmlNodePtr cur);
  void ParseContact(xmlNodePtr cur);
  void ParseCyclicDriver(xmlNodePtr cur);
  void ParseStepDriver(xmlNodePtr cur);
  void ParseDataTarget(xmlNodePtr cur);

  void OutputKinematics();
  void OutputProgramState();
  
  // parts of the model
    
  dmArticulation *m_Robot;
  dmIntegrator *m_Integrator;
  MyEnvironment *m_Environment;
  MyMobileBaseLink *m_MobileBaseLink;
  
  map<string, dmForce *>m_ForceList;
  map<string, StrapForce *>m_MuscleList;
  map<string, dmRigidBody *>m_LinkList;
  map<string, DataTarget *>m_TargetList;

  // keep track of simulation time
  
  double m_SimulationTime; // current time
  double m_IDT; // step size
  long long m_StepCount; // number of steps taken
  
  // and calculated energy
  
  double m_MechanicalEnergy;
  double m_MetabolicEnergy;
  double m_BMR;

  // FitnessType
  FitnessType m_FitnessType;
  double m_TimeLimit;
  double m_MechanicalEnergyLimit;
  double m_MetabolicEnergyLimit;

  double m_DefaultJointLimitSpringConstant;
  double m_DefaultJointLimitDamperConstant;
  double m_DefaultJointFriction;
  double m_DefaultMuscleActivationK;
  double m_DefaultMuscleForcePerUnitArea;
  double m_DefaultMuscleVMaxFactor;
  double m_DefaultMuscleDensity;
  double m_DefaultMuscleWidth;
  double m_DefaultMuscleFastTwitchProportion;
  bool m_DefaultMuscleAerobic;
  bool m_DefaultMuscleAllowReverseWork;

  // some control values
  bool m_OutputKinematicsFlag;
  string m_OutputKinematicsFile;
  string m_OutputModelStateFilename;
  double m_OutputModelStateAtTime;

  // internal memory allocations
  // it will crash if these are exceeded but they should be plenty big enough
  char m_Buffer[kBufferSize];
  char *m_BufferPtrs[kBufferSize];
  double m_DoubleList[kBufferSize];

  // for fitness calculations
  double m_KinematicMatchFitness;


  // values for experiments
#ifdef ENERGY_PARTITION_EXPERIMENT
  double m_PositiveMechanicalWork;
  double m_NegativeMechanicalWork;
  double m_PositiveContractileWork;
  double m_NegativeContractileWork;
  double m_PositiveSerialElasticWork;
  double m_NegativeSerialElasticWork;
  double m_PositiveParallelElasticWork;
  double m_NegativeParallelElasticWork;
#endif
  
};



#endif //__SIMULATION_H__
