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

/*
 *$Id$
 */
#include <string.h>
#include "cr-style.h"

/**
 *@file
 *The definition of the #CRStyle class.
 */


/**
 *A property ID.
 *Each supported css property has an ID which is
 *an entry into a property "population" jump table.
 *each entry of the property population jump table
 *contains code to tranform the literal form of
 *a property value into a strongly typed value.
 */
enum CRPropertyID
{
        PROP_ID_NOT_KNOWN = 0,
	PROP_ID_PADDING_TOP,
	PROP_ID_PADDING_RIGHT,
	PROP_ID_PADDING_BOTTOM,
	PROP_ID_PADDING_LEFT,
	PROP_ID_BORDER_TOP_WIDTH,
	PROP_ID_BORDER_RIGHT_WIDTH,
	PROP_ID_BORDER_BOTTOM_WIDTH,
	PROP_ID_BORDER_LEFT_WIDTH,
	PROP_ID_BORDER_TOP_STYLE,
	PROP_ID_BORDER_RIGHT_STYLE,
	PROP_ID_BORDER_BOTTOM_STYLE,
	PROP_ID_BORDER_LEFT_STYLE,
        PROP_ID_BORDER_TOP_COLOR,
        PROP_ID_BORDER_RIGHT_COLOR,
        PROP_ID_BORDER_BOTTOM_COLOR,
        PROP_ID_BORDER_LEFT_COLOR,
        PROP_ID_BORDER_TOP,
        PROP_ID_BORDER_RIGHT,
        PROP_ID_BORDER_BOTTOM,
        PROP_ID_BORDER_LEFT,
        PROP_ID_BORDER,
	PROP_ID_MARGIN_TOP,
	PROP_ID_MARGIN_RIGHT,
	PROP_ID_MARGIN_BOTTOM,
	PROP_ID_MARGIN_LEFT,
	PROP_ID_DISPLAY,
	PROP_ID_POSITION,
	PROP_ID_TOP,
	PROP_ID_RIGHT,
	PROP_ID_BOTTOM,
	PROP_ID_LEFT,
	PROP_ID_FLOAT,
	PROP_ID_WIDTH,
        PROP_ID_COLOR,
        PROP_ID_BACKGROUND_COLOR,

        /*should be the last one.*/
        NB_PROP_IDS
} ;


typedef struct _CRPropertyDesc CRPropertyDesc ;

struct _CRPropertyDesc
{
	const guchar * name ;
	enum CRPropertyID prop_id ;
} ;

static CRPropertyDesc gv_prop_table [] =

{
	{"padding-top", PROP_ID_PADDING_TOP},
	{"padding-right", PROP_ID_PADDING_RIGHT},
	{"padding-bottom", PROP_ID_PADDING_BOTTOM},
	{"padding-left", PROP_ID_PADDING_LEFT},
	{"border-top-width", PROP_ID_BORDER_TOP_WIDTH},
	{"border-right-width", PROP_ID_BORDER_RIGHT_WIDTH},
	{"border-bottom-width", PROP_ID_BORDER_BOTTOM_WIDTH},
	{"border-left-width", PROP_ID_BORDER_LEFT_WIDTH},
	{"border-top-style", PROP_ID_BORDER_TOP_STYLE},
	{"border-right-style", PROP_ID_BORDER_RIGHT_STYLE},
	{"border-bottom-style", PROP_ID_BORDER_BOTTOM_STYLE},
	{"border-left-style", PROP_ID_BORDER_LEFT_STYLE},
        {"border-top", PROP_ID_BORDER_TOP},
        {"border-right", PROP_ID_BORDER_RIGHT},
        {"border-bottom", PROP_ID_BORDER_BOTTOM},
        {"border-left", PROP_ID_BORDER_LEFT},
        {"border", PROP_ID_BORDER},
	{"margin-top", PROP_ID_MARGIN_TOP},
	{"margin-right", PROP_ID_MARGIN_RIGHT},
	{"margin-bottom", PROP_ID_MARGIN_BOTTOM},
	{"margin-left", PROP_ID_MARGIN_LEFT},
	{"display", PROP_ID_DISPLAY},
	{"position", PROP_ID_POSITION},
	{"top", PROP_ID_TOP},
	{"right", PROP_ID_RIGHT},
	{"bottom", PROP_ID_BOTTOM},
	{"left", PROP_ID_LEFT},
	{"float", PROP_ID_FLOAT},
	{"width", PROP_ID_WIDTH},
        {"color", PROP_ID_COLOR},
        {"background-color", PROP_ID_BACKGROUND_COLOR},

        /*must be the last one*/
	{NULL, 0}
} ;

/**
 *A the key/value pair of this hash table
 *are:
 *key => name of the the css propertie found in gv_prop_table
 *value => matching property id found in gv_prop_table.
 *So this hash table is here just to retrieval of a property id
 *from a property name.
 */
static GHashTable *gv_prop_hash = NULL ;

/**
 *incremented by each new instance of #CRStyle
 *and decremented at the it destroy time.
 *When this reaches zero, gv_prop_hash is destroyed.
 */
static gulong gv_prop_hash_ref_count = 0 ;

static enum CRStatus
cr_style_init_properties (void) ;

enum CRDirection
{
        DIR_TOP = 0,
        DIR_RIGHT,
        DIR_BOTTOM,
        DIR_LEFT,

        /*must be the last one*/
        NB_DIRS
} ;

static enum CRStatus
cr_style_set_props_to_defaults (CRStyle *a_this) ;

static enum CRStatus
set_prop_padding_x_from_value (CRStyle *a_style,                          
                               CRTerm *a_value,
                               enum CRDirection a_dir) ;

static enum CRStatus
set_prop_border_x_width_from_value (CRStyle *a_style,
                                    CRTerm *a_value,
                                    enum CRDirection a_dir) ;

static enum CRStatus
set_prop_border_x_style_from_value (CRStyle *a_style,
                                    CRTerm *a_value,
                                    enum CRDirection a_dir) ;

