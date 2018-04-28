// Simulation.cpp - this simulation object is used to encapsulate
// a dynamechs simulation

// this is an encapsulated version of the main routine used in most of Scott McMillan's examples
// with various bits and pieces added and removed and moved.

// modified by wis to include a specific definintion of the model
// to allow me to customise things a bit more easily

#ifdef USE_OPENGL
#include <gl.h>
#include "LoadObj.h"
#endif

#include <dm.h>           // DynaMechs typedefs, globals, etc.
#include <dmSystem.hpp>         // DynaMechs simulation code.
#include <dmArticulation.hpp>
#include <dmIntegrator.hpp>
#include <dmIntegEuler.hpp>
#include <dmForce.hpp>
#include <dmRigidBody.hpp>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <assert.h>
#include <typeinfo>

#include "Util.h"
#include "DebugControl.h"
#include "MAMuscle.h"
#include "MyMobileBaseLink.h"
#include "MyRevoluteLink.h"
#include "StrapForceAnchor.h"
#include "ModifiedContactModel.h"
#include "CyclicDriver.h"
#include "StepDriver.h"
#include "DataTarget.h"
#include "DataFile.h"
#include "PGDMath.h"
#include "UGMMuscle.h"
#include "MyEnvironment.h"

#include "Simulation.h"

Simulation::Simulation()
{
    // set some variables
    m_SimulationTime = 0;
    m_StepCount = 0;
    m_Robot = 0;
    m_Integrator = 0;
    m_Environment = 0;
    m_IDT = 0;
    m_MechanicalEnergy = 0;
    m_MetabolicEnergy = 0;
    m_FitnessType = DistanceTravelled;
    m_BMR = 0;
    m_OutputModelStateAtTime = 0;
    m_TimeLimit = 0;
    m_MechanicalEnergyLimit = 0;
    m_MetabolicEnergyLimit = 0;
    m_OutputKinematicsFlag = false;
    m_OutputModelStateFilename = "ModelState.xml";
    m_OutputKinematicsFile = "Kinematics.txt";
    m_KinematicMatchFitness = 0;

    // values for experiments
#ifdef ENERGY_PARTITION_EXPERIMENT
    m_PositiveMechanicalWork = 0;
    m_NegativeMechanicalWork = 0;
    m_PositiveContractileWork = 0;
    m_NegativeContractileWork = 0;
    m_PositiveSerialElasticWork = 0;
    m_NegativeSerialElasticWork = 0;
    m_PositiveParallelElasticWork = 0;
    m_NegativeParallelElasticWork = 0;
#endif
    
    // and clear the global error flag
    gDynamechsError = 0;

}

//----------------------------------------------------------------------------
Simulation::~Simulation()
{
#ifdef ENERGY_PARTITION_EXPERIMENT
    cout << "m_PositiveMechanicalWork " << m_PositiveMechanicalWork <<
    " m_NegativeMechanicalWork " << m_NegativeMechanicalWork <<
    " m_PositiveContractileWork " << m_PositiveContractileWork <<
    " m_NegativeContractileWork " << m_NegativeContractileWork <<
    " m_PositiveSerialElasticWork " << m_PositiveSerialElasticWork <<
    " m_NegativeSerialElasticWork " << m_NegativeSerialElasticWork <<
    " m_PositiveParallelElasticWork " << m_PositiveParallelElasticWork <<
    " m_NegativeParallelElasticWork " << m_NegativeParallelElasticWork <<
    "\n";
#endif
    
    // get rid of all those memory alloactions
    if (m_Integrator)
        delete m_Integrator;
    if (m_Robot)
        delete m_Robot;
    if (m_Environment)
        delete m_Environment;

    {
        map<string, StrapForce *>::const_iterator iter;
        for (iter=m_MuscleList.begin(); iter != m_MuscleList.end(); iter++)
            delete iter->second;
    }
    {
        map<string, dmForce *>::const_iterator iter;
        for (iter=m_ForceList.begin(); iter != m_ForceList.end(); iter++)
            delete iter->second;
    }
    {
        map<string, dmRigidBody *>::const_iterator iter;
        for (iter=m_LinkList.begin(); iter != m_LinkList.end(); iter++)
            delete iter->second;
    }
}

//----------------------------------------------------------------------------
int Simulation::LoadModel(char *xmlDataBuffer)
{
    xmlDocPtr doc;
    xmlNodePtr cur;
    int size = strlen(xmlDataBuffer);

    if (gDebug == SimulationDebug)
    {
        *gDebugStream << "Simulation::LoadModel " << size << "\n" <<
        xmlDataBuffer << "\n";
    }
        
    // do the basic XML parsing

    doc = xmlParseMemory(xmlDataBuffer, size);

    if (doc == NULL ) 
    {
        fprintf(stderr,"Document not parsed successfully. \n");
        return 1;
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL) 
    {
        fprintf(stderr,"Empty document\n");
        xmlFreeDoc(doc);
        return 1;
    }
    
    if (xmlStrcmp(cur->name, (const xmlChar *) "GAITSYM")) 
    {
        fprintf(stderr,"Document of the wrong type, root node != GAITSYM");
        xmlFreeDoc(doc);
        return 1;
    }

    // now parse the elements in the file
    
    try
    {
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"GLOBAL"))) ParseGlobal(cur);
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"MOBILE_BASE_LINK"))) ParseMobileBaseLink(cur);
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"REVOLUTE_LINK"))) ParseRevoluteLink(cur);
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"ROBOT"))) ParseRobot(cur);
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"MUSCLE"))) ParseMuscle(cur);
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"CONTACT"))) ParseContact(cur);
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"CYCLIC_DRIVER"))) ParseCyclicDriver(cur);
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"STEP_DRIVER"))) ParseStepDriver(cur);
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"DATA_TARGET"))) ParseDataTarget(cur);

            cur = cur->next;
        }
    }

    catch(...)
    {
        cerr << "Error parsing XML file\n";
        xmlFreeDoc(doc);
        return 1;
    }
    
    xmlFreeDoc(doc);
    return 0;
}


