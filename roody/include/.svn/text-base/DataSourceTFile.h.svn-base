//
// DataSourceTFile.h
//
#ifndef DataSourceTFileH
#define DataSourceTFileH

#include "DataSourceTDirectory.h"
#include "TFile.h"

class TDirectory;

class DataSourceTFile: public DataSourceTDirectory
{
 protected:
  std::string fFilename;
  TFile* fFile;

 public:
  DataSourceTFile(const char* filename)
  {
    fFilename = filename;
    fFile = new TFile(filename, "READ");
    SetDirectory(fFile);
  }
  
  ~DataSourceTFile()
  {
    fFile->Close();
    delete fFile;
    fFile = NULL;
    fFilename = "";
  }

  void Reopen()
  {
    fFile->Close();
    delete fFile;
    printf("Reopen %s\n", fFilename.c_str());
    fFile = new TFile(fFilename.c_str(), "READ");
    SetDirectory(fFile);
  }
};

#endif
// end
