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
 *The definition
 *of the #CRNum class.
 */

#include "cr-num.h"
#include "string.h"


/**
 *The default constructor of
 *#CRNum.
 *@return the newly built instance of
 *#CRNum.
 */
CRNum *
cr_num_new (void)
{
	CRNum * result = NULL ;
	
	result = g_try_malloc (sizeof (CRNum)) ;

	if (result == NULL)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}

	memset (result, 0, sizeof (CRNum)) ;

	return result ;	
}


/**
 *A constructor of #CRNum.
 *@param a_is_natural whether if the intance of #CRNum is 
 *a natural number or not.
 *@param a_integer_part the integer part of the instance 
 *of #CRNum
 *@param a_decimal_part in case the instance of #CRNum
 *natural number (but a decimal one) this parameter
 *is the decimal part of the instance of #CRNum.
 *@return the newly built instance of #CRNum or
 *NULL if an error arises.
 */
CRNum *
cr_num_new_with_vals (gboolean a_is_natural,
		      glong a_integer_part,
		      glong a_decimal_part)
{
	CRNum * result = NULL ;

	result = cr_num_new () ;
	
	g_return_val_if_fail (result, NULL) ;

	result->is_natural = a_is_natural ;
	result->int_part = a_integer_part ;

	if (a_is_natural == FALSE)
	{
		result->dec_part = a_decimal_part ;
	}

	return result ;
}

/**
 *Returns the string representation of the
 *current instance of #CRNum.
 *@param a_this the current instance of #CRNum.
 *@return the newly built string representation
 *of the current instance of #CRNum. The returned
 *string is NULL terminated. The caller *must*
 *free the returned string.
 */
guchar *
cr_num_to_string (CRNum *a_this)
{
        guchar *tmp_char1 = NULL, *tmp_char2 = NULL, *result = NULL ;

        g_return_val_if_fail (a_this, NULL) ;
        
        tmp_char1 = g_strdup_printf ("%ld", a_this->int_part) ;
        
        g_return_val_if_fail (tmp_char1, NULL) ;

        if (a_this->is_natural == FALSE)
        {                
                tmp_char2 = g_strdup_printf (".%ld", a_this->dec_part) ;
        }

        if (tmp_char2)
        {
                result = g_strconcat (tmp_char1, tmp_char2, NULL) ;
                g_free (tmp_char1) ;
                g_free (tmp_char2) ;
        }
        else
        {
                result = tmp_char1 ;
        }

        return result ;
}

/**
 *The destructor of #CRNum.
 *@param a_this the this pointer of
 *the current instance of #CRNum.
 */
void
cr_num_destroy (CRNum *a_this)
{
	g_return_if_fail (a_this) ;

	g_free (a_this) ;
}