//----------------------------------------------------------------------------
void Simulation::UpdateSimulation()
{
    double v, w;
    double q[7], qd[7];
    double kinematicError;

    // run the simulation
    m_Integrator->simulate(m_IDT);

    // update the time counter
    m_SimulationTime += m_IDT;

#ifdef ENERGY_TRANSFORMATION_EXPERIMENT
    static double lastOutputTime = -999999;
    bool outputStrainEnergyFlag = false;
    double totalESE = 0;
    double totalEPE = 0;
    if (m_SimulationTime - lastOutputTime > 0.01)
    {
        outputStrainEnergyFlag = true;
        lastOutputTime = m_SimulationTime;
    }
#endif
        // update the step counter
    m_StepCount++;

    {
        map<string, StrapForce *>::const_iterator iter;
        for (iter=m_MuscleList.begin(); iter != m_MuscleList.end(); iter++)
        {
            if (typeid(*iter->second) == typeid(MAMuscle))
            {
                double v = iter->second->GetDriver()->GetValue(m_SimulationTime);
                ((MAMuscle *)(iter->second))->SetAlpha(v);
                iter->second->UpdateWorldPositions();
                m_MechanicalEnergy += iter->second->GetPower() * m_IDT;
                m_MetabolicEnergy += dynamic_cast<MAMuscle *>(iter->second)->GetMetabolicPower() * m_IDT;
#ifdef ENERGY_PARTITION_EXPERIMENT
                if (iter->second->GetPower() > 0)
                    m_PositiveMechanicalWork += iter->second->GetPower() * m_IDT;
                else
                    m_NegativeMechanicalWork += iter->second->GetPower() * m_IDT;
#endif
            }
            else if (typeid(*iter->second) == typeid(UGMMuscle))
            {
                double v = iter->second->GetDriver()->GetValue(m_SimulationTime);
                ((UGMMuscle *)(iter->second))->SetStim(v, m_IDT);
                iter->second->UpdateWorldPositions();
                m_MechanicalEnergy += iter->second->GetPower() * m_IDT;
                m_MetabolicEnergy += dynamic_cast<UGMMuscle *>(iter->second)->GetMetabolicPower() * m_IDT;
#ifdef ENERGY_PARTITION_EXPERIMENT
                if (iter->second->GetPower() > 0)
                    m_PositiveMechanicalWork += iter->second->GetPower() * m_IDT;
                else
                    m_NegativeMechanicalWork += iter->second->GetPower() * m_IDT;
                
                if (dynamic_cast<UGMMuscle *>(iter->second)->GetVCE() < 0)
                    m_PositiveContractileWork += -1 * dynamic_cast<UGMMuscle *>(iter->second)->GetVCE() *
                        dynamic_cast<UGMMuscle *>(iter->second)->GetFCE() * m_IDT;
                else
                    m_NegativeContractileWork += -1 * dynamic_cast<UGMMuscle *>(iter->second)->GetVCE() *
                        dynamic_cast<UGMMuscle *>(iter->second)->GetFCE() * m_IDT;
                
                if (dynamic_cast<UGMMuscle *>(iter->second)->GetVSE() < 0)
                    m_PositiveSerialElasticWork += -1 * dynamic_cast<UGMMuscle *>(iter->second)->GetVSE() *
                        dynamic_cast<UGMMuscle *>(iter->second)->GetFSE() * m_IDT;
                else
                    m_NegativeSerialElasticWork += -1 * dynamic_cast<UGMMuscle *>(iter->second)->GetVSE() *
                        dynamic_cast<UGMMuscle *>(iter->second)->GetFSE() * m_IDT;

                if (dynamic_cast<UGMMuscle *>(iter->second)->GetVPE() < 0)
                    m_PositiveParallelElasticWork += -1 * dynamic_cast<UGMMuscle *>(iter->second)->GetVPE() *
                        dynamic_cast<UGMMuscle *>(iter->second)->GetFPE() * m_IDT;
                else
                    m_NegativeParallelElasticWork += -1 * dynamic_cast<UGMMuscle *>(iter->second)->GetVPE() *
                        dynamic_cast<UGMMuscle *>(iter->second)->GetFPE() * m_IDT;
#endif
#ifdef ENERGY_TRANSFORMATION_EXPERIMENT
                if (outputStrainEnergyFlag)
                {
                    cerr << iter->second->getName() << " "
                        << m_SimulationTime << " "
                        << m_Robot->getForKinStruct(0)->p_ICS[0] << " "
                        << dynamic_cast<UGMMuscle *>(iter->second)->GetESE() << " "
                        << dynamic_cast<UGMMuscle *>(iter->second)->GetEPE() << "\n";
                    totalESE += dynamic_cast<UGMMuscle *>(iter->second)->GetESE();
                    totalEPE += dynamic_cast<UGMMuscle *>(iter->second)->GetEPE();
                }
#endif
            }
        }
        m_MetabolicEnergy += m_BMR * m_IDT;
#ifdef ENERGY_TRANSFORMATION_EXPERIMENT
        if (outputStrainEnergyFlag)
        {
            cerr << "total_se_pe" << " "
                << m_SimulationTime << " "
                << m_Robot->getForKinStruct(0)->p_ICS[0] << " "
                << totalESE << " "
                << totalEPE << "\n";
        }
#endif
    }

    {
        map<string, DataTarget *>::const_iterator iter;
        for (iter=m_TargetList.begin(); iter != m_TargetList.end(); iter++)
        {
            if (iter->second->GetTargetValue(m_SimulationTime, &v, &w))
            {
                iter->second->GetTarget()->getState(q, qd);
                kinematicError = 0;
                switch (iter->second->GetDataType())
                {
                    case DataTarget::Q0:
                        kinematicError = q[0] - v;
                        break;
                    case DataTarget::Q1:
                        kinematicError = q[1] - v;
                        break;
                    case DataTarget::Q2:
                        kinematicError = q[2] - v;
                        break;
                    case DataTarget::Q3:
                        kinematicError = q[3] - v;
                        break;
                    case DataTarget::XP:
                        kinematicError = q[4] - v;
                        break;
                    case DataTarget::YP:
                        kinematicError = q[5] - v;
                        break;
                    case DataTarget::ZP:
                        kinematicError = q[6] - v;
                        break;
                    case DataTarget::THETA:
                        kinematicError = q[0] - v;
                        break;
                    case DataTarget::XRV:
                        kinematicError = qd[0] - v;
                        break;
                    case DataTarget::YRV:
                        kinematicError = qd[1] - v;
                        break;
                    case DataTarget::ZRV:
                        kinematicError = qd[2] - v;
                        break;
                    case DataTarget::XV:
                        kinematicError = qd[3] - v;
                        break;
                    case DataTarget::YV:
                        kinematicError = qd[4] - v;
                        break;
                    case DataTarget::ZV:
                        kinematicError = qd[5] - v;
                        break;
                    case DataTarget::THETAV:
                        kinematicError = qd[0] - v;
                        break;
                }
                m_KinematicMatchFitness -= w * ABS(kinematicError);
                if (gDebug == FitnessDebug) *gDebugStream <<
                    "Simulation::UpdateSimulation m_SimulationTime " << m_SimulationTime <<
                    " DataTarget->name " << iter->second->getName() <<
                    " kinematicError " << kinematicError << 
                    " m_KinematicMatchFitness " << m_KinematicMatchFitness << "\n";
            }
        }
    }

#ifdef ENERGY_TRANSFORMATION_EXPERIMENT
    if (outputStrainEnergyFlag)
    {
        unsigned int i;
        unsigned int numLinks;
        const dmABForKinStruct *theDmABForKinStruct;
        const dmRigidBody *theDmRigidBody;
        CartesianVector a_g;
        double potentialEnergy, kineticEnergy;
        double totalPotentialEnergy = 0;
        double totalKineticEnergy = 0;

        m_Environment->getGravity(a_g);

        numLinks = m_Robot->getNumLinks();
        for (i = 0; i < numLinks; i++)
        {
            theDmABForKinStruct = m_Robot->getForKinStruct(i);
            theDmRigidBody = dynamic_cast<dmRigidBody *>(m_Robot->getLink(i));
            potentialEnergy = theDmRigidBody->getPotentialEnergy(*theDmABForKinStruct, a_g);
            kineticEnergy = theDmRigidBody->getKineticEnergy(*theDmABForKinStruct);
            cerr << theDmRigidBody->getName() << " "
                << m_SimulationTime << " "
                << m_Robot->getForKinStruct(0)->p_ICS[0] << " "
                << potentialEnergy << " "
                << kineticEnergy << "\n";
            totalPotentialEnergy += potentialEnergy;
            totalKineticEnergy += kineticEnergy;
        }
        cerr << "total_pe_ke" << " "
            << m_SimulationTime << " "
            << m_Robot->getForKinStruct(0)->p_ICS[0] << " "
            << totalPotentialEnergy << " "
            << totalKineticEnergy << "\n";
    }
#endif    

    if (m_OutputKinematicsFlag) OutputKinematics();
    if (m_OutputModelStateAtTime > 0.0)
    {
        if (m_SimulationTime >= m_OutputModelStateAtTime)
        {
            OutputProgramState();
            m_OutputModelStateAtTime = 0.0;
        }
    }
    
}

