/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
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
                const guchar *name ;
		glong red ;
		glong green ;
		glong blue ;
                gboolean is_percentage ;
	} ;

	CRRgb *
	cr_rgb_new (void) ;

	CRRgb *
	cr_rgb_new_with_vals (gulong a_red, gulong a_green, 
                              gulong a_blue, gboolean a_is_percentage) ;

        enum CRStatus
        cr_rgb_set (CRRgb *a_this, gulong a_red,
                    gulong a_green, gulong a_blue,
                    gboolean a_is_percentage) ;
        
        enum CRStatus
        cr_rgb_set_from_rgb (CRRgb *a_this, CRRgb *a_rgb) ;

        enum CRStatus
        cr_rgb_set_from_name (CRRgb *a_this, const guchar *a_color_name) ;

        guchar *
        cr_rgb_to_string (CRRgb *a_this) ;

        void
        cr_rgb_dump (CRRgb *a_this, FILE *a_fp) ;

	void
	cr_rgb_destroy (CRRgb *a_this) ;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*__CR_RGB_H__*/
