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

static glong
cr_box_get_rightmost_x (CRBox *a_this) ;

static glong
cr_box_get_bottommost_y (CRBox *a_this) ;

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

/**
 *Guess the type of a box from the 'position' rule
 *contained in its style data structure.
 *@param a_style the style data structure associated to the box.
 */
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

static glong
cr_box_get_bottommost_y (CRBox *a_this)
{
        return (a_this->outer_edge.y
                +
                a_this->outer_edge.y_offset
                +
                a_this->outer_edge.height) ;
}

/**
 *Computes the abscissa of the rightmost side
 *of the current box.
 *@param a_box the current box.
 *@return a positve or 0 number if the computation went well,
 *-1 otherwise.
 */
static glong
cr_box_get_rightmost_x (CRBox *a_this)
{       
        if (!a_this)
                return 0 ;

        return (a_this->outer_edge.x 
                +
                a_this->outer_edge.x_offset
                +
                a_this->outer_edge.width) ;
}

/**
 *Lay the box out according to "Normal flow"
 *as decribed in css2 spec chap 9.4.
 *In normal flow, a box belongs to a formating context
 *that may be block or inline. In block formating context,
 *boxes are laid out verticaly, one under an other.
 *In inline formatting context, boxes are laid out horizontally,
 *usually from the left to the right, unless we support bidi.
 *@param a_this the current box.
 */
static enum CRStatus
cr_box_layout_normal (CRBox *a_this)
{
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && a_this->style,
                              CR_BAD_PARAM_ERROR) ;

        /*
         *Only boxes that have
         *the position rule set to 'static' or 'relative'
         *can be part of a normal formatting context.
         */
        if (a_this->style->position != POSITION_STATIC
            && a_this->style->position != POSITION_RELATIVE)
        {
                return CR_UNEXPECTED_POSITION_SCHEME ;
        }

        /*
         *TODO
         *compute the "computed values" of the style data structure.
         */
        switch (a_this->type)
        {
        case BOX_TYPE_BLOCK:
        case BOX_TYPE_ANONYMOUS_BLOCK:
        {
                CRBox *cont_box = a_this->parent ;
                /************************************
                 *We are in a block formating context 
                 ************************************/

                /*
                 *position the 'x' of the top
                 *left corner of this box
                 *at the leftmost abscissa of it's 
                 *containing box.
                 *Position the 'y' of 
                 *the top left corner of this
                 *just under the previous box.
                 */                
                if (!cont_box)
                        a_this->outer_edge.x = 0 ;
                else
                        a_this->outer_edge.x = 
                                cont_box->inner_edge.x ;
                
                a_this->outer_edge.y =
                        cr_box_get_bottommost_y (a_this->prev) ;

                a_this->outer_edge.x =
                        cr_box_get_rightmost_x (a_this->prev) ;

                /*******************************************
                 *Now, compute the inner edge of this box;
                 *which means 
                 *1/set the left border and 
                 *left padding edges.
                 *2/compute the left most x and topmost y of
                 *the inner box and.
                 *3/Compute the outer edge of the containing
                 *box; this is recursive.
                 *******************************************/

                /*
                 *1/ => left side of outer edge is separated from
                 *left side of border edge by "margin-left"... same
                 *principle applies for padding edge and inner edge.
                 */

                /*
                 *TODO: collapse this margin !!!. 
                 *See css2 chap 8.3.1 to see what "collapsing" means.
                 */
                a_this->border_edge.x =
                        a_this->outer_edge.x
                        - 
                        a_this->style->margin_left.val ;
                a_this->border_edge.y =
                        a_this->outer_edge.y
                        -
                        a_this->style->margin_top.val ;

                a_this->padding_edge.x =
                        a_this->border_edge.x 
                        -
                        a_this->style->border_left_width.val ;
                a_this->padding_edge.y =
                        a_this->border_edge.y
                        -
                        a_this->style->border_top_width.val ;

                /*
                 *Now 2/
                 */
                a_this->inner_edge.x =
                        a_this->padding_edge.x
                        -
                        a_this->style->padding_left.val ;
                a_this->inner_edge.y =
                        a_this->padding_edge.y
                        -
                        a_this->style->padding_left.val ;

                /*
                 *And now, 3/
                 */
                if (a_this->children)
                {
                        cr_box_layout (a_this->children) ;
                }
                else
                {
                        /*
                         *this box may have a content.
                         *TODO: compute it's width and height.
                         *then, when computed, update the
                         *children max width size in the parent box.
                         */                        
                }

                /*******************************************
                 *Inner edge position (x,y) computing is 
                 *finished. (we have it's width).
                 *So now, we can compute the widths of the
                 *remaining three other boxes 
                 *(padding edge, border edge and outer edge)
                 ******************************************/
                
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

        return status ;
}

static enum CRStatus
cr_box_layout (CRBox *a_this)
{
        CRBox *cur_box = NULL ;

        g_return_val_if_fail (a_this && a_this->style,
                              CR_BAD_PARAM_ERROR) ;

        for (cur_box = a_this ; cur_box ; 
             cur_box = cur_box->next)
        {
                switch (cur_box->style->position)
                {
                case POSITION_STATIC:
                case POSITION_RELATIVE:
                        cr_box_layout_normal (cur_box) ;
                        break ;

                case POSITION_ABSOLUTE:
                case POSITION_FIXED:
                        /*cr_box_layout_absolute (a_this) ;*/
                        break ;

                case POSITION_INHERIT:
                        break ;
                }
        }
        return CR_OK ;
}

/*******************************
 *Public methods
 *******************************/

/**
 *Instanciates a new #CRBoxData.
 *@param a_node the xml node to store in the box.
 *@return the newly built #CRBoxData, or null if an error arises.
 */
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

/**
 *Destructor of #CRBoxData.
 *@param a_this the current instance 
 *of #CRBoxData to be destroyed.
 */
void
cr_box_data_destroy (CRBoxData *a_this)
{
        if (!a_this)
                return ;

        g_free (a_this) ;
}

/**
 *Instanciates a new #CRBoxContent and set the
 *content to text content.
 *@param a_text the text content.
 */
CRBoxContent *
cr_box_content_new_from_text (guchar *a_text)
{
        CRBoxContent *result = NULL ;

        result = g_try_malloc (sizeof (CRBoxContent)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }
        memset (result, 0, sizeof (CRBoxContent)) ;
        result->u.text = g_strdup (a_text) ;
        result->type = TEXT_CONTENT_TYPE ;
        return result ;
}

/**
 *Destructor of #CRBoxContent.
 *@param a_this the current instance of #CRBoxContent
 *to be destroyed.
 */
void
cr_box_content_destroy (CRBoxContent *a_this)
{
        if (!a_this)
                return ;

        switch (a_this->type)
        {
        case TEXT_CONTENT_TYPE:
                if (a_this->u.text)
                {
                        g_free (a_this->u.text) ;
                        a_this->u.text = NULL ;
                }
                break ;

        default:
                cr_utils_trace_info ("Unrecognized box content type") ;
                cr_utils_trace_info ("This seems to be a mem leak") ;
                break ;
        }
        g_free (a_this) ;
        return ;
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

