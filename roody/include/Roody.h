//
// Roody.h
//
// $Id: Roody.h 236 2010-10-19 18:41:15Z olchansk $
//

#ifndef Roody_H
#define Roody_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <assert.h>

class THbookFile;
class TPeakFindPanel;
class TGListTreeItem;
class MTGListTree;
class RoodyXML;
class TPad;
class TGPopupMenu;
class TGStatusBar;
class TGTextButton;
class TCanvas;
class TVirtualPad;

typedef struct mxml_struct *PMXML_NODE;
class CanvasLimits;

#include "TGFrame.h"
#include "DataSourceBase.h"

struct PadObject
{
  int          fPadNumber;
  TObject*     fObject;
  ObjectPath   fSource;
  std::string  fDrawOption;

  PadObject() { assert(!"PadObject::PadObject() constructor should not be called directly!"); };
  PadObject(int padnum, TObject*obj, const ObjectPath* src, const char* drawOption)
  {
    fPadNumber  = padnum;
    fObject     = obj;
    fSource     = *src;
    fDrawOption = drawOption;
  };
  ~PadObject()
  {
    fObject = NULL;
  };
};

struct PadObjectVec : std::vector<PadObject>
{
};

//#include "RQ_OBJECT.h"
//
// Only required if a class is constructed which doesn't inherit from
// a class which has the line RQ_OBJECT(class name)
//
class Roody : public TGMainFrame
{
  //  RQ_OBJECT("Roody")

public:
  Roody(const TGWindow* p = NULL,UInt_t w = 800,UInt_t h = 800);
  virtual ~Roody();

  void RestoreFile(const char* xmlfilename);
  bool OpenFile(const char* filename);
  void ConnectServer(const char* server, bool complain = true);
  void ConnectNetDirectory(const char* server, bool complain = true);

  TObject* GetObject(const ObjectPath& src);
  TObject* RereadObject(const ObjectPath& src);

  Bool_t ProcessMessage( Long_t, Long_t, Long_t );
  void CloseWindow();

  void PopupPlot(int, int); // menu for each normal item
  void PopupPlotFolder(int, int); // menu for each folder
  void PopupPlotSource(int, int); // menu for each source
  void PopupTopGroup( int, int ); // top level group menu
  void PopupGroup( int, int ); // menu for each group
  TGListTreeItem* PopupNewGroup(int x, int y); // make new group

  void PeakFind();
  void SelectPad( Int_t, Int_t, Int_t, TObject* );
  void AxisLimits();
  void SetCut( TObject * );
  void ZoomOption();

  void RefreshAll();

  void RedrawCanvas(TVirtualPad* pad = NULL, bool reread = false);
  void AddObjectToVec(PadObjectVec* padvec, TVirtualPad* pad, const ObjectPath* src, TObject* obj, const char* drawOpt);
  void AddPadToVec(PadObjectVec* padvec, TVirtualPad* pad, bool reread = false);

  ClassDef(Roody,0); // ROOT implementation of ONLINE/OFFLINE GUI

protected:
  enum ECommandMenuEntry
  {
      M_NEW_CANVAS,
      M_REDRAW_CANVAS,

      M_FILE_OPEN,
      M_FILE_ONLINE,
      M_FILE_NetDirectory,
      M_FILE_SAVE_DEFAULT,
      M_FILE_SAVE,
      M_FILE_RESTORE,
      M_FILE_EXIT,

      M_REFRESH_OFF,
      M_REFRESH_NOW,
      M_REFRESH_1SEC,
      M_REFRESH_3SEC,
      M_REFRESH_5SEC,
      M_REFRESH_10SEC,
      M_REFRESH_DIALOG,

      M_REOPEN_BUTTON,
      M_REFRESH_BUTTON,

      M_ZONES_11,
      M_ZONES_12,
      M_ZONES_13,
      M_ZONES_21,
      M_ZONES_22,
      M_ZONES_33,
      M_ZONES_44,
      M_ZONES_USER,
      M_ZONES_DIALOG,

      M_PLOT_NEW,
      M_PLOT_SAME,
      M_PLOT_REPLACE,
      M_PLOT_NEXT,

      C_PLOT_FOLDER,
      C_RESET_FOLDER,

      C_PLOT_NEW,
      C_PLOT_SAME,
      C_PLOT_REPLACE,
      C_PLOT_NEXT,
      C_RESET_OBJECT,

      C_REOPEN_SOURCE,
      C_CLOSE_SOURCE,

      C_NEW_GROUP,
      C_DELETE_GROUP,
      C_DRAW_GROUP,
      C_ADD_TO_GROUP,
      C_RESET_GROUP,

      M_RESET_ALL,

