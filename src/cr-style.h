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
#ifndef __CR_STYLE_H__
#define __CR_STYLE_H__

#include "cr-utils.h"

/**
 *@file
 *The declaration of the #CRStyle class.
 */
G_BEGIN_DECLS

typedef struct _CRStyle CRStyle ;

/**
 *The css2 style class.
 *Contains computed and actual values
 *of inferred from the declarations found
 *in the stylesheets.
 *See css2 spec chapter 6.
 */
struct _CRStyle
{

	/**padding properties, in pixel*/

	gulong padding_top ;
	gulong padding_right ;
	gulong padding_bottom ;
	gulong padding_left ;

	/**border properties*/

	gulong border_top_width ;
	gulong border_right_width ;
	gulong border_bottom_width ;
	gulong border_left_width ;

	gulong border_top_color ;
	gulong border_right_color ;
	gulong border_bottom_color ;
	gulong border_left_color ;

	/**margin properties, in pixel*/

	gulong margin_top ;
	gulong margin_right ;
	gulong margin_bottom;
	gulong margin_left ;


} ;

G_END_DECLS

#endif /*__CR_STYLE_H__*/
