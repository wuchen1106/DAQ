/********************************************************************\

  Name:         multi.h
  Created by:   Stefan Ritt

  Contents:     Multimeter Class Driver Header File

  $Id: multi.h 2753 2005-10-07 14:55:31Z ritt $

\********************************************************************/

/* class driver routines */
INT cd_multi(INT cmd, PEQUIPMENT pequipment);
INT cd_multi_read(char *pevent, int);