static enum CRStatus
set_prop_margin_x_from_value (CRStyle *a_style, CRTerm *a_value,
                              enum CRDirection a_dir) ;

static enum CRStatus
set_prop_display_from_value (CRStyle *a_style, CRTerm *a_value) ;

static enum CRStatus
set_prop_position_from_value (CRStyle *a_style, CRTerm *a_value) ;

static enum CRStatus
set_prop_x_from_value (CRStyle *a_style, CRTerm *a_value,
                       enum CRDirection a_dir) ;

static enum CRStatus
set_prop_float (CRStyle *a_style, CRTerm *a_value) ;

static enum CRStatus
set_prop_width (CRStyle *a_style, CRTerm *a_value) ;

static enum CRStatus
set_prop_color_rgb (CRStyle *a_style, CRTerm *a_value) ;

static enum CRStatus
set_prop_background_color_rgb (CRStyle *a_style, CRTerm *a_value) ;

static enum CRStatus
set_prop_border_x_color_from_value (CRStyle *a_style, CRTerm *a_value,
                                    enum CRDirection a_dir) ;

static enum CRStatus
set_prop_border_x_from_value (CRStyle *a_style, CRTerm *a_value,
                              enum CRDirection a_dir) ;

static enum CRStatus
set_prop_border_from_value (CRStyle *a_style, CRTerm *a_value) ;

static enum CRStatus
cr_style_init_properties (void)
{

	if (!gv_prop_hash)
        {
                gulong i = 0 ;

                gv_prop_hash = g_hash_table_new (g_str_hash,
                                                 g_str_equal) ;
                if (!gv_prop_hash)
                {
                        cr_utils_trace_info ("Out of memory") ;
                        return CR_ERROR ;
                }

                /*load gv_prop_hash from gv_prop_table*/
                for (i = 0 ; gv_prop_table[i].name ; i++)
                {
                        g_hash_table_insert 
                                (gv_prop_hash,
                                 (gpointer)gv_prop_table[i].name,
                                 GINT_TO_POINTER 
                                 (gv_prop_table[i].prop_id)) ;
                }
        }

        return CR_OK ;
}

/**
 *Sets the style properties to their default values
 *according to the css2 spec.
 *@param a_this the current instance of #CRStyle.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_style_set_props_to_defaults (CRStyle *a_this)
{
        glong i = 0 ;

        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        for (i = 0 ; i < NB_NUM_PROPS ; i++)
        {
                switch (i)
                {
                case NUM_PROP_WIDTH:
                case NUM_PROP_TOP:
                case NUM_PROP_RIGHT:
                case NUM_PROP_BOTTOM:
                case NUM_PROP_LEFT:
                        cr_num_set (&a_this->num_props[i].sv,
                                    0, NUM_AUTO) ;
                        break ;

                case NUM_PROP_PADDING_TOP:
                case NUM_PROP_PADDING_RIGHT:
                case NUM_PROP_PADDING_BOTTOM:
                case NUM_PROP_PADDING_LEFT:
                case NUM_PROP_BORDER_TOP:
                case NUM_PROP_BORDER_RIGHT:
                case NUM_PROP_BORDER_BOTTOM:
                case NUM_PROP_BORDER_LEFT:
                case NUM_PROP_MARGIN_TOP:
                case NUM_PROP_MARGIN_RIGHT:
                case NUM_PROP_MARGIN_BOTTOM:
                case NUM_PROP_MARGIN_LEFT:                
                        cr_num_set (&a_this->num_props[i].sv,
                                    0, NUM_LENGTH_PX) ;
                        break ;

                default:
                        cr_utils_trace_info ("Unknown property") ;
                        break ;
                }
        }

        for (i = 0 ; i < NB_RGB_PROPS ; i++)
        {
                
                switch (i)
                {
                        /*default foreground color is black*/
                case RGB_PROP_COLOR:
                        cr_rgb_set (&a_this->rgb_props[i].sv, 0, 0, 0,
                                    FALSE) ;
                        break ;
                        
                        /*default background color is white*/
                case RGB_PROP_BACKGROUND_COLOR:
                        cr_rgb_set (&a_this->rgb_props[i].sv, 
                                    255, 255, 255, FALSE) ;
                        break ;

                default:
                        cr_rgb_set (&a_this->rgb_props[i].sv, 0, 0, 0,
                                    FALSE) ;
                break ;
                }
        }

        for (i = 0 ; i < NB_BORDER_STYLE_PROPS ; i++)
        {
                a_this->border_style_props[i] = BORDER_STYLE_NONE ;
        }

        a_this->display = DISPLAY_INLINE ;
        a_this->position = POSITION_STATIC ;
        a_this->float_type = FLOAT_NONE ;
        a_this->parent_style = NULL ;

        return CR_OK ;
}


static enum CRPropertyID
cr_style_get_prop_id (const guchar * a_prop)
{
        gpointer * raw_id = NULL ;

        if (!gv_prop_hash)
        {
                cr_style_init_properties () ;
        }

        raw_id = g_hash_table_lookup (gv_prop_hash,
                                      a_prop) ;
        if (!raw_id)
        {
                return PROP_ID_NOT_KNOWN ;
        }
        return GPOINTER_TO_INT (raw_id) ;
}


