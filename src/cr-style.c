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
#include <string.h>
#include "cr-style.h"



/**
 *@file
 *The definition of the #CRStyle class.
 */


/**
 *A property ID.
 *Each css property has an ID which is
 *en entry into a property "population" jump table.
 *each entry of the property population jump table
 *contains code to tranform the literal form of
 *a property value into a strongly typed value.
 */
enum CRPropertyID
{
	PROP_PADDING_TOP,
	PROP_PADDING_RIGHT,
	PROP_PADDING_BOTTOM,
	PROP_PADDING_LEFT,
	PROP_BORDER_TOP_WIDTH,
	PROP_BORDER_RIGHT_WIDTH,
	PROP_BORDER_BOTTOM_WIDTH,
	PROP_BORDER_LEFT_WIDTH,
	PROP_BORDER_TOP_STYLE,
	PROP_BORDER_RIGHT_STYLE,
	PROP_BORDER_BOTTOM_STYLE,
	PROP_BORDER_LEFT_STYLE,
	PROP_MARGIN_TOP,
	PROP_MARGIN_RIGHT,
	PROP_MARGIN_BOTTOM,
	PROP_MARGIN_LEFT,
	PROP_DISPLAY,
	PROP_POSITION,
	PROP_TOP,
	PROP_RIGHT,
	PROP_BOTTOM,
	PROP_LEFT,
	PROP_FLOAT,
	PROP_WIDTH	
} ;


typedef struct _CRPropertyDesc CRPropertyDesc ;

struct _CRPropertyDesc
{
	const guchar * name ;
	enum CRPropertyID prop_id ;	
} ;

static CRPropertyDesc gv_prop_table [] =

{
	{"padding-top", PROP_PADDING_TOP},
	{"padding-right", PROP_PADDING_RIGHT},
	{"padding-bottom", PROP_PADDING_BOTTOM},
	{"padding-left", PROP_PADDING_LEFT},
	{"border-top-width", PROP_BORDER_TOP_WIDTH},
	{"border-right-width", PROP_BORDER_RIGHT_WIDTH},
	{"border-bottom-width", PROP_BORDER_BOTTOM_WIDTH},
	{"border-left-width", PROP_BORDER_LEFT_WIDTH},
	{"border-top-style", PROP_BORDER_TOP_STYLE},
	{"border-right-style", PROP_BORDER_RIGHT_STYLE},
	{"border-bottom-style", PROP_BORDER_BOTTOM_STYLE},
	{"border-left-style", PROP_BORDER_LEFT_STYLE},
	{"margin-top", PROP_MARGIN_TOP},
	{"margin-right", PROP_MARGIN_RIGHT},
	{"margin-bottom", PROP_MARGIN_BOTTOM},
	{"margin-left", PROP_MARGIN_LEFT},
	{"display", PROP_DISPLAY},
	{"position", PROP_POSITION},
	{"top", PROP_TOP},
	{"right", PROP_RIGHT},
	{"bottom", PROP_BOTTOM},
	{"left", PROP_LEFT},
	{"float", PROP_FLOAT},
	{"width", PROP_WIDTH},
	{NULL, 0}
		
} ;

static GHashTable *gv_prop_hash = 0 ;


static void
cr_style_init_properties (void) ;


static void
cr_style_init_properties (void)
{
	return ;
}

CRStyle *
cr_style_new (void)
{
	CRStyle *result = NULL ;

	result = g_try_malloc (sizeof (CRStyle)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRStyle)) ;

	return result ;
}


enum CRStatus
cr_style_new_from_ruleset (CRStatement *a_this, 
			   CRStyle **a_style)
{
	return CR_OK ;
}

void
cr_style_destroy (CRStyle *a_this)
{
	g_return_if_fail (a_this) ;

	g_free (a_this) ;
}
