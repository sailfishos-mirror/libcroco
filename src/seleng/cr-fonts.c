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

#include "cr-fonts.h"
#include <string.h>

CRFontFamily *
cr_font_family_new (enum CRFontFamilyType a_type, guchar *a_name)
{
	CRFontFamily *result = NULL ;

	result = g_try_malloc (sizeof (CRFontFamily)) ;

	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}

	memset (result, 0, sizeof (CRFontFamily)) ;
	result->type = a_type ;

	cr_font_family_set_name (result, a_name) ;

	return result ;
}

enum CRStatus
cr_font_family_set_name (CRFontFamily *a_this, guchar *a_name)
{
	g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

	/*
	 *only non generic font families can have a name
	 */

	if (a_this->type != FONT_FAMILY_NON_GENERIC)
	{
		return CR_BAD_PARAM_ERROR ;
	}

	if (a_this->name)
	{
		g_free (a_this->name) ;
		a_this->name = NULL ;
	}
       
	a_this->name = a_name ;
	return CR_OK ;
}


CRFontFamily *
cr_font_family_append (CRFontFamily *a_this, 
		       CRFontFamily *a_family_to_append)
{
	CRFontFamily *cur_ff = NULL ;

	g_return_val_if_fail (a_family_to_append,
			      NULL) ;

	if (!a_this)
		return a_family_to_append ;

	for (cur_ff = a_this ; 
	     cur_ff && cur_ff->next; 
	     cur_ff = cur_ff->next ) ;
	
	cur_ff->next = a_family_to_append;
	a_family_to_append->prev = cur_ff ;

	return a_this ;

	
}

CRFontFamily *
cr_font_family_prepend (CRFontFamily *a_this, 
			CRFontFamily *a_family_to_prepend)
{
	g_return_val_if_fail (a_this && a_family_to_prepend,
			      NULL) ;

	if (!a_this)
		return a_family_to_prepend ;

	a_family_to_prepend->next = a_this ;
	a_this->prev = a_family_to_prepend ;

	return CR_OK ;	
}


enum CRStatus
cr_font_family_destroy (CRFontFamily *a_this)
{
	CRFontFamily *cur_ff = NULL ;

	g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

	for (cur_ff = a_this ; 
	     cur_ff && cur_ff->next ;
	     cur_ff = cur_ff->next) 
		;

	for (; cur_ff ; cur_ff = cur_ff->prev)
	{
		if (a_this->name)
		{
			g_free (a_this->name) ;
			a_this->name = NULL ;
		}

		if (cur_ff->next)
		{
			g_free (cur_ff->next) ;
			
		}

		if (cur_ff->prev == NULL)
		{
			g_free (a_this) ;
		}
	}

	return CR_OK ;		
}

/***************************************************
 *'font-size' manipulation functions definitions
 ***************************************************/

CRFontSize *
cr_font_size_new (void)
{
        CRFontSize *result = NULL ;

        result = g_try_malloc (sizeof (CRFontSize)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }
        memset (result, 0, sizeof (CRFontSize)) ;

        return result ;        
}

enum CRStatus
cr_font_size_clear (CRFontSize *a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        switch (a_this->type)
        {
        case PREDEFINED_ABSOLUTE_FONT_SIZE:
        case RELATIVE_FONT_SIZE:
        case INHERITED_FONT_SIZE:
                memset (a_this, 0, sizeof (CRFontSize)) ;
                break ;

        case ABSOLUTE_FONT_SIZE:
                if (a_this->value.absolute)
                {
                        cr_num_destroy (a_this->value.absolute) ;
                }
                memset (a_this, 0, sizeof (CRFontSize)) ;
                break ;

        default:
                return CR_UNKNOWN_TYPE_ERROR ;
        }

        return CR_OK ;
}

enum CRStatus
cr_font_size_copy (CRFontSize *a_dst, CRFontSize *a_src)
{
        g_return_val_if_fail (a_dst && a_src, CR_BAD_PARAM_ERROR) ;

        switch (a_src->type)
        {
        case PREDEFINED_ABSOLUTE_FONT_SIZE:
        case RELATIVE_FONT_SIZE:
        case INHERITED_FONT_SIZE:
                cr_font_size_clear (a_dst) ;
                memcpy (a_dst, a_src, sizeof (CRFontSize)) ;
                break ;

        case ABSOLUTE_FONT_SIZE:                
                if (a_src->value.absolute)
                {
                        cr_font_size_clear (a_dst) ;
                        if (!a_dst->value.absolute)
                        {
                                a_dst->value.absolute = cr_num_new () ;
                        }
                        cr_num_copy (a_dst->value.absolute,
                                     a_src->value.absolute) ;
                        a_dst->type = a_src->type ;
                }
                break ;

        default:
                return CR_UNKNOWN_TYPE_ERROR ;
        }
        return CR_OK ;
}

void
cr_font_size_destroy (CRFontSize *a_font_size)
{
        g_return_if_fail (a_font_size) ;

        if (a_font_size->type == ABSOLUTE_FONT_SIZE
            && a_font_size->value.absolute)
        {
                cr_num_destroy (a_font_size->value.absolute) ;
                a_font_size->value.absolute = NULL ;
        }
}

/*******************************************************
 *'font-size-adjust' manipulation function definition
 *******************************************************/

CRFontSizeAdjust *
cr_font_size_adjust_new (void)
{
        CRFontSizeAdjust *result = NULL ;

        result = g_try_malloc (sizeof (CRFontSizeAdjust)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }
        memset (result, 0, sizeof (CRFontSizeAdjust)) ;

        return result ;
}

void
cr_font_size_adjust_destroy (CRFontSizeAdjust *a_this)
{
        g_return_if_fail (a_this) ;

        if (a_this->type == FONT_SIZE_ADJUST_NUMBER
            && a_this->num)
        {
                cr_num_destroy (a_this->num) ;
                a_this->num = NULL ;
        }
}