static enum CRStatus
set_prop_padding_x_from_value (CRStyle *a_style,
                               CRTerm *a_value,
                               enum CRDirection a_dir)
{
        enum CRStatus status = CR_OK ;
        CRNum *num_val = NULL, *parent_num_val = NULL ;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR) ;

        if (a_value->type != TERM_NUMBER
            && a_value->type != TERM_IDENT)
                return CR_BAD_PARAM_ERROR ;

        switch (a_dir)
        {
        case DIR_TOP:
                num_val = &a_style->num_props[NUM_PROP_PADDING_TOP].sv ;
                parent_num_val = 
                        &a_style->parent_style->
                        num_props[NUM_PROP_PADDING_TOP].sv ;
                break ;
                
        case DIR_RIGHT:
                num_val = &a_style->num_props[NUM_PROP_PADDING_RIGHT].sv ;
                parent_num_val = 
                        &a_style->parent_style-> 
                        num_props[NUM_PROP_PADDING_RIGHT].sv;

                num_val = &a_style->num_props[NUM_PROP_PADDING_RIGHT].sv ;
                parent_num_val =
                        &a_style->parent_style->
                        num_props[NUM_PROP_PADDING_RIGHT].sv ;
                break ;

        case DIR_BOTTOM:
                num_val = 
                        &a_style->num_props[NUM_PROP_PADDING_BOTTOM].sv ;
                parent_num_val = 
                        &a_style->parent_style->
                        num_props[NUM_PROP_PADDING_BOTTOM].sv ;
                break ;

        case DIR_LEFT:
                num_val = & a_style->num_props[NUM_PROP_PADDING_LEFT].sv ;
                parent_num_val = 
                        &a_style->parent_style->
                        num_props[NUM_PROP_PADDING_LEFT].sv ;
                break ;

        default:
                return CR_BAD_PARAM_ERROR ;
        }

        if (a_value->type == TERM_IDENT)
        {
                if (a_value->content.str
                    && a_value->content.str->str
                    && !strncmp ((guchar*)"inherited", 
                                 a_value->content.str->str,
                                 strlen ("inherited")))
                {
                        cr_num_copy (num_val, parent_num_val) ;
                        return CR_OK ;
                }
                else
                        return CR_UNKNOWN_TYPE_ERROR ;
        }

        g_return_val_if_fail (a_value->type == TERM_NUMBER
                              && a_value->content.num,
                              CR_UNKNOWN_TYPE_ERROR) ;

        switch (a_value->content.num->type)
        {
        case NUM_LENGTH_EM:
        case NUM_LENGTH_EX:
        case NUM_LENGTH_PX:
        case NUM_LENGTH_IN:
        case NUM_LENGTH_CM:
        case NUM_LENGTH_MM:
        case NUM_LENGTH_PT:
        case NUM_LENGTH_PC:
        case NUM_PERCENTAGE:
                status = cr_num_copy (num_val, a_value->content.num) ;
                break ;
        default:
                status = CR_UNKNOWN_TYPE_ERROR ;
                break ;
        }

        return status ;
}


static enum CRStatus
set_prop_border_x_width_from_value (CRStyle *a_style,
                                    CRTerm *a_value,
                                    enum CRDirection a_dir)
{
        enum CRStatus status = CR_OK ;
        CRNum *num_val = NULL ;

        g_return_val_if_fail (a_value
                              && a_style,
                              CR_BAD_PARAM_ERROR) ;

        switch (a_dir)
        {
        case DIR_TOP:
                num_val = &a_style->num_props[NUM_PROP_BORDER_TOP].sv ;
                break ;

        case DIR_RIGHT:
                num_val = 
                        &a_style->num_props[NUM_PROP_BORDER_RIGHT].sv ;
                break ;

        case DIR_BOTTOM:
                num_val = &a_style->num_props[NUM_PROP_BORDER_BOTTOM].sv ;
                break ;

        case DIR_LEFT:
                num_val = &a_style->num_props[NUM_PROP_BORDER_LEFT].sv ;
                break ;

        default:
                return CR_BAD_PARAM_ERROR ;
                break ;
        }


        if (a_value->type == TERM_IDENT)
        {
                if (a_value->content.str && a_value->content.str->str)
                {
                        if (!strncmp ("thin", 
                                      a_value->content.str->str,
                                      strlen ("thin")))
                        {
                                cr_num_set (num_val, BORDER_THIN,
                                            NUM_LENGTH_PX) ;
                        }
                        else if (!strncmp ("medium",
                                           a_value->content.str->str,
                                           strlen ("medium")))
                        {
                                cr_num_set (num_val, BORDER_MEDIUM,
                                            NUM_LENGTH_PX) ;
                        }
                        else if (!strncmp ("thick",
                                           a_value->content.str->str,
                                           strlen ("thick")))
                        {
                                cr_num_set (num_val, BORDER_THICK,
                                            NUM_LENGTH_PX) ;
                        }
                        else
                        {
                                return CR_UNKNOWN_TYPE_ERROR ;
                        }
                }
        }
        else if (a_value->type != TERM_NUMBER
                 || a_value->content.num == NULL)
        {
                return CR_UNKNOWN_TYPE_ERROR ;
        }
        
        return status ;
}


static enum CRStatus
set_prop_border_x_style_from_value (CRStyle *a_style,
                                    CRTerm *a_value,
                                    enum CRDirection a_dir)
{
        enum CRStatus status = CR_OK ;
        enum CRBorderStyle *border_style_ptr, *parent_border_style_ptr ;

        g_return_val_if_fail (a_style && a_value, 
                              CR_BAD_PARAM_ERROR) ;
     
        switch (a_dir)
        {
        case DIR_TOP:
                border_style_ptr = &a_style->
                        border_style_props[BORDER_STYLE_PROP_TOP] ;
                parent_border_style_ptr = (a_style->parent_style)?
                        &a_style->parent_style->
                        border_style_props[BORDER_STYLE_PROP_TOP]: NULL ;

                break ;

        case DIR_RIGHT:
                border_style_ptr = 
                        &a_style->border_style_props[BORDER_STYLE_PROP_RIGHT] ;

                parent_border_style_ptr = (a_style->parent_style)?
                        &a_style->parent_style->
                        border_style_props[BORDER_STYLE_PROP_RIGHT] : NULL ;
                break ;

        case DIR_BOTTOM:
                border_style_ptr = &a_style->
                        border_style_props[BORDER_STYLE_PROP_BOTTOM] ;
                parent_border_style_ptr = (a_style->parent_style)?
                        &a_style->parent_style->
                        border_style_props[BORDER_STYLE_PROP_BOTTOM] : NULL;
                break ;

        case DIR_LEFT:
                border_style_ptr = &a_style->
                        border_style_props[BORDER_STYLE_PROP_LEFT] ;
                parent_border_style_ptr = (a_style->parent_style)?
                        &a_style->parent_style->
                        border_style_props[BORDER_STYLE_PROP_LEFT] : NULL;
                break ;

        default:
                break ;
        }
        
