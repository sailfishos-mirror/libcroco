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

static CRRgb gv_standard_colors[] = 
{
	{ "aliceblue",   240, 248, 255, 0},
	{ "antiquewhite",   250, 235, 215, 0},
	{ "aqua",     0, 255, 255, 0},
	{ "aquamarine",   127, 255, 212, 0},
	{ "azure",   240, 255, 255, 0},
	{ "beige",   245, 245, 220, 0},
	{ "bisque",   255, 228, 196, 0},
	{ "black",     0,   0,   0, 0},
	{ "blanchedalmond",   255, 235, 205, 0},
	{ "blue",     0,   0, 255, 0},
	{ "blueviolet",   138,  43, 226, 0},
	{ "brown",   165,  42,  42, 0},
	{ "burlywood",   222, 184, 135, 0},
	{ "cadetblue",    95, 158, 160, 0},
	{ "chartreuse",   127, 255,   0, 0},
	{ "chocolate",   210, 105,  30, 0},
	{ "coral",   255, 127,  80, 0},
	{ "cornflowerblue",   100, 149, 237, 0},
	{ "cornsilk",   255, 248, 220, 0},
	{ "crimson",   220,  20,  60, 0},
	{ "cyan",     0, 255, 255, 0},
	{ "darkblue",     0,   0, 139, 0},
	{ "darkcyan",     0, 139, 139, 0},
	{ "darkgoldenrod",   184, 134,  11, 0},
	{ "darkgray",   169, 169, 169, 0},
	{ "darkgreen",     0, 100,   0, 0},
	{ "darkgrey",   169, 169, 169, 0},
	{ "darkkhaki",   189, 183, 107, 0},
	{ "darkmagenta",   139,   0, 139, 0},
	{ "darkolivegreen",    85, 107,  47, 0},
	{ "darkorange",   255, 140, 0, 0},
	{ "darkorchid",   153,  50, 204, 0},
	{ "darkred",   139,   0,   0, 0},
	{ "darksalmon",   233, 150, 122, 0},
	{ "darkseagreen",   143, 188, 143, 0},
	{ "darkslateblue",    72,  61, 139, 0},
	{ "darkslategray",    47,  79,  79, 0},
	{ "darkslategrey",    47,  79,  79, 0},
	{ "darkturquoise",     0, 206, 209, 0},
	{ "darkviolet",   148,   0, 211, 0},
	{ "deeppink",   255,  20, 147, 0},
	{ "deepskyblue",     0, 191, 255, 0},
	{ "dimgray",   105, 105, 105, 0},
	{ "dimgrey",   105, 105, 105, 0},
	{ "dodgerblue",    30, 144, 255, 0},
	{ "firebrick",   178,  34,  34, 0},
	{ "floralwhite",   255, 250, 240, 0},
	{ "forestgreen",    34, 139,  34, 0},
	{ "fuchsia",   255,   0, 255, 0},
	{ "gainsboro",   220, 220, 220, 0},
	{ "ghostwhite",   248, 248, 255, 0},
	{ "gold",   255, 215,   0, 0},
	{ "goldenrod",   218, 165,  32, 0},
	{ "gray",   128, 128, 128, 0},
	{ "grey",   128, 128, 128, 0},
	{ "green",     0, 128,   0, 0},
	{ "greenyellow",   173, 255,  47, 0},
	{ "honeydew",   240, 255, 240, 0},
	{ "hotpink",   255, 105, 180, 0},
	{ "indianred",   205,  92,  92, 0},
	{ "indigo",    75,   0, 130, 0},
	{ "ivory",   255, 255, 240, 0},
	{ "khaki",   240, 230, 140, 0},
	{ "lavender",   230, 230, 250, 0},
	{ "lavenderblush",   255, 240, 245, 0},
	{ "lawngreen",   124, 252,   0, 0},
	{ "lemonchiffon",   255, 250, 205, 0},
	{ "lightblue",   173, 216, 230, 0},
	{ "lightcoral",   240, 128, 128, 0},
	{ "lightcyan",   224, 255, 255, 0},
	{ "lightgoldenrodyellow",   250, 250, 210, 0},
	{ "lightgray",   211, 211, 211, 0},
	{ "lightgreen",   144, 238, 144, 0},
	{ "lightgrey",   211, 211, 211, 0},
	{ "lightpink",   255, 182, 193, 0},
	{ "lightsalmon",   255, 160, 122, 0},
	{ "lightseagreen",    32, 178, 170, 0},
	{ "lightskyblue",   135, 206, 250, 0},
	{ "lightslategray",   119, 136, 153, 0},
	{ "lightslategrey",   119, 136, 153, 0},
	{ "lightsteelblue",   176, 196, 222, 0},
	{ "lightyellow",   255, 255, 224, 0},
	{ "lime",     0, 255,   0, 0},
	{ "limegreen",    50, 205,  50, 0},
	{ "linen",   250, 240, 230, 0},
	{ "magenta",   255,   0, 255, 0},
	{ "maroon",   128,   0,   0, 0},
	{ "mediumaquamarine",   102, 205, 170, 0},
	{ "mediumblue",     0,   0, 205, 0},
	{ "mediumorchid",   186,  85, 211, 0},
	{ "mediumpurple",   147, 112, 219, 0},
	{ "mediumseagreen",    60, 179, 113, 0},
	{ "mediumslateblue",   123, 104, 238, 0},
	{ "mediumspringgreen",     0, 250, 154, 0},
	{ "mediumturquoise",    72, 209, 204, 0},
	{ "mediumvioletred",   199,  21, 133, 0},
	{ "midnightblue",    25,  25, 112, 0},
	{ "mintcream",   245, 255, 250, 0},
	{ "mistyrose",   255, 228, 225, 0},
	{ "moccasin",   255, 228, 181, 0},
	{ "navajowhite",   255, 222, 173, 0},
	{ "navy",     0,   0, 128, 0},
	{ "oldlace",   253, 245, 230, 0},
	{ "olive",   128, 128,   0, 0},
	{ "olivedrab",   107, 142,  35, 0},
	{ "orange",   255, 165,   0, 0},
	{ "orangered",   255,  69,   0, 0},
	{ "orchid",   218, 112, 214, 0},
	{ "palegoldenrod",   238, 232, 170, 0},
	{ "palegreen",   152, 251, 152, 0},
	{ "paleturquoise",   175, 238, 238, 0},
	{ "palevioletred",   219, 112, 147, 0},
	{ "papayawhip",   255, 239, 213, 0},
	{ "peachpuff",   255, 218, 185, 0},
	{ "peru",   205, 133,  63, 0},
	{ "pink",   255, 192, 203, 0},
	{ "plum",   221, 160, 221, 0},
	{ "powderblue",   176, 224, 230, 0},
	{ "purple",   128,   0, 128, 0},
	{ "red",   255,   0,   0, 0},
	{ "rosybrown",   188, 143, 143, 0},
	{ "royalblue",    65, 105, 225, 0},
	{ "saddlebrown",   139,  69,  19, 0},
	{ "salmon",   250, 128, 114, 0},
	{ "sandybrown",   244, 164,  96, 0},
	{ "seagreen",    46, 139,  87, 0},
	{ "seashell",   255, 245, 238, 0},
	{ "sienna",   160,  82,  45, 0},
	{ "silver",   192, 192, 192, 0},
	{ "skyblue",   135, 206, 235, 0},
	{ "slateblue",   106,  90, 205, 0},
	{ "slategray",   112, 128, 144, 0},
	{ "slategrey",   112, 128, 144, 0},
	{ "snow",   255, 250, 250, 0},
	{ "springgreen",     0, 255, 127, 0},
	{ "steelblue",    70, 130, 180, 0},
	{ "tan",   210, 180, 140, 0},
	{ "teal",     0, 128, 128, 0},
	{ "thistle",   216, 191, 216, 0},
	{ "tomato",   255,  99,  71, 0},
	{ "turquoise",    64, 224, 208, 0},
	{ "violet",   238, 130, 238, 0},
	{ "wheat",   245, 222, 179, 0},
	{ "white",   255, 255, 255, 0},
	{ "whitesmoke",   245, 245, 245, 0},
	{ "yellow",   255, 255,   0, 0},
	{ "yellowgreen",   154, 205,  50, 0}
} ;

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