      M_HELP_ABOUT,
      M_HELP_CONTENTS,
      
      M_CURRENT_PAD,
      M_NEXT_PAD,
      M_NEW_PAD
  };

  enum EDrawDestination
  {
      D_PLOT_DEFAULT = -1,
      D_PLOT_NEW     = M_PLOT_NEW,
      D_PLOT_SAME    = M_PLOT_SAME,
      D_PLOT_REPLACE = M_PLOT_REPLACE,
      D_PLOT_NEXT    = M_PLOT_NEXT
  };

  typedef std::vector<TGListTreeItem*> ItemVec;
  
  MTGListTree    *fContents;
  TGListTreeItem *fTreeItemOnline;
  TGListTreeItem *fTreeItemFiles;
  TGListTreeItem *fTreeItemGroups;

private:
  TGPopupMenu *fMenuRefresh;
  TGPopupMenu *fMenuZones;
  TGPopupMenu *fMenuPlot;
  TGPopupMenu *fPopupMenu;
  TGStatusBar *fStatusBar;

  static TPeakFindPanel *fgPeakFindPanel;

  EDrawDestination fDefaultDrawDestination; // where to Draw() things, value is one of D_PLOT_XXX

  int  fUpdateTimerSec;  // refresh time in sec, 0 means no refresh
  bool fUpdatePause;

  int fZoneColumns;   // default zone setting: columns
  int fZoneRows;      // default zone setting: rows

  std::map<TVirtualPad*,int> fCanvasColumns;
  std::map<TVirtualPad*,int> fCanvasRows;

  std::vector<std::string> fRootFiles;
  std::vector<std::string> fHbookFiles;
  std::vector<std::string> fOnlineFiles;

  TTimer *fUpdateTimer; // used for updating online histograms

  ItemVec fGroupFolders;

  TGPopupMenu *fAddToGroupPopup;
  int fXSave, fYSave;

  Int_t fCanvasCount;

  std::map<int,CanvasLimits*> fCanvasLimits;

  TCanvas *fZoomCanvas;

  TGTextButton *fReopenButton;
  TGTextButton *fRefreshButton;

  void LayoutGUI();
  void LayoutMenuBar();
  void OpenFileDialog();
  bool OpenRootFile(const char* filename);
  bool OpenHbookFile(const char* filename);

  bool OpenNetDirectory(const char* dest);

  void AddDataSource(DataSourceBase* source, MTGListTree* tree, TGListTreeItem* branch, bool reopen=false);

  void AddPeakFind();
  void AddAxisLimits();
  void AddSetCut();
  void AddZoomOption();

  void SetupZones(int columns, int rows);
  void SetDestination(EDrawDestination newdest);
  void PlotItems(const ItemVec* items, EDrawDestination dest);
  void UpdateObjectClass(TGListTreeItem *item, TObject* obj);
  void DrawItem(TGListTreeItem *item, EDrawDestination dest = D_PLOT_DEFAULT);
  void DrawItemOnPad(TGListTreeItem *item, TVirtualPad* dest, bool replace);
  void DrawItemsOnNewCanvas(const char* title, const ItemVec* items);
  std::string GetZoneSetting(int columns,int rows);

  void StartUpdateTimer();

  void SetRefreshRate(int newrefresh);

  void OpenRefreshDialog();
  void OpenZoneDialog();
  void UncheckAllZones();

  void CloseSource(TGListTreeItem* item);
  void ReopenSource(TGListTreeItem* item);

  void GetFolderItems(ItemVec* items, TGListTreeItem* item);

  TGListTreeItem* MakeNewGroup(const char* groupname);
  void AddToGroup(TGListTreeItem* groupItem, ItemVec* items);
  void DeleteGroup(TGListTreeItem* groupItem);
  void DrawGroup(TGListTreeItem* groupItem);
  void AddHistogramToGroup(TGListTreeItem *groupItem, const char* name, TGListTreeItem *origItem );

  void OpenRestoreDialog();
  void OpenSaveDialog();
  void SaveFile( char const * );
  void SaveFilePadContents(RoodyXML& xml, std::ofstream& output, std::string space, const PadObjectVec& contents);

  std::string GetRunNumber( char const * );

  TCanvas* MakeNewCanvas(const char* title, int columns, int rows, int topx=0, int topy=0, int width=0, int height=0);

  void SetZonesUser();
  void SetupZonesMenu();

  void ResetAll();
  void ResetMultiple(const ItemVec* items);
  void ResetItem(TGListTreeItem* item);

  TObject* ApplyCanvasLimits(const CanvasLimits* limits, const ObjectPath* src, TObject *h);

  void RestoreObjects(RoodyXML *xml, TVirtualPad *pad, PMXML_NODE parent);
};

#endif