        if (a_value->type != TERM_IDENT
            || !a_value->content.str)
        {
                return CR_UNKNOWN_TYPE_ERROR ;
        }

        if (!strncmp ("none", 
                      a_value->content.str->str, 
                      strlen ("none")))
        {
                *border_style_ptr = BORDER_STYLE_NONE ;
        }
        else if (!strncmp ("hidden", 
                           a_value->content.str->str,
                           strlen ("hidden")))
        {
                *border_style_ptr = BORDER_STYLE_HIDDEN ;
        }
        else if (!strncmp ("dotted", 
                           a_value->content.str->str, 
                           strlen ("dotted")))
        {
                *border_style_ptr = BORDER_STYLE_DOTTED ;
        }
        else if (!strncmp ("dashed", 
                           a_value->content.str->str, 
                           strlen ("dashed")))
        {
                *border_style_ptr = BORDER_STYLE_DASHED ;
        }
        else if (!strncmp ("solid", 
                           a_value->content.str->str, 
                           strlen ("solid")))
        {
                *border_style_ptr = BORDER_STYLE_SOLID ;
        }
        else if (!strncmp ("double", 
                           a_value->content.str->str, 
                           strlen ("double")))
        {
                *border_style_ptr = BORDER_STYLE_DOUBLE ;
        }
        else if (!strncmp ("groove", 
                           a_value->content.str->str, 
                           strlen ("groove")))
        {
                *border_style_ptr = BORDER_STYLE_GROOVE ;
        }
        else if (!strncmp ("ridge", 
                           a_value->content.str->str, 
                           strlen ("ridge")))
        {
                *border_style_ptr = BORDER_STYLE_RIDGE ;
        }
        else if (!strncmp ("inset", 
                           a_value->content.str->str, 
                           strlen ("inset")))
        {
                *border_style_ptr = BORDER_STYLE_INSET ;
        }
        else if (!strncmp ("outset", 
                           a_value->content.str->str, 
                           strlen ("outset")))
        {
                *border_style_ptr = BORDER_STYLE_OUTSET ;
        }
        else if (!strncmp ("inherit", 
                           a_value->content.str->str, 
                           strlen ("inherit")))
        {
                if (parent_border_style_ptr)
                        *border_style_ptr = *parent_border_style_ptr ;
        }
        else
        {
                status = CR_UNKNOWN_TYPE_ERROR ;
        }

        return status ;
}

static enum CRStatus
set_prop_margin_x_from_value (CRStyle *a_style, CRTerm *a_value,
                              enum CRDirection a_dir)
{
        enum CRStatus status = CR_OK ;
        CRNum *num_val = NULL, *parent_num_val = NULL ;

        g_return_val_if_fail (a_style && a_value, 
                              CR_BAD_PARAM_ERROR) ;

        switch (a_dir)
        {
        case DIR_TOP:
                num_val = &a_style->num_props[NUM_PROP_MARGIN_TOP].sv ;
                parent_num_val = 
                        &a_style->parent_style->
                        num_props[NUM_PROP_MARGIN_TOP].sv ;
                break ;

        case DIR_RIGHT:
                num_val = 
                        &a_style->num_props[NUM_PROP_MARGIN_RIGHT].sv ;

                parent_num_val = 
                        &a_style->parent_style->
                        num_props[NUM_PROP_MARGIN_RIGHT].sv ;
                break ;

        case DIR_BOTTOM:
                num_val = &a_style->num_props[NUM_PROP_MARGIN_BOTTOM].sv ;
                parent_num_val = 
                        &a_style->parent_style->num_props[NUM_PROP_MARGIN_BOTTOM].sv ;
                break ;

        case DIR_LEFT:
                num_val = &a_style->num_props[NUM_PROP_MARGIN_LEFT].sv ;
                parent_num_val = 
                        &a_style->parent_style->
                        num_props[NUM_PROP_MARGIN_LEFT].sv ;
                break ;

        default:
                break ;
        }

        switch (a_value->type)
        {
        case TERM_IDENT:
                if (a_value->content.str 
                    && a_value->content.str->str
                    && !strncmp (a_value->content.str->str,
                                 "inherit", strlen ("inherit")))
                {
                        status = cr_num_copy (num_val, parent_num_val) ;
                }
                else if (a_value->content.str 
                         && a_value->content.str->str
                         && !strncmp (a_value->content.str->str,
                                      "auto", strlen ("auto")))
                {
                        status = cr_num_set (num_val, 0.0, NUM_AUTO) ;
                }
                else
                {
                        status = CR_UNKNOWN_TYPE_ERROR ;
                }

        case TERM_NUMBER:
                status = cr_num_copy (num_val, a_value->content.num) ;
                break ;

        default:
                status = CR_UNKNOWN_TYPE_ERROR ;
                break ;
        }

        return status ;
}

struct CRPropDisplayValPair
{
        const guchar *prop_name ;
        enum CRDisplayType type;
} ;

