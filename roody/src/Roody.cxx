//
// Roody.cxx
//
// $Id: Roody.cxx 236 2010-10-19 18:41:15Z olchansk $
//

#include <stdio.h>
#include <math.h>

#include "RoodyXML.h"
#include "Roody.h"

#include "TAxisLimitsDialog.h"

#include "TROOT.h"
#include "TCutG.h"
#include "TNtuple.h"
#include "TGraph.h"
#include "TVirtualX.h"
#include "TGListTree.h"
#include "TGClient.h"
#include "TGLabel.h"
#include "TGButton.h"
#include "TGMsgBox.h"
#include "TGMenu.h"
#include "TGCanvas.h"
#include "TGTextDialog.h"
#include "TCanvas.h"
#include "TFrame.h"
#include "TH1.h"
#include "TSystem.h"
#include "TFile.h"
#include "TKey.h"
#include "TGStatusBar.h"
#include "TExec.h"
#include "TStyle.h"
#include "TTimer.h"
#include "TContextMenu.h"
#include "TClassMenuItem.h"
#include "TSocket.h"
#include "MTGListTree.h"
#include "TPeakFindPanel.h"
#include "TGFileDialog.h"
#include "TPaveLabel.h"

#ifdef HAVE_HBOOK
#include "THbookKey.h"
#include "THbookFile.h"
#include "THbookTree.h"
#include "THbookBranch.h"
#endif

void breakhere(void)
{
  // nothing
}

class MemDebug : public TObject
{
public:

  MemDebug() // ctor
  {
    printf("MemDebug %p created!\n", this);
  }

  ~MemDebug() // dtor
  {
    printf("MemDebug %p destroyed!\n", this);
    breakhere();
  }

  TObject* Clone()
  {
    TObject* obj = new MemDebug();
    printf("MemDebug %p cloned into %p!\n", this, obj);
    return obj;
  }
};

#include "DataSourceBase.h"
#include "DataSourceTDirectory.h"
#include "DataSourceTNetFolder.h"
#include "DataSourceTFile.h"
#ifdef HAVE_NETDIRECTORY
#include "DataSourceNetDirectory.h"
#endif

enum Action
  {
    A_ITEM   = 1,
    A_FOLDER = 2,
    A_SOURCE = 3,
    A_GROUP  = 4,
  };

TList* gAllObjects = NULL;

std::map<TGListTreeItem*, Action> gItemActions;

std::map<ObjectPath,TObject*> gObjectCache;

typedef std::map<std::string,DataSourceBase*> DataSourcesMap;
DataSourcesMap gDataSources;

std::map<TGListTreeItem*,ObjectPath>   gItemSources;
std::map<ObjectPath,std::string>       gObjectClasses;

std::map<std::string,std::vector<TGListTreeItem*> > gGroupContents;

static TGListTreeItem* FindItem(ObjectPath src)
{
  std::map<TGListTreeItem*,ObjectPath>::iterator curr = gItemSources.begin();
  std::map<TGListTreeItem*,ObjectPath>::iterator end  = gItemSources.end();
  for (; curr != end; curr++)
    {
      TGListTreeItem* p = (*curr).first;
      ObjectPath s = (*curr).second;
      if (s == src)
	return p;
    }
  return NULL;
}

TObject* Roody::GetObject(const ObjectPath& src)
{
  if (src.size() < 1)
    {
      //printf("GetObject %s return NULL\n", src.toString().c_str());
      return NULL;
    }

  TObject *obj = gObjectCache[src];
  if (obj)
    {
      //printf("GetObject %s return %p (cached)\n", src.toString().c_str(), obj);
      return obj;
    }

  DataSourceBase* d = gDataSources[src.front()];
  if (d)
    {
      obj = d->GetObject(src);

      if (obj == NULL)
	{
	  std::string s;
	  s += "Cannot access ";
	  s += src.toString();
	  fStatusBar->SetText(s.c_str());
#if 0
	  obj = new TPaveLabel(0.1,0.1,0.9,0.9,s.c_str());
#endif
	}
    }
  else
    {
      std::string s;
      s += "No source for ";
      s += src.toString();
      fStatusBar->SetText(s.c_str());
#if 0
      obj = new TPaveLabel(0.1,0.1,0.9,0.9,s.c_str());
#endif
    }


  gObjectCache[src] = obj;
  
  //printf("GetObject %s return %p\n", src.toString().c_str(), obj);

  return obj;
}

TObject* Roody::RereadObject(const ObjectPath& src)
{
  TObject *obj = gObjectCache[src];
  if (obj)
    {
      gObjectCache.erase(src);
    }

  TObject *xobj = GetObject(src);

  if (xobj == NULL)
    {
      xobj = obj;
      gObjectCache[src] = obj;
    }

  // if new object is same as the old one, do not delete it!
  if (obj != xobj)
    {
      //printf("delete %p\n", obj);
      delete obj;
    }

  return xobj;
}

std::map<TVirtualPad*,PadObjectVec> gPadContents;

ClassImp(Roody);

TPeakFindPanel *Roody::fgPeakFindPanel = 0;

Roody::Roody(const TGWindow *p, UInt_t w, UInt_t h) // ctor
  : TGMainFrame(p?p:gClient->GetRoot(), w, h)
{
  if (gAllObjects == NULL)
    {
      gAllObjects = new TList();
      gROOT->GetListOfCleanups()->Add(gAllObjects);
    }

  //
  fPopupMenu = 0;
  fAddToGroupPopup = 0;
  //
  fCanvasCount = 0;
  fZoomCanvas = 0;
  //
  SetWindowName("Roody");
  //
#if 0
  gStyle->SetPalette( 1 );
  //
  gStyle->SetTitleTextColor( kBlue );
  gStyle->SetStatTextColor( kBlue );
  gStyle->SetTitleBorderSize( 0 );
  gStyle->SetStatBorderSize( 0 );
  //
  gStyle->SetFuncColor( kBlue );
  gStyle->SetOptFit( 1111 );
#endif
  gStyle->SetOptStat( 1001111 );
  //
  LayoutGUI();
  AddPeakFind();
  AddAxisLimits();
  AddSetCut();
  AddZoomOption();
  //
  Resize( GetDefaultSize() );
  MapSubwindows();
  Layout(); // member function of TGCompositeFrame
  MapWindow();
  fStatusBar->SetText("Welcome to Roody");
  //
  fZoneRows = 1;  // default zone row setting
  fZoneColumns = 1;  // default zone column setting
  fUpdateTimerSec = 0;  //default refresh time
  fUpdatePause = false;
  fUpdateTimer = new TTimer(0,kTRUE);
  //
  TQObject::Connect( fUpdateTimer,"Timeout()","Roody",this,"RefreshAll()" );
  StartUpdateTimer(); // Refresh off by default
  //
}

static std::string PathToString(const ObjectPath& src)
{
  std::string s = src.front();
  for (unsigned int j=1; j<src.size(); j++)
    {
      s += "|";
      s += src[j];
    }
  return s;
}

static std::vector<std::string> split(char sep, const std::string& str)
{
  std::vector<std::string> vec;
  
  char buf[1024];
  strncpy(buf, str.c_str(), sizeof(buf));
  buf[sizeof(buf)-1] = 0;
  
  char* s = buf;
  char* p = buf;
  
  while (s[0] != 0)
    {
      while (*p != 0)
	{
	  if (*p == sep)
	    {
	      *p = 0;
	      p++;
	      break;
	    }
	  p++;
	}
      
      vec.push_back(s);
      s = p;
    }

  return vec;
}

static std::vector<int> toInt(const std::vector<std::string>& vec)
{
  std::vector<int> ivec;
  for (unsigned int i=0; i<vec.size(); i++)
    ivec.push_back(atoi(vec[i].c_str()));
  return ivec;
}

static ObjectPath StringToPath(const std::string& source)
{
  ObjectPath path;
  std::vector<std::string> s = split('|', source);
  for (unsigned int i=0; i<s.size(); i++)
    path.push_back(s[i]);
  return path;
}

#if 0
static ObjectPath StringToPath(const std::string& source)
{
  ObjectPath path;
  
  char buf[1024];
  strncpy(buf, source.c_str(), sizeof(buf));
  buf[sizeof(buf)-1] = 0;
  
  char* s = buf;
  char* p = buf;
  
  while (s[0] != 0)
    {
      while (*p != 0)
	{
	  if (*p == '|')
	    {
	      *p = 0;
	      p++;
	      break;
	    }
	  p++;
	}
      
      path.push_back(s);
      s = p;
    }

  return path;
}
#endif

void Roody::RestoreObjects(RoodyXML*xml, TVirtualPad *pad, PMXML_NODE parent)
{
  for (int j=0; ; j++)
    {
      j = xml->FindNode(parent, j, "object");
      if (j<0)
	break;

      PMXML_NODE xobject = xml->GetNode(parent, j);
      
      std::string xclass     = xml->GetNodeString(xobject, xml->FindNode(xobject, 0, "class"));
      std::string name       = xml->GetNodeString(xobject, xml->FindNode(xobject, 0, "name"));
      std::string source     = xml->GetNodeString(xobject, xml->FindNode(xobject, 0, "source"));
      std::string drawOption = xml->GetNodeString(xobject, xml->FindNode(xobject, 0, "drawOption"));
      
      if (source.length() < 1)
	continue;
      
      ObjectPath path = StringToPath(source);
      TGListTreeItem* item = FindItem(path);
      
      if (!item)
	{
	  std::string s;
	  s += "Cannot find object \"";
	  s += name;
	  s += "\"";
	  new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
	  continue;
	}
      
      DrawItemOnPad(item, pad, true);
    }
}

