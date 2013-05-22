//
// DataSourceTDirectory.cxx
//
// $Id: DataSourceTDirectory.cxx 236 2010-10-19 18:41:15Z olchansk $
//

#include "DataSourceTDirectory.h"

#include "TDirectory.h"
#include "TKey.h"
#include "TFolder.h"

#include <assert.h>

DataSourceTDirectory::DataSourceTDirectory() // ctor
{
  fName = "";
  fDir = NULL;
}

DataSourceTDirectory::DataSourceTDirectory(TDirectory* source) // ctor
{
  SetDirectory(source);
}

DataSourceTDirectory::~DataSourceTDirectory() // dtor
{
  fDir = NULL;
}

void DataSourceTDirectory::SetDirectory(TDirectory* source)
{
  assert(source != NULL);
  fName = source->GetName();
  fDir  = source;
}

void DataSourceTDirectory::EnumerateTFolder(ObjectList* list, TFolder* f, ObjectPath path)
{
  TIterator *iterator = f->GetListOfFolders()->MakeIterator();
  while (1)
    {
      TNamed *obj = (TNamed*)iterator->Next();
      if (obj == NULL)
	break;
      
      const char* classname = obj->ClassName();
      const char* name      = obj->GetName();
      
      if (strcmp(classname,"TFolder") == 0)
	{
	  TFolder* f = (TFolder*)obj;
	  ObjectPath p = path;
	  p.push_back(name);
	  EnumerateTFolder(list, f, p);
	}
      else if (strcmp(classname,"TDirectory") == 0)
	{
	  TDirectory* d = (TDirectory*)obj;
	  ObjectPath p = path;
	  p.push_back(name);
	  EnumerateTDirectory(list, d, p);
	}
      else
	{
	  ObjectPath p = path;
	  std::string s;
	  s += name;
	  //s += "(";
	  //s += classname;
	  //s += ")";
	  p.push_back(s);
	  list->push_back(p);
	}
    }
  
  delete iterator;
}

TObject* DataSourceTDirectory::DirGetObject(TDirectory* dir, const char* name)
{
  TObject *obj = dir->FindObject(name);
  if (obj)
    return obj;

  //printf("DirGetObject: %s\n", name);
  obj = dir->Get(name);
  //dir->Add(obj);
  return obj;
}

void DataSourceTDirectory::EnumerateTDirectory(ObjectList* list, TDirectory* dir, ObjectPath path)
{
  TIter next = dir->GetListOfKeys();
  while(1)
    {
      TKey *key = (TKey*)next();
      if (key == NULL)
	break;
      
      const char* classname = key->GetClassName();
      const char* name      = key->GetName();

      //printf("Enumerating keys: %s %s\n", classname, name);
      
      if (strcmp(classname,"TFolder") == 0)
	{
	  TFolder* f = (TFolder*)DirGetObject(dir, name);
	  //printf("TDirectory get TFolder %s -> %p\n", name, f);
	  if (f)
	    {
	      ObjectPath p = path;
	      p.push_back(name);
	      EnumerateTFolder(list, f, p);
	    }
	}
      else if ((strcmp(classname,"TDirectory") == 0) ||
	       (strcmp(classname,"TDirectoryFile") == 0))
	{
	  TDirectory* d = (TDirectory*)DirGetObject(dir, name);
	  if (d)
	    {
	      ObjectPath p = path;
	      p.push_back(name);
	      EnumerateTDirectory(list, d, p);
	    }
	}
      else
	{
	  ObjectPath p = path;
	  std::string s;
	  s += name;
	  //s += "(";
	  //s += classname;
	  //s += ")";
	  p.push_back(s);
	  list->push_back(p);
	}
    }
}

ObjectList DataSourceTDirectory::GetList()
{
  ObjectList list;
  ObjectPath path;
  path.push_back(fName);
  EnumerateTDirectory(&list, fDir, path);
  return list;
}

TObject* DataSourceTDirectory::GetObject(const ObjectPath& path)
{
  const char *name = path.back().c_str();
  printf("Get object %s from %s\n", path.toString().c_str(), fName.c_str());
  TDirectory* dir = fDir;
  
  int np = path.size();
  for (int i=1; i<np; i++)
    {
      TNamed *obj = (TNamed*)DirGetObject(dir, path[i].c_str());
      if (!obj)
	{
	  printf("Unexpected error: cannot get object %s from TDirectory %s\n",path[i].c_str(), dir->GetName());
	  dir->Print();
	  return NULL;
	}

      if (i==np-1)
	{
	  return obj;
	}
      else if (obj->InheritsFrom(TDirectory::Class()))
	{
	  dir = (TDirectory*)obj;
	}
      else if (obj->InheritsFrom(TFolder::Class()))
	{
	  TFolder *f = (TFolder*)obj;
	  TObject *xxx = (TObject*)f->FindObjectAny(name);
	  if (xxx)
	    return xxx;
	  
	  printf("Unexpected error: cannot get object %s from TFolder %s\n",name, f->GetName());
	  f->Print();
	  return NULL;
	}
      else
	{
	  printf("Unexpected error: untested code path, object \'%s\' in TDirectory \'%s\', looking for \'%s\', i: %d, np: %d\n",obj->GetName(), dir->GetName(), name, i, np);
	  return obj;
	}
    }
  return NULL;
}

// end
