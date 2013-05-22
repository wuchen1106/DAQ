/********************************************************************
  RoodyXML.h, J. Chuma, TRIUMF
  $Id: RoodyXML.h 169 2006-09-26 22:34:44Z olchansk $
********************************************************************/
#ifndef RoodyXML_H
#define RoodyXML_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif
#include "mxml.h"
#ifdef __cplusplus
}
#endif

class RoodyXML
{
public:
  unsigned int level_;
  bool elementIsOpen_;
  std::vector<std::string> names_;
  std::ofstream outputFile_;
  char const *encoding_;
  char const *version_;
  PMXML_NODE documentNode_, roodyNode_, histParent_;

  int FindNode(PMXML_NODE parent, int startIndex, const char* name);
  PMXML_NODE  GetNode(PMXML_NODE parent, int index);
  std::string GetNodeString(PMXML_NODE node, int index);
  //void PrintTree( int, PMXML_NODE );

public:
  RoodyXML();
  virtual ~RoodyXML();
  //
  // write
  //
  std::ofstream &OpenFileForWrite( char const * );
  std::string Encode( std::string const & );
  //
  // read
  //
  bool OpenFileForRead( char const * );

  //void PrintTree( int );
};

#endif   // RoodyXML_H
