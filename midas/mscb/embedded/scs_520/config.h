/********************************************************************\

  Name:         config.h
  Created by:   Stefan Ritt


  Contents:     Configuration file for 
                for SCS-520 analog I/O with current option

  $Id: config.h 4918 2010-12-14 12:49:37Z ritt $

\********************************************************************/

/* Define device name. Used by various parts of the framwork to enable
   or disable device-specific features */
#define SCS_520

/* define CPU type */
#define CPU_C8051F000

/* include file for CPU registers etc. */
#include <c8051F000.h>

/* CPU clock speed */
#define CLK_25MHZ

/* ports for LED's */
#define LED_0 P3 ^ 4

/* polarity of LED: "0" if a low level activates the LEDs, 
   "1" if a high level activates the LEDs */
#define LED_ON 0

/* port pin for RS485 enable signal */
#define RS485_EN_PIN P3 ^ 5
