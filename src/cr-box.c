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

#include <string.h>
#include "cr-box.h"

/**
 *@file
 *The definition file of the #CRBox class.
 */


static enum CRBoxType
cr_box_guess_type (CRStyle *a_style) ;


static enum CRStatus
cr_box_layout_normal (CRBox *a_this) ;


static enum CRStatus
cr_box_layout_float (CRBox *a_this) ;

static enum CRStatus
cr_box_layout_absolute (CRBox *a_this) ;

static enum CRStatus
cr_box_layout (CRBox *a_this) ;


/******************************
 *Private methods
 ******************************/

static enum CRBoxType
cr_box_guess_type (CRStyle *a_style)
{
        enum CRBoxType box_type = BOX_TYPE_INLINE ;
        
        if (!a_style)
                return box_type ;

        switch (a_style->display)
        {
        case DISPLAY_NONE:
                break ;

        case DISPLAY_INLINE:
        case DISPLAY_MARKER:
                box_type = BOX_TYPE_INLINE ;
                break ;
        
        case DISPLAY_BLOCK:                
        case DISPLAY_LIST_ITEM:
        case DISPLAY_TABLE:
        case DISPLAY_INLINE_TABLE:
        case DISPLAY_TABLE_ROW_GROUP:
        case DISPLAY_TABLE_HEADER_GROUP:
        case DISPLAY_TABLE_FOOTER_GROUP:
        case DISPLAY_TABLE_ROW:
        case DISPLAY_TABLE_COLUMN_GROUP:
        case DISPLAY_TABLE_COLUMN:
        case DISPLAY_TABLE_CELL:
        case DISPLAY_TABLE_CAPTION:
                box_type = BOX_TYPE_BLOCK ;
                break ;

        case DISPLAY_COMPACT:
                box_type = BOX_TYPE_COMPACT ;
                break ;

        case DISPLAY_RUN_IN:
                box_type = BOX_TYPE_RUN_IN ;
                break ;

        case DISPLAY_INHERIT:
                if (a_style->parent_style)
                        box_type =  
                                cr_box_guess_type (a_style->parent_style) ;
                break ;

        default:
                box_type = BOX_TYPE_INLINE ;
                break ;
        }

        return box_type ;
}


static enum CRStatus
cr_box_layout_normal (CRBox *a_this)
{
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && a_this->style,
                              CR_BAD_PARAM_ERROR) ;

        if (a_this->style->position != POSITION_STATIC
            && a_this->style->position != POSITION_RELATIVE)
        {
                return CR_UNEXPECTED_POSITION_SCHEME ;
        }

        if (a_this->parent == NULL 
            && a_this->prev == NULL)
        {
                a_this->outer_edge.x = 0;
                a_this->outer_edge.y = 0;
        }
        else
        {
                /*this is not the topmost root box.
                 *So,
                 *1/depending on the width/height 
                 *on the preceding
                 *box and on the type 
                 *(inline/block etc...) of the current box,
                 *set the top left corner 
                 *position of the current box.
                 *2/compute the content edge.
                 *3/compute the border edge.
                 *4/compute the outer edge (margin edge).
                 */
                switch (a_this->type)
                {
                case BOX_TYPE_BLOCK:
                case BOX_TYPE_ANONYMOUS_BLOCK:
                {
                        /*a_this->outer_edge.x = */
                                break ;
                }
                case BOX_TYPE_COMPACT:
                case BOX_TYPE_RUN_IN:
                case BOX_TYPE_INLINE:
                case BOX_TYPE_ANONYMOUS_INLINE:
                        break ;

                default:
                        break ;
                }
        }

        return status ;
}

static enum CRStatus
cr_box_lay_out (CRBox *a_this)
{
        g_return_val_if_fail (a_this && a_this->style,
                              CR_BAD_PARAM_ERROR) ;

        switch (a_this->style->position)
        {
        case POSITION_STATIC:
        case POSITION_RELATIVE:
                cr_box_layout_normal (a_this) ;
                break ;

        case POSITION_ABSOLUTE:
        case POSITION_FIXED:
                /*cr_box_layout_absolute (a_this) ;*/
                break ;

        case POSITION_INHERIT:
                break ;
        }
        return CR_OK ;
}

