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
cr_rgb_new_with_vals (gulong a_red, gulong a_green, 
                      gulong a_blue, gboolean a_is_percentage)
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
 *Serialises the rgb into a zero terminated string.
 *@param a_this the instance of #CRRgb to serialize.
 *@return the zero terminated string containing the serialised
 *rgb. MUST BE FREED by the caller using g_free().
 */
guchar *
cr_rgb_to_string (CRRgb *a_this)
{
        guchar *result = NULL ;
        GString *str_buf = NULL ;

        str_buf = g_string_new (NULL) ;
        g_return_val_if_fail (str_buf, NULL) ;

        if (a_this->is_percentage == TRUE)
        {
                g_string_append_printf (str_buf, "%ld", a_this->red) ;

                g_string_append_c (str_buf, '%') ;
                g_string_append_printf (str_buf,", ") ;
                
                g_string_append_printf (str_buf,"%ld", a_this->green) ;
                g_string_append_c (str_buf, '%') ; ;
                g_string_append_printf (str_buf,", ") ;

                g_string_append_printf (str_buf,"%ld", a_this->blue) ;
                g_string_append_c (str_buf, '%') ;
        }
        else
        {
                g_string_append_printf (str_buf,"%ld", a_this->red) ;
                g_string_append_printf (str_buf,", ") ;
                
                g_string_append_printf (str_buf,"%ld", a_this->green) ;
                g_string_append_printf (str_buf,", ") ;

                g_string_append_printf (str_buf,"%ld", a_this->blue) ;
        }

        if (str_buf)
        {
                result = str_buf->str ;
                g_string_free (str_buf, FALSE) ;                
        }

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
        guchar *str = NULL ;

        g_return_if_fail (a_this) ;

        str = cr_rgb_to_string (a_this) ;

        if (str)
        {
                fprintf (a_fp, "%s",str) ;
                g_free (str) ;
                str = NULL ;
        }
}

/**
 *Sets rgb values to the RGB.
 *If the rgb values are percentages, make
 *sure that the sum of the 3 values makes 100%.
 *@param a_this the current instance of #CRRgb.
 *@param a_red the red value.
 *@param a_green the green value.
 *@param a_blue the blue value.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_rgb_set (CRRgb *a_this, gulong a_red,
            gulong a_green, gulong a_blue,
            gboolean a_is_percentage)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;
        if (a_is_percentage != FALSE)
        {
                g_return_val_if_fail (a_red <= 100
                                      && a_green <= 100
                                      && a_blue <= 100,
                                      CR_BAD_PARAM_ERROR) ;
        }

        a_this->is_percentage = a_is_percentage ;

        a_this->red = a_red ;
        a_this->green = a_green ;
        if (a_is_percentage != FALSE)
        {
                if (a_red + a_green >= 100)
                {
                        a_green = 100 - a_red ;
                }
                a_this->blue = 100 - a_red - a_green ;
        }
        else
        {
                a_this->blue = a_blue ;
        }

        return CR_OK ;
}

/**
 *Sets the rgb from an other one.
 *@param a_this the current instance of #CRRgb.
 *@param a_rgb the rgb to "copy"
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_rgb_set_from_rgb (CRRgb *a_this, CRRgb *a_rgb)
{
        g_return_val_if_fail (a_this && a_rgb,
                              CR_BAD_PARAM_ERROR) ;

        cr_rgb_set (a_this, a_rgb->red, a_rgb->green,
                    a_rgb->blue, a_rgb->is_percentage) ;

        return CR_OK ;
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
