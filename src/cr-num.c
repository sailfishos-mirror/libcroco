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
cr_num_new_with_val (gdouble a_val,
                     enum CRNumType a_type)
{
	CRNum * result = NULL ;

	result = cr_num_new () ;
	
	g_return_val_if_fail (result, NULL) ;

        result->val = a_val ;
        result->type = a_type ;

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
        gdouble test_val = 0.0 ;

        guchar *tmp_char1 = NULL, * tmp_char2 = NULL, *result = NULL ;

        g_return_val_if_fail (a_this, NULL) ;
        
        test_val = a_this->val - (glong) a_this->val ;

        if (!test_val)
        {
                tmp_char1 = g_strdup_printf ("%ld", (glong)a_this->val) ;
        }        
        else
        {
                tmp_char1 = g_strdup_printf ("%.3f", a_this->val) ;
        }
        
        g_return_val_if_fail (tmp_char1, NULL) ;

        switch (a_this->type)
        {
        case NUM_LENGTH_EM:                
                tmp_char2 = (guchar*) "em";
                break ;

        case NUM_LENGTH_EX:
                tmp_char2 = (guchar*) "ex";
                break ;
                
        case NUM_LENGTH_PX:
                tmp_char2 = (guchar*) "px";
                break ;

        case NUM_LENGTH_IN:
                tmp_char2 = (guchar*) "in";
                break ;
                
        case NUM_LENGTH_CM:
                tmp_char2 = (guchar*) "cm";
                break ;

        case NUM_LENGTH_MM:
                tmp_char2 = (guchar*) "mm";
                break ;

        case NUM_LENGTH_PT:
                tmp_char2 = (guchar*) "pt";
                break ;

        case NUM_LENGTH_PC:
                tmp_char2 = (guchar*) "pc";
                break ;
                
        case NUM_ANGLE_DEG:
                tmp_char2 = (guchar*) "deg";
                break ;

        case NUM_ANGLE_RAD:
                tmp_char2 = (guchar*) "rad";
                break ;

        case NUM_ANGLE_GRAD:
                tmp_char2 = (guchar*) "grad";
                break ;

        case NUM_TIME_MS:
                tmp_char2 = (guchar*) "ms";
                break ;

        case NUM_TIME_S:
                tmp_char2 = (guchar*) "s";
                break ;

        case NUM_FREQ_HZ:
                tmp_char2 = (guchar*) "Hz";
                break ;

        case NUM_FREQ_KHZ:
                tmp_char2 = (guchar*) "KHz";
                break ;

        case NUM_PERCENTAGE:
                tmp_char2 = (guchar*) "%";
                break ;

        default:
                break ;
        }

        if (tmp_char2)
        {
                result = g_strconcat (tmp_char1, tmp_char2, NULL) ;
                g_free (tmp_char1) ;                
        }
        else
        {
                result = tmp_char1 ;
        }

        return result ;
}


enum CRStatus
cr_num_copy (CRNum *a_dest, CRNum *a_src)
{
        g_return_val_if_fail (a_dest && a_src,
                              CR_BAD_PARAM_ERROR) ;

        memcpy (a_dest, a_src, sizeof (CRNum)) ;

        return CR_OK ;
}


enum CRStatus
cr_num_set (CRNum *a_this, gdouble a_val, enum CRNumType a_type)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        a_this->val= a_val ;
        a_this->type = a_type ;

        return CR_OK ;
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