void Roody::RestoreFile(const char* xmlFilename)
{
  int inode;
  RoodyXML xml;

  if (!xmlFilename)
    {
      xmlFilename = "default.xml";
      if (!xml.OpenFileForRead(xmlFilename))
	return;
    }
  else
    {
      if (!xml.OpenFileForRead(xmlFilename))
	{
	  std::string s = "Failed to open XML file: ";
	  s += xmlFilename;
	  fStatusBar->SetText(s.c_str());
	  new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
	  return;
	}
    }

  //xml.PrintTree( 0 );

  for (inode = 0; ; inode++)
    {
      inode = xml.FindNode(xml.roodyNode_, inode, "online");
      if (inode < 0)
	break;
  
      std::string serverName = xml.GetNodeString(xml.roodyNode_, inode);

      ConnectServer(serverName.c_str());
    }
  
  for (inode = 0; ; inode++)
    {
      inode = xml.FindNode(xml.roodyNode_, inode, "file");
      if (inode < 0)
	break;
  
      std::string fileName = xml.GetNodeString(xml.roodyNode_, inode);

      TGListTreeItem *fileItem = fContents->FindChildByName(fTreeItemFiles,fileName.c_str()); 
      if (!fileItem)
	{
	  if (OpenFile(fileName.c_str()))
	    { 
	      std::string s("Could not open data file \"");
	      s += fileName + "\"\nerror occurred while parsing xml file \"" + xmlFilename; 
	      new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
	      //return;
	    }
	}
    }
  
  inode = xml.FindNode(xml.roodyNode_, 0, "graphics");
  
  if (inode >= 0)
    {
      PMXML_NODE xgraphics = xml.GetNode(xml.roodyNode_, inode);
      
      std::string zones = xml.GetNodeString(xgraphics, xml.FindNode(xgraphics, 0, "zones"));
      int refresh = atoi(xml.GetNodeString(xgraphics, xml.FindNode(xgraphics, 0, "refresh")).c_str());

      std::vector<int> izones = toInt(split('x', zones));
      
      fZoneColumns = izones[0];
      fZoneRows    = izones[1];
      SetupZonesMenu();

      SetRefreshRate(refresh);
    }

  for (int i=0; ; i++)
    {
      i = xml.FindNode(xml.roodyNode_, i, "canvas");
      if (i<0)
	break;

      PMXML_NODE xcanvas = xml.GetNode(xml.roodyNode_, i);

      std::string name  = xml.GetNodeString(xcanvas, xml.FindNode(xcanvas, 0, "name"));
      std::string size  = xml.GetNodeString(xcanvas, xml.FindNode(xcanvas, 0, "size"));
      std::string zones = xml.GetNodeString(xcanvas, xml.FindNode(xcanvas, 0, "zones"));
      //printf("canvas name: %s\n", name.c_str());

      std::vector<int> izones = toInt(split('x', zones));
      std::vector<int> isize  = toInt(split(' ', size));

      TCanvas* c = MakeNewCanvas(NULL, izones[0], izones[1], isize[0], isize[1], isize[2], isize[3]);

      RestoreObjects(&xml, c, xcanvas);

      for (int j=0; ; j++)
	{
	  j = xml.FindNode(xcanvas, j, "pad");
	  if (j<0)
	    break;

	  PMXML_NODE xpad = xml.GetNode(xcanvas, j);

	  int number = atoi(xml.GetNodeString(xpad, xml.FindNode(xpad, 0, "number")).c_str());
	  RestoreObjects(&xml, c->cd(number), xpad);
	}
    }

  for (int i=0; ; i++)
    {
      i = xml.FindNode(xml.roodyNode_, i, "group");
      if (i<0)
	break;

      PMXML_NODE xgroup = xml.GetNode(xml.roodyNode_, i);

      std::string groupname  = xml.GetNodeString(xgroup, xml.FindNode(xgroup, 0, "name"));
      //printf("group name: %s\n", groupname.c_str());

      fContents->OpenItem(fTreeItemGroups);

      TGListTreeItem *groupItem = MakeNewGroup(groupname.c_str());
      assert(groupItem);

      for (int j=0; ; j++)
	{
	  j = xml.FindNode(xgroup, j, "histogram");
	  if (j<0)
	    break;

	  PMXML_NODE xobject = xml.GetNode(xgroup, j);

	  std::string name   = xml.GetNodeString(xobject, xml.FindNode(xobject, 0, "name"));
	  std::string source = xml.GetNodeString(xobject, xml.FindNode(xobject, 0, "source"));

	  ObjectPath path = StringToPath(source);
	  TGListTreeItem* item = FindItem(path);

	  AddHistogramToGroup(groupItem, name.c_str(), item);
	}
    }
}

void Roody::SetupZonesMenu()
{
  UncheckAllZones();
  switch( fZoneColumns )
  {
    case 1:
    {
      switch( fZoneRows )
      {
        case 1:
          fMenuZones->CheckEntry( M_ZONES_11 );
          break;
        case 2:
          fMenuZones->CheckEntry( M_ZONES_12 );
          break;
        case 3:
          fMenuZones->CheckEntry( M_ZONES_13 );
          break;
        default:
          SetZonesUser();
      }
      break;
    }
    case 2:
    {
      switch( fZoneRows )
      {
        case 1:
          fMenuZones->CheckEntry( M_ZONES_21 );
          break;
        case 2:
          fMenuZones->CheckEntry( M_ZONES_22 );
          break;
        default:
          SetZonesUser();
      }
      break;
    }
    case 3:
    {
      switch( fZoneRows )
      {
        case 3:
          fMenuZones->CheckEntry( M_ZONES_33 );
          break;
        default:
          SetZonesUser();
      }
      break;
    }
    case 4:
    {
      switch( fZoneRows )
      {
        case 3:
          fMenuZones->CheckEntry( M_ZONES_44 );
          break;
        default:
          SetZonesUser();
      }
      break;
    }
    default:
      SetZonesUser();
  }
}

void Roody::SetZonesUser()
{
  fMenuZones->DeleteEntry( M_ZONES_USER );
  std::stringstream ss;
  ss << fZoneColumns << "x" << fZoneRows;
  fMenuZones->AddEntry( ss.str().c_str(), M_ZONES_USER,
                        0, 0, fMenuZones->GetEntry(M_ZONES_DIALOG) );
  fMenuZones->CheckEntry( M_ZONES_USER );
}

void Roody::SaveFilePadContents(RoodyXML& xml, std::ofstream& output, std::string space, const PadObjectVec& contents)
{
  for (unsigned int i=0; i<contents.size(); i++)
    {
      output << space << "<object>\n";
      ObjectPath src = contents[i].fSource;
      TObject *obj = GetObject(src);
      //printf("lookup object %p\n", obj);
      assert(obj);
      output << space << "  <class>" << obj->ClassName() << "</class>\n";
      output << space << "  <name>"  << xml.Encode(obj->GetName()) << "</name>\n";
      if (src.front() != "")
	{
	  output << space << "  <source>";
	  output << xml.Encode(PathToString(src));
	  output << "</source>\n"; 
	  output << space << "  <drawOption>";
	  output << xml.Encode(contents[i].fDrawOption);
	  output << "</drawOption>\n"; 
	}
      output << space << "</object>\n";
    }
}

void Roody::SaveFile(const char* xmlFilename)
{
  fStatusBar->SetText((std::string("Saving session into: ") + xmlFilename).c_str());

  RoodyXML xml;
  std::ofstream &output( xml.OpenFileForWrite(xmlFilename) );
  if( output.rdstate() != std::ios::goodbit  )
  {
    std::string s("Failed to open XML file \"");
    s += xmlFilename;
    new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
    return;
  }
  output << "<roody>\n";
  //
  // write online section
  //
  for (unsigned int i=0; i<fOnlineFiles.size(); i++)
    {
      output << "  <online>";
      output << xml.Encode(fOnlineFiles[i].c_str());
      output << "</online>\n";
    }
  // 
  // write file section
  // 
  TGListTreeItem *temp = fTreeItemFiles->GetFirstChild();
  if( temp ) 
  { 
    output << "  <file>" << xml.Encode(temp->GetText()) << "</file>\n";
    TGListTreeItem *temp2;
    while( (temp2=temp->GetNextSibling()) )
    { 
      output << "  <file>" << xml.Encode(temp2->GetText()) << "</file>\n";
      temp = temp2;
    } 
  } 
  //
  // write graphics section
  //
  output << "  <graphics>\n";
  std::string zones = GetZoneSetting(fZoneColumns, fZoneRows);
  output << "    <zones>" << xml.Encode(zones) << "</zones>\n";
  output << "    <refresh>" << fUpdateTimerSec << "</refresh>\n";
  output << "  </graphics>\n";
  //
  // write canvas section
  //
  TIter next = gROOT->GetListOfCanvases();
  while (1)
  {
    TCanvas *canvas = (TCanvas*)next();
    if (!canvas)
      break;

    output << "  <canvas>\n";
    output << "    <name>" << canvas->GetName() << "</name>\n";
    output << "    <size>" << canvas->GetWindowTopX() << " " << canvas->GetWindowTopY() << " " << canvas->GetWindowWidth() << " " << canvas->GetWindowHeight() << "</size>\n";
    output << "    <zones>" << fCanvasColumns[canvas] << "x" << fCanvasRows[canvas] << "</zones>\n";

    if (gPadContents[canvas].size() > 0)
      {
	SaveFilePadContents(xml, output, "    ", gPadContents[canvas]);
      }
    else
      {
	for (int i=1; i<1000; i++)
	  {
	    TVirtualPad *pad = canvas->cd(i);
	    //printf("canvas %p subpad %d is %p\n", canvas, i, pad);
	    if (!pad || pad->GetNumber()!=i)
	      break;
	    
	    output << "    <pad>\n";
	    output << "      <number>" << pad->GetNumber() << "</number>\n";
	    SaveFilePadContents(xml, output, "      ", gPadContents[pad]);
	    output << "    </pad>\n";
	  }
      }

    output << "  </canvas>\n";
  }
  //
  // write group section
  //
  std::size_t nGroups = fGroupFolders.size();
  for( std::size_t i=0; i<nGroups; ++i )
  {
    output << "  <group>\n";
    TGListTreeItem *groupFolder = fGroupFolders[i];
    output << "    <name>" << xml.Encode(groupFolder->GetText()) << "</name>\n";

    ItemVec items;
    GetFolderItems(&items, groupFolder);

    for (unsigned int j=0; j<items.size(); j++)
      {
	TGListTreeItem* item = items[j];
	output << "    <histogram>\n";
	output << "      <name>" << xml.Encode(item->GetText()) << "</name>\n";
	output << "      <source>" << xml.Encode(PathToString(gItemSources[item])) << "</source>\n";
	output << "    </histogram>\n";
      }
    output << "  </group>\n";
  }
  output << "</roody>\n";
}

Roody::~Roody()
{
  fUpdateTimer->Stop();
  TQObject::Disconnect(fUpdateTimer);
  TQObject::Disconnect(this);
  //Timers
  //delete fUpdateTimer;
  //
  //Cleanup();
  //
}

void Roody::LayoutGUI()
{
  LayoutMenuBar(); // Lay out the MenuBar
  //
  // Lay out the rest of the GUI
  //
  //TGLayoutHints* frameLayout1 = new TGLayoutHints( kLHintsTop|kLHintsLeft, 5, 5, 5, 5 );
  TGLayoutHints* frameLayout2 = new TGLayoutHints( kLHintsTop|kLHintsExpandX|kLHintsExpandY, 5, 5, 5, 5 );
  TGLayoutHints* frameLayout3 = new TGLayoutHints( kLHintsTop|kLHintsExpandX, 0, 0, 0, 0 );
  //
  TGVerticalFrame* verticalFrame = new TGVerticalFrame( this, 10, 10 );
  //
  TGHorizontalFrame* horizontalFrame = new TGHorizontalFrame( verticalFrame, 10, 10 );
  //
  fReopenButton = new TGTextButton( horizontalFrame, "Reopen", M_REOPEN_BUTTON );
  fReopenButton->Associate( this );
  fReopenButton->SetToolTipText( "click to reopen data sources", 250 );
  horizontalFrame->AddFrame( fReopenButton, new TGLayoutHints(kLHintsCenterX,2,2,2,2) );
  fRefreshButton = new TGTextButton( horizontalFrame, "Refresh NOW", M_REFRESH_BUTTON );
  fRefreshButton->Associate( this );
  fRefreshButton->SetToolTipText( "click to refresh online histograms once right now", 250 );
  horizontalFrame->AddFrame( fRefreshButton, new TGLayoutHints(kLHintsCenterX,2,2,2,2) );
  verticalFrame->AddFrame( horizontalFrame, frameLayout3 );
  //
  TGCanvas *canvas = new TGCanvas( verticalFrame, 340, 300 );
  fContents = new MTGListTree(canvas, kHorizontalFrame );
  fContents->AddItem( 0, "/" );
  TGListTreeItem* top = fContents->GetFirstItem();
  fTreeItemGroups = fContents->AddItem(top, "Groups" );
  fTreeItemFiles  = fContents->AddItem(top, "Offline" );
  fTreeItemOnline = fContents->AddItem(top, "Online" );
  fContents->OpenItem(top);
  fContents->Associate( this );
  verticalFrame->AddFrame(canvas, frameLayout2 );
  //
  AddFrame( verticalFrame, frameLayout2 );
  //
  TGLayoutHints* statusBarLayout = new TGLayoutHints( kLHintsBottom|kLHintsLeft|kLHintsExpandX,0, 0, 2, 0 );
  fStatusBar = new TGStatusBar( this, 50, 10, kHorizontalFrame );
  AddFrame( fStatusBar, statusBarLayout );
}

