/*********************************************************************
 
  Name:         TPeakFindPanel.cxx
  Created by:   Greg King
 
  Contents:     New Peak Find Panel

  $Id: TPeakFindPanel.cxx 234 2010-04-21 00:15:43Z olchansk $

*********************************************************************/

#include "TPeakFindPanel.h"
#include <TMath.h>

#include <memory>

Int_t gNp = 30;

Double_t PeakFcn( Double_t *x, Double_t *par )
{
  Double_t result = par[0] + par[1]*x[0];
  for( std::size_t p=0; static_cast<Int_t>(p)<gNp; ++p )
  {
    std::size_t i = 3*p+2;
    Double_t norm  = par[i++];
    Double_t mean  = par[i++];
    Double_t sigma = par[i];
    result += norm*TMath::Gaus( x[0], mean, sigma );
  }
  return result;
}

TPeakFindPanel::TPeakFindPanel( TGWindow const *p, TGWindow const *main, UInt_t w, UInt_t h )
  : TGTransientFrame( p, main, w, h ) // ctor
{
  fSpectrum = 0;
  fNfound = 0;
  fSigma = 1.25;
  fThreshold = 0.05;  // peaks with amplitude less than fThreshold*highest_peak are discarded
  //
  fResolution = 3.0;  // fResolution of 3. higher value -> smaller allowed distance between peaks
  //                     if set to 1. -> 3 fSigma between peaks
  //
  fNumberOfPeaks = 10; // the number of peaks to be documented or fit
  //
  // max of 2*npeaks and 100 is the number of peaks that TSpectrum will search for
  //
  fAddPoly = kFALSE;
  fDoFit = kFALSE;
  fDoUnzoom = kFALSE;
  //
  //LayoutMenuBar();
  LayoutGUI();
  //
}

TPeakFindPanel::~TPeakFindPanel() // dtor
{
//  assert(!"TPeakFindPanel::~TPeakFindPanel: This object should never be deleted!");
  //if( fSpectrum )delete fSpectrum;
  //Cleanup(); // cleans up widgets
}

void TPeakFindPanel::CloseWindow()
{
  TGTransientFrame::UnmapWindow();
}

void TPeakFindPanel::LayoutMenuBar()
{
  TGPopupMenu *menuFile = new TGPopupMenu( fClient->GetRoot() );
  menuFile->AddEntry( "&Close", M_FILE_EXIT );
  menuFile->Associate( this );
  //
  TGMenuBar *menuBar = new TGMenuBar( this, 1, 1, kRaisedFrame );
  menuBar->AddPopup( "&File", menuFile, new TGLayoutHints(kLHintsTop|kLHintsLeft,5,0,0,0) );
  //
  AddFrame( menuBar, new TGLayoutHints(kLHintsNormal|kLHintsExpandX,0,0,0,10) );
}

