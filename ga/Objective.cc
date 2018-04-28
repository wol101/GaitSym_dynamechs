// Objective.cc - class to hold details of running objectives

using namespace std;

#if defined (USE_FS)

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <float.h>
#include <string.h>

#include "Objective.h"
#include "Genome.h"
#include "Debug.h"
#include "XMLConverter.h"

extern XMLConverter gXMLConverter;

// constructor
Objective::Objective()
{
  m_RunID = 0;
  m_ValidFlag = false;
  m_Fitness = 0;
  m_SynchronousFlag = false;
  
  m_ProgramName = "./objective";
  m_ScoreName[0] = 0;
  m_GenomeName[0] = 0;
}

// destructor
Objective::~Objective()
{
}

// set an objective run going
// might benefit from more checking

void Objective::Run(int id, Genome *genome)
{
  char theCommand[256];
  
  assert(m_ValidFlag == false);
  m_ValidFlag = true;
  m_RunID = id;
  m_Genome = genome;
  
  // set up the command
  sprintf(m_GenomeName, "genome_%d.tmp", m_RunID);
  sprintf(m_ScoreName, "score_%d.tmp", m_RunID);
  if (m_SynchronousFlag)
    sprintf(theCommand, "%s -c %s --score %s", 
        m_ProgramName, m_GenomeName, m_ScoreName);
  else
    sprintf(theCommand, "%s -c %s --score %s &", 
        m_ProgramName, m_GenomeName, m_ScoreName);
  
  // write out the genome data

  gXMLConverter.ApplyGenome(genome);
  int len;
  char *buf = (char *)gXMLConverter.GetFormattedXML(&len);
  ofstream out(m_GenomeName);
  out.write(buf, len);
  out.close();
  
  // run the objective command
  system(theCommand);
  
  // cerr << "Running " << m_RunID << "\n";
}

// check to see whether the command has finished
// and read in the score (we do it here because it makes a bit more sense)

bool Objective::CheckFinished()
{
  struct stat buf;
  FILE *in;
  int c;
  
  if (m_SynchronousFlag == false) // need to check whether finished
  {
    if (m_ValidFlag == false) return false;

    if (stat(m_ScoreName, &buf) != 0) return false;

    if (buf.st_size < sizeof(double)) return false;
  }
  
  in = fopen(m_ScoreName, "rb");
  if (in == 0) return false;
  
  c = fread(&m_Fitness, sizeof(double), 1, in);
  fclose(in);
  
  
  if (c != 1) return false;

  if (gDebug == Debug_Objective) 
        cerr << "Objective::Checkfinished\tfinished\tm_RunID\t" << m_RunID << 
        "\tm_Fitness\t" << m_Fitness << "\n";
  
  return true;
}

// clean up the files and reset the valid flag
void Objective::CleanUp()
{
  assert(m_ValidFlag == true);
  
  unlink(m_GenomeName);
  unlink(m_ScoreName);
  m_ValidFlag = false;
}
  
#elif defined (USE_SOCKETS)

// Objective.cc - class to hold details of running objectives

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>

#include <pinet.h>

#include "Objective.h"
#include "Genome.h"
#include "Debug.h"
#include "Genome.h"
#include "DataFile.h"
#include "SocketMessages.h"
#include "XMLConverter.h"

extern XMLConverter gXMLConverter;

// the server
extern pt::ipstmserver g_svr;

// log file
extern ofstream gSocketLogFile;

// constructor
Objective::Objective()
{
  m_RunID = 0;
  m_RunningFlag = false;
  m_Fitness = 0;
  m_Genome = 0;
  m_TimeLimit = 0;

  m_ProgramName = "./objective";
  m_Client = 0;
}

// destructor
Objective::~Objective()
{
   if (m_Client) delete m_Client;
}

// set an objective run going
// returns false if 
bool Objective::Run()
{
  int len;
  char buffer[kSocketMaxMessageLength + 1];
  char *buf;
  buffer[kSocketMaxMessageLength] = 0;
  
  m_Client = new pt::ipstream();

  try
  {  
    if (g_svr.poll() == false)
    {
      // no client asking for a task
      delete m_Client;
      m_Client = 0;
      return false;
    }

    if (g_svr.serve(*m_Client) == false)
    {
      // some problem communicating
      delete m_Client;
      m_Client = 0;
      return false;
    }
  }
  catch (pt::estream* e)
  {
    delete e;
    delete m_Client;
    m_Client = 0;
    return false;
  }

  time_t theTime = time(0);
  if (gDebug == Debug_Sockets)
  {
    struct tm *theLocalTime = localtime(&theTime);
    char theTimeString[64];
    sprintf(theTimeString, "%04d-%02d-%02d_%02d.%02d.%02d", 
        theLocalTime->tm_year + 1900, theLocalTime->tm_mon + 1, 
        theLocalTime->tm_mday,
        theLocalTime->tm_hour, theLocalTime->tm_min,
        theLocalTime->tm_sec);
    pt::string ip = pt::iptostring(m_Client->get_ip());
    gSocketLogFile << (const char *)ip << "\tstart\t" << m_RunID << "\t" << theTimeString << "\n";
    gSocketLogFile.flush();
  }
  
  try
  {
    // read the control word
    if (m_Client->waitfor(100) == false) throw(0); // nothing to read
    m_Client->read(buffer, kSocketMaxMessageLength);
    if (strcmp(buffer, kSocketRequestTaskMessage) != 0) throw (0);

    // write the acknowledge control word
    strcpy(buffer, kSocketSendingTask);
    m_Client->write(buffer, kSocketMaxMessageLength);
    
    // and then the data passed as a 16 byte string containing the integer length
    // followed by the actual string (length bytes long)

    gXMLConverter.ApplyGenome(m_Genome);
    buf = (char *)gXMLConverter.GetFormattedXML(&len);
    len++; // make sure we send the terminating /0
    sprintf(buffer, "%d", len);
    m_Client->write(buffer, 16);
    m_Client->write(buf, len);
    m_Client->flush();
  }
  
  catch(int e)
  {
    delete m_Client;
    m_Client = 0;
    return false;
  }
  catch(pt::estream* e)
  {
    delete e;
    delete m_Client;
    m_Client = 0;
    return false;
  }
  
  m_RunningFlag = true;
  m_StartTime = theTime;
  return true;   
}