class OptStatMenu: public TGPopupMenu
{
public:
  OptStatMenu(const TGWindow* w)
    : TGPopupMenu(w)
  {
    AddEntry("Name",       'n');
    AddEntry("Entries",    'e');
    AddEntry("Mean", 'm');
    AddEntry("Mean with error", 'M');
    AddEntry("RMS",  'r');
    AddEntry("RMS with error",  'R');
    AddEntry("Underflows", 'u');
    AddEntry("Overflows",  'o');
    AddEntry("Integral",   'i');
    AddEntry("Skew",       's');
    AddEntry("Skew with error", 'S');
    AddEntry("Kurtosis",   'k');
    AddEntry("Kurtosis with error", 'K');
    //AddEntry( "user", M_ZONES_USER );
    //AddEntry( "&Set zones...", M_ZONES_DIALOG );
    //CheckEntry(M_ZONES_11); // place check mark on the default
    Associate(this);
    GetOptStat();
    SetOptStat();
  }

  Bool_t ProcessMessage( Long_t msg, Long_t parm1, Long_t parm2 )
  {
    //printf("OptStatMenu::ProcessMessage %d (%d, %d), parm1: %d, parm2: %d\n", msg, GET_MSG(msg), GET_SUBMSG(msg), parm1, parm2);

    if (GET_MSG(msg) == kC_COMMAND && GET_SUBMSG(msg) == kCM_MENU)
      {
	//printf("OptStatMenu::ProcessMessage menu message %d (%d, %d), parm1: %d \'%c\', parm2: %d\n", msg, GET_MSG(msg), GET_SUBMSG(msg), parm1, parm1, parm2);

	if (IsEntryChecked(parm1))
	  UnCheckEntry(parm1);
	else
	  CheckEntry(parm1);

	//Modified();

	SetOptStat();
      }
    return true;
  }

  void GetOptStat()
  {
    Int_t s = gStyle->GetOptStat();
    int ss;

    ss = s%10; s/=10; if (ss) CheckEntry('n');
    ss = s%10; s/=10; if (ss) CheckEntry('e');
    ss = s%10; s/=10; if (ss) CheckEntry('m'); if (ss>1) CheckEntry('M');
    ss = s%10; s/=10; if (ss) CheckEntry('r'); if (ss>1) CheckEntry('R');
    ss = s%10; s/=10; if (ss) CheckEntry('u');
    ss = s%10; s/=10; if (ss) CheckEntry('o');
    ss = s%10; s/=10; if (ss) CheckEntry('i');
    ss = s%10; s/=10; if (ss) CheckEntry('s'); if (ss>1) CheckEntry('S');
    ss = s%10; s/=10; if (ss) CheckEntry('k'); if (ss>1) CheckEntry('K');
  }

  void SetOptStat()
  {
    const TList* list = GetListOfEntries();
    TIter next(list);
    std::string s = "";
    while (TGMenuEntry *entry = (TGMenuEntry*)next())                                
      if (IsEntryChecked(entry->GetEntryId()))
	s += (char)entry->GetEntryId();
    gStyle->SetOptStat(s.c_str());
    printf("SetOptStat(\"%s\") got %d\n", s.c_str(), gStyle->GetOptStat());
  }
};

void Roody::LayoutMenuBar()
{
  TGPopupMenu* menuFile = new TGPopupMenu( fClient->GetRoot() );
  //
  menuFile->AddEntry( "&New canvas", M_NEW_CANVAS);
  menuFile->AddEntry( "Redraw current canvas", M_REDRAW_CANVAS);
  menuFile->AddSeparator();
  menuFile->AddEntry( "Open data file...", M_FILE_OPEN );
  menuFile->AddEntry( "Open MIDAS connection...", M_FILE_ONLINE );
  menuFile->AddEntry( "Open ROOT connection...", M_FILE_NetDirectory );
  menuFile->AddSeparator();
  menuFile->AddEntry( "Save session",    M_FILE_SAVE_DEFAULT );
  menuFile->AddEntry( "Save session as...",    M_FILE_SAVE );
  menuFile->AddEntry( "Restore session...", M_FILE_RESTORE );
  menuFile->AddSeparator();
  menuFile->AddEntry( "&Exit", M_FILE_EXIT );
  //
  fMenuRefresh = new TGPopupMenu( fClient->GetRoot() );
  fMenuRefresh->AddEntry( "&Off", M_REFRESH_OFF );
  fMenuRefresh->AddEntry( "&Refresh now", M_REFRESH_NOW );
  fMenuRefresh->AddEntry( "&1 second", M_REFRESH_1SEC );
  fMenuRefresh->AddEntry( "&3 seconds", M_REFRESH_3SEC );
  fMenuRefresh->AddEntry( "&5 seconds", M_REFRESH_5SEC );
  fMenuRefresh->AddEntry( "10 seconds", M_REFRESH_10SEC );
  fMenuRefresh->AddEntry( "&Set refresh period...", M_REFRESH_DIALOG );
  //
  fMenuZones = new TGPopupMenu( fClient->GetRoot() );
  fMenuZones->AddEntry( "1x1", M_ZONES_11 );  
  fMenuZones->AddEntry( "1x2", M_ZONES_12 );  
  fMenuZones->AddEntry( "1x3", M_ZONES_13 );  
  fMenuZones->AddEntry( "2x1", M_ZONES_21 );  
  fMenuZones->AddEntry( "2x2", M_ZONES_22 ); 
  fMenuZones->AddEntry( "3x3", M_ZONES_33 );
  fMenuZones->AddEntry( "4x4", M_ZONES_44 );
  fMenuZones->AddEntry( "user", M_ZONES_USER );
  fMenuZones->AddEntry( "&Set zones...", M_ZONES_DIALOG );
  fMenuZones->CheckEntry(M_ZONES_11); // place check mark on the default
  //
  OptStatMenu* optStatMenu = new OptStatMenu(fClient->GetRoot());
  //
  fMenuPlot = new TGPopupMenu( fClient->GetRoot() );
  fMenuPlot->AddEntry( "New canvas", M_PLOT_NEW );
  fMenuPlot->AddEntry( "Current pad", M_PLOT_REPLACE );
  fMenuPlot->AddEntry( "Overlay on Current pad", M_PLOT_SAME );
  fMenuPlot->AddEntry( "Next pad", M_PLOT_NEXT );
  SetDestination( D_PLOT_NEW );
  //
  TGPopupMenu* menuClear = new TGPopupMenu( fClient->GetRoot() );
  menuClear->AddEntry( "&Reset all", M_RESET_ALL );
  //
  TGPopupMenu* menuHelp = new TGPopupMenu( fClient->GetRoot() );
  //menuHelp->AddEntry( "Contents", M_HELP_CONTENTS );
  menuHelp->AddEntry( "&About", M_HELP_ABOUT );
  //
  menuFile->Associate( this );
  fMenuRefresh->Associate( this );
  fMenuZones->Associate( this );
  fMenuPlot->Associate( this );
  menuClear->Associate( this );
  menuHelp->Associate( this );
  //
  TGLayoutHints* menuBarLayout1 = new TGLayoutHints( kLHintsNormal|kLHintsExpandX, 0, 0, 0, 10 );
  TGLayoutHints* menuBarLayout2 = new TGLayoutHints( kLHintsTop|kLHintsLeft , 5, 0, 0, 0 );
  TGLayoutHints* menuBarLayout3 = new TGLayoutHints( kLHintsTop|kLHintsRight, 0, 5, 0, 0 );

  TGMenuBar *menuBar = new TGMenuBar( this, 1, 1, kRaisedFrame );
  menuBar->AddPopup( "&File", menuFile, menuBarLayout2 );
  menuBar->AddPopup( "&Refresh", fMenuRefresh, menuBarLayout2 );
  menuBar->AddPopup( "&Zones", fMenuZones, menuBarLayout2 );
  menuBar->AddPopup( "OptStat",  optStatMenu, menuBarLayout2 );
  menuBar->AddPopup( "&Plot in", fMenuPlot, menuBarLayout2 );
  menuBar->AddPopup( "&Reset", menuClear, menuBarLayout2 );
  menuBar->AddPopup( "&Help", menuHelp, menuBarLayout3 );
  //
  AddFrame( menuBar, menuBarLayout1 );
}

void Roody::AddSetCut()
{
  TClassMenuItem *menuItem = new TClassMenuItem( TClassMenuItem::kPopupUserFunction,
		                                 Roody::Class(),
						 "Send to Analyzer", "SetCut", this,
						 "TObject*", 0);
  TCutG::Class()->GetMenuList()->AddFirst(menuItem);
}

void Roody::SetCut( TObject *o )
{
  TCutG *cut = static_cast<TCutG*>(o);

  new TGMsgBox( gClient->GetRoot(), this, "Error", "Roody::SetCut() is not implemented, sorry.", kMBIconExclamation, 0, NULL );

#if 0  
  fSock->Send("SetCut");
  fSock->Send(cut->GetName());
  TMessage m (kMESS_OBJECT);
  m<<cut;
  fSock->Send(m);
#endif
}

void Roody::AddAxisLimits()
{
  TClassMenuItem *menuItem = new TClassMenuItem( TClassMenuItem::kPopupUserFunction,
                                                 Roody::Class(),
                                                 "AxisLimits", "AxisLimits", this, "" );
  TH1C::Class()->GetMenuList()->AddFirst(menuItem);
  TH1S::Class()->GetMenuList()->AddFirst(menuItem);
  TH1I::Class()->GetMenuList()->AddFirst(menuItem);
  TH1F::Class()->GetMenuList()->AddFirst(menuItem);
  TH1D::Class()->GetMenuList()->AddFirst(menuItem);
  TFrame::Class()->GetMenuList()->AddFirst(menuItem);
}

void Roody::AxisLimits()
{
  CanvasLimits *limits = fCanvasLimits[gPad->GetCanvas()->GetCanvasID()];
  if (!limits)
    {
      limits = new CanvasLimits();
      fCanvasLimits[gPad->GetCanvas()->GetCanvasID()] = limits;
    }
  new TAxisLimitsDialog( gClient->GetRoot(), NULL, 250, 100, limits, this);
}

