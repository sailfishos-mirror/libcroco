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
 *Each css property has an ID which is
 *en entry into a property "population" jump table.
 *each entry of the property population jump table
 *contains code to tranform the literal form of
 *a property value into a strongly typed value.
 */
enum CRPropertyID
{
        PROP_NOT_KNOWN,
	PROP_PADDING_TOP,
	PROP_PADDING_RIGHT,
	PROP_PADDING_BOTTOM,
	PROP_PADDING_LEFT,
	PROP_BORDER_TOP_WIDTH,
	PROP_BORDER_RIGHT_WIDTH,
	PROP_BORDER_BOTTOM_WIDTH,
	PROP_BORDER_LEFT_WIDTH,
	PROP_BORDER_TOP_STYLE,
	PROP_BORDER_RIGHT_STYLE,
	PROP_BORDER_BOTTOM_STYLE,
	PROP_BORDER_LEFT_STYLE,
	PROP_MARGIN_TOP,
	PROP_MARGIN_RIGHT,
	PROP_MARGIN_BOTTOM,
	PROP_MARGIN_LEFT,
	PROP_DISPLAY,
	PROP_POSITION,
	PROP_TOP,
	PROP_RIGHT,
	PROP_BOTTOM,
	PROP_LEFT,
	PROP_FLOAT,
	PROP_WIDTH	
} ;


typedef struct _CRPropertyDesc CRPropertyDesc ;

struct _CRPropertyDesc
{
	const guchar * name ;
	enum CRPropertyID prop_id ;	
} ;

static CRPropertyDesc gv_prop_table [] =

