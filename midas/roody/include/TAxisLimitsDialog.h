/*********************************************************************

  Name:         TAxisLimitsDialog.h
  Created by:   Joe Chuma
  $Id: TAxisLimitsDialog.h 215 2006-12-04 23:19:10Z olchansk $

*********************************************************************/

#ifndef TAxisLimitsDialog_H
#define TAxisLimitsDialog_H

#include <sstream>
#include <iostream>

#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGLayout.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TSocket.h>
#include <TMessage.h>
#include <TGMsgBox.h>
#include <TApplication.h>
#include <TROOT.h>
#include <TGListBox.h>
#include <TString.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TGMenu.h>
#include <TGTab.h>
#include <TGLabel.h>
#include <TGTextEntry.h>
#include <Rtypes.h>

#ifndef ROOT_TGFrame
#include <TGFrame.h>
#endif

#include "CanvasLimits.h"
#include "Roody.h"

class TAxisLimitsDialog : public TGMainFrame
{
protected:
  TGCompositeFrame *fF2, *fF3;          // sub frames
  TGCompositeFrame *fFX; // sub frames
  TGCompositeFrame *fFY; // sub frames
  TGCompositeFrame *fFR; // sub frames
  TGButton         *fOkButton;
  TGButton         *fCancelButton;
  TGButton         *fUnzoomXButton;
  TGButton         *fUnzoomYButton;
  TGButton         *fApplyButton;
  TGLayoutHints    *fL1, *fL2, *fL3, *fL4, *fL5;

  CanvasLimits*    fLimits;
  Roody*           fRoody;
  CanvasLimits     fSavedLimits;

  TGCheckButton*   fZoomX;
  TGTextEntry*     fEditXmin;
  TGTextEntry*     fEditXmax;
  TGCheckButton*   fZoomY;
  TGTextEntry*     fEditYmin;
  TGTextEntry*     fEditYmax;
  TGTextEntry*     fEditRebin;
  
public:
  TAxisLimitsDialog( TGWindow const *p, TGWindow const *main, UInt_t w, UInt_t h,
		     CanvasLimits* limits,
		     Roody* roody);
  ~TAxisLimitsDialog();

  void ReadValues();
  void CloseWindow();
  Bool_t ProcessMessage( Long_t, Long_t, Long_t );
  
  ClassDef(TAxisLimitsDialog,0)
};

#endif
