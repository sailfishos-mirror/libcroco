/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the 
 * GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
n * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the 
 * GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <string.h>
#include "cr-lay-eng.h"

#define PRIVATE(a_this) ((a_this)->priv)

struct _CRLayEngPriv
{
	
} ;


CRLayEng *
cr_lay_eng_new (void)
{
	CRLayEng *result = NULL;

	result = g_try_malloc (sizeof (CRLayEng)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRLayEng)) ;

	PRIVATE (result) = g_try_malloc (sizeof (CRLayEngPriv)) ;
	if (!PRIVATE (result))
	{
		cr_utils_trace_info ("Out of memory") ;
		g_free (result) ;
		result = NULL ;
		return NULL ;
	}
	memset (PRIVATE (result), 0, sizeof (CRLayEngPriv)) ;

	return result ;
}


void
cr_lay_eng_destroy (CRLayEng *a_this)
{
	g_return_if_fail (a_this) ;
	
	if (PRIVATE (a_this))
	{
		g_free (PRIVATE (a_this)) ;
		PRIVATE (a_this) = NULL ;
	}

	if (a_this)
	{
		g_free (a_this) ;
	}
}
