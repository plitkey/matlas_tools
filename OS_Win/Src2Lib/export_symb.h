/*
===============================================================================

  FILE:  export_symb.h
  
  CONTENTS:
  
    The header declares macros to export symbols to the shared libraries (DLLs or MEX files) 

  PROGRAMMERS:

    jluis@ualg.pt  -  http://w3.ualg.pt/~jluis/

  COPYRIGHT:

    (c) 2012, joaquim luis

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    CHANGE HISTORY:
  
    17 March 2014 -- FGI matlas tools, Commented out lines 67-70
	
===============================================================================
*/

#ifndef CPL_BASE_H_INCLUDED
#define CPL_BASE_H_INCLUDED

#ifndef CPL_DLL
#if defined(_MSC_VER) && !defined(CPL_DISABLE_DLL)
#  define CPL_DLL     __declspec(dllexport)
#else
#  if defined(USE_GCC_VISIBILITY_FLAG)
#    define CPL_DLL     __attribute__ ((visibility("default")))
#  else
#    define CPL_DLL
#  endif
#endif
#endif

#ifndef CPL_STDCALL
#if defined(_MSC_VER) && !defined(CPL_DISABLE_STDCALL)
#  define CPL_STDCALL     __stdcall
#else
#  define CPL_STDCALL
#endif
#endif


/* -------------------------------------------------------------------- */
/*      The following apparently allow you to use strcpy() and other    */
/*      functions judged "unsafe" by microsoft in VS 8 (2005).          */
/* -------------------------------------------------------------------- */
#ifdef _MSC_VER
#  ifndef _CRT_SECURE_NO_DEPRECATE
#    define _CRT_SECURE_NO_DEPRECATE
#  endif
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif
//#  ifdef MSVC_USE_VLD
//#    include <wchar.h>
//#    include <vld.h>
//#  endif
#endif

#endif