static enum CRStatus
set_prop_display_from_value (CRStyle *a_style, CRTerm *a_value)
{
        enum CRDisplayType default_display_val = DISPLAY_INLINE ;
        static const struct CRPropDisplayValPair disp_vals_map[] =
                {
                        {"none", DISPLAY_NONE},
                        {"inline", DISPLAY_INLINE},
                        {"block", DISPLAY_BLOCK},
                        {"run-in", DISPLAY_RUN_IN},
                        {"compact", DISPLAY_COMPACT},
                        {"marker", DISPLAY_MARKER},
                        {"table", DISPLAY_TABLE},
                        {"inline-table", DISPLAY_INLINE_TABLE},
                        {"table-row-group", DISPLAY_TABLE_ROW_GROUP},
                        {"table-header-group", DISPLAY_TABLE_HEADER_GROUP},
                        {"table-footer-group", DISPLAY_TABLE_FOOTER_GROUP},
                        {"table-row", DISPLAY_TABLE_ROW},
                        {"table-column-group", DISPLAY_TABLE_COLUMN_GROUP},
                        {"table-column", DISPLAY_TABLE_COLUMN},
                        {"table-cell", DISPLAY_TABLE_CELL},
                        {"table-caption", DISPLAY_TABLE_CAPTION},
                        {"inherit", DISPLAY_INHERIT},
                        {NULL, DISPLAY_NONE}
                } ;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR) ;

        /*Sets to its default value according to the css2 spec.*/
        a_style->display = default_display_val ;

        switch (a_value->type)
        {
        case TERM_IDENT:
        {
                int i = 0 ;

                if (!a_value->content.str || !a_value->content.str->str)
                        break ;

                for (i = 0; disp_vals_map[i].prop_name ; i++)
                {
                        if (!strncmp (disp_vals_map[i].prop_name,
                                      a_value->content.str->str,
                                      strlen 
                                      (disp_vals_map[i].prop_name)))
                        {
                                a_style->display = disp_vals_map[i].type ;
                                break ;
                        }
                }

                if (a_style->display == DISPLAY_INHERIT)
                {
                        if (a_style->parent_style)
                        {
                                a_style->display = 
                                        a_style->parent_style->display ;
                        }
                        else
                        {
                                a_style->display = default_display_val ;
                        }
                }
        }
        break ;

        default :
                break ;
        }

        return CR_OK ;
}

struct CRPropPositionValPair
{
        const guchar * name ;
        enum CRPositionType type ;
} ;

static enum CRStatus
set_prop_position_from_value (CRStyle *a_style, CRTerm *a_value)
{
        enum CRStatus status = CR_UNKNOWN_PROP_VAL_ERROR ;
        static const struct CRPropPositionValPair position_vals_map [] =
                {
                        {"static", POSITION_STATIC},
                        {"relative", POSITION_RELATIVE},
                        {"absolute", POSITION_ABSOLUTE},
                        {"fixed", POSITION_FIXED},
                        {"inherited", POSITION_INHERIT},
                        {NULL, POSITION_STATIC} 
                        /*must alwas be the last one*/
                } ;

        g_return_val_if_fail (a_value, CR_BAD_PARAM_ERROR) ;

        /*set to it's default value according to the css2 spec*/
        a_style->position = POSITION_STATIC ;

        switch (a_value->type)
        {
        case TERM_IDENT:
        {
                int i = 0 ;

                if (!a_value->content.str || !a_value->content.str->str)
                        break ;

                for (i = 0; position_vals_map[i].name ; i++)
                {
                        if (!strncmp (position_vals_map[i].name,
                                      a_value->content.str->str,
                                      strlen (position_vals_map[i].name)))
                        {
                                a_style->position = 
                                        position_vals_map[i].type ;
                                status = CR_OK ;
                                break ;
                        }
                }
                if (a_style->position == POSITION_INHERIT)
                {
                        if (a_style->parent_style)
                        {
                                a_style->position  =
                                        a_style->parent_style->position ;
                        }
                        else
                        {
                                a_style->position = POSITION_STATIC ;
                        }
                }
        }
                break ;

        default:
                break ;
        }

        return CR_OK ;
}

static enum CRStatus
set_prop_x_from_value (CRStyle *a_style, CRTerm *a_value,
                       enum CRDirection a_dir)
{
        CRNum *box_offset = NULL, *parent_box_offset = NULL ;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR) ;

        
        if (!(a_value->type == TERM_NUMBER)
            && !(a_value->type == TERM_IDENT))
        {
                return CR_UNKNOWN_PROP_VAL_ERROR ;
        }
        
        switch (a_dir)
        {
        case DIR_TOP:
                box_offset = &a_style->num_props[NUM_PROP_TOP].sv ;
                if (a_style->parent_style)
                        parent_box_offset = 
                                &a_style->parent_style->
                                num_props[NUM_PROP_TOP].sv ;
                break ;

        case DIR_RIGHT: 
                box_offset = &a_style->num_props[NUM_PROP_RIGHT].sv ;
                if (a_style->parent_style)
                        parent_box_offset = &a_style->parent_style->
                                num_props[NUM_PROP_RIGHT].sv ;
                break ;

        case DIR_BOTTOM:
                box_offset = &a_style->num_props[NUM_PROP_BOTTOM].sv ;
                if (a_style->parent_style)
                        parent_box_offset = 
                                &a_style->parent_style->
                                num_props[NUM_PROP_BOTTOM].sv ;
                break ;
        case DIR_LEFT:
                box_offset = &a_style->num_props[NUM_PROP_LEFT].sv ;
                if (a_style->parent_style)
                        parent_box_offset = 
                                &a_style->parent_style->
                                num_props[NUM_PROP_LEFT].sv ;
                break ;

        default:
                break ;
        }

        box_offset->type = NUM_AUTO ;

        if (a_value->type == TERM_NUMBER 
            && a_value->content.num)
        {
                cr_num_copy (box_offset, a_value->content.num) ;
        }
        else if (a_value->type == TERM_IDENT
                 && a_value->content.str
                 && a_value->content.str->str)
        {
                if (!strncmp ("inherit", 
                              a_value->content.str->str,
                              strlen ("inherit")))
                {
                        cr_num_copy (box_offset, 
                                     parent_box_offset) ;
                }
                else if (!strncmp ("auto", 
                                   a_value->content.str->str,
                                   strlen ("auto")))
                {
                        box_offset->type = NUM_AUTO ;
                }
        }

        return CR_OK ;
}