/*******************************
 *Public methods
 *******************************/

CRBoxData *
cr_box_data_new (xmlNode *a_node)
{
        CRBoxData *result = NULL ;

        result = g_try_malloc (sizeof (CRBoxData)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL;
        }
        memset (result, 0, sizeof (CRBoxData)) ;
        result->node = a_node ;
        return result ;
}

void
cr_box_data_destroy (CRBoxData *a_this)
{
        if (!a_this)
                return ;

        g_free (a_this) ;
}

/**
 *Instanciates a new box.
 *Everything is initialized to zero in it.
 *@return the newly created box.
 */
CRBox *
cr_box_new (CRStyle *a_style)
{
	CRBox *result = NULL ;
        CRStyle *style = a_style ;

	result = g_try_malloc (sizeof (CRBox)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		goto error ;
	}
	memset (result, 0, sizeof (CRBox)) ;

        if (!style)
        {
                style = cr_style_new () ;
                if (!style)
                {
                        cr_utils_trace_info 
                                ("Could not create style object") ;
                        cr_utils_trace_info ("System may be out of memory") ;
                        goto error ;
                }
        }
        result->style = style ;
        result->type = cr_box_guess_type (result->style) ;
	return result ;

 error:
        if (result)
        {
                cr_box_destroy (result) ;
                result = NULL ;
        }

        return NULL ;
} 

/**
 *Appends a child box to at the end of the current box's children.
 *@param a_this the current box.
 *@param a_to_append, the box to append.
 *@retrurn CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_box_append_child (CRBox *a_this, CRBox *a_to_append)
{
	CRBox * cur = NULL ;

	g_return_val_if_fail (a_this && a_to_append, CR_BAD_PARAM_ERROR) ;

	if (!a_this->children)
	{
		a_this->children = a_to_append ;
		return CR_OK ;
	}

	for (cur = a_this->children ; cur && cur->next ; cur = cur->next)
		;

	cur->next = a_to_append ;

	return CR_OK ;
}


/**
 *Inserts a sibling box between two adjacent sibling nodes.
 *@param a_prev the box after which we have to insert a new one.
 *@param a_next the box before which we have to insert a new one.
 *@param a_to_insert the node to insert.
 */
enum CRStatus
cr_box_insert_sibling (CRBox *a_prev,
                       CRBox *a_next,
                       CRBox *a_to_insert)
{
	g_return_val_if_fail (a_prev && a_prev->parent
			      && a_next && a_prev->next == a_next
			      && a_next->parent == a_prev->parent
			      && a_to_insert
			      && a_to_insert->parent != a_prev->parent,
			      CR_BAD_PARAM_ERROR) ;

	a_prev->next = a_to_insert ;
	a_to_insert->prev = a_prev ;
	a_to_insert->next = a_next ;
	a_next->prev = a_to_insert ;

	return CR_OK ;
}


/**
 *This is for debug purposes ...
 *Gives a string representation of the box tree.
 *@return the build string of NULL in case of an error.
 */
GString *
cr_box_to_string (CRBox *a_this, gulong a_nb_indent)
{
        g_return_val_if_fail (a_this, NULL) ;
        
        return NULL ;
}

/**
 *Destructor of #CRBox.
 *recursively destroys all
 *the children nodes of the current node.
 *@param a_this the current box to destroy.
 */
void
cr_box_destroy (CRBox *a_this)

{
	g_return_if_fail (a_this) ;

	if (a_this->children)
	{
		CRBox *cur = NULL;

		for (cur = a_this->children ; cur ; cur = cur->next)
		{
			cr_box_destroy (cur) ;
		}
		a_this->children = NULL ;
	}

	g_free (a_this) ;
}

