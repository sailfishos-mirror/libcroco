/* -*- Mode: C; indent-tabs-mode: ni; c-basic-offset: 8 -*- */

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
#include <string.h>
#include "cr-selector.h"

/**
 *Creates a new instance of #CRSelector.
 *@param a_simple_sel the initial simple selector list
 *of the current instance of #CRSelector.
 *@return the newly built instance of #CRSelector, or
 *NULL in case of failure.
 */
CRSelector*
cr_selector_new (CRSimpleSel *a_simple_sel)
{
	CRSelector *result = NULL ;

	result = g_try_malloc (sizeof (CRSelector)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRSelector)) ;
	result->simple_sel = a_simple_sel ;
	return result ;
}

/**
 *Appends a new instance of #CRSelector to the current selector list.
 *@param a_this the current instance of #CRSelector.
 *@param a_new the instance of #CRSelector to be appended.
 *@return the new list.
 */
CRSelector*
cr_selector_append (CRSelector *a_this, CRSelector *a_new)
{
	CRSelector *cur = NULL ;
	
	if (!a_this)
		return a_new ;

	/*walk forward the list headed by a_this to get the list tail*/
	for (cur = a_this ; cur && cur->next ; cur = cur->next) ;

	cur->next = a_new ;
	a_new->prev = cur ;

	return	a_this ;
}

/**
 *Prepends an element to the #CRSelector list.
 *@param a_this the current instance of #CRSelector list.
 *@param a_new the instance of #CRSelector.
 *@return the new list.
 */
CRSelector*
cr_selector_prepend (CRSelector *a_this, CRSelector *a_new)
{
	CRSelector *cur = NULL ;

	a_new->next = a_this ;
	a_this->prev = a_new ;
	
	for (cur = a_new ; cur && cur->prev ; cur = cur->prev) ;
	
	return cur ;
}

/**
 *append a simple selector to the current #CRSelector list.
 *@param a_this the current instance of #CRSelector.
 *@param a_simple_sel the simple selector to append.
 *@return the new list or NULL in case of failure.
 */
CRSelector*
cr_selector_append_simple_sel (CRSelector *a_this,
			       CRSimpleSel *a_simple_sel)
{
	CRSelector * selector = NULL ;

	selector = cr_selector_new (a_simple_sel) ;
	g_return_val_if_fail (selector, NULL) ;

	return cr_selector_append (a_this, selector) ;
}

/**
 *Serializes the current instance of #CRSelector to a file.
 *@param a_this the current instance of #CRSelector.
 *@param a_fp the destination file.
 */
void
cr_selector_dump (CRSelector *a_this, FILE *a_fp)
{
	if (a_this)
	{
		CRSelector *cur = NULL ;
		
		for (cur = a_this ; cur ; cur = cur->next)
		{
			if (cur->simple_sel)
			{
                                if (cur->prev)
                                        fprintf (a_fp,", ") ;

                                cr_simple_sel_dump (cur->simple_sel, a_fp) ;
			}
		}
	}
}

/**
 *Increments the ref count of the current instance
 *of #CRSelector.
 *@param a_this the current instance of #CRSelector.
 */
void
cr_selector_ref (CRSelector *a_this)
{
	g_return_if_fail (a_this) ;
	
	a_this->ref_count ++ ;
}

/**
 *Decrements the ref count of the current instance of
 *#CRSelector.
 *If the ref count reaches zero, the current instance of
 *#CRSelector is destroyed.
 *@param a_this the current instance of #CRSelector.
 *@return TRUE if this function destroyed the current instance
 *of #CRSelector, FALSE otherwise.
 */
gboolean
cr_selector_unref (CRSelector *a_this)
{
	g_return_val_if_fail (a_this, FALSE) ;

	if (a_this->ref_count)
	{
		a_this->ref_count -- ;
	}

	if (a_this->ref_count == 0)
	{
		cr_selector_destroy (a_this) ;
		return TRUE ;
	}
	
	return FALSE ;
}

/**
 *Destroys the selector list.
 *@param a_this the current instance of #CRSelector.
 */
void
cr_selector_destroy (CRSelector *a_this)
{
	CRSelector *cur = NULL ;

	g_return_if_fail (a_this) ;
	
	/*
	 *go and get the list tail. In the same time, free
	 *all the simple selectors contained in the list.
	 */
	for (cur = a_this ;cur && cur->next ; cur = cur->next)
	{
		if (cur->simple_sel)
		{
			cr_simple_sel_destroy (cur->simple_sel) ;
			cur->simple_sel = NULL ;
		}
	}

	if (cur)
	{
		if (cur->simple_sel)
		{
			cr_simple_sel_destroy (cur->simple_sel) ;
			cur->simple_sel = NULL ;
		}
	}

	/*in case the list has only one element*/
	if (cur && !cur->prev)
	{
		g_free (cur) ;
		return ;
	}

	/*walk backward the list and free each "next element"*/
	for (cur = cur->prev ; cur && cur->prev ; cur = cur->prev)
	{
		if (cur->next)
		{
			g_free (cur->next) ;
			cur->next = NULL ;
		}
	}

	if (!cur)
		return ;
	
	if (cur->next)
	{
		g_free (cur->next) ;
		cur->next = NULL ;
	}

	g_free (cur) ;
}
