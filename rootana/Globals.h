//
// Global variables for the ROOT analyzer
//
// Name: Globals.h
//
// $Id: Globals.h 64 2008-12-22 03:14:11Z olchansk $
//

// Run parameters

extern int  gRunNumber;
extern bool gIsRunning;
extern bool gIsPedestalsRun;
extern bool gIsOffline;

// Output files

extern TFile* gOutputFile;
extern TDirectory* gOnlineHistDir;

// ODB access

#include "VirtualOdb.h"

extern VirtualOdb* gOdb;

// end