//----------------------------------------------------------------------------
bool Simulation::TestForCatastrophy()
{
    // test the limits on the mobile base link
    if (m_MobileBaseLink->OutsideRange())
    {
        cerr << "Failed due to m_MobileBaseLink->OutsideRange() error\n";
        m_KinematicMatchFitness = -9999;
        return true;
    }

    // test everything is above ground
#ifdef ALL_JOINTS_ABOVE_GROUND
    unsigned int i;
    unsigned int numLinks;
    const dmABForKinStruct *theDmABForKinStruct;
    numLinks = m_Robot->getNumLinks();
    for (i = 0; i < numLinks; i++)
    {
        theDmABForKinStruct = m_Robot->getForKinStruct(i);
        if (theDmABForKinStruct->p_ICS[2] < 0.0)
        {
            cerr << "Failed due to joint below ground error in " << m_Robot->getLink()->getName() << "\n";
            m_KinematicMatchFitness = -9999;
            return true;
        }
    }
#endif
    
    // check the gDynamechsError status
    if (gDynamechsError)
    {
        cerr << "Failed due to gDynamechsError error\n";
        m_KinematicMatchFitness = -9999;
        return true;
    }
    
    return false;
}


//----------------------------------------------------------------------------
double Simulation::CalculateInstantaneousFitness()
{

    switch (m_FitnessType)
    {
        case DistanceTravelled:
        {
            return m_Robot->getForKinStruct(0)->p_ICS[0];
        }
        case KinematicMatch:
        {
            return m_KinematicMatchFitness;
        }
    }
    return 0;
}

//----------------------------------------------------------------------------
void
Simulation::Draw()
{
#ifdef USE_OPENGL            
    m_Environment->draw();

    {
        map<string, StrapForce *>::const_iterator iter;
        for (iter=m_MuscleList.begin(); iter != m_MuscleList.end(); iter++) iter->second->Draw();
    }
    {
        map<string, dmForce *>::const_iterator iter;
        for (iter=m_ForceList.begin(); iter != m_ForceList.end(); iter++) iter->second->draw();
    }
    
    m_Robot->draw();
#endif
}    