TObject* Roody::ApplyCanvasLimits(const CanvasLimits* limits, const ObjectPath* src, TObject *obj)
{
  if (!limits)
    return obj;

  if (obj->InheritsFrom(TH1::Class()))
    {
      TH1* h = (TH1*)obj;

      const char* name = h->GetName();
      
      if (limits->fRebin > 1)
	{
	  char rebinx[256];
	  sprintf(rebinx,"_rebin%d_",limits->fRebin);
	  
	  if (strstr(name,rebinx)!=NULL)
	    {
	      //printf("histogram %s is already rebinned %s (%d)\n",name,rebinx,fRebin);
	    }
	  else
	    {
	      TH1* hh = (TH1*)GetObject(*src);
	      assert(hh!=NULL);
	      
	      name = hh->GetName();
	      char buf[256];
	      sprintf(buf, "%s%s", name, rebinx);
	      h = hh->Rebin(limits->fRebin, buf); // object "hh" is cloned into "h"

	      delete obj;
	    }
	}
      else
	{
	  if (strstr(name,"_rebin")!=NULL)
	    {
	      h = (TH1*)GetObject(*src);
	      assert(h != NULL);
	    }
	}
      
      if (limits->fDoZoomXaxis)
	{
	  int ufirst = h->GetXaxis()->FindBin(limits->fXmin);
	  int ulast  = h->GetXaxis()->FindBin(limits->fXmax);
	  bool rescaleY = (ufirst != h->GetXaxis()->GetFirst()) || (ulast != h->GetXaxis()->GetLast());
	  //printf("set bins %d %d, rescaleY %d\n", ufirst, ulast, rescaleY);
	  h->GetXaxis()->SetRange(ufirst, ulast);
	  //h->GetXaxis()->SetRangeUser(limits->fXmin, limits->fXmax);
	  if (!limits->fDoZoomYaxis)
	    h->GetYaxis()->UnZoom();
	}
      
      if (limits->fDoZoomYaxis)
	{
	  h->SetMinimum(limits->fYmin);
	  h->SetMaximum(limits->fYmax);
	}
      
      if (limits->fForceUnzoomX)
	h->GetXaxis()->UnZoom();
      
      if (limits->fForceUnzoomY)
	h->GetYaxis()->UnZoom();

      return h;
    }
  else
    {
      return obj;
    }
}

static void DrawVec(TVirtualPad *canvas, const PadObjectVec* padvec)
{
  for (unsigned int i=0; i<padvec->size(); i++)
    {
      TVirtualPad* pad = canvas->cd((*padvec)[i].fPadNumber);

      if (!pad)
	{
	  //printf("canvas %p has no pad %d\n", canvas, (*padvec)[i].fPadNumber);
	  pad = canvas;
	}

      TList* xcontents = pad->GetListOfPrimitives();
      int xcount = 0;
      if (xcontents)
	xcount = xcontents->GetSize();

      //printf("Draw %p into canvas %p pad %d %p[%d-%d], opt \'%s\'\n", (*padvec)[i].fObject, canvas, (*padvec)[i].fPadNumber, pad, xcount, gPadContents[pad].size(), (*padvec)[i].fDrawOption.c_str());

      if (xcount == 0)
	gPadContents.erase(pad);

      (*padvec)[i].fObject->Draw((*padvec)[i].fDrawOption.c_str());

      gPadContents[pad].push_back((*padvec)[i]);

      pad->Modified();
    }

  canvas->Modified();
  canvas->Update();
}

void Roody::AddObjectToVec(PadObjectVec* padvec, TVirtualPad* pad, const ObjectPath* src, TObject* obj, const char* drawOpt)
{
  if (!obj)
    {
      fprintf(stderr,"Roody::AddObjectToVec: ERROR: attempt to draw a NULL object for %s!\n", src->toString().c_str());
      return;
    }

  pad->cd();

  TObject* xobj = ApplyCanvasLimits(fCanvasLimits[pad->GetCanvas()->GetCanvasID()], src, obj);

  if (xobj->InheritsFrom(TH1::Class()))
    {
      ((TH1*)xobj)->SetDirectory(0);
    }

  if (!gAllObjects->FindObject(xobj))
    gAllObjects->Add(xobj);

  xobj->SetBit(kCanDelete);
  xobj->SetBit(kMustCleanup);

  if (!drawOpt)
    drawOpt = "";

  //printf("add object %p to pad %p, draw opt \'%s\'\n", obj, pad, drawOpt);

  padvec->push_back(PadObject(pad->GetNumber(), xobj, src, drawOpt));
}

static void CopyAttrs(TObject* dest, TObject* src)
{
  //printf("copy attributes from %p to %p\n", src, dest);
  
  if (src->InheritsFrom(TAttLine::Class()) && dest->InheritsFrom(TAttLine::Class()))
    {
      TAttLine* hold = dynamic_cast<TAttLine*>(src);
      TAttLine* hnew = dynamic_cast<TAttLine*>(dest);
      hnew->SetLineColor(hold->GetLineColor());
      hnew->SetLineStyle(hold->GetLineStyle());
      hnew->SetLineWidth(hold->GetLineWidth());
    }
  
  if (src->InheritsFrom(TAttMarker::Class()) && dest->InheritsFrom(TAttMarker::Class()))
    {
      TAttMarker* hold = dynamic_cast<TAttMarker*>(src);
      TAttMarker* hnew = dynamic_cast<TAttMarker*>(dest);
      hnew->SetMarkerColor(hold->GetMarkerColor());
      hnew->SetMarkerSize(hold->GetMarkerSize());
      hnew->SetMarkerStyle(hold->GetMarkerStyle());
    }
  
  if (src->InheritsFrom(TAttFill::Class()) && dest->InheritsFrom(TAttFill::Class()))
    {
      TAttFill* hold = dynamic_cast<TAttFill*>(src);
      TAttFill* hnew = dynamic_cast<TAttFill*>(dest);
      hnew->SetFillColor(hold->GetFillColor());
      hnew->SetFillStyle(hold->GetFillStyle());
    }

  if (src->InheritsFrom(TH1::Class()) && dest->InheritsFrom(TH1::Class()))
    {
      TH1* hold = (TH1*)src;
      TH1* hnew = (TH1*)dest;
      hnew->GetXaxis()->SetRange(hold->GetXaxis()->GetFirst(),hold->GetXaxis()->GetLast());
    }
}

void Roody::AddPadToVec(PadObjectVec* padvec, TVirtualPad *pad, bool reread)
{
  PadObjectVec* contents = &gPadContents[pad];

  if (!contents || contents->size() <= 0)
    return;

  for (unsigned int i=0; i<contents->size(); i++)
    {
      const PadObject *po = &(*contents)[i];

      TObject* newobject = NULL;

      if (reread)
	newobject = RereadObject(po->fSource);
      else
	newobject = GetObject(po->fSource);

      if (!newobject)
	continue;

      TObject* plotobject = newobject->Clone();

      TObject *oldobject  = po->fObject;
      if (oldobject)
	{
	  // force a crash if oldobject has somehow been deleted
	  if (gAllObjects->FindObject(oldobject))
	    {
	      //printf("maybe crash on destroyed %p\n", oldobject);
	      oldobject->GetName();
	    }
	  else
	    {
	      //printf("maybe crash on destroyed %p ---> nope we know it is destroyed!\n", oldobject);
	      oldobject = NULL;
	    }
	}

      if (oldobject)
	CopyAttrs(plotobject, oldobject);

      AddObjectToVec(padvec, pad, &po->fSource, plotobject, po->fDrawOption.c_str());
    }

  pad->Modified();
}

void Roody::RedrawCanvas(TVirtualPad *canvas, bool reread)
{
  if (!canvas && gPad)
    canvas = gPad->GetCanvas();

  if (!canvas)
    return;

  int savedPad = gPad->GetNumber();

  PadObjectVec padvec;

  if (gPadContents[canvas].size() > 0)
    {
      AddPadToVec(&padvec, canvas, reread);
    }
  else
    {
      for (int i=1; i<1000; i++)
	{
	  TVirtualPad *pad = canvas->cd(i);
	  //printf("canvas %p subpad %d is %p\n", canvas, i, pad);
	  if (!pad || pad->GetNumber()!=i)
	    break;

	  AddPadToVec(&padvec, pad, reread);
	}
    }

  canvas->Clear();

  gPadContents.erase(canvas);
  for (int i=1; i<=fCanvasColumns[canvas]*fCanvasRows[canvas]; i++)
    gPadContents.erase(canvas->cd(i));

  canvas->Divide(fCanvasColumns[canvas], fCanvasRows[canvas]);

  gPadContents.erase(canvas);
  for (int i=1; i<=fCanvasColumns[canvas]*fCanvasRows[canvas]; i++)
    gPadContents.erase(canvas->cd(i));

  DrawVec(canvas, &padvec);

  canvas->cd(savedPad);
  canvas->Update();
}

void Roody::AddZoomOption()
{
  TClassMenuItem *menuItem = new TClassMenuItem( TClassMenuItem::kPopupUserFunction,
                                                 Roody::Class(),
                                                 "ZoomOption", "ZoomOption", this, "" );
  TH1C::Class()->GetMenuList()->AddFirst(menuItem);
  TH1S::Class()->GetMenuList()->AddFirst(menuItem);
  TH1I::Class()->GetMenuList()->AddFirst(menuItem);
  TH1F::Class()->GetMenuList()->AddFirst(menuItem);
  TH1D::Class()->GetMenuList()->AddFirst(menuItem);
  TFrame::Class()->GetMenuList()->AddFirst(menuItem);
}

void Roody::ZoomOption()
{
  // do not allow zoom on the zoom canvas
  //
  if (!gPad || !strcmp(gPad->GetCanvas()->GetName(),"Zoom Canvas"))
    return;
  //
  // find all histograms on the current pad, and store them in a vector
  //

  PadObjectVec* contents = &gPadContents[gPad];

  // make the zoom canvas (which is always 1x1),
  // and then draw the stored histograms on the zoom canvas

  TVirtualPad *padSave = gPad;

  if (fZoomCanvas)
    delete fZoomCanvas;
  fZoomCanvas = new TCanvas("Zoom Canvas", "Zoom Canvas");
  fZoomCanvas->cd();  // gPad should now be set to fZoomCanvas

  fZoomCanvas->Connect( "ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "Roody",
                        this, "SelectPad(Int_t,Int_t,Int_t,TObject*)" );

  PadObjectVec padvec;

  for (unsigned int i=0; i<contents->size(); i++)
    {
      ObjectPath *src = &(*contents)[i].fSource;
      TObject* obj = GetObject(*src);
      if (!obj)
	continue;
      TObject *clone = obj->Clone();
      AddObjectToVec(&padvec, fZoomCanvas, src, clone, (*contents)[i].fDrawOption.c_str());
    }

  DrawVec(fZoomCanvas, &padvec);

  if (padSave)
    padSave->cd();
}

void Roody::AddPeakFind()
{
  TClassMenuItem *menuItem = new TClassMenuItem( TClassMenuItem::kPopupUserFunction,
                                                 Roody::Class(),
                                                 "PeakFind", "PeakFind", this, "" );
  TH1C::Class()->GetMenuList()->AddFirst(menuItem);
  TH1S::Class()->GetMenuList()->AddFirst(menuItem);
  TH1I::Class()->GetMenuList()->AddFirst(menuItem);
  TH1F::Class()->GetMenuList()->AddFirst(menuItem);
  TH1D::Class()->GetMenuList()->AddFirst(menuItem);
  TFrame::Class()->GetMenuList()->AddFirst(menuItem);
}

void Roody::SetDestination(EDrawDestination dest)
{
  if (fDefaultDrawDestination)
    fMenuPlot->UnCheckEntry(fDefaultDrawDestination);
  
  fDefaultDrawDestination = dest;
  fMenuPlot->CheckEntry(fDefaultDrawDestination);
}

std::string Roody::GetZoneSetting(int columns,int rows)
{
  std::stringstream str;
  str << columns << "x" << rows;
  return str.str();
}

