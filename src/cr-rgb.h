/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 *This file is part of The Croco Library
 *
 *The Croco Library is free software; 
 *you can redistribute it and/or modify it under the terms of 
 *the GNU General Public License as 
 *published by the Free Software Foundation; either version 2, 
 *or (at your option) any later version.
 *
 *GNU The Croco Library is distributed in the hope 
 *that it will be useful, but WITHOUT ANY WARRANTY; 
 *without even the implied warranty of 
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *See the GNU General Public License for more details.
 *
 *You should have received a copy of the 
 *GNU General Public License along with The Croco Library; 
 *see the file COPYING. If not, write to 
 *the Free Software Foundation, Inc., 
 *59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *Copyright 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 */

/*
 *$Id$
 */

#include <stdio.h>
#include <glib.h>
#include "cr-utils.h"

#ifndef __CR_RGB_H__
#define __CR_RGB_H__

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct _CRRgb CRRgb ;
	struct _CRRgb
	{
                /*
                 *the unit of the rgb.
                 *Either NO_UNIT (integer) or 
                 *UNIT_PERCENTAGE (percentage).
                 */
                gboolean is_percentage ;
		glong red ;
		glong green ;
		glong blue ;
	} ;

	CRRgb *
	cr_rgb_new (void) ;

	CRRgb *
	cr_rgb_new_with_vals (glong a_red, glong a_green, 
                              glong a_blue, gboolean a_is_percentage) ;
        void
        cr_rgb_dump (CRRgb *a_this, FILE *a_fp) ;

	void
	cr_rgb_destroy (CRRgb *a_this) ;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*__CR_RGB_H__*/