enum CRStatus
cr_rgb_set_from_name (CRRgb *a_this, const guchar *a_color_name)
{
        gulong i = 0 ;
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && a_color_name, CR_BAD_PARAM_ERROR) ;

        for (i = 0 ; i < sizeof (gv_standard_colors); i++)
        {
                if (!strcmp (a_color_name, gv_standard_colors[i].name))
                {
                        cr_rgb_set_from_rgb (a_this, &gv_standard_colors[i]) ;
                        break ;
                }                
        }

        if (i < sizeof (gv_standard_colors))
                status = CR_OK ;
        else
                status = CR_UNKNOWN_TYPE_ERROR ;

        return status ;
}

enum CRStatus
cr_rgb_set_from_hex_str (CRRgb *a_this, const guchar * a_hex)
{
        enum CRStatus status = CR_OK ;
        gulong i = 0 ;
        guchar colors[3] = {0} ;

        g_return_val_if_fail (a_this && a_hex,
                              CR_BAD_PARAM_ERROR) ;

        if (strlen (a_hex) == 3)
        {                
                for (i = 0 ;i < 3 ; i++)
                {
                        switch (a_hex[i])
                        {
                        case '0' ... '9':
                                colors[i] = a_hex[i] - '0';
                                colors[i] = (colors[i] << 4) | colors[i] ;
                                break ;

                        case 'a' ... 'z':
                                colors[i] = 10 + a_hex[i] - 'a';
                                colors[i] = (colors[i] << 4) | colors[i] ;
                                break ;

                        case 'A' ... 'Z':
                                colors[i] = 10 + a_hex[i] - 'A';
                                colors[i] = (colors[i] << 4) | colors[i] ;
                                break ;
                        default:
                                status = CR_UNKNOWN_TYPE_ERROR ;
                        }
                }
        }
        else if (strlen (a_hex) == 6)
        {
                for (i = 0 ; i < 6 ; i++)
                {
                        
                        switch (a_hex[i])
                        {
                        case '0' ... '9':
                                colors[i/2] <<= 4 ;
                                colors[i/2] |= a_hex[i] - '0';
                                status = CR_OK ;
                                break ;

                        case 'a' ... 'z':
                                colors[i/2] <<= 4 ;
                                colors[i/2] |= 10 + a_hex[i] - 'a';
                                status = CR_OK ;
                                break ;

                        case 'A' ... 'Z':
                                colors[i/2] <<= 4 ;
                                colors[i/2] |= 10 + a_hex[i] - 'A';
                                status = CR_OK ;
                                break ;

                        default:
                                status = CR_UNKNOWN_TYPE_ERROR ;
                        }                        
                }
        }
        else
        {
                status = CR_UNKNOWN_TYPE_ERROR ;
        }
        
        if (status == CR_OK)
        {
                status = cr_rgb_set (a_this, colors[0],
                                     colors[1], colors[2],
                                     FALSE) ;
        }
        return status ;
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