void Simulation::ParseGlobal(xmlNodePtr cur)
{
    xmlChar *buf;
    double a, b;
    CartesianVector gravity;

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"IntegrationStepsize"));
    m_IDT = Util::Double(buf);
    xmlFree(buf);

    m_Environment = new MyEnvironment();
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"GravityVector"));
    Util::Double(buf, 3, gravity);
    m_Environment->setGravity(gravity);
    xmlFree(buf);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"GroundPlanarSpringConstant"));
    a = Util::Double(buf);
    m_Environment->setGroundPlanarSpringConstant(a);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"GroundNormalSpringConstant"));
    a = Util::Double(buf);
    m_Environment->setGroundNormalSpringConstant(a);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"GroundPlanarDamperConstant"));
    a = Util::Double(buf);
    m_Environment->setGroundPlanarDamperConstant(a);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"GroundNormalDamperConstant"));
    a = Util::Double(buf);
    m_Environment->setGroundNormalDamperConstant(a);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"GroundStaticFrictionCoeff"));
    a = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"GroundKineticFrictionCoeff"));
    b = Util::Double(buf);
    m_Environment->setFrictionCoeffs(a, b);
    xmlFree(buf);

    dmEnvironment::setEnvironment(m_Environment);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DefaultJointLimitSpringConstant"));
    m_DefaultJointLimitSpringConstant = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DefaultJointLimitDamperConstant"));
    m_DefaultJointLimitDamperConstant = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DefaultJointFriction"));
    m_DefaultJointFriction = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DefaultMuscleActivationK"));
    m_DefaultMuscleActivationK = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DefaultMuscleForcePerUnitArea"));
    m_DefaultMuscleForcePerUnitArea = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DefaultMuscleVMaxFactor"));
    m_DefaultMuscleVMaxFactor = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DefaultMuscleDensity"));
    m_DefaultMuscleDensity = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DefaultMuscleWidth"));
    m_DefaultMuscleWidth = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DefaultMuscleFastTwitchProportion"));
    m_DefaultMuscleFastTwitchProportion = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DefaultMuscleAerobic"));
    m_DefaultMuscleAerobic = Util::Bool(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DefaultMuscleAllowReverseWork"));
    m_DefaultMuscleAllowReverseWork = Util::Bool(buf);
    xmlFree(buf);
    
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"BMR"));
    m_BMR = Util::Double(buf);
    xmlFree(buf);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"TimeLimit"));
    m_TimeLimit = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"MechanicalEnergyLimit"));
    m_MechanicalEnergyLimit = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"MetabolicEnergyLimit"));
    m_MetabolicEnergyLimit = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"FitnessType"));
    if (strcmp((char *)buf, "DistanceTravelled") == 0) m_FitnessType = DistanceTravelled;
    else if (strcmp((char *)buf, "KinematicMatch") == 0) m_FitnessType = KinematicMatch;
    else cerr << "Unrecognised FitnessType [" << buf << "] defaulting to [DistanceTravelled]\n";
    xmlFree(buf);

    buf = xmlGetProp(cur, (const xmlChar *)"OutputModelStateFilename");
    if (buf)
    {
        m_OutputModelStateFilename = (char *)buf;
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"OutputModelStateAtTime"));
        m_OutputModelStateAtTime = Util::Double(buf);
        xmlFree(buf);
    }

    buf = xmlGetProp(cur, (const xmlChar *)"OutputKinematicsFile");
    if (buf)
    {
        m_OutputKinematicsFile = (char *)buf;
        xmlFree(buf);
        m_OutputKinematicsFlag = true;
    }        

    buf = xmlGetProp(cur, (const xmlChar *)"TerrainXDim");
    if (buf)
    {
        int xDim = Util::Int(buf);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"TerrainYDim"));
        int yDim = Util::Int(buf);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"TerrainGridSize"));
        double gridSize = Util::Double(buf);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"TerrainXOrigin"));
        double xOrigin = Util::Double(buf);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"TerrainYOrigin"));
        double yOrigin = Util::Double(buf);
        xmlFree(buf);
        
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"TerrainHeights"));
        int count = DataFile::CountTokens((char *)buf);
        Util::Double(buf, count, m_DoubleList);
        xmlFree(buf);

        m_Environment->setTerrainData(xDim, yDim, gridSize, xOrigin, yOrigin,
                                      m_DoubleList, count);
#ifdef USE_OPENGL
        m_Environment->drawInit();
#endif
    }
    
}

void Simulation::ParseMobileBaseLink(xmlNodePtr cur)
{
    xmlChar *buf;
    double mass;
    CartesianTensor moi;
    CartesianVector cm;
    double position[7];
    SpatialVector velocity;
    double velocityRange[6];
    double positionRange[6];
    
    MyMobileBaseLink *theMobileBaseLink = new MyMobileBaseLink();
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Name"));
    theMobileBaseLink->setName((const char *)buf);
    xmlFree(buf);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Mass"));
    mass = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"CM"));
    Util::Double(buf, 3, cm);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"MOI"));
    Util::Double(buf, 9, (double *)moi);
    theMobileBaseLink->setInertiaParameters(mass, moi, cm);
    xmlFree(buf);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Position"));
    Util::Double(buf, 7, position);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Velocity"));
    Util::Double(buf, 6, velocity);
    theMobileBaseLink->setState(position, velocity);
    xmlFree(buf);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"VelocityRange"));
    Util::Double(buf, 6, velocityRange);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"PositionRange"));
    Util::Double(buf, 6, positionRange);
    theMobileBaseLink->SetRanges(positionRange, velocityRange);
    xmlFree(buf);

    buf = xmlGetProp(cur, (const xmlChar *)"Restriction");
    if (buf)
    {
        if (strcmp((char *)buf, "None") == 0) theMobileBaseLink->SetRestriction(None);
        else if (strcmp((char *)buf, "Yeq0") == 0) theMobileBaseLink->SetRestriction(Yeq0);
        else cerr << "Unrecognised Restriction [" << buf << "] defaulting to [Yeq0]\n";
        xmlFree(buf);
    }
    

#ifdef USE_OPENGL
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Scale"));
    double scale = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"GraphicFile"));
    GLuint *dlist = new GLuint;
    *dlist = LoadObj((const char *)buf, scale, scale, scale);
    theMobileBaseLink->setUserData((void *) dlist);
    xmlFree(buf);
#endif
    
    m_LinkList[theMobileBaseLink->getName()] = theMobileBaseLink;
    m_MobileBaseLink = theMobileBaseLink;
}

void Simulation::ParseRevoluteLink(xmlNodePtr cur)
{
    xmlChar *buf;
    double mass;
    CartesianTensor moi;
    CartesianVector cm;
    double mdhA, mdhAlpha, mdhD, mdhTheta;
    double q, qd;
    double jointMin, jointMax;
    
    MyRevoluteLink *theRevoluteLink = new MyRevoluteLink();
    buf = xmlGetProp(cur, (const xmlChar *)"Name");
    theRevoluteLink->setName((const char *)buf);
    xmlFree(buf);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Mass"));
    mass = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"CM"));
    Util::Double(buf, 3, cm);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"MOI"));
    Util::Double(buf, 9, (double *)moi);
    theRevoluteLink->setInertiaParameters(mass, moi, cm);
    xmlFree(buf);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"MDHA"));
    mdhA = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"MDHAlpha"));
    mdhAlpha = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"MDHD"));
    mdhD = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"MDHTheta"));
    mdhTheta = Util::Double(buf);
    theRevoluteLink->setMDHParameters(mdhA, mdhAlpha, mdhD, mdhTheta);
    xmlFree(buf);

    theRevoluteLink->getState(&q, &qd);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"InitialJointVelocity"));
    qd = Util::Double(buf);
    theRevoluteLink->setState(&q, &qd);
    xmlFree(buf);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"JointMin"));
    jointMin = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"JointMax"));
    jointMax = Util::Double(buf);
    theRevoluteLink->setJointLimits(jointMin, jointMax,
                                    m_DefaultJointLimitSpringConstant,
                                    m_DefaultJointLimitDamperConstant);
    theRevoluteLink->setJointFriction(m_DefaultJointFriction);
    xmlFree(buf);

