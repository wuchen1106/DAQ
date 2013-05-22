/*********************************************************************

  Name:         MTGListTree.cxx
  Created by:   Matthias Schneebeli
  $Id: MTGListTree.cxx 232 2009-02-20 23:48:39Z olchansk $
                                                                       
  Contents:     MIROODAS        
  $Log$
  Revision 1.4  2005/03/10 18:29:43  chuma
  Fixed segmentation fault problems when connected online, then having the
  connection broken, then re-establishing the connection and then clicking
  on an online item in the main window.  The problem is in TGListTree in
  the ROOT code, and is fixed here by including 2 routines into MTGListTree.

  Revision 1.3  2004/09/24 22:08:35  chuma
  Added TNetFolder support

  Revision 1.2  2004/09/20 19:39:24  chuma
  multiple selection of histograms with CTRL key updated

  Revision 1.1  2004/09/16 16:30:36  schneebeli
  Linking on windows and multiple selection of items

  
*********************************************************************/

#include <iostream>
#include <algorithm>

#include "TGToolTip.h"
#include "Riostream.h"
#include "MTGListTree.h"

ClassImp(MTGListTree)

void MTGListTree::init()
{
  fControlPressed = false;
  AddInput(kKeyReleaseMask);
}

MTGListTree::MTGListTree(TGWindow *p, UInt_t w, UInt_t h, UInt_t options, ULong_t back) : TGListTree(p,w,h,options,back)
{
  init();
};

MTGListTree::MTGListTree(TGCanvas *p,UInt_t options,ULong_t back) : TGListTree(p,options,back)
{
  init();
};

/*
  The routines MDeleteItem and MPDeleteChildren were added to fix a problem in TGListTree
  where fSelected is being left set to a TGListTreeItem pointer after that pointer has
  been deleted.
*/

Int_t MTGListTree::MDeleteItem( TGListTreeItem *item )
{
  // Delete item from list tree
  //
  if( item->GetFirstChild() )MPDeleteChildren( item->GetFirstChild() );
  TGListTreeItem *tmp = item->GetFirstChild();
  tmp = 0;
  RemoveReference( item );
  if( fSelected == item )fSelected = 0; // line added by JChuma
  delete item;
  return 1;
}

void MTGListTree::MPDeleteChildren( TGListTreeItem *item )
{
  // Delete children of item from list.
  //
  TGListTreeItem *sibling;
  while (item)
  {
    if( item->GetFirstChild() )
    {
      MPDeleteChildren( item->GetFirstChild() );
      TGListTreeItem *tmp = item->GetFirstChild();
      tmp = 0;
    }
    sibling = item->GetNextSibling();
    if( fSelected == item )fSelected = 0;  // line added by JChuma
    delete item;
    item = sibling;
  }
}

Bool_t MTGListTree::HandleButton(Event_t *event)
{
   // Handle button events in the list tree.

   TGListTreeItem *item;

   if (fTip) fTip->Hide();

   Int_t page = 0;
   if (event->fCode == kButton4 || event->fCode == kButton5) {
      if (!fCanvas) return kTRUE;
      if (fCanvas->GetContainer()->GetHeight())
         page = Int_t(Float_t(fCanvas->GetViewPort()->GetHeight() *
                              fCanvas->GetViewPort()->GetHeight()) /
                              fCanvas->GetContainer()->GetHeight());
   }

   if (event->fCode == kButton4) {
      //scroll up
      Int_t newpos = fCanvas->GetVsbPosition() - page;
      if (newpos < 0) newpos = 0;
      fCanvas->SetVsbPosition(newpos);
      return kTRUE;
   }
   if (event->fCode == kButton5) {
      // scroll down
      Int_t newpos = fCanvas->GetVsbPosition() + page;
      fCanvas->SetVsbPosition(newpos);
      return kTRUE;
   }

   if( event->fType == kButtonPress )
   {
     if( (item = FindItem(event->fY)) != 0 )
     {
       // if (fSelected) fSelected->fActive = kFALSE;  // changed
       if( !fControlPressed )
       {
         HighlightItem(fSelected, kFALSE, kTRUE);  // to this
         UnselectAll(kTRUE);
       }
       //fLastY = event->fY;
       fSelected = item;
       //item->fActive = kTRUE; // this is done below w/redraw
       HighlightItem(item, kTRUE, kTRUE);
       SendMessage(fMsgWindow, MK_MSG(kC_LISTTREE, kCT_ITEMCLICK),
                   event->fCode, (event->fYRoot << 16) | event->fXRoot);
       Clicked(item, event->fCode);
       Clicked(item, event->fCode, event->fXRoot, event->fYRoot);
     }
   }
   if (event->fType == kButtonRelease) gVirtualX->SetInputFocus(fId);
   return kTRUE;
}

void MTGListTree::GetSelectedItems( std::vector<TGListTreeItem*> &items )
{
  items.clear();
  TGListTreeItem *item = fFirst; // fFirst set in TGListTree
  GetSelectedItemsRecursive( item, items );
}

void MTGListTree::GetSelectedItemsRecursive( TGListTreeItem *item, std::vector<TGListTreeItem*> &items )
{
  while( item )
  {
    TGListTreeItem *firstChild = item->GetFirstChild();
    if( firstChild )GetSelectedItemsRecursive( firstChild, items );
    if( item->IsActive() && find(items.begin(),items.end(),item)==items.end() )items.push_back( item );
    item = item->GetNextSibling();
  }
}

Bool_t MTGListTree::HandleKey(Event_t *event)
{
   char   input[10];
   UInt_t keysym;
   if (event->fType == kGKeyPress) {
      gVirtualX->LookupString(event, input, sizeof(input), keysym);
      if (keysym==4129) {
         fControlPressed = true;
      }
   }
   if (event->fType == kKeyRelease) {
      gVirtualX->LookupString(event, input, sizeof(input), keysym);
      if (keysym==4129) {
         fControlPressed = false;
      }
   }
   return TGListTree::HandleKey(event);
}