void Roody::SetupZones(int columns,int rows)
{
  // set up the pads on the canvas
  // if the canvas already has things drawn on it,
  // redraw them in the new pads
  //
  fZoneColumns = columns;
  fZoneRows = rows;

  if (!gPad)
  {
    MakeNewCanvas(NULL, columns, rows);
    return;
  }

  TVirtualPad *canvas = gPad->GetCanvas()->cd(); // gPad should now be the parent canvas

#if 0
 FIXME: why are we resetting the user choice?!?
  if (fZoneColumns*fZoneRows==1)
    SetDestination(D_PLOT_REPLACE);
  else
    SetDestination(D_PLOT_NEXT);
#endif

  PadObjectVec padvec;

  if (gPadContents[canvas].size() > 0)
    {
      AddPadToVec(&padvec, canvas);
    }
  else
    {
      for (int i=1; i<1000; i++)
	{
	  TVirtualPad *pad = canvas->cd(i);
	  //printf("canvas %p subpad %d is %p\n", canvas, i, pad);
	  if (!pad || pad->GetNumber()!=i)
	    break;

	  if (i > columns*rows)
	    break;

	  AddPadToVec(&padvec, pad);
	}
    }

  gPadContents.erase(canvas);
  for (int i=1; i<=fCanvasColumns[canvas]*fCanvasRows[canvas]; i++)
    gPadContents.erase(canvas->cd(i));

  canvas->Clear();
  canvas->Divide(columns, rows);

  fCanvasColumns[canvas] = columns;
  fCanvasRows[canvas] = rows;

  gPadContents.erase(canvas);
  for (int i=1; i<=fCanvasColumns[canvas]*fCanvasRows[canvas]; i++)
    gPadContents.erase(canvas->cd(i));

  DrawVec(canvas, &padvec);
}

void Roody::SelectPad( Int_t ev, Int_t x, Int_t y, TObject* obj )
{
  // Makes the pad active for all mousebutton clicks
  if( ev==kButton1Down || ev==kButton3Down )
    gPad->GetCanvas()->HandleInput( kButton2Down, x, y );
}

void Roody::DrawItem(TGListTreeItem *item, EDrawDestination dest)
{
  if (dest == D_PLOT_DEFAULT)
    dest = fDefaultDrawDestination;

  if( !gPad || dest==D_PLOT_NEW)
    MakeNewCanvas(NULL, fZoneColumns, fZoneRows);

  if (dest == D_PLOT_NEXT )
    {
      int currentNumber = gPad->GetNumber();
      if (currentNumber > 0) // select next pad
	{
	  gPad->GetCanvas()->cd(currentNumber+1);
	  if (gPad->GetNumber() == currentNumber)
	    gPad->GetCanvas()->cd(1);
	}
    }

  DrawItemOnPad(item, gPad, dest != D_PLOT_SAME);
}

void Roody::DrawItemOnPad(TGListTreeItem *item, TVirtualPad *dest, bool replace)
{
  if (replace)
    dest->Clear();

  ObjectPath src = gItemSources[item];
  TObject* obj = GetObject(src);
  if (!obj)
    return;

  if (strcmp(gObjectClasses[src].c_str(), obj->ClassName()) != 0)
    {
      //printf("update object class for %s: %s->%s!\n", obj->GetName(), gObjectClasses[src].c_str(), obj->ClassName());
      gObjectClasses[src] = obj->ClassName();
      UpdateObjectClass(item, obj);
    }

  PadObjectVec padvec;

  if (0)
  {
  }
  else if( obj->InheritsFrom(TTree::Class()) )
  {
    static_cast<TTree*>(obj)->StartViewer();
  }
  else if( obj->InheritsFrom(TNtuple::Class()) )
  {
    static_cast<TNtuple*>(obj)->StartViewer();
  }
  else if( obj->InheritsFrom(TPad::Class()) )
  {
    static_cast<TPad*>(obj)->Draw();
  }
  else if( obj->InheritsFrom(TH1::Class()) )
  {
    TH1 *clone = (TH1*)obj->Clone();

    if (replace)
      {
	AddObjectToVec(&padvec, dest, &src, clone, NULL);
      }
    else
      {
	int color = 1 + gPadContents[dest].size();
	clone->SetLineColor(color);
	//MemDebug *d = new MemDebug();
	//AddObjectToPad(dest, &src, d, "SAME");
	AddObjectToVec(&padvec, dest, &src, clone, "SAME");
      }
  }
  else if( obj->InheritsFrom(TCutG::Class()) )
  {
    TCutG *c = static_cast<TCutG*>(obj);
    c->Draw();
  }
  else if( obj->InheritsFrom(TGraph::Class()) )
  {
    TGraph *clone = (TGraph*)obj->Clone();

    if (replace)
      AddObjectToVec(&padvec, dest, &src, clone, "AC*");
    else
      AddObjectToVec(&padvec, dest, &src, clone, "C*");
  }
  else
  {
    TObject *clone = obj->Clone();

    if (replace)
      AddObjectToVec(&padvec, dest, &src, clone, NULL);
    else
      AddObjectToVec(&padvec, dest, &src, clone, NULL); // do not know how to overplot unknown objects
  }

  DrawVec(dest->GetCanvas(), &padvec);
}

void Roody::PeakFind()
{
  if( fgPeakFindPanel )
    fgPeakFindPanel->MapRaised();
  else
    fgPeakFindPanel = new TPeakFindPanel(gClient->GetRoot(),0,250,220);
}

void Roody::PopupPlot(int x, int y)
{
  if( fPopupMenu )delete fPopupMenu;
  //
  fPopupMenu = new TGPopupMenu( gClient->GetRoot() );
  fPopupMenu->AddEntry( "Plot on new canvas",     C_PLOT_NEW );
  fPopupMenu->AddEntry( "Plot on current pad",    C_PLOT_REPLACE );
  fPopupMenu->AddEntry( "Overlay on Current pad", C_PLOT_SAME );
  fPopupMenu->AddEntry( "Plot on next pad",       C_PLOT_NEXT );
  fPopupMenu->AddSeparator();
  fPopupMenu->AddEntry( "Add to new group...", C_ADD_TO_GROUP );
  for (unsigned int i=0; i<fGroupFolders.size(); i++)
    {
      std::string s;
      s += "Add to group ";
      s += fGroupFolders[i]->GetText();
      fPopupMenu->AddEntry( s.c_str(), C_ADD_TO_GROUP + 1000 * (i+1));
    }
  fPopupMenu->AddSeparator();
  fPopupMenu->AddEntry( "Reset", C_RESET_OBJECT );
  fPopupMenu->PlaceMenu( x, y, false, true );
  fXSave = x;
  fYSave = y;
  fPopupMenu->Associate( this );
}

void Roody::PopupPlotFolder(int x, int y)
{
  if( fPopupMenu )delete fPopupMenu;
  //
  fPopupMenu = new TGPopupMenu( gClient->GetRoot() );
  fPopupMenu->AddEntry("Plot on new canvas", C_PLOT_FOLDER);
  fPopupMenu->AddSeparator();
  fPopupMenu->AddEntry("Reset", C_RESET_FOLDER);
  fPopupMenu->PlaceMenu( x, y, false, true );
  fXSave = x;
  fYSave = y;
  fPopupMenu->Associate( this );
}

void Roody::PopupPlotSource(int x, int y)
{
  if( fPopupMenu )delete fPopupMenu;
  //
  fPopupMenu = new TGPopupMenu( gClient->GetRoot() );
  fPopupMenu->AddEntry("Plot all on new canvas", C_PLOT_FOLDER);
  fPopupMenu->AddSeparator();
  fPopupMenu->AddEntry("Reset all", C_RESET_FOLDER);
  fPopupMenu->AddSeparator();
  fPopupMenu->AddEntry("Reopen",    C_REOPEN_SOURCE);
  fPopupMenu->AddEntry("Close",     C_CLOSE_SOURCE);
  fPopupMenu->PlaceMenu( x, y, false, true);
  fXSave = x;
  fYSave = y;
  fPopupMenu->Associate(this);
}

void Roody::PopupTopGroup( int x, int y )
{
  if( fPopupMenu )delete fPopupMenu;
  fPopupMenu = new TGPopupMenu( gClient->GetRoot() );
  fPopupMenu->AddEntry( "Make new group", C_NEW_GROUP );
  fPopupMenu->PlaceMenu( x, y, false, true );
  fPopupMenu->Associate( this );
}

void Roody::PopupGroup( int x, int y )
{
  if( fPopupMenu )delete fPopupMenu;
  fPopupMenu = new TGPopupMenu( gClient->GetRoot() );
  fPopupMenu->AddEntry( "Draw group on new canvas", C_DRAW_GROUP );
  fPopupMenu->AddEntry( "Reset group", C_RESET_GROUP );
  fPopupMenu->AddEntry( "Delete group", C_DELETE_GROUP );
  fPopupMenu->PlaceMenu( x, y, false, true );
  fPopupMenu->Associate( this );
}

TGListTreeItem* Roody::PopupNewGroup(int x, int y)
{
  TGListTreeItem* newgroup = NULL;
  TObjString objString;
  new TGTextDialog(gClient->GetRoot(), this, x, y, "Create new group: ", "", &objString);

  const char* name = objString.GetString();

  if (name && strlen(name) > 0 && strcmp(name,"-1") != 0)
    newgroup = MakeNewGroup(name);

  return newgroup;
}

TGListTreeItem* Roody::MakeNewGroup(const char *name)
{
  TGListTreeItem* item = fContents->FindChildByName(fTreeItemGroups, name);
  if (!item)
    {
      item = fContents->AddItem(fTreeItemGroups, name);
      gItemActions[item] = A_GROUP;
      fGroupFolders.push_back(item); // fGroupFolders is a vector of TGListTreeItem*
      fContents->OpenItem(fTreeItemGroups);
      fContents->Resize();
      Layout();
    }
  return item;
}

void Roody::AddToGroup(TGListTreeItem* group, ItemVec* items)
{
  if (group == NULL)
    group = PopupNewGroup(100, 100);

  if (group)
    for (unsigned int i=0; i<items->size(); i++)
      AddHistogramToGroup(group, (*items)[i]->GetText(), (*items)[i]);
}

void Roody::DeleteGroup(TGListTreeItem* groupItem)
{
  ItemVec old = fGroupFolders;
  fGroupFolders.clear(); // std::vector does not have an "erase" method
  for (unsigned int i=0; i<old.size(); i++)
    {
      if (old[i] != groupItem)
	fGroupFolders.push_back(old[i]);
    }
      
  fContents->MDeleteItem(groupItem);
  fContents->Resize();
}

TCanvas* Roody::MakeNewCanvas(const char* title, int columns, int rows, int topx, int topy, int width, int height)
{
  std::string name;

  if (title == NULL)
    {
      std::stringstream ss;
      ss << "canvas" << fCanvasCount++;
      name = ss.str();
    }
  else
    {
      name = title;
    }

  TCanvas* canvas;
  if (width*height==0)
    canvas = new TCanvas(name.c_str(), name.c_str());
  else
    canvas = new TCanvas(name.c_str(), name.c_str(), topx, topy, width, height);

  canvas->cd();

  if (columns*rows==1)
    {
      SetDestination(D_PLOT_REPLACE);
    }
  else
    {
      SetDestination(D_PLOT_NEXT);
      canvas->Divide(columns, rows );
      canvas->cd(columns*rows);
    }

  fCanvasColumns[canvas] = columns;
  fCanvasRows[canvas] = rows;

  canvas->Modified();
  canvas->Update();

  canvas->Connect( "ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "Roody",
                           this, "SelectPad(Int_t,Int_t,Int_t,TObject*)" );
  return canvas;
}