#ifdef USE_OPENGL
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Scale"));
    double scale = Util::Double(buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"GraphicFile"));
    GLuint *dlist = new GLuint;
    *dlist = LoadObj((const char *)buf, scale, scale, scale);
    theRevoluteLink->setUserData((void *) dlist);
    xmlFree(buf);
#endif
    
    m_LinkList[theRevoluteLink->getName()] = theRevoluteLink;
}

void Simulation::ParseRobot(xmlNodePtr cur)
{
    xmlChar *buf;
    int i, count;
    m_Robot = new dmArticulation();
    CartesianVector pos = {0.0, 0.0, 0.0};
    Quaternion quat = {0.0, 0.0, 0.0, 1.0};
    m_Robot->setRefSystem(quat, pos);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Name"));
    m_Robot->setName((const char *)buf);
    xmlFree(buf);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"RootLink"));
    m_Robot->addLink(m_LinkList[(const char *)buf], 0);
    xmlFree(buf);

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"LinkPairs"));
    strncpy(m_Buffer, (const char *)buf, kBufferSize - 1);
    xmlFree(buf);
    m_Buffer[kBufferSize - 1] = 0;
    count = DataFile::ReturnTokens(m_Buffer, m_BufferPtrs, kBufferSize);
    for (i = 0; i < count - 1; i += 2)
    {
        m_Robot->addLink(m_LinkList[m_BufferPtrs[i]], m_LinkList[m_BufferPtrs[i + 1]]);
    }
    m_Integrator = (dmIntegrator *) new dmIntegEuler();
    m_Integrator->addSystem(m_Robot);
}

void Simulation::ParseMuscle(xmlNodePtr cur)
{
    xmlChar *buf;
    double position[3];
    double mdhA, mdhAlpha, mdhD, mdhTheta;
    double radius;
    MyRevoluteLink *myRevoluteLink;
    StrapForceAnchor *strapForceAnchor;
    double originAngle, insertionAngle;
    double dummy;
    StrapForce *muscle;

    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Type"));
    if (strcmp((const char *)buf, "MinettiAlexander") == 0)
    {
        muscle = new MAMuscle();
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Name"));
        ((MAMuscle *)muscle)->setName((const char *)buf);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"PCA"));
        ((MAMuscle *)muscle)->SetF0(Util::Double(buf) * m_DefaultMuscleForcePerUnitArea);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"FibreLength"));
        ((MAMuscle *)muscle)->SetVMax(Util::Double(buf) * m_DefaultMuscleVMaxFactor);
        xmlFree(buf);
        ((MAMuscle *)muscle)->SetK(m_DefaultMuscleActivationK);
    }
    else if (strcmp((const char *)buf, "UmbergerGerritsenMartin") == 0)
    {
        muscle = new UGMMuscle();
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Name"));
        ((UGMMuscle *)muscle)->setName((const char *)buf);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"PCA"));
        double PCSA = Util::Double(buf);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"FibreLength"));
        double optimumLength = Util::Double(buf);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"TendonLength"));
        double tendonLength = Util::Double(buf);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"SerialStrainModel"));
        StrainModel serialStrainModel;
        if (strcmp((const char *)buf, "Linear") == 0)
            serialStrainModel = linear;
        else if (strcmp((const char *)buf, "Square") == 0)
            serialStrainModel = square;
        else throw 1;
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"SerialStrainAtFmax"));
        double serialStrainAtFmax = Util::Double(buf);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"ParallelStrainModel"));
        StrainModel parallelStrainModel;
        if (strcmp((const char *)buf, "Linear") == 0)
            parallelStrainModel = linear;
        else if (strcmp((const char *)buf, "Square") == 0)
            parallelStrainModel = square;
        else throw 1;
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"ParallelStrainAtFmax"));
        double parallelStrainAtFmax = Util::Double(buf);
        xmlFree(buf);
        ((UGMMuscle *)muscle)->SetModellingConstants(m_DefaultMuscleForcePerUnitArea,
                                   m_DefaultMuscleVMaxFactor,
                                                     m_DefaultMuscleDensity);

        ((UGMMuscle *)muscle)->SetFibreComposition(m_DefaultMuscleFastTwitchProportion);
        ((UGMMuscle *)muscle)->SetMuscleGeometry(PCSA, optimumLength, m_DefaultMuscleWidth, tendonLength,
                                                 serialStrainModel, serialStrainAtFmax,
                                                 parallelStrainModel, parallelStrainAtFmax);
        ((UGMMuscle *)muscle)->SetAerobic(m_DefaultMuscleAerobic);
        ((UGMMuscle *)muscle)->AllowReverseWork(m_DefaultMuscleAllowReverseWork);
    }
    else
    {
        cerr << "Unrecognised Muscle Type:" << buf << "\n";
        xmlFree(buf);
        return;
    }

    m_MuscleList[muscle->getName()] = muscle;

    // check presence of MidPoint
    buf = xmlGetProp(cur, (const xmlChar *)"MidPoint");
    if (buf)
    {
        xmlFree(buf);
        muscle->SetNumAnchors(3);

        StrapForceAnchor *strapForceAnchor = new StrapForceAnchor();
        strcpy(m_Buffer, muscle->getName());
        strcat(m_Buffer, "OriginAnchor");
        strapForceAnchor->setName(m_Buffer);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Origin"));
Util::Double(buf, 3, position);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"OriginLinkName"));
        strapForceAnchor->SetStrapForce(m_LinkList[(const char *)buf], position, muscle, 0);
        m_LinkList[(const char *)buf]->addForce(strapForceAnchor);
        m_ForceList[strapForceAnchor->getName()] = strapForceAnchor;
        xmlFree(buf);

        strapForceAnchor = new StrapForceAnchor();
        strcpy(m_Buffer, muscle->getName());
        strcat(m_Buffer, "MidPointAnchor");
        strapForceAnchor->setName(m_Buffer);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"MidPoint"));
