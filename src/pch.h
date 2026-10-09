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

/* mdldec: MSVC-only. MinGW is not supported. */

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
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#ifndef strdup
#define strdup _strdup
#endif

#include "decompile.h"

#endif /* _PCH_H */
