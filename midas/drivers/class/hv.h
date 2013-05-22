/********************************************************************\

  Name:         hv.h
  Created by:   Stefan Ritt

  Contents:     High Voltage Class Driver

  $Id: hv.h 2753 2005-10-07 14:55:31Z ritt $

\********************************************************************/

/* class driver routines */
INT cd_hv(INT cmd, PEQUIPMENT pequipment);
INT cd_hv_read(char *pevent, int);