Util::Double(buf, 3, position);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"MidPointLinkName"));
        strapForceAnchor->SetStrapForce(m_LinkList[(const char *)buf], position, muscle, 1);
        m_LinkList[(const char *)buf]->addForce(strapForceAnchor);
        m_ForceList[strapForceAnchor->getName()] = strapForceAnchor;
        xmlFree(buf);

        strapForceAnchor = new StrapForceAnchor();
        strcpy(m_Buffer, muscle->getName());
        strcat(m_Buffer, "InsertionAnchor");
        strapForceAnchor->setName(m_Buffer);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Insertion"));
Util::Double(buf, 3, position);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"InsertionLinkName"));
        strapForceAnchor->SetStrapForce(m_LinkList[(const char *)buf], position, muscle, 2);
        m_LinkList[(const char *)buf]->addForce(strapForceAnchor);
        m_ForceList[strapForceAnchor->getName()] = strapForceAnchor;
        xmlFree(buf);

        muscle->SetStrapForceType(ThreePoint);
        return;
    }

    // check presence of Radius
    buf = xmlGetProp(cur, (const xmlChar *)"Radius");
    if (buf)
    {
        radius = Util::Double(buf);
        xmlFree(buf);
        
        muscle->SetNumAnchors(5);

        strapForceAnchor = new StrapForceAnchor();
        strcpy(m_Buffer, muscle->getName());
        strcat(m_Buffer, "InsertionAnchor");
        strapForceAnchor->setName(m_Buffer);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Insertion"));
Util::Double(buf, 3, position);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"InsertionLinkName"));
        strapForceAnchor->SetStrapForce(m_LinkList[(const char *)buf], position, muscle, 4);
        m_LinkList[(const char *)buf]->addForce(strapForceAnchor);
        m_ForceList[strapForceAnchor->getName()] = strapForceAnchor;
        THROWIFZERO(myRevoluteLink = dynamic_cast<MyRevoluteLink *>(m_LinkList[(const char *)buf]));
        myRevoluteLink->getMDHParameters(&mdhA, &mdhAlpha, &mdhD, &mdhTheta);
        xmlFree(buf);

        if (radius > 0)
Util::PointToCircleTangentIntercept(position[0], position[1], radius,
                                    &insertionAngle, &dummy);
        else
Util::PointToCircleTangentIntercept(position[0], position[1], radius,
                                    &dummy, &insertionAngle);
        
        strapForceAnchor = new StrapForceAnchor();
        strcpy(m_Buffer, muscle->getName());
        strcat(m_Buffer, "OriginAnchor");
        strapForceAnchor->setName(m_Buffer);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Origin"));
Util::Double(buf, 3, position);
        xmlFree(buf);
        THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"OriginLinkName"));
        strapForceAnchor->SetStrapForce(m_LinkList[(const char *)buf], position, muscle, 0);
        m_LinkList[(const char *)buf]->addForce(strapForceAnchor);
        m_ForceList[strapForceAnchor->getName()] = strapForceAnchor;
        xmlFree(buf);

        if (radius > 0)
Util::PointToCircleTangentIntercept(position[0] - mdhA, position[1], radius,
                                    &dummy, &originAngle);
        else
Util::PointToCircleTangentIntercept(position[0] - mdhA, position[1], radius,
                                    &originAngle, &dummy);

        // origin and insertion angles must be the same sign
        // this may not work for the general case
        if (insertionAngle < 0)
        {
            if (originAngle > 0) originAngle -= 2 * M_PI;
        }
        else if (insertionAngle > 0)
        {
            if (originAngle < 0) originAngle += 2 * M_PI;
        }

        // dummy position (calculated in StrapForce)
        position[0] = 0; position[1] = 0; position[2] = 0;

        strapForceAnchor = new StrapForceAnchor();
        strcpy(m_Buffer, muscle->getName());
        strcat(m_Buffer, "Radius0Anchor");
        strapForceAnchor->setName(m_Buffer);
        strapForceAnchor->SetStrapForce(myRevoluteLink, position, muscle, 1);
        myRevoluteLink->addForce(strapForceAnchor);
        m_ForceList[strapForceAnchor->getName()] = strapForceAnchor;

        strapForceAnchor = new StrapForceAnchor();
        strcpy(m_Buffer, muscle->getName());
        strcat(m_Buffer, "Radius1Anchor");
        strapForceAnchor->setName(m_Buffer);
        strapForceAnchor->SetStrapForce(myRevoluteLink, position, muscle, 2);
        myRevoluteLink->addForce(strapForceAnchor);
        m_ForceList[strapForceAnchor->getName()] = strapForceAnchor;

        strapForceAnchor = new StrapForceAnchor();
        strcpy(m_Buffer, muscle->getName());
        strcat(m_Buffer, "Radius2Anchor");
        strapForceAnchor->setName(m_Buffer);
        strapForceAnchor->SetStrapForce(myRevoluteLink, position, muscle, 3);
        myRevoluteLink->addForce(strapForceAnchor);
        m_ForceList[strapForceAnchor->getName()] = strapForceAnchor;
        
        muscle->SetStrapForceType(Radius);
        muscle->SetRadiusParameters(radius, insertionAngle, originAngle);
        return;
    }
    
    // default 2 attachment point muscle
    muscle->SetNumAnchors(2);

    strapForceAnchor = new StrapForceAnchor();
    strcpy(m_Buffer, muscle->getName());
    strcat(m_Buffer, "OriginAnchor");
    strapForceAnchor->setName(m_Buffer);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Origin"));
    Util::Double(buf, 3, position);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"OriginLinkName"));
    strapForceAnchor->SetStrapForce(m_LinkList[(const char *)buf], position, muscle, 0);
    m_LinkList[(const char *)buf]->addForce(strapForceAnchor);
    m_ForceList[strapForceAnchor->getName()] = strapForceAnchor;
    xmlFree(buf);

    strapForceAnchor = new StrapForceAnchor();
    strcpy(m_Buffer, muscle->getName());
    strcat(m_Buffer, "InsertionAnchor");
    strapForceAnchor->setName(m_Buffer);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Insertion"));
    Util::Double(buf, 3, position);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"InsertionLinkName"));
    strapForceAnchor->SetStrapForce(m_LinkList[(const char *)buf], position, muscle, 1);
    m_LinkList[(const char *)buf]->addForce(strapForceAnchor);
    m_ForceList[strapForceAnchor->getName()] = strapForceAnchor;
    xmlFree(buf);

    muscle->SetStrapForceType(TwoPoint);
}

