//
// DataSourceTNetFolder.h
//
// $Id: DataSourceTNetFolder.h 236 2010-10-19 18:41:15Z olchansk $
//

#include "DataSourceBase.h"

#include "TSocket.h"
#include "TFolder.h"

#include <stdint.h>

class DataSourceTNetFolder: public DataSourceBase
{
 private:
  TSocket *fSocket;

 public:
  DataSourceTNetFolder(TSocket* socket) // ctor
    {
      fName = socket->GetName();
      fName += ":";
      char buf[256];
      sprintf(buf, "%d", socket->GetPort());
      fName += buf;
      fSocket = socket;
    }

  ~DataSourceTNetFolder() // dtor
    {
      fSocket = NULL;
    }

  void Close()
    {
      fSocket->Close();
    }

  void Reopen()
    {
      std::string host = fSocket->GetName();
      int         port = fSocket->GetPort();
      fSocket->Close();
      delete fSocket;
      fSocket = new TSocket(host.c_str(), port);
    }

  static TObjArray* ListRemoteObjects(TSocket* sock, const char* foldername)
    {
      size_t p = 0;
      
      {
	std::string str = "GetPointer ";
	str += foldername;
	if (!sock->Send(str.c_str()))
	  return NULL;
	
	TMessage *m = NULL;
	if (!sock->Recv(m))
	  return NULL;
	
	if (!m)
	  return NULL;
	
	*m >> p;
	delete m;
	m = NULL;
      }

      if (!sock->Send("GetListOfFolders"))
	return NULL;

      TMessage m(kMESS_ANY);
      m.Reset();
      m << p;

      if (!sock->Send(m))
	return NULL;

      TMessage *mr = 0;
      if (!sock->Recv(mr))
	return NULL;

      TObjArray *list = NULL;

      if (mr)
	{
	  list = (TObjArray *) mr->ReadObjectAny(mr->GetClass());
	}
      
      delete mr;
      return list;
    }

  static TObject* ReadRemoteObject(TSocket* sock, const char* foldername, const char* objname)
    {
      printf("ReadRemoteObject %s from folder %s\n", objname, foldername);

      uint32_t p = 0;
      
      {
	std::string str = "GetPointer ";
	str += foldername;
	if (!sock->Send(str.c_str()))
	  return NULL;
	
	TMessage *m = NULL;
	if (!sock->Recv(m))
	  return NULL;
	
	if (!m)
	  return NULL;
	
	*m >> p;
	delete m;
	m = NULL;
      }

      printf("Folder %s pointer %d\n", foldername, (int)p);

      {
	std::string str = "FindObject ";
	str += objname;
	if (!sock->Send(str.c_str()))
	  return NULL;

	TMessage m(kMESS_ANY);
	m.Reset();
	m << p;
	if (!sock->Send(m))
	  return NULL;

	TMessage *mr = 0;

	if (!sock->Recv(mr))
	  return NULL;

	TObject *obj = NULL;

	if (mr)
	  {
	    bool adddir = TH1::AddDirectoryStatus();
	    TH1::AddDirectory(false);

	    obj = (TObject*)mr->ReadObjectAny(0);

	    TH1::AddDirectory(adddir);
	  }
	
	delete mr;
	return obj;
      }
    }

  static void ExecuteRemoteCommand(TSocket* socket, const char *line)
    {
      if (!socket->IsValid())
	return;
      
      // The line is executed by the CINT of the server
      socket->Send("Execute");
      socket->Send(line);
    }

  void EnumerateTFolder(ObjectList* list, TFolder* f, ObjectPath path)
  {
    TIterator   *iterator = f->GetListOfFolders()->MakeIterator();
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
	//else if (strcmp(classname,"TDirectory") == 0)
	//  {
	//    TDirectory* d = (TDirectory*)obj;
	//    ObjectPath p = path;
	//    p.push_back(name);
	//    EnumerateTDirectory(list, d, p);
	//  }
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

  TFolder* ReadFolder(const char* foldername, ObjectPath path, const char* name)
    {
      TObject* obj = ReadRemoteObject(fSocket, foldername, name);
      if (!obj)
	return NULL;

      if (obj->InheritsFrom(TFolder::Class()))
	return (TFolder*)obj;

      ObjectPath p = path;
      p.push_back(name);

      return NULL;
    }

  void Enumerate(ObjectList* list, const char* foldername, ObjectPath path)
  {
    TCollection *contents = ListRemoteObjects(fSocket, foldername);
    if (!contents)
      return;
    TIterator   *iterator = contents->MakeIterator();
    while (1)
      {
	TNamed *obj = (TNamed*)iterator->Next();
	if (obj == NULL)
	  break;

	const char* classname = obj->ClassName();
	const char* name      = obj->GetName();

	if (strcmp(classname,"TObjString") == 0)
	  {
	    TObject* xobj = ReadRemoteObject(fSocket, foldername, name);
	    if (!xobj)
	      continue;
	    
	    if (xobj->InheritsFrom(TFolder::Class()))
	      {
		TFolder *fff = (TFolder*)xobj;
		ObjectPath p = path;
		p.push_back(name);
		EnumerateTFolder(list, fff, p);
		continue;
	      }
	  }

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

  ObjectList GetList()
  {
    ObjectList list;
    ObjectPath path;
    path.push_back(fName);
    const char *topfolder = "histos";
    path.push_back(topfolder);
    Enumerate(&list, topfolder, path);
    return list;
  }

  TObject* GetObject(const ObjectPath& path)
    {
      //gDirectory->pwd();
      //gDirectory->ls();
      
      const char *name = path.back().c_str();
      printf("Get object %s from %s\n", name, fName.c_str());

      const char *foldername = path[path.size()-2].c_str();

      TObject* obj = ReadRemoteObject(fSocket, foldername, name);

      return obj;
    }

  void ResetAll()
  {
    printf("ResetAll %s\n", fName.c_str());

    ExecuteRemoteCommand(fSocket, "gAnalyzer->ResetAllHistos()");
  }

  void ResetObject(const ObjectPath& path)
  {
    printf("ResetObject %s\n", path.toString().c_str());

    std::string name = path.back();

    std::string s;
    s = "gAnalyzer->GetHisto(\"";
    s += name;
    s += "\")->Reset()";
    ExecuteRemoteCommand(fSocket, s.c_str());

    if (fSocket->IsValid())
      {
	fSocket->Send( "Command" );
	fSocket->Send( name.c_str() );
	fSocket->Send( "Reset" );
      }
  }

};

// end
