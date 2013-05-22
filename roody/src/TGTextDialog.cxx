/*********************************************************************

  Name:         TGTextDialog.cxx
  Created by:   Stephan Ritt
  $Id: TGTextDialog.cxx 169 2006-09-26 22:34:44Z olchansk $
  
*********************************************************************/
//////////////////////////////////////////////////////////////////////
// This Code is written by Stephan Ritt                             //
// found in rmidas.c, a component of the MIDAS                      //
// (see http://midas.triumf.ca/)                                    //
//////////////////////////////////////////////////////////////////////

#include "TGTextDialog.h"

ClassImp(TGTextDialog)

TGTextDialog::TGTextDialog( TGWindow const *p, const TGWindow *main,
                            UInt_t w, UInt_t h, char *label, char const *preLoad,
                            TObjString *ret_str,
                            UInt_t options)
    : TGTransientFrame(p, main, w, h, options)
{
  // Create a dialog to enter a single line text entry
  //
  fRetObjStr = ret_str;
  //
  ChangeOptions( (GetOptions()&~kVerticalFrame)|kHorizontalFrame );
  //
  fF1 = new TGCompositeFrame( this, 60, 20, kVerticalFrame | kFixedWidth );
  fF2 = new TGCompositeFrame( this, 60, 20, kHorizontalFrame );
  //
  fOkButton = new TGTextButton( fF1, new TGHotString("Ok"), 1 );
  fCancelButton = new TGTextButton( fF1, new TGHotString("Cancel"), 2 );
  fF1->Resize( fOkButton->GetDefaultWidth()+40, GetDefaultHeight() );
  //
  fOkButton->Associate(this);
  fCancelButton->Associate(this);
  //
  fL1 = new TGLayoutHints( kLHintsTop | kLHintsExpandX, 2, 2, 3, 0 );
  fL21 = new TGLayoutHints( kLHintsTop | kLHintsRight, 2, 5, 10, 0 );
  //
  fF1->AddFrame( fOkButton, fL1 );
  fF1->AddFrame( fCancelButton, fL1 );
  AddFrame( fF1, fL21 );
  //
  fLabel = new TGLabel( fF2, new TGHotString(label) );
  //
  fBLabel = new TGTextBuffer(200);
  fOkButton->SetState(kButtonDisabled);
  //
  fText = new TGTextEntry( fF2, fBLabel );
  fText->Associate(this);
  fText->Resize( 220, fText->GetDefaultHeight() );
  fText->SelectAll();
  if( strlen(preLoad) )fText->SetText( preLoad );
  //
  fL5 = new TGLayoutHints( kLHintsLeft|kLHintsCenterY, 3, 5, 0, 0 );
  fL6 = new TGLayoutHints( kLHintsLeft|kLHintsCenterY, 0, 2, 0, 0 );
  //
  fF2->AddFrame( fLabel, fL5 );
  fF2->AddFrame( fText, fL5 );
  AddFrame( fF2, fL1 );
  //
  MapSubwindows();
  Resize( GetDefaultSize() );
  //
  // position relative to the parent's window
  Int_t ax, ay;
  if( main )
  {
    Window_t wdum;
    gVirtualX->TranslateCoordinates(
     main->GetId(), GetParent()->GetId(),
     (Int_t)(((TGFrame *)main)->GetWidth()-fWidth)>>1,
     (Int_t)(((TGFrame *)main)->GetHeight()-fHeight)>>1,
     ax, ay, wdum );
  }
  else
  {
    UInt_t root_w, root_h;
    gVirtualX->GetWindowSize( fClient->GetRoot()->GetId(), ax, ay,
                              root_w, root_h );
    ax = (root_w-fWidth) >> 1;
    ay = (root_h-fHeight) >> 1;
  }
  //
  Move( ax, ay );
  SetWMPosition( ax, ay );
  //
  SetWindowName("Enter Text");
  //
  SetMWMHints( kMWMDecorAll|kMWMDecorMaximize|kMWMDecorMenu,
               kMWMFuncAll|kMWMFuncMaximize|kMWMFuncResize,
               kMWMInputModeless );
  //
  MapWindow();
  fClient->WaitFor(this);
}

TGTextDialog::~TGTextDialog()
{
  // Clean up text dialog
  delete fCancelButton;
  delete fOkButton;
  delete fText;
  delete fLabel;
  delete fF1; delete fF2;
  delete fL1; delete fL5; delete fL6; delete fL21;
}

void TGTextDialog::CloseWindow()
{
  // Close the dialog.
  // On close the dialog will be deleted and cannot be re-used.
  //
  DeleteWindow();
}

Bool_t TGTextDialog::ProcessMessage( Long_t msg, Long_t parm1, Long_t dummy )
{
  switch (GET_MSG(msg))
  {
    case kC_COMMAND:
    {
      switch (GET_SUBMSG(msg))
      {
        case kCM_BUTTON:
        {
          switch (parm1)
          {
            case 1:
            {
              fRetObjStr->SetString(fText->GetText());
              CloseWindow();
              break;
            }
            case 2:
            {
              fRetObjStr->SetString("-1");
              CloseWindow();
              break;
            }
          }
          break;
        }
        default:
        {
          break;
        }
      }
      break;  
    }
    case kC_TEXTENTRY:
    {
      switch (GET_SUBMSG(msg))
      {
        case kTE_TEXTCHANGED:
        {
          char const *string = fBLabel->GetString();
          strlen(string)==0 ? fOkButton->SetState(kButtonDisabled) :
                              fOkButton->SetState(kButtonUp);
          break;
        }
        case kTE_ENTER:
        {
          char const *string = fBLabel->GetString();
          if( strlen(string) != 0 )
          {
            fRetObjStr->SetString(fText->GetText());
            CloseWindow();
          }
          break;
        }
        default:
        {
          break;
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
  return kTRUE;
}