{
	{"padding-top", PROP_PADDING_TOP},
	{"padding-right", PROP_PADDING_RIGHT},
	{"padding-bottom", PROP_PADDING_BOTTOM},
	{"padding-left", PROP_PADDING_LEFT},
	{"border-top-width", PROP_BORDER_TOP_WIDTH},
	{"border-right-width", PROP_BORDER_RIGHT_WIDTH},
	{"border-bottom-width", PROP_BORDER_BOTTOM_WIDTH},
	{"border-left-width", PROP_BORDER_LEFT_WIDTH},
	{"border-top-style", PROP_BORDER_TOP_STYLE},
	{"border-right-style", PROP_BORDER_RIGHT_STYLE},
	{"border-bottom-style", PROP_BORDER_BOTTOM_STYLE},
	{"border-left-style", PROP_BORDER_LEFT_STYLE},
	{"margin-top", PROP_MARGIN_TOP},
	{"margin-right", PROP_MARGIN_RIGHT},
	{"margin-bottom", PROP_MARGIN_BOTTOM},
	{"margin-left", PROP_MARGIN_LEFT},
	{"display", PROP_DISPLAY},
	{"position", PROP_POSITION},
	{"top", PROP_TOP},
	{"right", PROP_RIGHT},
	{"bottom", PROP_BOTTOM},
	{"left", PROP_LEFT},
	{"float", PROP_FLOAT},
	{"width", PROP_WIDTH},
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
        DIR_TOP,
        DIR_RIGHT,
        DIR_BOTTOM,
        DIR_LEFT
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
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        cr_num_set (&a_this->padding_top,
                    0, NUM_LENGTH_PX) ;

        cr_num_set (&a_this->padding_top,
                    0, NUM_LENGTH_PX) ;
        cr_num_set (&a_this->padding_right,
                    0, NUM_LENGTH_PX) ;
        cr_num_set (&a_this->padding_bottom,
                    0, NUM_LENGTH_PX) ;
        cr_num_set (&a_this->padding_left,
                    0, NUM_LENGTH_PX) ;

        cr_num_set (&a_this->border_top_width,
                    0, BORDER_MEDIUM) ;
        cr_num_set (&a_this->border_right_width,
                    0, BORDER_MEDIUM) ;
        cr_num_set (&a_this->border_bottom_width,
                    0, BORDER_MEDIUM) ;
        cr_num_set (&a_this->border_left_width,
                    0, BORDER_MEDIUM) ;

        /*default foreground color is black*/
        cr_rgb_set (&a_this->color, 0, 0, 0,
                    FALSE) ;
        cr_rgb_set_from_rgb (&a_this->border_top_color,
                             &a_this->color) ;
        cr_rgb_set_from_rgb (&a_this->border_right_color,
                             &a_this->color) ;
        cr_rgb_set_from_rgb (&a_this->border_bottom_color,
                             &a_this->color) ;
        cr_rgb_set_from_rgb (&a_this->border_left_color,
                             &a_this->color) ;

        a_this->border_top_style = BORDER_STYLE_NONE ;
        a_this->border_right_style = BORDER_STYLE_NONE ;
        a_this->border_bottom_style = BORDER_STYLE_NONE ;
        a_this->border_left_style = BORDER_STYLE_NONE ;

        cr_num_set (&a_this->margin_top, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_this->margin_right, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_this->margin_bottom, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_this->margin_left, 0, NUM_LENGTH_PX) ;

        a_this->display = DISPLAY_INLINE ;
        a_this->position = POSITION_STATIC ;

        a_this->top.type = OFFSET_AUTO;
        cr_num_set (&a_this->top.num, 0, NUM_LENGTH_PX) ;
        a_this->right.type = OFFSET_AUTO;
        cr_num_set (&a_this->right.num, 0, NUM_LENGTH_PX) ;
        a_this->bottom.type = OFFSET_AUTO;
        cr_num_set (&a_this->bottom.num, 0, NUM_LENGTH_PX) ;
        a_this->left.type = OFFSET_AUTO;
        cr_num_set (&a_this->left.num, 0, NUM_LENGTH_PX) ;

        a_this->float_type = FLOAT_NONE ;

        a_this->width.type = WIDTH_AUTO ;
        cr_num_set (&a_this->width.num, 0, NUM_LENGTH_PX) ;

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
                return PROP_NOT_KNOWN ;
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
                num_val = &a_style->padding_top ;
                parent_num_val = &a_style->parent_style->padding_top ;
                break ;
                
        case DIR_RIGHT:
                num_val = &a_style->padding_right ;
                parent_num_val = &a_style->parent_style->padding_right ;
                break ;

        case DIR_BOTTOM:
                num_val = &a_style->padding_bottom ;
                parent_num_val = &a_style->parent_style->padding_bottom ;
                break ;

        case DIR_LEFT:
                num_val = & a_style->padding_left ;
                parent_num_val = &a_style->parent_style->padding_left ;
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
        CRNum *num_val = NULL, *parent_num_val = NULL ;

        g_return_val_if_fail (a_value
                              && a_style->parent_style, 
                              CR_BAD_PARAM_ERROR) ;

        switch (a_dir)
        {
        case DIR_TOP:
                num_val = &a_style->border_top_width ;
                parent_num_val = 
                        &a_style->parent_style->border_top_width ;
                break ;

        case DIR_RIGHT:
                num_val = 
                        &a_style->border_right_width ;

                parent_num_val = 
                        &a_style->parent_style->border_right_width;

                break ;

        case DIR_BOTTOM:
                num_val = &a_style->border_bottom_width ;
                parent_num_val = 
                        &a_style->parent_style->border_bottom_width;
                break ;

        case DIR_LEFT:
                num_val = &a_style->border_left_width ;
                parent_num_val = 
                        &a_style->parent_style->border_left_width;
                break ;

        default:
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
        

        switch (a_value->content.num->type)
        {
        case NUM_GENERIC:
        case NUM_LENGTH_EM:
        case NUM_LENGTH_EX:
        case NUM_LENGTH_PX:
        case NUM_LENGTH_IN:
        case NUM_LENGTH_CM:
        case NUM_LENGTH_MM:
        case NUM_LENGTH_PT:
        case NUM_LENGTH_PC:
                status = cr_num_copy (num_val, a_value->content.num) ;
                break ;
        default :
                status = CR_ERROR ;
                break ;
        }

        return status ;
}


static enum CRStatus
set_prop_border_x_style_from_value (CRStyle *a_style,
                                    CRTerm *a_value,
                                    enum CRDirection a_dir)
{
        g_return_val_if_fail (a_style && a_value, 
                              CR_BAD_PARAM_ERROR) ;

        enum CRStatus status = CR_OK ;
        enum CRBorderStyle *border_style_ptr, *parent_border_style_ptr ;

        g_return_val_if_fail (a_value
                              && a_style->parent_style, 
                              CR_BAD_PARAM_ERROR) ;

        switch (a_dir)
        {
        case DIR_TOP:
                border_style_ptr = &a_style->border_top_style ;
                parent_border_style_ptr = 
                        &a_style->parent_style->border_top_style ;
                break ;

        case DIR_RIGHT:
                border_style_ptr = 
                        &a_style->border_right_style ;

                parent_border_style_ptr = 
                        &a_style->parent_style->border_right_style;

                break ;

        case DIR_BOTTOM:
                border_style_ptr = &a_style->border_bottom_style ;
                parent_border_style_ptr = 
                        &a_style->parent_style->border_bottom_style;
                break ;

        case DIR_LEFT:
                border_style_ptr = &a_style->border_left_style ;
                parent_border_style_ptr = 
                        &a_style->parent_style->border_left_style;
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
                num_val = &a_style->margin_top ;
                parent_num_val = 
                        &a_style->parent_style->margin_top ;
                break ;

        case DIR_RIGHT:
                num_val = 
                        &a_style->margin_right ;

                parent_num_val = 
                        &a_style->parent_style->margin_right ;
                break ;

        case DIR_BOTTOM:
                num_val = &a_style->margin_bottom ;
                parent_num_val = 
                        &a_style->parent_style->margin_bottom ;
                break ;

        case DIR_LEFT:
                num_val = &a_style->margin_left ;
                parent_num_val = 
                        &a_style->parent_style->margin_left ;
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
        CRBoxOffset *box_offset = NULL, *parent_box_offset = NULL ;
        
        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR) ;

        
        if (!(a_value->type == TERM_NUMBER)
            && !(a_value->type == TERM_IDENT))
        {
                return CR_UNKNOWN_PROP_VAL_ERROR ;
        }
        
        switch (a_dir)
        {
        case DIR_TOP:
                box_offset = &a_style->top ;
                if (a_style->parent_style)
                        parent_box_offset = &a_style->parent_style->top ;
                break ;
        case DIR_RIGHT: 
                box_offset = &a_style->right ;
                if (a_style->parent_style)
                        parent_box_offset = &a_style->parent_style->right ;
                break ;
        case DIR_BOTTOM:
                box_offset = &a_style->bottom ;
                if (a_style->parent_style)
                        parent_box_offset = 
                                &a_style->parent_style->bottom;
                break ;
        case DIR_LEFT:
                box_offset = &a_style->left ;
                if (a_style->parent_style)
                        parent_box_offset = &a_style->parent_style->left ;
                break ;

        default:
                break ;
        }

        box_offset->type = OFFSET_AUTO ;

        if (a_value->type == TERM_NUMBER 
            && a_value->content.num)
        {
                cr_num_copy (&box_offset->num, a_value->content.num) ;
                box_offset->type = OFFSET_DEFINED ;
        }
        else if (a_value->type == TERM_IDENT
                 && a_value->content.str
                 && a_value->content.str->str)
        {
                if (!strncmp ("inherit", 
                              a_value->content.str->str,
                              strlen ("inherit")))
                {
                        cr_num_copy (&box_offset->num, 
                                     &parent_box_offset->num) ;
                        box_offset->type = OFFSET_DEFINED ;
                }
                else if (!strncmp ("auto", 
                                   a_value->content.str->str,
                                   strlen ("auto")))
                {
                        box_offset->type = OFFSET_AUTO ;
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

        
        a_style->width.type = WIDTH_AUTO ;
        
        if (a_value->type == TERM_IDENT)
        {
                if (a_value->content.str
                    && a_value->content.str->str)
                {
                        if (!strncmp ("auto",
                                      a_value->content.str->str,
                                      strlen ("auto")))
                        {
                                a_style->width.type = WIDTH_AUTO ;
                        }
                        else if (!strncmp ("inherit",
                                           a_value->content.str->str,
                                           strlen ("inherit")))
                        {
                                a_style->width.type = 
                                        a_style->parent_style->width.type;

                                cr_num_copy 
                                        (&a_style->width.num,
                                         &a_style->parent_style->width.num);
                        }
                }
        }
        else if (a_value->type == TERM_NUMBER)
        {
                if (a_value->content.num)
                {
                        cr_num_copy (&a_style->width.num,
                                     a_value->content.num) ;
                        a_style->width.type = WIDTH_DEFINED ;
                }
        }

        return CR_OK ;
}


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
        CRDeclaration *decl = NULL ;        

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

        for (decl = a_stmt->kind.ruleset->decl_list ;
             decl ; decl = decl->next)
        {
                cr_style_set_style_from_decl 
                        (*a_style, decl, a_parent_style) ;
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

        enum CRPropertyID prop_id = PROP_NOT_KNOWN ;

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
        case PROP_PADDING_TOP:
                status = set_prop_padding_x_from_value 
                        (a_this, value, DIR_TOP) ;
                break ;

        case PROP_PADDING_RIGHT:
                status = set_prop_padding_x_from_value 
                        (a_this, value, DIR_RIGHT) ;
                break ;
        case PROP_PADDING_BOTTOM:
                status = set_prop_padding_x_from_value 
                        (a_this, value, DIR_RIGHT) ;
                break ;

        case PROP_PADDING_LEFT:
                status = set_prop_padding_x_from_value 
                        (a_this, value, DIR_LEFT) ;
                break ;
                
        case PROP_BORDER_TOP_WIDTH:
                status = 
                        set_prop_border_x_width_from_value (a_this, value,
                                                            DIR_TOP) ;
                break ;

        case PROP_BORDER_RIGHT_WIDTH:
                status = 
                        set_prop_border_x_width_from_value (a_this, value,
                                                            DIR_RIGHT) ;
                break ;

        case PROP_BORDER_BOTTOM_WIDTH:
                status = 
                        set_prop_border_x_width_from_value (a_this, value,
                                                            DIR_BOTTOM) ;
                break ;

        case PROP_BORDER_LEFT_WIDTH:
                status = 
                        set_prop_border_x_width_from_value (a_this, value,
                                                            DIR_BOTTOM) ;
                break ;

        case PROP_BORDER_TOP_STYLE:
                status = 
                        set_prop_border_x_style_from_value (a_this, value,
                                                            DIR_TOP) ;
                break ;

        case PROP_BORDER_RIGHT_STYLE:
                status = 
                        set_prop_border_x_style_from_value (a_this, value,
                                                            DIR_RIGHT) ;
                break ;

        case PROP_BORDER_BOTTOM_STYLE:
                status = 
                        set_prop_border_x_style_from_value (a_this, value,
                                                            DIR_BOTTOM) ;
                break ;

        case PROP_BORDER_LEFT_STYLE: 
                status = 
                        set_prop_border_x_style_from_value (a_this, value,
                                                            DIR_LEFT) ;
                break ;

        case PROP_MARGIN_TOP:
                status = 
                        set_prop_margin_x_from_value (a_this, value,
                                                      DIR_TOP) ;
                break ;

        case PROP_MARGIN_RIGHT:
                status = 
                        set_prop_margin_x_from_value (a_this, value,
                                                      DIR_RIGHT) ;
                break ;

        case PROP_MARGIN_BOTTOM:
                status = 
                        set_prop_margin_x_from_value (a_this, value,
                                                      DIR_BOTTOM) ;
                break ;

        case PROP_MARGIN_LEFT:
                status = 
                        set_prop_margin_x_from_value (a_this, value,
                                                      DIR_TOP) ;
                break ;

        case PROP_DISPLAY:
                status = 
                        set_prop_display_from_value (a_this, value) ;
                break ;

        case PROP_POSITION:
                status = set_prop_position_from_value (a_this, value) ;
                break ;

        case PROP_TOP:
                status = set_prop_x_from_value (a_this, value,
                                                DIR_TOP) ;
                break ;

        case PROP_RIGHT:
                status = set_prop_x_from_value (a_this, value,
                                                DIR_RIGHT) ;
                break ;

        case PROP_BOTTOM:
                status = set_prop_x_from_value (a_this, value,
                                                DIR_BOTTOM) ;
                break ;

        case PROP_LEFT:
                status = set_prop_x_from_value (a_this, value,
                                                DIR_LEFT) ;
                break ;

        case PROP_FLOAT:
                status = set_prop_float (a_this, value) ;
                break ;

        case PROP_WIDTH:
                status = set_prop_width (a_this, value) ;
                break ;

        default:
                return CR_UNKNOWN_TYPE_ERROR ;
        }

        return status ;
        
}

enum CRStatus
cr_style_ref (CRStyle *a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        a_this->ref_count ++ ;
        return CR_OK ;
}

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
 *Destructor of the #CRStyle class.
 *@param a_this the instance to destroy.
 */
void
cr_style_destroy (CRStyle *a_this)
{
	g_return_if_fail (a_this) ;

	g_free (a_this) ;
}
