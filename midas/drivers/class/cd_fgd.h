/********************************************************************\

  Name:         cd_fgd.h
  Created by:   Pierre-Andre Amaudruz

  Contents:     FGD / T2K Class Driver header file

  $Id: cd_fgd.h 3390 2006-10-21 06:29:55Z amaudruz $

\********************************************************************/

/* class driver routines */
INT cd_fgd(INT cmd, PEQUIPMENT pequipment);
INT cd_fgd_read(char *pevent, int);