void Simulation::ParseContact(xmlNodePtr cur)
{
    xmlChar *buf;
    int count;
    ModifiedContactModel *contactModel = new ModifiedContactModel();
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Name"));
    contactModel->setName((const char *)buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"ContactPositions"));
    count = DataFile::CountTokens((char *)buf);
    Util::Double(buf, count, m_DoubleList);
    contactModel->setContactPoints(count / 3, (CartesianVector *)m_DoubleList);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"LinkName"));
    m_LinkList[(const char *)buf]->addForce(contactModel);
    m_ForceList[contactModel->getName()] = contactModel;
    xmlFree(buf);
}

void Simulation::ParseCyclicDriver(xmlNodePtr cur)
{
    xmlChar *buf;
    int count;
    CyclicDriver *cyclicDriver = new CyclicDriver();
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Name"));
    cyclicDriver->setName((const char *)buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DurationValuePairs"));
    count = DataFile::CountTokens((char *)buf);
Util::Double(buf, count, m_DoubleList);
    cyclicDriver->SetValueDurationPairs(count, m_DoubleList);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Target"));
    m_MuscleList[(const char *)buf]->SetDriver(cyclicDriver);
    xmlFree(buf);

    // check presence of PhaseDelay
    buf = xmlGetProp(cur, (const xmlChar *)"PhaseDelay");
    if (buf)
    {
        cyclicDriver->SetPhaseDelay(Util::Double(buf));
        xmlFree(buf);
    }
}

void Simulation::ParseStepDriver(xmlNodePtr cur)
{
    xmlChar *buf;
    int count;
    StepDriver *stepDriver = new StepDriver();
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Name"));
    stepDriver->setName((const char *)buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DurationValuePairs"));
    count = DataFile::CountTokens((char *)buf);
Util::Double(buf, count, m_DoubleList);
    stepDriver->SetValueDurationPairs(count, m_DoubleList);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Target"));
    m_MuscleList[(const char *)buf]->SetDriver(stepDriver);
    xmlFree(buf);
}

void Simulation::ParseDataTarget(xmlNodePtr cur)
{
    xmlChar *buf;
    int count;
    DataTarget *dataTarget = new DataTarget();
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Name"));
    dataTarget->setName((const char *)buf);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DurationValuePairs"));
    count = DataFile::CountTokens((char *)buf);
Util::Double(buf, count, m_DoubleList);
    dataTarget->SetValueDurationPairs(count, m_DoubleList);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"Target"));
    dataTarget->SetTarget(m_LinkList[(const char *)buf]);
    xmlFree(buf);
    THROWIFZERO(buf = xmlGetProp(cur, (const xmlChar *)"DataType"));
    if (strcmp((const char *)buf, "XP") == 0) dataTarget->SetDataType(DataTarget::XP);
    else if (strcmp((const char *)buf, "YP") == 0) dataTarget->SetDataType(DataTarget::YP);
    else if (strcmp((const char *)buf, "ZP") == 0) dataTarget->SetDataType(DataTarget::ZP);
    else if (strcmp((const char *)buf, "Q0") == 0) dataTarget->SetDataType(DataTarget::Q0);
    else if (strcmp((const char *)buf, "Q1") == 0) dataTarget->SetDataType(DataTarget::Q1);
    else if (strcmp((const char *)buf, "Q2") == 0) dataTarget->SetDataType(DataTarget::Q2);
    else if (strcmp((const char *)buf, "Q3") == 0) dataTarget->SetDataType(DataTarget::Q3);
    else if (strcmp((const char *)buf, "THETA") == 0) dataTarget->SetDataType(DataTarget::THETA);
    else if (strcmp((const char *)buf, "XV") == 0) dataTarget->SetDataType(DataTarget::XV);
    else if (strcmp((const char *)buf, "YV") == 0) dataTarget->SetDataType(DataTarget::YV);
    else if (strcmp((const char *)buf, "ZV") == 0) dataTarget->SetDataType(DataTarget::ZV);
    else if (strcmp((const char *)buf, "XRV") == 0) dataTarget->SetDataType(DataTarget::XRV);
    else if (strcmp((const char *)buf, "YRV") == 0) dataTarget->SetDataType(DataTarget::YRV);
    else if (strcmp((const char *)buf, "ZRV") == 0) dataTarget->SetDataType(DataTarget::ZRV);
    else if (strcmp((const char *)buf, "THETAV") == 0) dataTarget->SetDataType(DataTarget::THETAV);
    else throw(0);
    xmlFree(buf);

    // check presence of Weight
    buf = xmlGetProp(cur, (const xmlChar *)"Weight");
    if (buf)
    {
        dataTarget->SetWeight(Util::Double(buf));
        xmlFree(buf);
    }    
        
    m_TargetList[dataTarget->getName()] = dataTarget;
}

// function to produce a file of link kinematics in tab delimited format