// call this routine to send a "Busy" message to any client that tries to connect
// returns true if a "Busy" message is sent
bool Objective::ReportBusy()
{
  char buffer[kSocketMaxMessageLength + 1];
  buffer[kSocketMaxMessageLength] = 0;
  
  pt::ipstream theClient;

  try
  {  
    if (g_svr.poll() == false)
    {
      // no client asking for a task
      return false;
    }

    if (g_svr.serve(theClient) == false)
    {
      // some problem communicating
      return false;
    }
  }
  catch (pt::estream* e)
  {
    delete e;
    return false;
  }
  
  try
  {
    // read the control word
    if (theClient.waitfor(100) == false) throw(0); // nothing to read
    theClient.read(buffer, kSocketMaxMessageLength);
    if (strcmp(buffer, kSocketRequestTaskMessage) != 0) throw (0);

    // write the "busy" control word
    strcpy(buffer, kSocketBusy);
    theClient.write(buffer, kSocketMaxMessageLength);
    theClient.flush();
  }
  
  catch(int e)
  {
    return false;
  }
  catch(pt::estream* e)
  {
    delete e;
    return false;
  }
  
  return true;   
}

// check to see whether the command has finished
// and read in the score (we do it here because it makes a bit more sense)

bool Objective::CheckRunning()
{
  char buffer[kSocketMaxMessageLength];
  time_t theTime;
  time_t runTime;
  char theTimeString[64];
  struct tm *theLocalTime;
  pt::string ip;
    
  if (m_RunningFlag == false) return false; // nothing to check
  
  assert(m_Client); // must exist
  
  theTime = time(0);
  runTime = theTime - m_StartTime;
  
  try
  {
    if (m_TimeLimit) // check to see whether we are over time
    {
       if (runTime > m_TimeLimit)
       {
          throw(1);
       }
    }
    
    // is there a result for us?
    if (m_Client->waitfor(1) == false) return true; // still running
    m_Client->read(buffer, kSocketMaxMessageLength);
    if (strcmp(buffer, kSocketSendingScore) != 0) throw (0);
    
    m_Client->read(buffer, 64); // result is a 64 byte string
    m_Fitness = strtod(buffer, 0);
    
    theLocalTime = localtime(&theTime);
    sprintf(theTimeString, "%04d-%02d-%02d_%02d.%02d.%02d", 
        theLocalTime->tm_year + 1900, theLocalTime->tm_mon + 1, 
        theLocalTime->tm_mday,
        theLocalTime->tm_hour, theLocalTime->tm_min,
        theLocalTime->tm_sec);
    ip = pt::iptostring(m_Client->get_ip());
    gSocketLogFile << (const char *)ip << "\tfinish\t" << m_RunID << "\t" << theTimeString << 
        "\t" << buffer << "\t" << runTime << "\n";
    gSocketLogFile.flush();
    
    m_RunningFlag = false;
    return false;
  }
  
  catch (int e)
  {
    theLocalTime = localtime(&theTime);
    sprintf(theTimeString, "%04d-%02d-%02d_%02d.%02d.%02d", 
        theLocalTime->tm_year + 1900, theLocalTime->tm_mon + 1, 
        theLocalTime->tm_mday,
        theLocalTime->tm_hour, theLocalTime->tm_min,
        theLocalTime->tm_sec);
    ip = pt::iptostring(m_Client->get_ip());
    gSocketLogFile << (const char *)ip << "\ttimeout_abort\t" << m_RunID << "\t" << theTimeString << 
        "\t" << runTime << "\n";
    delete m_Client;
    m_Client = 0;
    m_RunningFlag = false;
    while (Run() == false) usleep(10000);
    return m_RunningFlag;
  }
  catch (pt::estream* e)
  {
    delete e;
    theLocalTime = localtime(&theTime);
    sprintf(theTimeString, "%04d-%02d-%02d_%02d.%02d.%02d", 
        theLocalTime->tm_year + 1900, theLocalTime->tm_mon + 1, 
        theLocalTime->tm_mday,
        theLocalTime->tm_hour, theLocalTime->tm_min,
        theLocalTime->tm_sec);
    ip = pt::iptostring(m_Client->get_ip());
    gSocketLogFile << (const char *)ip << "\tstream_abort\t" << m_RunID << "\t" << theTimeString << 
        "\t" << runTime << "\n";
    delete m_Client;
    m_Client = 0;
    m_RunningFlag = false;
    while (Run() == false) usleep(10000);
    return m_RunningFlag;
  }
}



#endif


