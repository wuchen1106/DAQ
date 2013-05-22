//
// DataSourceBase.h
//
// $Id: DataSourceBase.h 169 2006-09-26 22:34:44Z olchansk $
//

#ifndef DataSourceBaseH
#define DataSourceBaseH

#include <string>
#include <vector>

class TObject;

struct ObjectPath: std::vector<std::string>
{
  std::string toString() const
  {
    if (size() < 1)
      return "(empty)";
    std::string s;
    s += this->front();
    for (unsigned int i=1; i<this->size(); i++)
      {
	s += "/";
	s += (*this)[i];
      }
    return s;
  }
};

typedef std::vector<ObjectPath>  ObjectList;

class DataSourceBase
{
 protected:
  std::string fName; // source name

 public:
  std::string GetName() { return fName; }

  virtual ObjectList GetList() = 0;
  virtual TObject*   GetObject(const ObjectPath& objectPathname) = 0; // return an object for the given path

  virtual void ResetObject(const ObjectPath& objectPathname) { /* do nothing */ };
  virtual void ResetAll() { /* do nothing */ };

  virtual void Reopen() { };
  virtual void Close()  { };
  virtual ~DataSourceBase() { }; // dtor

  virtual std::string toString()
    {
      std::string out;

      out += "Data source: " + fName + "\n";
      out += "Contents:\n";
      ObjectList list = GetList();
      int n = list.size();
      for (int i=0; i<n; i++)
	{
	  ObjectPath obj = list[i];
	  out += "  Object " + obj.front();
	  int nn = obj.size();
	  for (int j=1; j<nn; j++)
	    out += "/" + obj[j];
	  out += "\n";
	}

      return out;
    }

 protected:
  DataSourceBase() { }; // ctor
};

#endif
// end
