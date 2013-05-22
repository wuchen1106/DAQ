//
// DataSourceTDirectory.h
//
// $Id: DataSourceTDirectory.h 236 2010-10-19 18:41:15Z olchansk $
//
#ifndef DataSourceTDirectoryH
#define DataSourceTDirectoryH

#include "DataSourceBase.h"

class TFolder;
class TDirectory;

class DataSourceTDirectory: public DataSourceBase
{
 protected:
  TDirectory* fDir;

 public:
  DataSourceTDirectory(); // ctor
  DataSourceTDirectory(TDirectory* source); // ctor
  void SetDirectory(TDirectory* source);

  ~DataSourceTDirectory(); // dtor

  ObjectList GetList();

  TObject* GetObject(const ObjectPath& path);

 protected:
  void EnumerateTFolder(ObjectList* list, TFolder* f, ObjectPath path);

  TObject* DirGetObject(TDirectory* dir, const char* name);

  void EnumerateTDirectory(ObjectList* list, TDirectory* dir, ObjectPath path);
};

#endif
// end
