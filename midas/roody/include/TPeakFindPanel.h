/*********************************************************************
 
  Name:         TPeakFindPanel.cxx
  Created by:   Greg King
  $Id: TPeakFindPanel.h 169 2006-09-26 22:34:44Z olchansk $
 
  Contents:     New Peak Find Panel

*********************************************************************/

#ifndef TPEAKFINDPANEL_H
#define TPEAKFINDPANEL_H

#include <iostream>

#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif

#include "TROOT.h"
#include "TVirtualX.h"
#include "TGResourcePool.h"
#include "TGClient.h"
#include "TGIcon.h"
#include "TGLabel.h"
#include "TGButton.h"
#include "TGTextEntry.h"
#include "TGNumberEntry.h"
#include "TGMsgBox.h"
#include "TGMenu.h"
#include "TGCanvas.h"
#include "TGComboBox.h"
#include "TGSlider.h"
#include "TGDoubleSlider.h"
#include "RQ_OBJECT.h"
#include "TGLayout.h"
#include "TH1.h"
#include "TF1.h"
#include "TFitter.h"
#include "TVirtualFitter.h"
#include "TCanvas.h"
#include "TRandom.h"
#include "TGTab.h"
#include "TList.h"
#include "TGStatusBar.h"
#include "TSpectrum.h"
#include "TPolyMarker.h"

class TPeakFindPanel : public TGTransientFrame
{
private:
  TGNumberEntry *fNumberOfPeaksNE, *fResolutionNE, *fSigmaNE, *fThresholdNE;
  TGCheckButton *fAttemptFitCB, *fAddPolyFitCB, *fDoUnzoomCB;
  TGTextButton *fFindTB, *fDefaultsTB, *fCloseTB, *fClearPolyTB;
  TGVerticalFrame *fMainFrame;
  TGHorizontalFrame *fNumberOfPeaksHF, *fResolutionHF, *fSigmaHF, *fThresholdHF;
  TGHorizontalFrame *fFindDefaultsHF, *fCloseClearPolyHF;
  TSpectrum *fSpectrum;

  Int_t fNfound;
  Float_t *fXpeaks; //! [fNfound]
  Float_t *fYpeaks; //!

  void FindPeak();
  void DrawPeak();
  void LayoutGUI();
  void LayoutMenuBar();
  void GetXYBounds( Double_t &, Double_t & );
  void InsertionSort( Float_t *, Float_t *, Int_t, Int_t );
  void Clear();
  void SetDefaults();
  TH1* GetHistogram();

protected:
  enum ECommandIdentifiers
  {
      M_FILE_EXIT,
      M_ATTEMPT_FIT_PEAKS_BUTTON,
      M_ADD_POLY_BUTTON,
      M_DO_UNZOOM_BUTTON,
      M_FIND_BUTTON,
      M_DEFAULT_BUTTON,
      M_CLOSE_BUTTON,
      M_CLEAR_BUTTON,
      M_NUMBER_ENTRY_PEAKS,
      M_NUMBER_ENTRY_RESOLUTION,
      M_NUMBER_ENTRY_SIGMA,
      M_NUMBER_ENTRY_THRESHOLD
  };

  Double_t fSigma, fThreshold;
  Float_t fResolution;
  Int_t fNumberOfPeaks;
  TF1 *fFit;
  Bool_t fDoFit, fAddPoly, fDoUnzoom;

public:
  TPeakFindPanel( TGWindow const *, TGWindow const *, UInt_t, UInt_t );
  virtual ~TPeakFindPanel();

  virtual void CloseWindow();
  virtual Bool_t ProcessMessage( Long_t, Long_t, Long_t );

  ClassDef( TPeakFindPanel, 1 );
};

#endif