void TPeakFindPanel::LayoutGUI()
{
  fMainFrame = new TGVerticalFrame( this, 250, 240 );
  //
  fAttemptFitCB = new TGCheckButton( fMainFrame, "Attempt to fit peaks", M_ATTEMPT_FIT_PEAKS_BUTTON );
  fAttemptFitCB->Associate( this );
  fAttemptFitCB->SetToolTipText( "Attempt to fit peaks with a multiply gaussian function" );
  fMainFrame->AddFrame( fAttemptFitCB,
                        new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,3,3,3) );
  //
  fAddPolyFitCB = new TGCheckButton( fMainFrame, "Add fit to function list", M_ADD_POLY_BUTTON );
  fAddPolyFitCB->Associate( this );
  fAddPolyFitCB->SetToolTipText("Add the fit to the list of functions");
  fMainFrame->AddFrame( fAddPolyFitCB,
                        new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,3,3,3) );
  //
  fDoUnzoomCB = new TGCheckButton( fMainFrame, "Display after fit unzoomed", M_DO_UNZOOM_BUTTON );
  fDoUnzoomCB->Associate( this );
  fDoUnzoomCB->SetToolTipText("Toggle Unzoom for display");
  fMainFrame->AddFrame( fDoUnzoomCB,
                        new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,3,3,3) );
  //
  fNumberOfPeaksHF = new TGHorizontalFrame( fMainFrame, 10, 10 );
  fNumberOfPeaksNE = new TGNumberEntry( fNumberOfPeaksHF, 10, 3, M_NUMBER_ENTRY_PEAKS, 
                                        (TGNumberFormat::EStyle) 0, 
                                        (TGNumberFormat::EAttribute) 2,
                                        (TGNumberFormat::ELimit) 3, -0.1, 500.1 );
  TGLabel *numberOfPeaksLabel = new TGLabel( fNumberOfPeaksHF, "Number of peaks" );
  numberOfPeaksLabel->SetTextJustify( 1 );
  fNumberOfPeaksNE->Associate(this);
  fNumberOfPeaksHF->AddFrame( numberOfPeaksLabel,
                              new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fNumberOfPeaksHF->AddFrame( fNumberOfPeaksNE,
                              new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fMainFrame->AddFrame( fNumberOfPeaksHF,
                        new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,3,3,3) );
  //
  fResolutionHF = new TGHorizontalFrame( fMainFrame, 12, 12 );
  fResolutionNE = new TGNumberEntry( fResolutionHF, 3.0, 3,
                                     M_NUMBER_ENTRY_RESOLUTION,
                                     (TGNumberFormat::EStyle) 2, 
                                     (TGNumberFormat::EAttribute) 2,
                                     (TGNumberFormat::ELimit) 3, 0.01, 12.0 );
  TGLabel *resolutionLabel = new TGLabel( fResolutionHF, "Resolution" );
  resolutionLabel->SetTextJustify(1);
  fResolutionNE->Associate(this);
  fResolutionHF->AddFrame( resolutionLabel,
                           new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fResolutionHF->AddFrame( fResolutionNE,
                           new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fMainFrame->AddFrame( fResolutionHF,
                        new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,3,3,3) );
  //
  fSigmaHF = new TGHorizontalFrame( fMainFrame, 12, 12 );
  fSigmaNE = new TGNumberEntry( fSigmaHF, 1.25, 3, M_NUMBER_ENTRY_SIGMA,
                                (TGNumberFormat::EStyle) 2, 
                                (TGNumberFormat::EAttribute) 2,
                                (TGNumberFormat::ELimit) 3, 1., 30.0 );
  TGLabel *sigmaLabel = new TGLabel( fSigmaHF, "Sigma" );
  sigmaLabel->SetTextJustify(1);
  fSigmaNE->Associate(this);
  fSigmaHF->AddFrame( sigmaLabel,
                      new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fSigmaHF->AddFrame( fSigmaNE,
                      new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fMainFrame->AddFrame( fSigmaHF,
                        new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,3,3,3) );
  //
  fThresholdHF = new TGHorizontalFrame( fMainFrame, 12, 12 );
  fThresholdNE = new TGNumberEntry( fThresholdHF, 0.05, 3,
                                    M_NUMBER_ENTRY_THRESHOLD,
                                    (TGNumberFormat::EStyle) 4, 
                                    (TGNumberFormat::EAttribute) 2,
                                    (TGNumberFormat::ELimit) 3, 0.00001, 0.95 );
  TGLabel *thresholdLabel = new TGLabel( fThresholdHF, "Threshold" );
  thresholdLabel->SetTextJustify( 1 );
  fThresholdNE->Associate(this);
  fThresholdHF->AddFrame( thresholdLabel,
                          new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fThresholdHF->AddFrame( fThresholdNE,
                          new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fMainFrame->AddFrame( fThresholdHF,
                        new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,3,3,3) );
  //
  //  static const TGNumberFormat::EStyle kNESInteger
  //  static const TGNumberFormat::EStyle kNESRealOne
  //  static const TGNumberFormat::EStyle kNESRealTwo
  //  static const TGNumberFormat::EStyle kNESRealThree
  //  static const TGNumberFormat::EStyle kNESRealFour
  //  static const TGNumberFormat::EStyle kNESReal
  //  static const TGNumberFormat::EStyle kNESDegree
  //  static const TGNumberFormat::EStyle kNESMinSec
  //  static const TGNumberFormat::EStyle kNESHourMin
  //  static const TGNumberFormat::EStyle kNESHourMinSec
  //  static const TGNumberFormat::EStyle kNESDayMYear
  //  static const TGNumberFormat::EStyle kNESMDayYear
  //  static const TGNumberFormat::EStyle kNESHex
  //  static const TGNumberFormat::EAttribute kNEAAnyNumber
  //  static const TGNumberFormat::EAttribute kNEANonNegative
  //  static const TGNumberFormat::EAttribute kNEAPositive
  //  static const TGNumberFormat::ELimit kNELNoLimits
  //  static const TGNumberFormat::ELimit kNELLimitMin
  //  static const TGNumberFormat::ELimit kNELLimitMax
  //  static const TGNumberFormat::ELimit kNELLimitMinMax
  //  static const TGNumberFormat::EStepSize kNSSSmall
  //  static const TGNumberFormat::EStepSize kNSSMedium
  //  static const TGNumberFormat::EStepSize kNSSLarge
  //  static const TGNumberFormat::EStepSize kNSSHuge
  //
  fFindDefaultsHF = new TGHorizontalFrame( fMainFrame, 12, 12 );
  fFindTB = new TGTextButton( fFindDefaultsHF, "Find", M_FIND_BUTTON );
  fFindTB->Associate( this );
  fFindTB->SetToolTipText( "Find the peaks" );
  fFindDefaultsHF->AddFrame( fFindTB,
                             new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fDefaultsTB = new TGTextButton( fFindDefaultsHF, "Defaults", M_DEFAULT_BUTTON );
  fDefaultsTB->Associate( this );
  fDefaultsTB->SetToolTipText( "Reset peak finder panel defaults" );
  fFindDefaultsHF->AddFrame( fDefaultsTB,
                             new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fMainFrame->AddFrame( fFindDefaultsHF,
                        new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,3,3,3) );
  //
  fCloseClearPolyHF = new TGHorizontalFrame( fMainFrame, 12, 12 );
  fCloseTB = new TGTextButton( fCloseClearPolyHF, "Close", M_CLOSE_BUTTON );
  fCloseTB->Associate( this );
  fCloseTB->SetToolTipText( "Cancel action and close fit panel" );
  fCloseClearPolyHF->AddFrame( fCloseTB,
                               new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fClearPolyTB = new TGTextButton( fCloseClearPolyHF, "Clear", M_CLEAR_BUTTON );
  fClearPolyTB->Associate( this );
  fClearPolyTB->SetToolTipText( "Remove polymarkers" );
  fCloseClearPolyHF->AddFrame( fClearPolyTB,
                               new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,0,0,0) );
  fMainFrame->AddFrame( fCloseClearPolyHF,
                        new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,0,3,3,3) );
  //
  AddFrame( fMainFrame,
            new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX|kLHintsExpandY,12,3,3,3) );
  //
  SetWindowName( "Peak Finder" );
  //
  Resize( 250, 240 );
  MapSubwindows();
  Layout();
  MapWindow();
  SetDefaults();
}

TH1* TPeakFindPanel::GetHistogram()
{
  if (!gPad)
    return 0;

  TList *list = gPad->GetListOfPrimitives();
  TIter next(list);
  while(1)
    {
      TObject* object = next();
      if (!object) return 0;

      if (object->InheritsFrom(TH1F::Class()) || object->InheritsFrom(TH1D::Class()))
	return static_cast<TH1*>(object);
    }
}

Bool_t TPeakFindPanel::ProcessMessage( Long_t msg, Long_t parm1, Long_t parm2 )
{
  /*
    M_FILE_EXIT,
    M_ATTEMPT_FIT_PEAKS_BUTTON,
    M_ADD_POLY_BUTTON,
    M_FIND_BUTTON,
    M_DEFAULT_BUTTON,
    M_CLOSE_BUTTON,
    M_CLEAR_BUTTON,
    M_NUMBER_ENTRY_PEAKS,
    M_NUMBER_ENTRY_RESOLUTION,
    M_NUMBER_ENTRY_SIGMA,
    M_NUMBER_ENTRY_THRESHOLD
  */
  switch (GET_MSG(msg))
    {
    case kC_COMMAND:
      switch (GET_SUBMSG(msg))
      {
      case kCM_CHECKBUTTON:
        switch (parm1)
          {
          case M_ATTEMPT_FIT_PEAKS_BUTTON:
            fAttemptFitCB->GetState()==kButtonUp ? fDoFit=kFALSE : fDoFit=kTRUE;
            break;
          case M_ADD_POLY_BUTTON:
            fAddPolyFitCB->GetState()==kButtonUp ? fAddPoly=kFALSE : fAddPoly=kTRUE;
            break;
          case M_DO_UNZOOM_BUTTON:
            fDoUnzoomCB->GetState()==kButtonUp ? fDoUnzoom=kFALSE : fDoUnzoom=kTRUE;
            break;
          default:
            break;
          }
        break;
      case kCM_BUTTON:
        switch (parm1)
          {
          case M_FIND_BUTTON:
            GetHistogram();
            FindPeak();
            if( fDoFit )DrawPeak();
            break;
          case M_DEFAULT_BUTTON:
            SetDefaults();
            break;
          case M_CLOSE_BUTTON:
            Clear();
            CloseWindow();
            break;
          case M_CLEAR_BUTTON:
            Clear();
            break;
          default:
            break;
          }
      case kCM_MENU:
        switch (parm1)
          {
          case M_FILE_EXIT:
            CloseWindow();
            break;
          default:
            break;
          }
        break;
      default:
        break;
      }
      break;
    case kC_TEXTENTRY:
      switch (GET_SUBMSG(msg))
        {
        case kTE_TEXTCHANGED:
          switch (parm1)
            {
            case M_NUMBER_ENTRY_PEAKS:
              fNumberOfPeaks = static_cast<Int_t>(fNumberOfPeaksNE->GetNumber());
              break;
            case M_NUMBER_ENTRY_RESOLUTION:
              fResolution  = fResolutionNE->GetNumber();
              break;
            case M_NUMBER_ENTRY_SIGMA:
              fSigma = fSigmaNE->GetNumber();
              break;
            case M_NUMBER_ENTRY_THRESHOLD:
              fThreshold = fThresholdNE->GetNumber();
              break;
            }
          break;
        default:
          break;
        }
    default:
      break;
    }
  return kTRUE;
}

void TPeakFindPanel::Clear()
{
  TH1* h = GetHistogram();
  if (!h) return;

  TList *primitives = h->GetListOfFunctions();
  TIter next( primitives );
  while(1)
    {
      TObject* obj = next();
      if (!obj) break;

      if (obj->InheritsFrom(TPolyMarker::Class()) ||
          obj->InheritsFrom(TF1::Class()))
        delete primitives->Remove(obj);
    }

  h->Draw();
  gPad->Modified();
  gPad->Update();
}

void TPeakFindPanel::DrawPeak()
{
  if( fNumberOfPeaks < fNfound )
  {
    InsertionSort( fXpeaks, fYpeaks, fNfound, 1 );
    // sorts all the peaks from largest to smallest
    InsertionSort( fYpeaks, fXpeaks, fNumberOfPeaks, -1 );
    // sorts the fNumberOfPeaks biggest peaks from smallest x value
    // to largest x value
  }
  // will use TVirtualFitter ... hopefully will find out more about
  // the TSpectrum method
  std::cout << "Found " << fNumberOfPeaks << " or " << fNfound
            << " useful peaks to fit" << std::endl;
  std::cout << "Now fitting: Be patient" << std::endl;
  //
  double xMin, xMax;
  GetXYBounds( xMin, xMax );
  //
  TF1 *fline = new TF1( "fline", "pol1", xMin, xMax );
  TH1 *h = GetHistogram();
  if (h)
    h->Fit( "fline", "qn" );
  delete fline;
  //
  Double_t *par = new Double_t[3*fNumberOfPeaks+2];
  //
  //par = new Double_t[3*fNumberOfPeaks+2];
  //par[0] = fline->GetParameter(0);
  //par[1] = fline->GetParameter(1);
  par[0] = 0;
  par[1] = 0;
  //
  Int_t nPeaks;
  for( nPeaks=0; (nPeaks<fNumberOfPeaks) && (nPeaks<fNfound); ++nPeaks )
  {
    par[3*nPeaks+2] = fYpeaks[nPeaks];
    par[3*nPeaks+3] = fXpeaks[nPeaks];
    par[3*nPeaks+4] = fSigma;
  }
  //
  if (h)
    {
      TIter next( h->GetListOfFunctions() );
      TObject *obj;
      while ( (obj = next()) )
        {
          if( obj->InheritsFrom(TF1::Class()) )
            {
              fFit = static_cast<TF1*>(obj);
              delete h->GetListOfFunctions()->Remove(fFit);
            }
        } // removes all TF1 objects from the list of functions
    }
  //
  gNp = nPeaks; // since gNp is used in function PeakFcn
  fFit = new TF1( "fit", PeakFcn, xMin, xMax, 3*nPeaks+2 );
  //
  TVirtualFitter::Fitter( h, 3*nPeaks+2 );
  fFit->SetParameters( par );
  fFit->SetParName( 0, "Intercept" );
  fFit->SetParName( 1, "Slope" );
  //
  for( Int_t p=0; (p<fNumberOfPeaks) && (p<fNfound); ++p )
  {
    std::auto_ptr<TString> name1( new TString(Form("%3d",p)) );
    std::auto_ptr<TString> name2( new TString(Form("%3d",p)) );
    std::auto_ptr<TString> name3( new TString(Form("%3d",p)) );
    name1->Prepend( "Norm, Peak " );
    name2->Prepend( "Mean, Peak " );
    name3->Prepend( "Sigma, Peak " );
    fFit->SetParName( 3*p+2, name1->Data() );
    fFit->SetParName( 3*p+3, name2->Data() );
    fFit->SetParName( 3*p+4, name3->Data() );
  }
  fFit->SetNpx( 1000 );
  fFit->SetLineColor( 2 );
  //
  if (h)
    {
      fAddPoly ? h->Fit("fit","ME+") : h->Fit("fit","ME");
      h->Draw();
    }
  gPad->Modified();
  gPad->Update();

  delete par;
}

void TPeakFindPanel::FindPeak()
{
  TH1* h = GetHistogram();
  if( !h )return;
  //
  // store the parameters
  //
  TList *primitives;
  if( fSpectrum == 0 )
  {
    fSpectrum = new TSpectrum( 2*fNumberOfPeaks, fResolution );
  }
  else // look for the maximum of 100 or 2*nPeaks peaks,
  {
    delete fSpectrum;
    fSpectrum = new TSpectrum( 2*fNumberOfPeaks, fResolution );
    primitives = h->GetListOfFunctions();
    //
    // remove all polymarkers from the histogram list
    //
    while( primitives->FindObject("TPolyMarker") )
    {
      delete primitives->Remove(primitives->FindObject("TPolyMarker"));
    }
  }
  //TSpectrum::TSpectrum( Int_t maxpositions, Float_t resolution )
  //   : TNamed( "Spectrum", "Miroslav Morhac peak finder" )
  //  maxpositions:  maximum number of peaks
  //  resolution:    determines resolution of the neighboring peaks
  //                 default value is 1 correspond to 3 fSigma distance
  //                 between peaks. Higher values allow higher resolution
  //                 (smaller distance between peaks) ...
  //                   requires more processing time
  fNfound = fSpectrum->Search( h, fSigma, "", fThreshold );
  primitives = h->GetListOfFunctions();
  while( primitives->FindObject("TPolyMarker") )
  {
    delete primitives->Remove(primitives->FindObject("TPolyMarker"));
  }
  //
  // ONE-DIMENSIONAL PEAK SEARCH FUNCTION
  // This function searches for peaks in source spectrum in h
  // The number of found peaks and their positions are written into
  // the members fNpeaks and fPositionX
  //
  // Function parameters:
  //   h:        pointer to the histogram of source spectrum
  //   fSigma:   sigma of searched peaks, for details we refer to manual
  //   fThreshold: (default=0.05)  peaks with amplitude less than
  //       fThreshold*highest_peak are discarded
  //
  //   if option is not equal to "goff" (goff is the default), then
  //   a polymarker object is created and added to the list of functions of
  //   the histogram. The histogram is drawn with the specified option and
  //   the polymarker object drawn on top of the histogram
  //   The polymarker coordinates correspond to the npeaks peaks found in
  //   the histogram
  //   A pointer to the polymarker object can be retrieved later via:
  //    TList *functions = hin->GetListOfFunctions();
  //    TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker")
  //
  std::cout << "Found " << fNfound << " candidate peaks to fit" << std::endl;
  std::cout << "Will Identify " << fNumberOfPeaks << " or " << fNfound
            << " on the Histogram" << std::endl;
  //
  fXpeaks = fSpectrum->GetPositionX();
  fYpeaks = fSpectrum->GetPositionY();
  //
  if( fNumberOfPeaks < fNfound )
  {
    InsertionSort( fXpeaks, fYpeaks, fNfound, 1 );
    // sorts all the peaks from largest to smallest
    InsertionSort( fYpeaks, fXpeaks, fNumberOfPeaks, -1 );
    // sorts the fNumberOfPeaks biggest peaks from smallest x value
    // to largest x value
  }
  TPolyMarker *pm;
  fNumberOfPeaks < fNfound ?
      pm = new TPolyMarker( fNumberOfPeaks, fXpeaks, fYpeaks ) :
      pm = new TPolyMarker( fNfound, fXpeaks, fYpeaks );
  pm->SetMarkerStyle( 23 );
  pm->SetMarkerColor( kRed );
  pm->SetMarkerSize( 2 );
  //
  if( !h->FindObject("TPolyMarker") )
  {
    pm->Paint();
    h->GetListOfFunctions()->Add(pm);
  }
  else
  {
    delete h->GetListOfFunctions()->Remove(h->FindObject("TPolyMarker"));
    // might want to remove TPolyMarker from TCanvas->GetListOfPrimitives()
    // or gPad->GetListOfPrimitives()
    pm->Paint();
    h->GetListOfFunctions()->Add(pm);
  }
  gPad->Modified();
  gPad->Update();
}

void TPeakFindPanel::GetXYBounds( Double_t &xMin, Double_t &xMax )
{
  // Depends on the parameters set in the window ...
  // However, can always assume that they want the current displayed range
  //
  TH1* h = GetHistogram();
  TAxis *AxisX = h->GetXaxis();
  if(fDoUnzoom) AxisX->UnZoom();
  //xMin = AxisX->GetBinLowEdge( AxisX->GetFirst() );
  //xMax = AxisX->GetBinUpEdge( AxisX->GetLast() );
  xMin = AxisX->GetXmin();
  xMax = AxisX->GetXmax();
  /*
    =================================================================
    if ( currentRangeButton->isChecked()){
       xMin = AxisX->GetBinLowEdge( AxisX->GetFirst() );
       xMax = AxisX->GetBinUpEdge( AxisX->GetLast() );
    }else if(fullRangeButton->isChecked()){
       xMin = AxisX->GetXmin();
       xMax = AxisX->GetXmax();
    }else if(definedButton->isChecked()){
       ...

    }else{
       xMin = AxisX->GetBinLowEdge( AxisX->GetFirst() );
       xMax = AxisX->GetBinUpEdge( AxisX->GetLast() );
    }
    ==================================================================
  */
}

void TPeakFindPanel::InsertionSort( Float_t *x, Float_t *y, Int_t number, Int_t order )
{
  // where x is correlated to y, and y is to be sorted ...
  // therefore x must move when y moves
  //
  if( order == 1 ) // largest to smallest
  {
    for( Int_t f=1; f<number; ++f )
    {
      if( y[f] < y[f-1] )continue;
      Float_t ystore = y[f]; 
      Float_t xstore = x[f];
      Int_t i = f-1;
      while( (i>=0) && (y[i]<ystore) )
      {
	y[i+1] = y[i];
	x[i+1] = x[i];
	--i;
      }
      y[i+1] = ystore;
      x[i+1] = xstore;
    }
  }
  else if( order == -1 ) // smallest to largest
  {
    for( Int_t f=1; f<number; ++f )
    {
      if( y[f] > y[f-1] )continue;
      Float_t xstore = x[f]; 
      Float_t ystore = y[f];
      Int_t i = f-1;
      while( (i>=0) && (y[i]>ystore) )
      {
	x[i+1] = x[i];
	y[i+1] = y[i];
	--i;
      }
      y[i+1] = ystore;
      x[i+1] = xstore;
    }
  }
}

void TPeakFindPanel::SetDefaults()
{
  Clear();

  fAddPoly = kFALSE;
  fDoFit = kFALSE;
  fDoUnzoom = kFALSE;
  fSigma = 1.00;
  fNumberOfPeaks = 10;
  fResolution = 3.0;
  fThreshold = 0.05;

  fAttemptFitCB->SetState( kButtonUp );
  fAddPolyFitCB->SetState( kButtonUp );
  fDoUnzoomCB->SetState( kButtonUp );

  fNumberOfPeaksNE->SetNumber( fNumberOfPeaks );
  fResolutionNE->SetNumber( fResolution );
  fSigmaNE->SetNumber( fSigma );
  fThresholdNE->SetNumber( fThreshold );
}