static enum CRStatus
set_prop_float (CRStyle *a_style, CRTerm *a_value)
{
        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR) ;

        /*the default float type as specified by the css2 spec*/
        a_style->float_type = FLOAT_NONE ;

        if (a_value->type != TERM_IDENT
            || !a_value->content.str
            || !a_value->content.str->str) 
        {/*unknow type, the float type is set to it's default value*/
                return CR_OK ;
        }

        if (!strncmp ("none",
                      a_value->content.str->str,
                      strlen ("none")))
        {
                a_style->float_type = FLOAT_NONE ;
        }
        else if (!strncmp ("left",
                           a_value->content.str->str,
                           strlen ("left")))
        {
                a_style->float_type = FLOAT_LEFT ;
        }
        else if (!strncmp ("right",
                           a_value->content.str->str,
                           strlen ("right")))
        {
                a_style->float_type = FLOAT_RIGHT ;
        }
        else if (!strncmp ("inherit",
                           a_value->content.str->str,
                           strlen ("inherit")))
        {
                a_style->float_type = 
                        a_style->parent_style->float_type ;
        }

        return CR_OK ;        
}


static enum CRStatus
set_prop_width (CRStyle *a_style, CRTerm *a_value)
{
        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR) ;

        
        a_style->num_props[NUM_PROP_WIDTH].sv.type = NUM_AUTO ;
        
        if (a_value->type == TERM_IDENT)
        {
                if (a_value->content.str
                    && a_value->content.str->str)
                {
                        if (!strncmp ("auto",
                                      a_value->content.str->str,
                                      strlen ("auto")))
                        {
                                a_style->num_props[NUM_PROP_WIDTH].sv.type = 
                                        NUM_AUTO ;
                        }
                        else if (!strncmp ("inherit",
                                           a_value->content.str->str,
                                           strlen ("inherit")))
                        {
                                cr_num_copy 
                                        (&a_style->
                                         num_props[NUM_PROP_WIDTH].sv,
                                         &a_style->parent_style->
                                         num_props[NUM_PROP_WIDTH].sv) ;
                        }
                }
        }
        else if (a_value->type == TERM_NUMBER)
        {
                if (a_value->content.num)
                {
                        cr_num_copy (&a_style->num_props[NUM_PROP_WIDTH].sv,
                                     a_value->content.num) ;
                }
        }

        return CR_OK ;
}

static enum CRStatus
set_prop_color_rgb (CRStyle *a_style, CRTerm *a_value)
{
        g_return_val_if_fail (a_style && a_value,
                              CR_BAD_PARAM_ERROR) ;

        if (a_value->type == TERM_RGB)
        {
                if (a_value->content.rgb)
                {
                        cr_rgb_set_from_rgb
                                (&a_style->rgb_props[RGB_PROP_COLOR].sv,
                                 a_value->content.rgb) ;
                }

        }

        return CR_OK ;
}

static enum CRStatus
set_prop_background_color_rgb (CRStyle *a_style, CRTerm *a_value)
{
        g_return_val_if_fail (a_style && a_value,
                              CR_BAD_PARAM_ERROR) ;

        if (a_value->type == TERM_RGB)
        {
                if (a_value->content.rgb)
                {
                        cr_rgb_set_from_rgb
                                (&a_style->
                                 rgb_props[RGB_PROP_BACKGROUND_COLOR].sv,
                                 a_value->content.rgb) ;
                }
        }

        return CR_OK ;
}

/**
 *Sets border-top-color, border-right-color,
 *border-bottom-color or border-left-color properties
 *in the style structure. The value is taken from a
 *css2 term of type IDENT or RGB.
 *@param a_style the style structure to set.
 *@param a_value the css2 term to take the color information from.
 *@param a_dir the direction (TOP, LEFT, RIGHT, or BOTTOM).
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
set_prop_border_x_color_from_value (CRStyle *a_style, CRTerm *a_value,
                                    enum CRDirection a_dir)
{
        CRRgb *rgb_color = NULL ;
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_style && a_value,
                              CR_BAD_PARAM_ERROR) ;
        
        switch (a_dir)
        {
        case DIR_TOP:
                rgb_color = &a_style->rgb_props[RGB_PROP_BORDER_TOP_COLOR].sv ;
                break ;

        case DIR_RIGHT:
                rgb_color = &a_style->rgb_props[RGB_PROP_BORDER_RIGHT_COLOR].sv ;
                break ;

        case DIR_BOTTOM:
                rgb_color = &a_style->rgb_props[RGB_PROP_BORDER_BOTTOM_COLOR].sv;
                break ;

        case DIR_LEFT:
                rgb_color = &a_style->rgb_props[RGB_PROP_BORDER_LEFT_COLOR].sv ;
                break ;

        default:
                cr_utils_trace_info ("unknown DIR type") ;
                return CR_BAD_PARAM_ERROR ;
        }

        status = CR_UNKNOWN_PROP_VAL_ERROR ;

        if (a_value->type == TERM_IDENT)
        {
                if (a_value->content.str && a_value->content.str->str)
                {
                        status = cr_rgb_set_from_name 
                                (rgb_color, a_value->content.str->str) ;
                        
                }

                if (status != CR_OK)
                {
                        cr_rgb_set_from_name (rgb_color, "black") ;
                }
        }
        else if (a_value->type == TERM_RGB)
        {
                if (a_value->content.rgb)
                {
                        status = cr_rgb_set_from_rgb 
                                (rgb_color, a_value->content.rgb) ;
                }
        }

        return status ;
}


static enum CRStatus
set_prop_border_x_from_value (CRStyle *a_style, CRTerm *a_value,
                              enum CRDirection a_dir)
{
        CRTerm *cur_term = NULL ;

        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_style && a_value,
                              CR_BAD_PARAM_ERROR) ;

        for (cur_term = a_value ; cur_term ; cur_term = cur_term->next)
        {
                status = 
                        set_prop_border_x_width_from_value (a_style, cur_term,
                                                            a_dir) ;

                if (status != CR_OK)
                {
                        status = set_prop_border_x_style_from_value 
                                (a_style, cur_term, a_dir) ;
                }

                if (status != CR_OK)
                {
                        status = set_prop_border_x_color_from_value 
                                (a_style, cur_term, a_dir) ;
                }
        }

        return CR_OK ;
}

static enum CRStatus
set_prop_border_from_value (CRStyle *a_style, CRTerm *a_value)
{
        enum CRDirection direction = 0 ;

        g_return_val_if_fail (a_style && a_value,
                              CR_BAD_PARAM_ERROR) ;

        for (direction = 0 ; direction < NB_DIRS ; direction ++)
        {
                set_prop_border_x_from_value (a_style, a_value, direction) ;
        }

        return CR_OK ;
}


/******************
 *Public methods
 ******************/

