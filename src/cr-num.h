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


/**
 *@file
 *The declaration
 *of the #CRNum class.
 */

#ifndef __CR_NUM_H__
#define __CR_NUM_H__

#include <glib.h>
#include "cr-utils.h"

#ifdef __cpluplus
extern "C"
{
#endif

	typedef struct _CRNum CRNum ;

	/**
	 *An abstraction of a number (num)
	 *as defined in the css2 spec.
	 */
	struct _CRNum
	{
		gboolean is_natural ;

		/**
		 *integer part of the number
		 */
		glong int_part ;

		/**
		 *decimal part of the number.
		 */
		glong dec_part ;
	} ;

	CRNum *
	cr_num_new (void) ;
	
	CRNum *
	cr_num_new_with_vals (gboolean a_is_natural,
			      glong a_integer_part,
			      glong a_decimal_part) ;

        guchar *
        cr_num_to_string (CRNum *a_this) ;

	void
	cr_num_destroy (CRNum *a_this) ;

#ifdef __cpluplus
} /*extern "C"*/
#endif

#endif /*__CR_NUM_H__*/