void Roody::DrawItemsOnNewCanvas(const char* title, const ItemVec* items)
{
  int nItems = items->size();
  assert(nItems > 0);

  int columns = (int)sqrt(nItems);
  int rows = nItems/columns;

  while (rows*columns < nItems)
    rows++;

  TCanvas *c = MakeNewCanvas(title, columns, rows);

  for (int i=0; i<nItems; i++)
    DrawItemOnPad((*items)[i], c->cd(i+1), true);

  c->Modified();
  c->Update();
}

void Roody::DrawGroup(TGListTreeItem* groupItem)
{
  ItemVec items;

  GetFolderItems(&items, groupItem);

  if (items.size() < 1)
    {
      new TGMsgBox( gClient->GetRoot(), this, "Error", "group is empty",
		    kMBIconExclamation, 0, NULL );
      return;
    }

  std::string title( "Group canvas: " );
  title += groupItem->GetText();

  DrawItemsOnNewCanvas(title.c_str(), &items);
}

void Roody::SetRefreshRate(int newrefresh)
{
  //printf("SetRefreshRate %d %d\n", newrefresh, fUpdatePause);

  fMenuRefresh->UnCheckEntry(M_REFRESH_OFF);
  fMenuRefresh->UnCheckEntry(M_REFRESH_NOW);
  fMenuRefresh->UnCheckEntry(M_REFRESH_1SEC);
  fMenuRefresh->UnCheckEntry(M_REFRESH_3SEC);
  fMenuRefresh->UnCheckEntry(M_REFRESH_5SEC);
  fMenuRefresh->UnCheckEntry(M_REFRESH_10SEC);

  fUpdateTimerSec = newrefresh;

  switch( fUpdateTimerSec )
    {
    case 0:  fMenuRefresh->CheckEntry( M_REFRESH_OFF );  break;
    case 1:  fMenuRefresh->CheckEntry( M_REFRESH_1SEC ); break;
    case 3:  fMenuRefresh->CheckEntry( M_REFRESH_3SEC ); break;
    case 5:  fMenuRefresh->CheckEntry( M_REFRESH_5SEC ); break;
    case 10: fMenuRefresh->CheckEntry( M_REFRESH_10SEC ); break;
    }

  //fRefreshButton->SetEnabled(kTRUE);

  if( fUpdateTimerSec > 0 )
    {
      if (fUpdatePause)
	{
	  std::stringstream str;
	  str << "  Resume Refresh (" << fUpdateTimerSec << ")  ";
	  fRefreshButton->SetText( str.str().c_str() );
	  fRefreshButton->SetToolTipText( "click to resume refresh of online histograms", 250 );
	}
      else
	{
	  std::stringstream str;
	  str << "  Pause Refresh (" << fUpdateTimerSec << ")  ";
	  fRefreshButton->SetText( str.str().c_str() );
	  fRefreshButton->SetToolTipText( "click to pause refresh of online histograms", 250 );
	}
    }
  else
    {
      fRefreshButton->SetText( "     Refresh NOW      " );
      fRefreshButton->SetToolTipText( "click to refresh online histograms once right now", 250 );
    }

  StartUpdateTimer();
}

void Roody::UncheckAllZones()
{
  fMenuZones->UnCheckEntry(M_ZONES_11);
  fMenuZones->UnCheckEntry(M_ZONES_12);
  fMenuZones->UnCheckEntry(M_ZONES_13);
  fMenuZones->UnCheckEntry(M_ZONES_21);
  fMenuZones->UnCheckEntry(M_ZONES_22);
  fMenuZones->UnCheckEntry(M_ZONES_33);
  fMenuZones->UnCheckEntry(M_ZONES_44);
  fMenuZones->UnCheckEntry(M_ZONES_USER);
}

Bool_t Roody::ProcessMessage( Long_t msg, Long_t parm1, Long_t parm2 )
{
  // ProcessMessage overrides the virtual function in TGCompositeFrame
  // (which is the base class for TGMainFrame)
  //
  // to see the predefined message types look at 
  // $ROOTSYS/include/WidgetMessageTypes.h
  //

  //printf("ProcessMessage %d (%d, %d), parm1: %d, parm2: %d\n", msg, GET_MSG(msg), GET_SUBMSG(msg), parm1, parm2);

  switch (GET_MSG(msg))
  {
    case kC_LISTTREE: // an event in TreeView
    {
      switch (GET_SUBMSG(msg))
      {
        default:
          break;
        case kCT_ITEMCLICK:  // single mouse click
        {
          switch (parm1)
          {
            default:
              break;

            case kButton1: // left mouse button click
              break;

            case kButton2:
              break;

            case kButton3: // right mouse button click
            {
              // see http://root.cern.ch/root/htmldoc/TGListTree.html#TGListTree:description
              int x = parm2&0xFFFF;
              int y = (parm2&0xFFFF0000)>>16;
              //

	      ItemVec items;
              fContents->GetSelectedItems(items);

	      for (unsigned int i=0; i<items.size(); i++)
		{
		  if (items[i] == fTreeItemGroups)
		    {
		      PopupTopGroup( x, y );
		      return true;
		    }

		  Action act = gItemActions[items[i]];
		  //printf("Clicked on selected %s, action %d, parm1: %d\n", items[i]->GetText(), act, parm1);

		  switch (act)
		    {
		    default:
		      break;

		    case A_ITEM:
		      PopupPlot(x, y);
		      return true;

		    case A_FOLDER:
		      PopupPlotFolder(x, y);
		      return true;

		    case A_SOURCE:
		      PopupPlotSource(x, y);
		      return true;

		    case A_GROUP:
		      PopupGroup(x, y);
		      return true;
		    }
		}
              break;
            }
          }
          break;
        }
        case kCT_ITEMDBLCLICK:  // double mouse click
        {
	  TGListTreeItem* selected = fContents->GetSelected();
	  if (gItemActions[selected] == A_ITEM)
	    {
	      ItemVec items;
	      items.push_back(selected);
	      PlotItems(&items, D_PLOT_DEFAULT);
	      return true;
	    }
	  break;
        }
      }
      break;
    }
    case kC_COMMAND:  // 
    {
      switch (GET_SUBMSG(msg))
      {
        case kCM_BUTTON:  // a button was pressed
        {
          switch (parm1)
          {
	  case M_REOPEN_BUTTON:
	    printf("Reopen all data sources!\n");
	    gObjectCache.clear();
	    for (DataSourcesMap::iterator it=gDataSources.begin(); it!=gDataSources.end(); it++)
	      it->second->Reopen();
	    break;
	  case M_REFRESH_BUTTON:
	    if (fUpdateTimerSec == 0)
	      {
		fUpdatePause = false;
		RefreshAll();
	      }
	    else if (fUpdatePause)
	      {
		fUpdatePause = false;
		RefreshAll();
		SetRefreshRate(fUpdateTimerSec);
	      }
	    else
	      {
		fUpdatePause = true;
		SetRefreshRate(fUpdateTimerSec);
		//StartUpdateTimer();
	      }
	    break;
	  default:
	    break;
	  }
	  break;
	}
        case kCM_MENUSELECT:
        {
          break;
        }
        case kCM_MENU: // menu item event
        {
          switch (parm1)
          {
            case M_NEW_CANVAS:
	      MakeNewCanvas(NULL, fZoneColumns, fZoneRows);
              break;

            case M_REDRAW_CANVAS:
	      RedrawCanvas();
              break;

            case M_FILE_OPEN:
              fStatusBar->SetText("Opening ...");
              OpenFileDialog();
              fStatusBar->SetText(" ");
              break;

            case M_FILE_ONLINE:
	      ConnectServer(NULL);
              break;

            case M_FILE_NetDirectory:
	      ConnectNetDirectory(NULL);
              break;

	    case M_FILE_SAVE_DEFAULT:
	      SaveFile("default.xml");
	      break;

	    case M_FILE_SAVE:
              OpenSaveDialog();
              break;

	    case M_FILE_RESTORE:
              OpenRestoreDialog();
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;

            case M_HELP_ABOUT:
              new TGMsgBox( gClient->GetRoot(), this, "About Roody",
              "Roody is an application based solely on CERN-ROOT for histogram display.\nIt is meant for display of .root files and .hbook files.\nRunning this application in conjunction with either the Midas or ROME analyzer\nwill allow you to visualize online data retrieved through a socket port.\n\nThis is Roody svn revision $Id: Roody.cxx 236 2010-10-19 18:41:15Z olchansk $", kMBIconQuestion, kMBOk, NULL );
              break;

            case M_HELP_CONTENTS:
            {
              fStatusBar->SetText("HELP CONTENTS -- Not yet available");
              std::cout << "HELP CONTENTS -- Not yet available" << std::endl;
              break;
            }
            case M_REFRESH_OFF:
            {
	      SetRefreshRate(0); 
              break;
            }
            case M_REFRESH_NOW:
            {
	      RefreshAll();
              break;
            }
            case M_REFRESH_1SEC:
            {
	      SetRefreshRate(1); 
              break;
            }
            case M_REFRESH_3SEC:
            {
              SetRefreshRate(3); 
              break;
            }
            case M_REFRESH_5SEC:
            {
	      SetRefreshRate(5); 
              break;
            }
            case M_REFRESH_10SEC:
            {
	      SetRefreshRate(10); 
              break;
            }
            case M_REFRESH_DIALOG:
            {
              OpenRefreshDialog();
              break;
            }
            case M_ZONES_11:
            {
	      UncheckAllZones();
              fMenuZones->CheckEntry(M_ZONES_11);
              SetupZones( 1, 1 );
              break;
            }
            case M_ZONES_12:
            {
	      UncheckAllZones();
              fMenuZones->CheckEntry(M_ZONES_12);
              SetupZones( 1, 2 );
              break;
            }
            case M_ZONES_13:
            {
	      UncheckAllZones();
              fMenuZones->CheckEntry(M_ZONES_13);
              SetupZones( 1, 3 );
              break;
            }
            case M_ZONES_21:
            {
	      UncheckAllZones();
              fMenuZones->CheckEntry(M_ZONES_21);
              SetupZones( 2, 1 );
              break;
            }
            case M_ZONES_22:
            {
	      UncheckAllZones();
              fMenuZones->CheckEntry(M_ZONES_22);
              SetupZones( 2, 2 );
              break;
            }
            case M_ZONES_33:
            {
	      UncheckAllZones();
              fMenuZones->CheckEntry(M_ZONES_33);
              SetupZones( 3, 3 );
              break;
            }
            case M_ZONES_44:
            {
	      UncheckAllZones();
              fMenuZones->CheckEntry(M_ZONES_44);
              SetupZones( 4, 4 );
              break;
            }
            case M_ZONES_USER:
            {
              std::string name = fMenuZones->GetEntry(M_ZONES_USER)->GetName();
              if (name.find("user") == 0)
		OpenZoneDialog();
              else
		{
		  std::size_t x = name.find("x");
		  int cols = atoi(name.substr(0,x).c_str());
		  int rows = atoi(name.substr(x+1,name.size()-x-1).c_str());
		  SetupZones( cols, rows );
		}
              SetupZonesMenu();
              break;
            }
            case M_ZONES_DIALOG:
            {
              OpenZoneDialog();
              SetupZonesMenu();
              break;
            }
            case M_PLOT_NEW:
              SetDestination(D_PLOT_NEW);
              break;
            case M_PLOT_SAME:
              SetDestination(D_PLOT_SAME);
              break;
            case M_PLOT_REPLACE:
              SetDestination(D_PLOT_REPLACE);
              break;
            case M_PLOT_NEXT:
              SetDestination(D_PLOT_NEXT);
              break;

            case C_PLOT_FOLDER:
	      {
		ItemVec items;
		GetFolderItems(&items, fContents->GetSelected());
		DrawItemsOnNewCanvas(NULL, &items);
		break;
	      }
	    case C_PLOT_NEW:
	      {
		ItemVec items;
		fContents->GetSelectedItems(items);
		DrawItemsOnNewCanvas(NULL, &items);
		break;
	      }
            case C_PLOT_REPLACE:
	      {
		ItemVec items;
		fContents->GetSelectedItems(items);
		PlotItems(&items, D_PLOT_REPLACE);
		break;
	      }
            case C_PLOT_SAME:
	      {
		ItemVec items;
		fContents->GetSelectedItems(items);
		PlotItems(&items, D_PLOT_SAME);
		break;
	      }
            case C_PLOT_NEXT:
	      {
		ItemVec items;
		fContents->GetSelectedItems(items);
		PlotItems(&items, D_PLOT_NEXT);
		break;
	      }
            case C_CLOSE_SOURCE:
              CloseSource(fContents->GetSelected());
              break;
            case C_REOPEN_SOURCE:
              ReopenSource(fContents->GetSelected());
              break;

            case C_NEW_GROUP:
              PopupNewGroup(100, 100);
              break;
            case C_DRAW_GROUP:
              DrawGroup(fContents->GetSelected());
              break;
            case C_DELETE_GROUP:
              DeleteGroup(fContents->GetSelected());
              break;
            case C_ADD_TO_GROUP:
	      {
		ItemVec items;
		fContents->GetSelectedItems(items);
		AddToGroup(NULL, &items);
		break;
	      }
            case C_RESET_GROUP:
	      {
		ItemVec items;
		GetFolderItems(&items, fContents->GetSelected());
		ResetMultiple(&items);
		break;
	      }
            case C_RESET_OBJECT:
	      {
		ItemVec items;
		fContents->GetSelectedItems(items);
		ResetMultiple(&items);
		break;
	      }
            case C_RESET_FOLDER:
	      {
		ItemVec items;
		GetFolderItems(&items, fContents->GetSelected());
		ResetMultiple(&items);
		break;
	      }
            case M_RESET_ALL:
              ResetAll();
              break;

            default:
	      if (parm1%1000 == C_ADD_TO_GROUP)
		{
		  unsigned int igroup = (parm1-C_ADD_TO_GROUP)/1000 - 1;

		  if (igroup>=0 && igroup<fGroupFolders.size())
		    {
		      ItemVec items;
		      fContents->GetSelectedItems(items);
		      AddToGroup(fGroupFolders[igroup], &items);
		      return true;
		    }
		}

	      new TGMsgBox( gClient->GetRoot(), this, "Error", "Unimplemented menu command", kMBIconExclamation, 0, NULL );
              break;
          }
          break;
        }
        default:
          break;
      }
      break;
    }
  }
  return kTRUE;
}

