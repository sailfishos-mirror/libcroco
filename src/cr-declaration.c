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
#include "cr-declaration.h"

/**
 *@file
 *The definition of the #CRDeclaration class.
 */


/**
 *Dumps (serializes) one css declaration to a file.
 *@param a_this the current instance of #CRDeclaration.
 *@param a_fp the destination file pointer.
 *@param a_indent the number of indentation white char. 
 */
static void
dump (CRDeclaration *a_this, FILE *a_fp, glong a_indent)
{
	guchar *str = NULL, *tmp_str = NULL, *tmp_str2 = NULL;
	g_return_if_fail (a_this) ;

	if (a_this->property && a_this->property->str)
	{		
		tmp_str = g_strndup (a_this->property->str,
				     a_this->property->len) ;
		if (tmp_str)
		{
			tmp_str2 = g_strconcat (tmp_str, " : ", NULL) ;
			if (!tmp_str2) goto error ;
			if (tmp_str)
			{
				g_free (tmp_str) ;
				tmp_str = NULL ;
			}
			str = tmp_str2 ;	
		}

		if (str)
		{
			cr_utils_dump_n_chars (' ', a_fp, a_indent) ;
			fprintf (a_fp,"%s", str) ;
			g_free (str) ;
			str = NULL ;
		}
		else
			goto error ;

		if (a_this->value)
		{
			cr_term_dump (a_this->value, a_fp) ;
		}

	}

	
	return ;

 error:
	if (str)
	{
		g_free (str) ;
		str = NULL ;
	}
	if (tmp_str)
	{
		g_free (tmp_str) ;
		tmp_str = NULL ;
	}
	if (tmp_str2)
	{
		g_free (tmp_str2) ;
		tmp_str2 = NULL ;
	}
}

/**
 *Constructor of #CRDeclaration.
 *@param a_property the property string of the declaration
 *@param a_value the value expression of the declaration.
 *@return the newly built instance of #CRDeclaration, or NULL in
 *case of error.
 */
CRDeclaration *
cr_declaration_new (GString *a_property, CRTerm *a_value)
{
	CRDeclaration *result = NULL ;

	result = g_try_malloc (sizeof (CRDeclaration)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRDeclaration)) ;
	result->property = a_property ;
	result->value = a_value ;

	if (a_value)
	{
		cr_term_ref (a_value) ;
	}

	return result ;
}

/**
 *Appends a new declaration to the current declarations list.
 *@param a_this the current declaration list.
 *@param a_new the declaration to append.
 *@return the declaration list with a_new appended to it, or NULL
 *in case of error.
 */
CRDeclaration *
cr_declaration_append (CRDeclaration *a_this, CRDeclaration *a_new)
{
	CRDeclaration *cur = NULL ;

	g_return_val_if_fail (a_new, NULL) ;

	if (!a_this)
		return a_new ;

	for (cur = a_this ; cur && cur->next ; cur = cur->next) ;
	
	cur->next = a_new ;
	a_new->prev = cur ;

	return a_this ;
}

/**
 *prepends a declaration to the current declaration list.
 *@param a_this the current declaration list.
 *@param a_new the declaration to prepend.
 *@return the list with a_new prepended or NULL in case of error.
 */
CRDeclaration *
cr_declaration_prepend (CRDeclaration *a_this, CRDeclaration *a_new)
{
	CRDeclaration *cur = NULL ;

	g_return_val_if_fail (a_new, NULL) ;

	if (!a_this)
		return a_new ;

	a_this->prev = a_new ;
	a_new->next = a_this ;

	for (cur = a_new ; cur && cur->prev ; cur = cur->prev) ;

	return cur ;
}

/**
 *Appends a declaration to the current declaration list.
 *@param a_this the current declaration list.
 *@param a_prop the property string of the declaration to append.
 *@param a_value the value of the declaration to append.
 *@return the list with the new property appended to it, or NULL in
 *case of an error.
 */
CRDeclaration *
cr_declaration_append2 (CRDeclaration *a_this, GString *a_prop,
			CRTerm *a_value)
{
	CRDeclaration *new_elem = NULL ;

	new_elem = cr_declaration_new (a_prop, a_value) ;
	g_return_val_if_fail (new_elem, NULL) ;
	
	return cr_declaration_append (a_this, new_elem) ;
}

/**
 *Dumps a declaration list to a file.
 *@param a_this the current instance of #CRDeclaration.
 *@param a_fp the destination file.
 *@param a_indent the number of indentation white char.
 */
void
cr_declaration_dump (CRDeclaration *a_this, FILE *a_fp, glong a_indent,
		     gboolean a_one_per_line)
{
	CRDeclaration *cur = NULL ;

	g_return_if_fail (a_this) ;

	for (cur = a_this ; cur ; cur = cur->next)
	{
		if (cur->prev)
		{
			if (a_one_per_line == TRUE)
				fprintf (a_fp,";\n") ;
			else
				fprintf (a_fp,"; ") ;
		}
		dump (cur, a_fp, a_indent) ;
	}
}

/**
 *Increases the ref count of the current instance of #CRDeclaration.
 *@param a_this the current instance of #CRDeclaration.
 */
void 
cr_declaration_ref (CRDeclaration *a_this)
{
	g_return_if_fail (a_this) ;

	a_this->ref_count ++ ;
}

/**
 *Decrements the ref count of the current instance of #CRDeclaration.
 *If the ref count reaches zero, the current instance of #CRDeclaration
 *if destroyed.
 *@param a_this the current instance of #CRDeclaration.
 *@return TRUE if the current instance of #CRDeclaration has been destroyed
 *(ref count reached zero), FALSE otherwise.
 */
gboolean
cr_declaration_unref (CRDeclaration *a_this)
{
	g_return_val_if_fail (a_this, FALSE) ;

	if (a_this->ref_count)
	{
		a_this->ref_count -- ;
	}

	if (a_this->ref_count == 0)
	{
		cr_declaration_destroy (a_this) ;
		return TRUE ;
	}
	return FALSE ;
}


/**
 *Destructor of the declaration list.
 *@param a_this the current instance of #CRDeclaration.
 */
void
cr_declaration_destroy (CRDeclaration *a_this)
{
	CRDeclaration *cur = NULL ;
	g_return_if_fail (a_this) ;

	/*
	 *Go get the tail of the list.
	 *Meanwhile, free each property/value pair contained in the list.
	 */
	for (cur = a_this ; cur && cur->next; cur = cur->next)
	{
		if (cur->property)
		{
			g_string_free (cur->property, TRUE) ;
			cur->property = NULL ;
		}
		
		if (cur->value)
		{
			cr_term_destroy (cur->value) ;
			cur->value = NULL ;
		}
	}

	if (cur)
	{
		if (cur->property)
		{
			g_string_free (cur->property, TRUE) ;
			cur->property = NULL ;
		}
		
		if (cur->value)
		{
			cr_term_destroy (cur->value) ;
			cur->value = NULL ;
		}
	}

	/*in case the list contains only one element*/
	if (cur && !cur->prev)
	{
		g_free (cur) ;
		return ;
	}

	/*walk backward the list and free each "next" element*/
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
