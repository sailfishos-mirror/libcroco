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

#include <stdio.h>
#include <string.h>
#include "cr-rgb.h"

/**
 *The default constructor of #CRRgb.
 *@return the newly built instance of #CRRgb
 */
CRRgb *
cr_rgb_new (void)
{
        CRRgb *result = NULL ;

        result = g_try_malloc (sizeof (CRRgb)) ;

        if (result == NULL)
        {
                cr_utils_trace_info ("No more memory") ;
                return NULL ;
        }
        
        memset (result, 0, sizeof (CRRgb)) ;

        return result ;
}


/**
 *A constructor of #CRRgb.
 *@param a_red the red component of the color.
 *@param a_green the green component of the color.
 *@param a_blue the blue component of the color.
 *@param a_unit the unit of the rgb values.
 *(either percentage or integer values)
 *@return the newly built instance of #CRRgb.
 */
CRRgb *
cr_rgb_new_with_vals (glong a_red, glong a_green, 
                      glong a_blue, gboolean a_is_percentage)
{
        CRRgb *result =  NULL ;

        result = cr_rgb_new () ;
        
        g_return_val_if_fail (result, NULL) ;
        
        result->red = a_red ;
        result->green = a_green ;
        result->blue = a_blue ;
        result->is_percentage = a_is_percentage ;

        return result ;
}


/**
 *Dumps the current instance of #CRRgb
 *to a file.
 *@param a_this the "this pointer" of
 *the current instance of #CRRgb.
 *@param a_fp the destination file pointer.
 */
void
cr_rgb_dump (CRRgb *a_this, FILE *a_fp)
{
        g_return_if_fail (a_this) ;

        if (a_this->is_percentage == TRUE)
        {
                fprintf (a_fp,"%ld", a_this->red) ;
                fputc ('%', a_fp) ;
                fprintf (a_fp,", ") ;
                
                fprintf (a_fp,"%ld", a_this->green) ;
                fputc ('%', a_fp) ; ;
                fprintf (a_fp,", ") ;

                fprintf (a_fp,"%ld", a_this->blue) ;
                fputc ('%', a_fp) ;
        }
        else
        {
                fprintf (a_fp,"%ld", a_this->red) ;
                fprintf (a_fp,", ") ;
                
                fprintf (a_fp,"%ld", a_this->green) ;
                fprintf (a_fp,", ") ;

                fprintf (a_fp,"%ld", a_this->blue) ;
        }
}

/**
 *Destructor of #CRRgb.
 *@param a_this the "this pointer" of the
 *current instance of #CRRgb.
 */
void
cr_rgb_destroy (CRRgb *a_this)
{
        g_return_if_fail (a_this) ;
        
        g_free (a_this) ;
}