/**
 *Default constructor of #CRStyle.
 */
CRStyle *
cr_style_new (void)
{
	CRStyle *result = NULL ;

	result = g_try_malloc (sizeof (CRStyle)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRStyle)) ;
        gv_prop_hash_ref_count ++ ;

        /*set the style properties to their default values*/
        cr_style_set_props_to_defaults (result) ;

	return result ;
}


/**
 *Instanciates a new #CRStyle from a ruleset statement.
 *Actually walks through all declarations of the ruleset
 *statement and for each of them, sets the corresponding field
 *of the CRStyle structure.
 *@param a_stmt the ruleset statement.
 *@param a_parent_style the parent style: the style of the
 *parent xml element.
 *@param a_style out parameter. The newly built instance of #CRStyle.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_style_new_from_ruleset (CRStatement *a_stmt, 
                           CRStyle *a_parent_style,
			   CRStyle **a_style)
{
        g_return_val_if_fail (a_stmt                    
                              && a_stmt->type == RULESET_STMT
                              && a_stmt->kind.ruleset,
                              CR_BAD_PARAM_ERROR) ;

        if (! *a_style)
        {
                *a_style = cr_style_new () ;
                if (! *a_style)
                {
                        cr_utils_trace_info ("Out of memory") ;
                        return CR_ERROR ;
                }
        }

        return cr_style_set_style_from_ruleset (*a_style, a_stmt,
                                                a_parent_style) ;
}

/**
 *Sets an instance of #CRStyle from a ruleset type of css2 statement.
 *@param a_this the current instance of #CRStyle.
 *@param a_stmt the statement the css ruleset statement.
 *@param a_parent_style the style of the parent node.
 *@return CR_OK upon sucessfull statement, an error code otherwise.
 */
enum CRStatus
cr_style_set_style_from_ruleset (CRStyle *a_this, CRStatement *a_stmt,
                                 CRStyle *a_parent_style)
{
        CRDeclaration *decl = NULL ;

        g_return_val_if_fail (a_this
                              && a_stmt
                              && (a_stmt->type == RULESET_STMT),
                              CR_BAD_PARAM_ERROR) ;

        for (decl = a_stmt->kind.ruleset->decl_list ;
             decl ; decl = decl->next)
        {
                cr_style_set_style_from_decl
                        (a_this, decl, a_parent_style) ;
        }

        return CR_OK ;
}

