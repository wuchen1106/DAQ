/*********************************************************************

  Name:         MTGListTree.h
  Created by:   Matthias Schneebeli
  $Id: MTGListTree.h 223 2007-07-09 07:11:02Z olchansk $
                                                                       
*********************************************************************/

#ifndef MTGListTree_H
#define MTGListTree_H

#include <vector>

#include "TObjArray.h"
#include "TGListTree.h"

class MTGListTree : public TGListTree
{
friend class TGListTreeItem;
  
private:
  typedef std::vector<TGListTreeItem*>::iterator iterator;
  Bool_t fControlPressed;

  void init();

public:
  MTGListTree( TGWindow *, UInt_t, UInt_t, UInt_t, ULong_t =GetWhitePixel() );
  MTGListTree( TGCanvas *, UInt_t, ULong_t =GetWhitePixel() );
  Bool_t HandleButton( Event_t * );
  Bool_t HandleKey( Event_t * );
  void GetSelectedItems( std::vector<TGListTreeItem*> & );
  void GetSelectedItemsRecursive( TGListTreeItem *, std::vector<TGListTreeItem*> & );

  Int_t MDeleteItem( TGListTreeItem * );
  void MPDeleteChildren( TGListTreeItem * );
  
  ClassDef(MTGListTree, 1)
};
#endif

// end of file
