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
set_padding_x_from_value (CRStyle *a_style,                          
                          CRTerm *a_value,
                          enum CRDirection a_dir) ;

static enum CRStatus
set_border_x_width_from_value (CRStyle *a_style,
                               CRTerm *a_value,
                               enum CRDirection a_dir) ;

static enum CRStatus
set_border_x_style_from_value (CRStyle *a_style,
                               CRTerm *a_value,
                               enum CRDirection a_dir) ;

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
set_padding_x_from_value (CRStyle *a_style,
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
set_border_x_width_from_value (CRStyle *a_style,
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
set_border_x_style_from_value (CRStyle *a_style,
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

	return result ;
}


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
                status = set_padding_x_from_value 
                        (a_this, value, DIR_TOP) ;
                break ;

        case PROP_PADDING_RIGHT:
                status = set_padding_x_from_value 
                        (a_this, value, DIR_RIGHT) ;
                break ;
        case PROP_PADDING_BOTTOM:
                status = set_padding_x_from_value 
                        (a_this, value, DIR_RIGHT) ;
                break ;

        case PROP_PADDING_LEFT:
                status = set_padding_x_from_value 
                        (a_this, value, DIR_LEFT) ;
                break ;
                
        case PROP_BORDER_TOP_WIDTH:
                status = set_border_x_width_from_value (a_this, value,
                                                        DIR_TOP) ;
                break ;

        case PROP_BORDER_RIGHT_WIDTH:
                status = set_border_x_width_from_value (a_this, value,
                                                        DIR_RIGHT) ;
                break ;

        case PROP_BORDER_BOTTOM_WIDTH:
                status = set_border_x_width_from_value (a_this, value,
                                                        DIR_BOTTOM) ;
                break ;

        case PROP_BORDER_LEFT_WIDTH:
                status = set_border_x_width_from_value (a_this, value,
                                                        DIR_BOTTOM) ;
                break ;

        case PROP_BORDER_TOP_STYLE:
                status = set_border_x_style_from_value (a_this, value,
                                                        DIR_TOP) ;
                break ;

        case PROP_BORDER_RIGHT_STYLE:
                status = set_border_x_style_from_value (a_this, value,
                                                        DIR_RIGHT) ;
                break ;

        case PROP_BORDER_BOTTOM_STYLE:
                status = set_border_x_style_from_value (a_this, value,
                                                        DIR_BOTTOM) ;
                break ;

        case PROP_BORDER_LEFT_STYLE: 
                status = set_border_x_style_from_value (a_this, value,
                                                        DIR_LEFT) ;
                break ;

        case PROP_MARGIN_TOP:
                break ;

        case PROP_MARGIN_RIGHT:
                break ;

        case PROP_MARGIN_BOTTOM:
                break ;

        case PROP_MARGIN_LEFT:
                break ;

        case PROP_DISPLAY:
                break ;

        case PROP_POSITION:
                break ;

        case PROP_TOP:
                break ;

        case PROP_RIGHT:
                break ;

        case PROP_BOTTOM:
                break ;

        case PROP_LEFT:
                break ;

        case PROP_FLOAT:
                break ;

        case PROP_WIDTH:
                break ;

        default:
                return CR_UNKNOWN_TYPE_ERROR ;
        }

        return status ;
        
}

void
cr_style_destroy (CRStyle *a_this)
{
	g_return_if_fail (a_this) ;

	g_free (a_this) ;
}
