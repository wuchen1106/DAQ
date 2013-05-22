//
// DataSourceNetDirectory.h
//
// $Id: DataSourceTDirectory.h 174 2006-10-07 00:24:13Z jonty $
//
#ifndef DataSourceTNetDirectoryH
#define DataSourceTNetDirectoryH

#include "DataSourceTDirectory.h"
#include "TNetDirectory.h"

class TFolder;
class TDirectory;

class DataSourceNetDirectory: public DataSourceTDirectory
{
 protected:
  TNetDirectory* fNetDir;

 public:
  DataSourceNetDirectory(TNetDirectory* source)
    : DataSourceTDirectory(source)
  {
    fNetDir = source;
  }
  
  ~DataSourceNetDirectory()
  {
    fNetDir = NULL;
  }

  void Reopen()
  {
    fNetDir->Reconnect();
  }
  
  void ResetObject(const ObjectPath& path)
  {
    printf("ResetObject %s\n", path.toString().c_str());
    
    std::string s = path[1];
    for (unsigned int i=2; i<path.size(); i++)
      {
	s += "/";
	s += path[i];
      }
    fNetDir->ResetTH1(s.c_str());

    //std::string name = path.back();
    //fNetDir->ResetTH1(name.c_str());
  }

  void ResetAll()
  {
    printf("ResetAll\n");
    fNetDir->ResetTH1("");
  }

};

#endif
// end
