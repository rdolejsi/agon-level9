/***********************************************************************
*
* Level 9 interpreter
* Version 5.2.agon1
* Copyright (c) 1996-2023 Glen Summers and contributors.
* Contributions from David Kinder, Alan Staniforth, Simon Baldwin,
* Dieter Baron and Andreas Scherrer.
*
* Agon 8-bit ez80 version (w/ 24-bit addressing) by Roman Dolejsi.
* Platform-specific header file.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
*
\***********************************************************************/

#include <stdbool.h>
#include <mos_api.h>

#ifndef L9TYPEDEF
#define L9TYPEDEF
typedef uint8_t L9BYTE;
typedef uint16_t L9UINT16;
typedef uint32_t L9UINT32;
typedef bool L9BOOL;
#endif