void Roody::AddHistogramToGroup(TGListTreeItem *groupItem, const char* name, TGListTreeItem *origItem )
{
  TGListTreeItem *newitem = fContents->AddItem(groupItem, name);
  gItemActions[newitem] = A_ITEM;
  fContents->OpenItem(groupItem);
  fContents->Resize();
  Layout();
  gItemSources[newitem] = gItemSources[origItem];
}

void Roody::ReopenSource(TGListTreeItem* item)
{
  const char* name = item->GetText();
  DataSourceBase *d = gDataSources[name];
  if (d)
    {
      std::string str;
      str += "Reopening ";
      str += name;
      fStatusBar->SetText(str.c_str());
      gObjectCache.clear();
      d->Reopen();
      AddDataSource(d, fContents, item->GetParent(), true);
      str  = "Connected to ";
      str += name;
      fStatusBar->SetText(str.c_str());
    }
}

void Roody::CloseSource(TGListTreeItem* item)
{
  const char* name = item->GetText();
  DataSourceBase *d = gDataSources[name];
  if (d)
    {
      std::string str;
      str += "Closing ";
      str += name;
      fStatusBar->SetText(str.c_str());
      d->Close();
      delete d;
      gDataSources[name] = NULL;
    }

  fContents->MDeleteItem(item);
  fContents->Resize();
  Layout();
}

void Roody::GetFolderItems(ItemVec* items, TGListTreeItem *folder)
{
  // fill "items" with all items in folder "folder", including subfolders

  for (TGListTreeItem *item = folder->GetFirstChild(); item != NULL; item = item->GetNextSibling())
    {
      if (gItemActions[item] == A_ITEM) // is it a sub-folder?
	items->push_back(item);
      else
	GetFolderItems(items, item);
    }
}

void Roody::ResetAll()
{
  for (unsigned int i=0; i<fOnlineFiles.size(); i++)
    {
      DataSourceBase* d = gDataSources[fOnlineFiles[i]];
      if (d)
	d->ResetAll();
    }

  for (unsigned int i=0; i<fRootFiles.size(); i++)
    {
      DataSourceBase* d = gDataSources[fRootFiles[i]];
      if (d)
	d->ResetAll();
    }

  RefreshAll();
}

void Roody::ResetMultiple(const ItemVec* items)
{
  for (unsigned int i=0; i<items->size(); i++)
    ResetItem((*items)[i]);
}

void Roody::ResetItem(TGListTreeItem* item)
{
  ObjectPath src = gItemSources[item];
  if (src.size() < 1)
    return;

  DataSourceBase* d = gDataSources[src.front()];
  if (d)
    d->ResetObject(src);
}

void Roody::PlotItems(const ItemVec* items, EDrawDestination dest)
{
  for (unsigned int i=0; i<items->size(); i++)
    DrawItem((*items)[i], dest);
  Layout();
}

void Roody::CloseWindow()
{
  fUpdateTimer->Stop();
  TQObject::Disconnect(fUpdateTimer);
  TQObject::Disconnect(this);
  gApplication->Terminate(0);
}

bool Roody::OpenFile(const char* filename)
{
  if (strstr(filename,".root"))
    return OpenRootFile(filename);
#ifdef HAVE_HBOOK
  else
    return OpenHbookFile(filename);
#endif
  return false;
}


void Roody::OpenFileDialog()
{
  TGFileInfo fileinfo;
  fileinfo.fFilename = 0;
#ifdef HAVE_HBOOK
  char const *fileTypes[] = {"Root files","*.root","Hbook files","*.hbook","Any","*",0,0};
#else
  char const *fileTypes[] = {"Root files","*.root","Any","*",0,0};
#endif
  fileinfo.fFileTypes = fileTypes;

  new TGFileDialog( fClient->GetRoot(), this, kFDOpen, &fileinfo );

  if (fileinfo.fFilename)
    OpenFile(fileinfo.fFilename);
  else
    fStatusBar->SetText("NO FILE SELECTED");
}

void Roody::AddDataSource(DataSourceBase* d, MTGListTree* tree, TGListTreeItem* branch, bool reopen)
{
  gDataSources[d->GetName()] = d;
  std::string s = d->toString();
  //printf("Data source: [%s]\n", s.c_str());

  ObjectList list = d->GetList();

  bool first = true;
  int n = list.size();
  for (int i=0; i<n; i++)
    {
      ObjectPath p = list[i];
      int np = p.size();
      TGListTreeItem *xitem = branch;
      for (int j=0; j<np-1; j++)
	{
	  TGListTreeItem *item = tree->FindChildByName(xitem, p[j].c_str());

	  if (!item)
	    {
	      item = tree->AddItem(xitem,p[j].c_str());
	      if (j==0)
		gItemActions[item] = A_SOURCE;
	      else
		gItemActions[item] = A_FOLDER;
	    }

	  if (first && !reopen)
	    tree->OpenItem(xitem);

	  xitem = item;
	}

      if (first && !reopen)
	tree->OpenItem(xitem);

      first = false;

      std::string name = p.back();
      TGListTreeItem* iii = tree->FindChildByName(xitem, name.c_str());
      //printf("Looking for %s, %p\n", name.c_str(), iii);
      if (!iii)
	iii = tree->AddItem(xitem, name.c_str(), gClient->GetPicture("diamond.xpm"), gClient->GetPicture("diamond.xpm"));

      gItemActions[iii]   = A_ITEM;
      gItemSources[iii]   = p;
    }

  tree->Resize();
  Layout();
}

bool Roody::OpenRootFile( char const *filename )
{
  if( fContents->FindChildByName(fTreeItemFiles,filename) )
  {
#if 0
    std::string s("Error in loading file: \"");
    s += std::string(filename) + "\" is already loaded";
    new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
#endif
    return true;
  }

#if 0
  TFile *f = new TFile( filename, "READ" );
  if( !f )
  {
    new TGMsgBox( gClient->GetRoot(), this, "Error", "Error in loading file: could not allocate TFile object",
                  kMBIconExclamation, 0, NULL );
    return true;
  }
  if( f->IsZombie() )
  {
    std::string s("Error in loading file: \"");
    s += std::string(filename) + "\" is not a root file or the file doesn't exist";
    new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
    f->Close();
    delete f;
    return true;
  }

  DataSourceTDirectory* d = new DataSourceTDirectory(f);
#endif
  DataSourceTFile* d = new DataSourceTFile(filename);

  AddDataSource(d, fContents, fTreeItemFiles);

  return false;
}

bool Roody::OpenNetDirectory(char const *dest)
{
#ifdef HAVE_NETDIRECTORY
  if( fContents->FindChildByName(fTreeItemFiles, dest))
    {
#if 0
      std::string s("Error in loading file: \"");
      s += std::string(dest) + "\" is already loaded";
      new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
#endif
      return true;
    }

  TNetDirectory *f = new TNetDirectory(dest);

  if( f->IsZombie() )
    {
      std::string s("Error in loading file: \"");
      s += std::string(dest) + "\" is not a root file or the file doesn't exist";
      new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
      f->Close();
      delete f;
      return true;
    }

  fRootFiles.push_back(dest);

  DataSourceTDirectory* d = new DataSourceNetDirectory(f);

  AddDataSource(d, fContents, fTreeItemFiles);

  return false;
#else
  std::string s("No support for TNetDirectory!");
  new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
  return true;
#endif
}

#ifdef HAVE_HBOOK
bool Roody::OpenHbookFile( char const *filename )
{
  THbookFile *f = new THbookFile( filename );
  if( !f )
  {
    std::string s("Error in loading file: \"");
    s += std::string(filename) + "\" could not create THbookFile object";
    new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
    return true;
  }
  if( !f->IsOpen() )
  {
    std::string s("Error in loading file: \"");
    s += std::string(filename) + "\" is not an hbook file or the file doesn't exist";
    new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
    delete f;
    return true;
  }
  if( fContents->FindChildByName(fTreeItemFiles,filename) )
  {
    std::string s("Error in loading file: \"");
    s += std::string(filename) + "\" is already loaded";
    new TGMsgBox( gClient->GetRoot(), this, "Error", s.c_str(), kMBIconExclamation, 0, NULL );
    delete f;
    return true;
  }

#if 0
  fContents->AddItem( fTreeItemFiles,
                                filename,(void*)f,
                                gClient->GetPicture("rootdb_t.xpm"),
                                gClient->GetPicture("rootdb_t.xpm") );
  fTreeItemCurrent = fContents->FindChildByName( fTreeItemFiles, filename );
  fContents->OpenItem( fTreeItemFiles );
  DisplayHBook(f, fTreeItemCurrent );
  fContents->OpenItem( fTreeItemCurrent );
  fContents->Resize();
  Layout();
#endif

#if 0
  DataSourceTDirectory* d = new DataSourceTHbookFile(f);

  AddDataSource(d, fContents, fTreeItemFiles);
#endif

  return false;
}

