/*********************************************************************

  Name:         TAxisLimitsDialog.cxx
  Created by:   Joe Chuma
  $Id: TAxisLimitsDialog.cxx 215 2006-12-04 23:19:10Z olchansk $
  
*********************************************************************/
#include <iostream>

#include "TAxisLimitsDialog.h"

ClassImp(TAxisLimitsDialog)

#define BUTTON_OK      1
#define BUTTON_APPLY   2
#define BUTTON_CANCEL  3
#define BUTTON_UNZOOMX 4
#define BUTTON_UNZOOMY 5

TAxisLimitsDialog::TAxisLimitsDialog( TGWindow const *p, TGWindow const *main,
                                      UInt_t w, UInt_t h,
				      CanvasLimits* limits,
				      Roody* roody)
//    : TGTransientFrame(p,main,w,h,kVerticalFrame)
    : TGMainFrame(main,w,h,kVerticalFrame)
{
  fSavedLimits = *limits;
  fLimits = limits;
  fRoody  = roody;

  //
  fF2 = new TGCompositeFrame( this, 200, 20, kHorizontalFrame );

  fFX = new TGCompositeFrame( this, 200, 20, kHorizontalFrame );
  fFY = new TGCompositeFrame( this, 200, 20, kHorizontalFrame );
  fFR = new TGCompositeFrame( this, 200, 20, kHorizontalFrame );

  //
  fL1 = new TGLayoutHints( kLHintsLeft|kLHintsCenterY, 3, 5, 2, 2 );
  fL2 = new TGLayoutHints( kLHintsLeft|kLHintsCenterY, 5, 5, 2, 2 );
  fL3 = new TGLayoutHints( kLHintsLeft|kLHintsCenterY, 5, 3, 2, 2 );
  fL4 = new TGLayoutHints( kLHintsCenterX, 5, 5, 5, 5 );
  fL5 = new TGLayoutHints( kLHintsCenterX|kLHintsCenterY, 0, 0, 0, 0 );
  //

  char buf[256];

  fZoomX = new TGCheckButton(fFX,"Fix X axis range:");
  if (fLimits->fDoZoomXaxis) fZoomX->SetState(kButtonDown);
  fFX->AddFrame(fZoomX, fL1);
  fFX->AddFrame(new TGLabel(fFX, "minimum: "), fL1);
  sprintf(buf,"%15f",limits->fXmin);
  fEditXmin = new TGTextEntry(fFX, buf);
  fFX->AddFrame(fEditXmin, fL2);
  fFX->AddFrame(new TGLabel(fFX, "maximum: "),fL2);
  sprintf(buf,"%15f",limits->fXmax);
  fEditXmax = new TGTextEntry(fFX, buf);
  fFX->AddFrame(fEditXmax,fL3);

  fZoomY = new TGCheckButton(fFY,"Fix Y axis range:");
  if (fLimits->fDoZoomYaxis) fZoomY->SetState(kButtonDown);
  fFY->AddFrame(fZoomY, fL1);
  fFY->AddFrame(new TGLabel(fFY, "minimum: "), fL1);
  sprintf(buf,"%15f",limits->fYmin);
  fEditYmin = new TGTextEntry(fFY, buf);
  fFY->AddFrame(fEditYmin, fL2);
  fFY->AddFrame(new TGLabel(fFY, "maximum: "),fL2);
  sprintf(buf,"%15f",limits->fYmax);
  fEditYmax = new TGTextEntry(fFY, buf);
  fFY->AddFrame(fEditYmax,fL3);

  fFR->AddFrame(new TGLabel(fFR, "re-bin histograms (0=no rebin): "), fL1);
  sprintf(buf,"%6d",limits->fRebin);
  fEditRebin = new TGTextEntry(fFR, buf);
  fFR->AddFrame(fEditRebin,fL3);

  //
  fUnzoomXButton = new TGTextButton( fF2, new TGHotString("&Unzoom X axis"), BUTTON_UNZOOMX);
  fUnzoomXButton->Associate( this );
  //
  fUnzoomYButton = new TGTextButton( fF2, new TGHotString("&Unzoom Y axis"), BUTTON_UNZOOMY);
  fUnzoomYButton->Associate( this );
  //
  fApplyButton = new TGTextButton( fF2, new TGHotString("    &Ok    "), BUTTON_OK);
  fApplyButton->Associate( this );
  //
  fOkButton = new TGTextButton( fF2, new TGHotString("    &Apply    "), BUTTON_APPLY);
  fOkButton->Associate( this );
  //
  fCancelButton = new TGTextButton( fF2, new TGHotString("&Cancel"), BUTTON_CANCEL);
  fCancelButton->Associate( this );
  //
  fF2->AddFrame( fUnzoomXButton, fL4 );
  fF2->AddFrame( fUnzoomYButton, fL4 );
  fF2->AddFrame( fApplyButton,   fL4 );
  fF2->AddFrame( fOkButton,      fL4 );
  fF2->AddFrame( fCancelButton,  fL4 );
  //
  AddFrame( fFX, fL5 );
  AddFrame( fFY, fL5 );
  AddFrame( fFR, fL5 );
  AddFrame( fF2, fL5 );
  //
  MapSubwindows();
  Resize( GetDefaultSize() );
  //
  // position relative to the parent's window
  //
  Int_t ax, ay;
  if( main )
  {
    Window_t wdum;
    gVirtualX->TranslateCoordinates( main->GetId(), GetParent()->GetId(),
                                     (Int_t)(((TGFrame *)main)->GetWidth()-fWidth)>>1,
                                     (Int_t)(((TGFrame *)main)->GetHeight()-fHeight)>>1,
                                     ax, ay, wdum );
  }
  else
  {
    UInt_t root_w, root_h;
    gVirtualX->GetWindowSize( fClient->GetRoot()->GetId(), ax, ay, root_w, root_h );
    ax = (root_w-fWidth) >> 1;
    ay = (root_h-fHeight) >> 1;
  }
  Move( ax, ay );
  SetWMPosition( ax, ay );
  //
  SetWindowName("Set histogram limits for this canvas");
  //
  SetMWMHints( kMWMDecorAll|kMWMDecorMaximize|kMWMDecorMenu,
               kMWMFuncAll|kMWMFuncMaximize|kMWMFuncResize,
               kMWMInputModeless );
  MapWindow();
  Resize( GetDefaultSize() );
  //fClient->WaitFor( this );
}

