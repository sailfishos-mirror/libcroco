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

#include <string.h>
#include "cr-box.h"


/**
 *@file
 *The definition file of the #CRBox class.
 */

/**
 *Instanciates a new box.
 *Everything is initialized to zero in it.
 *@return the newly created box.
 */
CRBox *
cr_box_new (void)
{
	CRBox *result = NULL ;

	result = g_try_malloc (sizeof (CRBox)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRBox)) ;

	return result ;
} 


/**
 *Appends a child box to at the end of the current box's children.
 *@param a_this the current box.
 *@param a_to_append, the box to append.
 *@retrurn CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_box_append_child (CRBox *a_this, CRBox *a_to_append)
{
	CRBox * cur = NULL ;

	g_return_val_if_fail (a_this && a_to_append, CR_BAD_PARAM_ERROR) ;

	if (!a_this->children)
	{
		a_this->children = a_to_append ;
		return CR_OK ;
	}

	for (cur = a_this->children ; cur && cur->next ; cur = cur->next)
		;

	cur->next = a_to_append ;

	return CR_OK ;
}


/**
 *Inserts a sibling box between two adjacent sibling nodes.
 *@param a_prev the box after which we have to insert a new one.
 *@param a_next the box before which we have to insert a new one.
 *@param a_to_insert the node to insert.
 */
enum CRStatus
cr_box_insert_sibling (CRBox *a_prev,
                       CRBox *a_next,
                       CRBox *a_to_insert)
{
	g_return_val_if_fail (a_prev && a_prev->parent
			      && a_next && a_prev->next == a_next
			      && a_next->parent == a_prev->parent
			      && a_to_insert
			      && a_to_insert->parent != a_prev->parent,
			      CR_BAD_PARAM_ERROR) ;

	a_prev->next = a_to_insert ;
	a_to_insert->prev = a_prev ;
	a_to_insert->next = a_next ;
	a_next->prev = a_to_insert ;

	return CR_OK ;
}

/**
 *Destructor of #CRBox.
 *recursively destroys all
 *the children nodes of the current node.
 *@param a_this the current box to destroy.
 */
void
cr_box_destroy (CRBox *a_this)

{
	g_return_if_fail (a_this) ;

	if (a_this->children)
	{
		CRBox *cur = NULL;

		for (cur = a_this->children ; cur ; cur = cur->next)
		{
			cr_box_destroy (cur) ;
		}
		a_this->children = NULL ;
	}

	g_free (a_this) ;
}

