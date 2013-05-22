#ifndef TRootanaEventLoop_hxx_seen
#define TRootanaEventLoop_hxx_seen

// ROOTANA includes
#include "TMidasFile.h"
#include "TMidasOnline.h"
#include "TMidasEvent.h"
#include "VirtualOdb.h"
#include "TDataContainer.hxx"

// ROOT includes
#include "TApplication.h"
#include "TDirectory.h"
#include <TTimer.h>
#include <TFile.h>

// C++ includes
#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <assert.h>
#include <typeinfo>


/// This is a base class for event loops that are derived from rootana.
/// 
/// The user should create a class that derives from this TRootanaEventLoop class 
/// and then fill in the methods that they want to implement.
///
/// The user must implement the method ProcessMidasEvent(), which will get executed 
/// on each event.
/// 
/// The user can also implement methods like Initialize, BeginRun, EndRun, Finalize
/// if there are actions they want to execute at certain points.
///
/// The event loop will work in both offline and online mode (online only if the user has MIDAS installed).
/// 
/// In example of this type of event loop is shown in examples/analyzer_example.cxx
///
class TRootanaEventLoop {
 
public:
  virtual ~TRootanaEventLoop ();


  static TRootanaEventLoop& Get(void);
  
  /// Method to get the data container that event loop owns.
  TDataContainer* GetDataContainer(){return fDataContainer;};


  /// The main method, called for each event.  Users must implement this
  /// function!
  virtual bool ProcessMidasEvent(TDataContainer& dataContainer) = 0;
  //virtual bool ProcessEvent(TMidasEvent& event) = 0;


  /// Called after the arguments are processes but before reading the first
  /// event is read
  virtual void Initialize(void);
  
  /// Called before the first event of a file is read, but you should prefer
  /// Initialize() for general initialization.  This method will be called
  /// once for each input file.  
  virtual void BeginRun(int transition,int run,int time);
  
  /// Called after the last event of a file is read, but you should prefer
  /// Finalize() for general finalization.  This method will be called once
  /// for each input file.
  virtual void EndRun(int transition,int run,int time);
  
  /// Called after the last event has been processed, but before any open
  /// output files are closed.
  virtual void Finalize();

  /// Called when there is a usage error.  This code should print a usage
  /// message and then return. 
  virtual void Usage(void);
  
  /// Check an option and return true if it is valid.  
  /// The return value is used to flag errors during option handling.  If
  /// the options are valid, then CheckOption should return true to indicate
  /// success.  If there is a problem processing the options, then CheckOption
  /// should return false.  If this returns false, then the event loop will
  /// print the Usage message and exit with a non zero value (i.e. indicate
  /// failure).
  virtual bool CheckOption(std::string option);

  /// Are we processing online data?
  bool IsOnline() const {return !fIsOffline;};
  
  /// Are we processing offline data?
  bool IsOffline() const {return fIsOffline;};
  
  /// Current Run Number
  int GetCurrentRunNumber() const {return fCurrentRunNumber;};

  /// Current Run Number
  void SetCurrentRunNumber(int run) {fCurrentRunNumber = run;};

  /// Method to actually process the Midas information, either as file or online.
  int ExecuteLoop(int argc, char *argv[]);

  int ProcessMidasFile(TApplication*app,const char*fname);

#ifdef HAVE_MIDAS
  int ProcessMidasOnline(TApplication*app, const char* hostname, const char* exptname);
#endif


  /// This static templated function will make it a little easier
  /// for users to create the singleton instance.
  template<typename T>
  static void CreateSingleton()
  {
    if(fTRootanaEventLoop)
      std::cout << "Singleton has already been created" << std::endl;
    else
      fTRootanaEventLoop = new T();
  } 
  

  /// Disable automatic creation of MainWindow
  void DisableAutoMainWindow(){  fCreateMainWindow = false;}

  /// Get pointer to ODB variables
  VirtualOdb* GetODB(){return fODB;}


  /// Open output ROOT file
  void OpenRootFile(int run);

  /// Cloe output ROOT file
  void CloseRootFile();
  
  void DisableRootOutput(bool disable=true){fDisableRootOutput = disable;};

protected:

  bool CreateOutputFile(std::string name, std::string options = "RECREATE"){
    
    fOutputFile = new TFile(name.c_str(),options.c_str());
    
    return true;
  }


  TRootanaEventLoop ();

  /// The static pointer to the singleton instance.
  static TRootanaEventLoop* fTRootanaEventLoop;

  /// TDirectory for online histograms.
  TDirectory* fOnlineHistDir;



private:
  
  /// Help Message
  void PrintHelp();

  /// Output ROOT file
  TFile *fOutputFile;

  /// Variable for disabling/enabling Root output
  bool fDisableRootOutput;

  /// Pointer to the ODB access instance
  VirtualOdb* fODB;
 
  /// Are we processing offline or online data?
  bool fIsOffline;

  /// Current run number
  int fCurrentRunNumber;


  /// Pointer to the physics event; the physics event is what we pass to user.
  /// The midas event is accessible through physics event.
  /// We make a single instance of the physics event for whole execution,
  /// because sometimes the decoded information needs to persist 
  /// across multiple midas events.
  TDataContainer *fDataContainer;


  // ________________________________________________
  // Variables for online analysis


  // ________________________________________________
  // Variables for offline analysis
  int fMaxEvents;

  // The TApplication...
  TApplication *fApp;

  // Should we automatically create a Main Window?
  bool fCreateMainWindow;


};
#endif
