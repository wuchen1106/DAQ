/*********************************************************************

  Name:         TGTextDialog.h
  Created by:   Stephan Ritt
  $Id: TGTextDialog.h 169 2006-09-26 22:34:44Z olchansk $
                                                                       
*********************************************************************/

#ifndef TGTextDialog_H
#define TGTextDialog_H

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

#ifndef ROOT_TGFrame
#include <TGFrame.h>
#endif

class TGTextDialog : public TGTransientFrame
{
protected:
  TGCompositeFrame *fF1, *fF2;                // sub frames
  TGButton         *fOkButton;                // ok button
  TGButton         *fCancelButton;            // cancel button
  TGLayoutHints    *fL1, *fL5, *fL6, *fL21;   // layout hints
  TGTextEntry      *fText;                    // text entry widget
  TGTextBuffer     *fBLabel;                  // text buffer
  TGLabel          *fLabel;                   // label
  TObjString       *fRetObjStr;               // return string
  
public:
  TGTextDialog( TGWindow const *, TGWindow const *, UInt_t, UInt_t,
                char *, char const *, TObjString *, UInt_t =kVerticalFrame );
  virtual ~TGTextDialog();
  
  virtual void   CloseWindow();
  virtual Bool_t ProcessMessage( Long_t, Long_t, Long_t );
  
  ClassDef(TGTextDialog, 1)
};
#endif
// end of file
