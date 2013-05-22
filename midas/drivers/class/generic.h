/********************************************************************\

  Name:         generic.h
  Created by:   Stefan Ritt

  Contents:     Generic Class Driver header file

  $Id: generic.h 2753 2005-10-07 14:55:31Z ritt $

\********************************************************************/

/* class driver routines */
INT cd_gen(INT cmd, PEQUIPMENT pequipment);
INT cd_gen_read(char *pevent, int);