void
Simulation::OutputKinematics()
{
    unsigned int myNumLinks = m_Robot->getNumLinks();
    const dmABForKinStruct *myDmABForKinStruct;
    unsigned int i;
    int j, k;
    static bool firstTimeFlag = true;
    static ofstream outputKinematicsFile;
    double mass;
    CartesianTensor inertia;
    CartesianVector cg_pos;
    CartesianVector world_cg_pos;
    Quaternion q;

    // first time through output the column headings
    if (firstTimeFlag)
    {
        outputKinematicsFile.open(m_OutputKinematicsFile.c_str());

        outputKinematicsFile << "time\t";
        for (i = 0; i < myNumLinks; i++)
        {
            outputKinematicsFile <<
            m_Robot->getLink(i)->getName()
            << "_X\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_Y\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_Z\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_RXV\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_RYV\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_RZV\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_XV\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_YV\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_ZV\t";
#ifdef OUTPUTROTATIONMATRIX    
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_R00\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_R01\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_R02\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_R10\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_R11\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_R12\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_R20\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_R21\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_R22\t";
#endif
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_QW\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_QX\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_QY\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_QZ\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_CMX\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_CMY\t";
            outputKinematicsFile <<
                m_Robot->getLink(i)->getName()
                << "_CMZ";
            if (i != myNumLinks - 1) outputKinematicsFile << "\t";
            else outputKinematicsFile << "\n";
        }
        firstTimeFlag = false;
    }

    // start by outputting the simulation time
    outputKinematicsFile << m_SimulationTime << "\t";

    for (i = 0; i < myNumLinks; i++)
    {
        // output the position
        myDmABForKinStruct = m_Robot->getForKinStruct(i);
        outputKinematicsFile << myDmABForKinStruct->p_ICS[0] << "\t"
            << myDmABForKinStruct->p_ICS[1] << "\t"
            << myDmABForKinStruct->p_ICS[2] << "\t";
        outputKinematicsFile << myDmABForKinStruct->v[0] << "\t"
            << myDmABForKinStruct->v[1] << "\t"
            << myDmABForKinStruct->v[2] << "\t"
            << myDmABForKinStruct->v[3] << "\t"
            << myDmABForKinStruct->v[4] << "\t"
            << myDmABForKinStruct->v[5] << "\t";
        // then the orientation
#ifdef OUTPUTROTATIONMATRIX    
        for (j = 0; j < 3; j++)
        {
            for (k = 0; k < 3; k++)
            {
                outputKinematicsFile << myDmABForKinStruct->R_ICS[j][k] << "\t";
            }
        }
#endif
        // calculate quaternion
        buildQuaternion(myDmABForKinStruct->R_ICS, q);
        outputKinematicsFile << q[3] << "\t";
        outputKinematicsFile << q[0] << "\t";
        outputKinematicsFile << q[1] << "\t";
        outputKinematicsFile << q[2] << "\t";
        
        // get CM
        dynamic_cast<dmRigidBody *>(m_Robot->getLink(i))->getInertiaParameters(mass, inertia, cg_pos);
        Util::ConvertLocalToWorldP(myDmABForKinStruct, cg_pos, world_cg_pos);
        outputKinematicsFile << world_cg_pos[0] << "\t";
        outputKinematicsFile << world_cg_pos[1] << "\t";
        outputKinematicsFile << world_cg_pos[2];
        if (i != myNumLinks - 1) outputKinematicsFile << "\t";
        else outputKinematicsFile << "\n";
    }
}


// output the simulation state in an XML format that can be re-read

void
Simulation::OutputProgramState()
{
    xmlDocPtr doc;
    xmlNodePtr rootNode;
    xmlNodePtr newNode;
    xmlAttrPtr newAttr;
    double q[7];
    double qd[7];
    double mdhA, mdhAlpha, mdhD, mdhTheta;
    
    doc = xmlNewDoc((xmlChar *)"1.0");
    if (doc == 0) return;

    rootNode = xmlNewDocNode(doc, 0, (xmlChar *)"GAITSYM", 0);
    xmlDocSetRootElement(doc, rootNode);
    
    map<string, dmRigidBody *>::const_iterator iter;
    for (iter=m_LinkList.begin(); iter != m_LinkList.end(); iter++)
    {
        if (typeid(*iter->second) == typeid(MyMobileBaseLink))
        {
            MyMobileBaseLink *p = dynamic_cast<MyMobileBaseLink *>(iter->second);
            p->getState(q, qd);
                        
            newNode = xmlNewTextChild(rootNode, 0, (xmlChar *)"MOBILE_BASE_LINK", 0);
            newAttr = xmlNewProp(newNode, (xmlChar *)"Name", (xmlChar *)p->getName());
            sprintf(m_Buffer, " %.17g %.17g %.17g %.17g %.17g %.17g %.17g ", q[0], q[1], q[2], q[3], q[4], q[5], q[6]);
            newAttr = xmlNewProp(newNode, (xmlChar *)"Position", (xmlChar *)m_Buffer);
            sprintf(m_Buffer, " %.17g %.17g %.17g %.17g %.17g %.17g ", qd[0], qd[1], qd[2], qd[3], qd[4], qd[5]);
            newAttr = xmlNewProp(newNode, (xmlChar *)"Velocity", (xmlChar *)m_Buffer);
        }
        else if (typeid(*iter->second) == typeid(MyRevoluteLink))
        {
            MyRevoluteLink *p = dynamic_cast<MyRevoluteLink *>(iter->second);
            p->getState(q, qd);
            p->getMDHParameters(&mdhA, &mdhAlpha, &mdhD, &mdhTheta);

            newNode = xmlNewTextChild(rootNode, 0, (xmlChar *)"REVOLUTE_LINK", 0);
            newAttr = xmlNewProp(newNode, (xmlChar *)"Name", (xmlChar *)iter->second->getName());
            sprintf(m_Buffer, " %.17g ", mdhTheta);
            newAttr = xmlNewProp(newNode, (xmlChar *)"MDHTheta", (xmlChar *)m_Buffer);
            sprintf(m_Buffer, " %.17g ", qd[0]);
            newAttr = xmlNewProp(newNode, (xmlChar *)"InitialJointVelocity", (xmlChar *)m_Buffer);
        }
    }

    xmlSaveFormatFile(m_OutputModelStateFilename.c_str(), doc, 1);

    xmlFreeDoc(doc);
}

void Simulation::SetOutputKinematicsFile(const char *filename)
{
    if (filename)
    {
        m_OutputKinematicsFile = filename;
        m_OutputKinematicsFlag = true;
    }
    else
    {
        m_OutputKinematicsFlag = false;
    }
}

void Simulation::SetOutputModelStateFile(const char *filename)
{
    if (filename)
    {
        m_OutputModelStateFilename = filename;
    }
}

bool Simulation::ShouldQuit()
{
    if (m_TimeLimit > 0)
        if (m_SimulationTime > m_TimeLimit) return true;
    if (m_MechanicalEnergyLimit > 0)
        if (m_MechanicalEnergy > m_MechanicalEnergyLimit) return true;
    if (m_MetabolicEnergyLimit > 0)
        if (m_MetabolicEnergy > m_MetabolicEnergyLimit) return true;
    return false;
}
