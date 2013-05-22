/********************************************************************\

   Name:         strlcpy.h
   Created by:   Stefan Ritt

   Contents:     Header file for strlcpy.c

   $Id: strlcpy.h 169 2006-09-26 22:34:44Z olchansk $

\********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EXPRT
#if defined(EXPORT_DLL)
#define EXPRT __declspec(dllexport)
#else
#define EXPRT
#endif
#endif

size_t EXPRT strlcpy(char *dst, const char *src, size_t size);
size_t EXPRT strlcat(char *dst, const char *src, size_t size);

#ifdef __cplusplus
}
#endif
