/*********************************************************************
 
  Name:         CanvasLimits.h
  Created by:   K.Olchanski
 
  Contents:     Canvas-wide histogram settings

  $Id: CanvasLimits.h 169 2006-09-26 22:34:44Z olchansk $

*********************************************************************/

#ifndef INLCUDE_CanvasLimitsH
#define INLCUDE_CanvasLimitsH

struct CanvasLimits
{
  Bool_t fDoZoomXaxis;   // apply X-axis limits?
  Axis_t fXmin;
  Axis_t fXmax;
  Bool_t fDoZoomYaxis;   // apply Y-axis limits?
  Axis_t fYmin;
  Axis_t fYmax;
  bool fForceUnzoomX;
  bool fForceUnzoomY;

  int fRebin;

  CanvasLimits() // ctor
  {
    fDoZoomXaxis = kFALSE;
    fXmin = 0.0;
    fXmax = 0.0;
    fDoZoomYaxis = kFALSE;
    fYmin = 0.0;
    fYmax = 0.0;
    fForceUnzoomX = false;
    fForceUnzoomY = false;
    fRebin = 0;
  }
};

#endif
// end file