/**
 *Walks through a css2 property declaration, and populated the
 *according field(s) in the #CRStyle structure.
 *If the properties or their value(s) are/is not known, 
 *sets the corresponding field(s) of #CRStyle to its/their default 
 *value(s)
 *@param a_this the instance of #CRStyle to set.
 *@param a_decl the declaration from which the #CRStyle fields are set.
 *@param a_parent_style the style of the parent xml node. The parent
 *xml node is the parent node of the node that generated this style 
 *structure.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_style_set_style_from_decl (CRStyle *a_this, CRDeclaration *a_decl,
                              CRStyle *a_parent_style)
{
        CRTerm *value = NULL ;
        enum CRStatus status = CR_OK ;

        enum CRPropertyID prop_id = PROP_ID_NOT_KNOWN ;

        g_return_val_if_fail (a_this && a_decl
                              && a_decl
                              && a_decl->property
                              && a_decl->property->str,
                              CR_BAD_PARAM_ERROR) ;

        a_this->parent_style = a_parent_style ;

        prop_id = cr_style_get_prop_id (a_decl->property->str) ;

        value = a_decl->value ;
        switch (prop_id)
        {
        case PROP_ID_PADDING_TOP:
                status = set_prop_padding_x_from_value 
                        (a_this, value, DIR_TOP) ;
                break ;

        case PROP_ID_PADDING_RIGHT:
                status = set_prop_padding_x_from_value 
                        (a_this, value, DIR_RIGHT) ;
                break ;
        case PROP_ID_PADDING_BOTTOM:
                status = set_prop_padding_x_from_value 
                        (a_this, value, DIR_BOTTOM) ;
                break ;

        case PROP_ID_PADDING_LEFT:
                status = set_prop_padding_x_from_value 
                        (a_this, value, DIR_LEFT) ;
                break ;
                
        case PROP_ID_BORDER_TOP_WIDTH:
                status = 
                        set_prop_border_x_width_from_value (a_this, value,
                                                            DIR_TOP) ;
                break ;

        case PROP_ID_BORDER_RIGHT_WIDTH:
                status = 
                        set_prop_border_x_width_from_value (a_this, value,
                                                            DIR_RIGHT) ;
                break ;

        case PROP_ID_BORDER_BOTTOM_WIDTH:
                status = 
                        set_prop_border_x_width_from_value (a_this, value,
                                                            DIR_BOTTOM) ;
                break ;

        case PROP_ID_BORDER_LEFT_WIDTH:
                status = 
                        set_prop_border_x_width_from_value (a_this, value,
                                                            DIR_LEFT) ;
                break ;

        case PROP_ID_BORDER_TOP_STYLE:
                status = 
                        set_prop_border_x_style_from_value (a_this, value,
                                                            DIR_TOP) ;
                break ;

        case PROP_ID_BORDER_RIGHT_STYLE:
                status = 
                        set_prop_border_x_style_from_value (a_this, value,
                                                            DIR_RIGHT) ;
                break ;

        case PROP_ID_BORDER_BOTTOM_STYLE:
                status = 
                        set_prop_border_x_style_from_value (a_this, value,
                                                            DIR_BOTTOM) ;
                break ;

        case PROP_ID_BORDER_LEFT_STYLE: 
                status = 
                        set_prop_border_x_style_from_value (a_this, value,
                                                            DIR_LEFT) ;
                break ;

        case PROP_ID_BORDER_TOP_COLOR:
                status =
                        set_prop_border_x_color_from_value (a_this, value,
                                                            DIR_TOP) ;
                break ;

        case PROP_ID_BORDER_RIGHT_COLOR:
                status =
                        set_prop_border_x_color_from_value (a_this, value,
                                                            DIR_RIGHT) ;
                break ;

        case PROP_ID_BORDER_BOTTOM_COLOR:
                status =
                        set_prop_border_x_color_from_value (a_this, value,
                                                            DIR_BOTTOM) ;
                break ;

        case PROP_ID_BORDER_LEFT_COLOR:
                status =
                        set_prop_border_x_color_from_value (a_this, value,
                                                            DIR_BOTTOM) ;
                break ;

        case PROP_ID_BORDER_TOP:
                status =
                        set_prop_border_x_from_value (a_this, value,
                                                      DIR_TOP) ;
                break ;

        case PROP_ID_BORDER_RIGHT:
                status =
                        set_prop_border_x_from_value (a_this, value,
                                                      DIR_TOP) ;
                break ;

        case PROP_ID_BORDER_BOTTOM:
                status =
                        set_prop_border_x_from_value (a_this, value,
                                                      DIR_BOTTOM) ;
                break ;

        case PROP_ID_BORDER_LEFT:
                status =
                        set_prop_border_x_from_value (a_this, value,
                                                      DIR_BOTTOM) ;
                break ;

        case PROP_ID_MARGIN_TOP:
                status = 
                        set_prop_margin_x_from_value (a_this, value,
                                                      DIR_TOP) ;
                break ;

        case PROP_ID_BORDER:
                status =
                        set_prop_border_from_value (a_this, value) ;
                break ;

        case PROP_ID_MARGIN_RIGHT:
                status = 
                        set_prop_margin_x_from_value (a_this, value,
                                                      DIR_RIGHT) ;
                break ;

        case PROP_ID_MARGIN_BOTTOM:
                status = 
                        set_prop_margin_x_from_value (a_this, value,
                                                      DIR_BOTTOM) ;
                break ;

        case PROP_ID_MARGIN_LEFT:
                status = 
                        set_prop_margin_x_from_value (a_this, value,
                                                      DIR_LEFT) ;
                break ;

        case PROP_ID_DISPLAY:
                status = 
                        set_prop_display_from_value (a_this, value) ;
                break ;

        case PROP_ID_POSITION:
                status = set_prop_position_from_value (a_this, value) ;
                break ;

        case PROP_ID_TOP:
                status = set_prop_x_from_value (a_this, value,
                                                DIR_TOP) ;
                break ;

        case PROP_ID_RIGHT:
                status = set_prop_x_from_value (a_this, value,
                                                DIR_RIGHT) ;
                break ;

        case PROP_ID_BOTTOM:
                status = set_prop_x_from_value (a_this, value,
                                                DIR_BOTTOM) ;
                break ;

        case PROP_ID_LEFT:
                status = set_prop_x_from_value (a_this, value,
                                                DIR_LEFT) ;
                break ;

        case PROP_ID_FLOAT:
                status = set_prop_float (a_this, value) ;
                break ;

        case PROP_ID_WIDTH:
                status = set_prop_width (a_this, value) ;
                break ;

        case PROP_ID_COLOR:
                status = set_prop_color_rgb (a_this, value) ;
                break ;

        case PROP_ID_BACKGROUND_COLOR:
                status = set_prop_background_color_rgb (a_this, value) ;

        default:
                return CR_UNKNOWN_TYPE_ERROR ;

        }

        return status ;
}

/**
 *Increases the reference count
 *of the current instance of #CRStyle.
 *@param a_this the current instance of #CRStyle.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_style_ref (CRStyle *a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        a_this->ref_count ++ ;
        return CR_OK ;
}


/**
 *Decreases the reference count of
 *the current instance of #CRStyle.
 *If the reference count reaches 0, the
 *instance of #CRStyle is destoyed.
 *@param a_this the current instance of #CRStyle.
 *@return TRUE if the instance has been destroyed, FALSE
 *otherwise.
 */
gboolean
cr_style_unref (CRStyle *a_this)
{
        g_return_val_if_fail (a_this,
                              FALSE) ;

        if (a_this->ref_count)
                a_this->ref_count -- ;

        if (!a_this->ref_count)
        {
                cr_style_destroy (a_this) ;
                return TRUE ;
        }

        return FALSE ;
}

/**
 *Duplicates the current instance of #CRStyle .
 *The newly created instance of #CRStyle must be
 *freed using cr_style_destroy ().
 *@param a_this the current instance of #CRStyle.
 *@return the newly duplicated instance of #CRStyle.
 */
CRStyle *
cr_style_dup (CRStyle *a_this)
{
        CRStyle *result = NULL ;

        g_return_val_if_fail (a_this, NULL) ;

        result = cr_style_new () ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }
        memcpy (result, a_this, sizeof (CRStyle)) ;

        return result ;
}

/**
 *Destructor of the #CRStyle class.
 *@param a_this the instance to destroy.
 */
void
cr_style_destroy (CRStyle *a_this)
{
	g_return_if_fail (a_this) ;

	g_free (a_this) ;
}