#if 0
class MyTHbookKey: public THbookKey
{
public:
  int GetID() const { return fID; };
};

void Roody::DisplayHBook( THbookFile *file, TGListTreeItem *item )
{
  TIter next( file->GetListOfKeys() );
  MyTHbookKey *key;
  while( (key=static_cast<MyTHbookKey*>(next())) )
  {
    TObject *obj = file->Get(key->GetID());
    //if (obj)
    //  DisplayObject( obj, item, "" );
    Layout();
  }
  fFileOnlineCanvas->Layout();
}
#endif
#endif

std::string Roody::GetRunNumber( char const *filename )
{
  int end = strlen( filename );
  while( --end >= 0 )
  {
    if( filename[end] <= '9' && filename[end] >= '0' )break;
  }
  int begin = end;
  while( --begin >= 0 )
  {
    if( filename[begin] < '0' || filename[begin] > '9' )break;
  }
  ++begin;
  int len = end-begin+1;
  return std::string(TString( filename+begin,len).Data());
}

void Roody::UpdateObjectClass(TGListTreeItem *item, TObject* obj)
{
  if (!item)
    return;

  if (!obj)
    return;

  std::string className = obj->ClassName();
  std::string name      = obj->GetName();
  std::string title     = obj->GetTitle();
  std::string itemDescription = name;
  //std::string runNumber = rn;
  //std::string file = fCurrentFile->GetName();

  if( className.find("TH1") != className.npos )
  {
#if 0
    if( title.find("Run#") == title.npos &&                   // Run# not in title and
        runNumber.find_first_not_of(" ") != runNumber.npos )  // runNumber not blank
    {
      std::string s( title );
      s += "    Run# ";
      s += runNumber;
      static_cast<TH1*>(obj)->SetTitle( s.c_str() );
    }
#endif
    if( name != title )
    {
      itemDescription += " ";
      itemDescription += title;
    }
    //item->Rename(itemDescription.c_str());
    item->SetPictures(gClient->GetPicture("h1_t.xpm"), gClient->GetPicture("h1_t.xpm"));
  }
#if 0
  else if( className.find("TH2") != className.npos )
  {
    if( title.find("Run#") == title.npos &&                   // Run# not in title and
        runNumber.find_first_not_of(" ") != runNumber.npos )  // runNumber not blank
    {
      std::string s( title );
      s += "    Run# ";
      s += runNumber;
      static_cast<TH2*>(obj)->SetTitle( s.c_str() );
    }
    if( name != title )
    {
      itemDescription += " ";
      itemDescription += title;
    }
    fContents->AddItem( item, itemDescription.c_str(),
                                  (void*)obj,
                                  gClient->GetPicture("h2_t.xpm"),
                                  gClient->GetPicture("h2_t.xpm") );
  }
  else if( className.find("TH3") != className.npos )
  {
    if( title.find("Run#") == title.npos &&                   // Run# not in title and
        runNumber.find_first_not_of(" ") != runNumber.npos )  // runNumber not blank
    {
      std::string s( title );
      s += "    Run# ";
      s += runNumber;
      static_cast<TH3*>(obj)->SetTitle( s.c_str() );
    }
    if( name != title )
    {
      itemDescription += " ";
      itemDescription += title;
    }
    fContents->AddItem( item, itemDescription.c_str(),
                                  (void*)obj,
                                  gClient->GetPicture("h3_t.xpm"),
                                  gClient->GetPicture("h3_t.xpm") );
  }
  else if( className.find("Tree") != className.npos )
    fContents->AddItem( item, name.c_str(),
                                  (void*)obj,
                                  gClient->GetPicture("tree_t.xpm"),
                                  gClient->GetPicture("tree_t.xpm") );
  else if( className.find("Ntuple") != className.npos )
    fContents->AddItem( item, name.c_str(),
                                  (void*)obj,
                                  gClient->GetPicture("ntuple_t.xpm"),
                                  gClient->GetPicture("ntuple_t.xpm") );
  else if( className.find("Profile") != className.npos )
  {
     if( className.find("2D") != className.npos )
       fContents->AddItem( item, name.c_str(),
                                     (void*)obj,
                                     gClient->GetPicture("h2_t.xpm"),
                                     gClient->GetPicture("h2_t.xpm") );
     else
       fContents->AddItem( item, name.c_str(),
                                     (void*)obj,
                                     gClient->GetPicture("profile_t.xpm"),
                                     gClient->GetPicture("profile_t.xpm") );
  }
  else if( className.find("TCanvas") != className.npos )
  {
    fContents->AddItem( item, name.c_str(),
                                  (void*)obj,
                                  gClient->GetPicture("h1_t.xpm"),
                                  gClient->GetPicture("h1_t.xpm") );
  }
  else if( className.find("TCutG") != className.npos )
  {
    fContents->AddItem( item, name.c_str(),
                                  (void*)obj,
                                  gClient->GetPicture("cut_t.xpm"),
                                  gClient->GetPicture("cut_t.xpm") );
  }
#endif
  else if (obj->InheritsFrom(TGraph::Class()))
  {
    item->SetPictures(gClient->GetPicture("h2_t.xpm"), gClient->GetPicture("h2_t.xpm"));
  }
  fContents->Resize();
}

void Roody::ConnectNetDirectory(const char*ss, bool displayMsgBox)
{
  if (ss == NULL)
    {
      static TObjString ostr;
      static std::string istr = "";
      new TGTextDialog( gClient->GetRoot(), this, 100, 100, "&Host name:", istr.c_str(), &ostr);
      ss = ostr.GetString();
      if (strcmp(ss,"-1") == 0)
	return;
      istr = ss;
    }

  std::string server = ss;
  if (strstr(ss, ":") == NULL)
    server += ":9091";

  OpenNetDirectory(server.c_str());
}

void Roody::ConnectServer(const char*ss, bool displayMsgBox)
{
  if (ss == NULL)
    {
      static TObjString ostr;
      static std::string istr = "";
      new TGTextDialog( gClient->GetRoot(), this, 100, 100, "&Host name:", istr.c_str(), &ostr);
      ss = ostr.GetString();
      if (strcmp(ss,"-1") == 0)
	return;
      istr = ss;
    }

  std::string server = ss;
  if (strstr(ss, ":") == NULL)
    server += ":9090";

  std::vector<std::string> s = split(':', server);

  std::string hostname = s[0];
  int port = atoi(s[1].c_str());
  if (port == 0)
    {
      std::string error;
      error += "Invalid port number in \'";
      error += server;
      error += "\'";
      new TGMsgBox(gClient->GetRoot(), this, "Error", error.c_str(), kMBIconExclamation, 0, NULL );
      return;
    }

  if (fContents->FindChildByName(fTreeItemOnline,server.c_str()))
    {
      std::string error;
      error += "Already connected to ";
      error += server;
      fStatusBar->SetText(error.c_str());
      return;
    }

  TSocket *sock = new TSocket(hostname.c_str(), port);
  if (!sock || !sock->IsValid())
    {
      if (sock)
	delete sock;
      sock = 0;

      if( displayMsgBox )
	{
	  std::string error;
	  error += "Cannot connect to Midas server \'";
	  error += server;
	  error += "\'";
	  fStatusBar->SetText(error.c_str());
	  new TGMsgBox( gClient->GetRoot(), this, "Error", error.c_str(), kMBIconExclamation, 0, NULL );
	}
      
      return;
    }

  DataSourceBase* d = new DataSourceTNetFolder(sock);
  fOnlineFiles.push_back(d->GetName());

  std::string msg = "Connected to ";
  msg += server;
  fStatusBar->SetText(msg.c_str());
  AddDataSource(d, fContents, fTreeItemOnline);
}

void Roody::StartUpdateTimer()
{
  //printf("StartUpdateTimer %d %d\n", fUpdateTimerSec, fUpdatePause);

  if (fUpdateTimerSec > 0 && !fUpdatePause)
  {
    fUpdateTimer->SetTime(fUpdateTimerSec*1000); // changes timer length and restarts it
    fUpdateTimer->Start();
  }
  else
  {
    fUpdateTimer->Stop();
  }
}

void Roody::RefreshAll()
{
  //printf("RefreshAll!\n");

  fUpdateTimer->Stop(); // so multiple updates do not occur at once

  TIter next = gROOT->GetListOfCanvases();
  while(1)
    {
      TCanvas *canvas = (TCanvas*)next();
      if (!canvas)
	break;

      RedrawCanvas(canvas, true);
    }

  StartUpdateTimer();
}

void Roody::OpenRefreshDialog()
{
  TObjString ostr;
  new TGTextDialog( gClient->GetRoot(), this, 100, 100, "Refresh time in seconds: ", "", &ostr);
  const char* str = ostr.GetString();
  if (strcmp(str,"-1")==0)
    fStatusBar->SetText("Refresh not changed");
  else
    SetRefreshRate(atoi(str));
}

void Roody::OpenZoneDialog()
{
  TObjString ostr;
  new TGTextDialog( gClient->GetRoot(), this, 100, 100,
                    "Zone Setting: (columns x rows, e.g. 2x2): ", "", &ostr);
  const char* str = ostr.GetString();
  if (strcmp(str,"-1")==0)
    {
      fStatusBar->SetText( "Zone settings not changed");
      return;
    }
  std::vector<std::string> sss = split('x', str);
  std::vector<int> iii = toInt(sss);

  int columns = iii[0];
  int rows    = iii[1];

  if (columns == 0)
    {
      columns = 1;
      std::string err;
      err += "Invalid number of columns in ";
      err += str;
      new TGMsgBox( gClient->GetRoot(), this, "Error", err.c_str(), kMBIconExclamation, 0, NULL );
    }

  if (rows == 0)
    {
      rows = 1;
      std::string err;
      err += "Invalid number of rows in ";
      err += str;
      new TGMsgBox( gClient->GetRoot(), this, "Error", err.c_str(), kMBIconExclamation, 0, NULL );
    }

  SetupZones( columns, rows );
  SetupZonesMenu();
}

void Roody::OpenSaveDialog()
{
  TGFileInfo fileinfo;
  fileinfo.fFilename = 0;
  char const *fileTypes[] = {"Save file","*.xml","Any file","*",0,0};
  fileinfo.fFileTypes = fileTypes;
  new TGFileDialog( fClient->GetRoot(), this, kFDOpen, &fileinfo);
  TString savefileName = fileinfo.fFilename;
  if( savefileName.Contains("-1") )
  {
    fStatusBar->SetText( "NO FILE SELECTED" );
  }
  else
  {
    if (!savefileName.Contains(".xml"))
      savefileName += ".xml";
    SaveFile(savefileName.Data());
  }
}

void Roody::OpenRestoreDialog()
{
  TGFileInfo fileinfo;
  fileinfo.fFilename = 0;
  char const *fileTypes[] = {"Save file","*.xml","Any file","*",0,0};
  fileinfo.fFileTypes = fileTypes;
  new TGFileDialog( fClient->GetRoot(), this, kFDOpen, &fileinfo );
  if( fileinfo.fFilename )
  {
    RestoreFile( fileinfo.fFilename );
  }
  else
  {
    fStatusBar->SetText( "NO FILE SELECTED" );
  }
}

// end of code
