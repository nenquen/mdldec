/*
===========================================================================
Copyright (C) 1996-2002, Valve LLC. All rights reserved.
Copyright (C) 2023 Toodles

This product contains software technology licensed from Id 
Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
All Rights Reserved.

Use, distribution, and modification of this source code and/or resulting
object code is restricted to non-commercial enhancements to products from
Valve LLC.  All other use, distribution, or modification is prohibited
without written permission from Valve LLC.
===========================================================================
*/

#ifndef _PCH_H
#define _PCH_H

/* mdldec: MSVC (Windows) and GCC/Clang (Linux) are both supported.
   The build is Windows-first: MSVC remains the reference compiler, but the
   POSIX path is exercised by the Nix flake on NixOS. */

#if !defined(_WIN32) && !defined(_DEFAULT_SOURCE)
/* Exposes POSIX strdup/strcasecmp under -std=c11. Feature-test macros are
   only honoured before the first libc header is included, so this must sit
   above every #include in this file. */
#define _DEFAULT_SOURCE
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <memory.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

/* strcasecmp/strdup are POSIX names; map them to MSVC equivalents. */
#ifdef _WIN32
#define strcasecmp _stricmp
#define strdup _strdup
#else
#include <strings.h>
#endif

#include "decompile.h"

#endif /* _PCH_H */
