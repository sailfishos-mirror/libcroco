/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of 
 * the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the 
 * GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */
#ifndef __CR_FONTS_H__
#define __CR_FONTS_H__
#endif

#include "cr-utils.h"

G_BEGIN_DECLS

enum CRFontFamilyType
{
	FONT_FAMILY_SANS_SERIF,
	FONT_FAMILY_SERIF,	
	FONT_FAMILY_CURSIVE,
	FONT_FAMILY_FANTASY,
	FONT_FAMILY_MONOSPACE,
	FONT_FAMILY_NON_GENERIC,
	/**/
	NB_FONT_FAMILIE_TYPES
} ;

typedef struct _CRFontFamily CRFontFamily ;

struct _CRFontFamily
{
	enum CRFontFamilyType type ;

	/*
	 *The name of the font family, in case
	 *it is non generic.
	 *Is set only if the type is FONT_FAMILY_NON_GENERIC.
	 */
	guchar *name ;

	CRFontFamily *next ;
	CRFontFamily *prev ;
} ;

CRFontFamily *
cr_font_family_new (enum CRFontFamilyType a_type, guchar *a_name) ;

CRFontFamily *
cr_font_family_append (CRFontFamily *a_this, 
		       CRFontFamily *a_family_to_append) ;

CRFontFamily *
cr_font_family_prepend (CRFontFamily *a_this, 
			CRFontFamily *a_family_to_prepend);

enum CRStatus
cr_font_family_destroy (CRFontFamily *a_this) ;

enum CRStatus
cr_font_family_set_name (CRFontFamily *a_this, guchar *a_name) ;

G_END_DECLS