TAxisLimitsDialog::~TAxisLimitsDialog()
{
  fRoody  = NULL;
  fLimits = NULL;
  delete fCancelButton;
  delete fOkButton;
  delete fUnzoomXButton;
  delete fUnzoomYButton;
  delete fApplyButton;
  delete fEditXmin;
  delete fEditXmax;
  delete fEditYmin;
  delete fEditYmax;
  delete fEditRebin;
  delete fF2;
  delete fL1; delete fL2; delete fL3; delete fL4; delete fL5;
}

void TAxisLimitsDialog::CloseWindow()
{
  // Close the dialog.
  // On close the dialog will be deleted and cannot be re-used.
  //
  DeleteWindow();
}

void TAxisLimitsDialog::ReadValues()
{
  fLimits->fDoZoomXaxis = fZoomX->IsDown();
  fLimits->fXmin = atof(fEditXmin->GetText());
  fLimits->fXmax = atof(fEditXmax->GetText());

  fLimits->fDoZoomYaxis = fZoomY->IsDown();
  fLimits->fYmin = atof(fEditYmin->GetText());
  fLimits->fYmax = atof(fEditYmax->GetText());

  fLimits->fRebin = atoi(fEditRebin->GetText());

  //printf("limits %f %f %d\n",fLimits->fXmin,fLimits->fXmax,fLimits->fDoZoomXaxis);
  //printf("rebin %d\n",fLimits->fRebin);
}

Bool_t TAxisLimitsDialog::ProcessMessage( Long_t msg, Long_t parm1, Long_t dummy )
{
  switch( GET_MSG(msg) )
  {
    case kC_COMMAND:
    {
      switch( GET_SUBMSG(msg) )
      {
        case kCM_BUTTON:
        {
          switch( parm1 )
          {
            case BUTTON_UNZOOMX:
	      fLimits->fDoZoomXaxis  = false;
	      fZoomX->SetState(kButtonUp);
	      fLimits->fForceUnzoomX = true;
	      fRoody->RedrawCanvas();
	      fLimits->fForceUnzoomX = false;
	      return kTRUE;

	    case BUTTON_UNZOOMY:
	      fLimits->fDoZoomYaxis = false;
	      fZoomY->SetState(kButtonUp);
	      fLimits->fForceUnzoomY = true;
	      fRoody->RedrawCanvas();
	      fLimits->fForceUnzoomY = false;
	      return kTRUE;

            case BUTTON_OK:
	      ReadValues();
	      CloseWindow();
              fRoody->RedrawCanvas();
	      return kTRUE;

	    case BUTTON_CANCEL:
	      CloseWindow();
	      *fLimits = fSavedLimits;
              fRoody->RedrawCanvas();
	      return kTRUE;

            case BUTTON_APPLY:
	      ReadValues();
              fRoody->RedrawCanvas();
	      return kTRUE;
          }
          break;
        }
        default:
          break;
      }
      break;  
    }
    default:
      break;
  }
  return kTRUE;
}

// end of code
