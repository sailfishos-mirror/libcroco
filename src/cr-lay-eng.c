/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the 
 * GNU Lesser General Public
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
#include "cr-lay-eng.h"
#include "cr-sel-eng.h"

#define PRIVATE(a_this) ((a_this)->priv)

/**
 *@file
 *The definition of the #CRLayEng class.
 *Highly unstable and experimental so far.
 *This is in developement so the api is *really* gonna change ...
 */

struct _CRLayEngPriv
{
	CRCascade *cascade ;
        CRSelEng *sel_eng ;
} ;

static CRBox *
cr_lay_eng_create_box_tree_real (CRLayEng * a_this,
                                 xmlNode *a_root_node,
                                 CRBox *a_parent_box) ;

/**********************
 *Private methods.
 **********************/

/**
 *Sets the box style values so that
 *it has no padding, no border, no margin.
 *The other style values are left as is cause
 *they must have been set prior to calling this
 *function.
 */
static void
init_anonymous_text_box (CRBox *a_box)
{
        g_return_if_fail (a_box && a_box->style) ;
        
        cr_num_set (&a_box->style->padding_top, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->padding_right, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->padding_bottom, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->padding_left, 0, NUM_LENGTH_PX) ;
        
        cr_num_set (&a_box->style->border_top_width, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->border_right_width, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->border_bottom_width, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->border_left_width, 0, NUM_LENGTH_PX) ;

        cr_num_set (&a_box->style->margin_top, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->margin_right, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->margin_bottom, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->margin_left, 0, NUM_LENGTH_PX) ;
        
        a_box->style->border_top_style = BORDER_STYLE_NONE ;
        a_box->style->border_right_style = BORDER_STYLE_NONE ;
        a_box->style->border_bottom_style = BORDER_STYLE_NONE ;
        a_box->style->border_left_style = BORDER_STYLE_NONE ;

        cr_num_set (&a_box->style->top.num, 0, NUM_LENGTH_PX) ;
        a_box->style->top.type = OFFSET_DEFINED ;
        cr_num_set (&a_box->style->right.num, 0, NUM_LENGTH_PX) ;
        a_box->style->right.type = OFFSET_DEFINED ;
        cr_num_set (&a_box->style->bottom.num, 0, NUM_LENGTH_PX) ;
        a_box->style->bottom.type = OFFSET_DEFINED ;
        cr_num_set (&a_box->style->left.num, 0, NUM_LENGTH_PX) ;
        a_box->style->left.type = OFFSET_DEFINED ;
        
        a_box->style->float_type = FLOAT_NONE ;
}

/**
 *Creates a box sub tree from an xml node tree.
 *@param a_this the current instance of #CRLayEng.
 *@param a_root_node the root node of the xml tree.
 *@param a_parent_box the root of the box tree to build.
 *@return the newly built box tree, or NULL if an error
 *happens.
 */
static CRBox *
cr_lay_eng_create_box_tree_real (CRLayEng * a_this,
                                 xmlNode *a_root_node,
                                 CRBox *a_parent_box)
{
        enum CRStatus status = CR_OK ;
        xmlNode *cur = NULL ;
        CRBox *cur_box = NULL ;
        CRBoxData *box_data = NULL ;

        g_return_val_if_fail (a_this
                              && PRIVATE (a_this)
                              && PRIVATE (a_this)->cascade
                              && a_root_node, NULL) ;

        if (!PRIVATE (a_this)->sel_eng)
        {
                PRIVATE (a_this)->sel_eng = cr_sel_eng_new () ;
                if (!PRIVATE (a_this)->sel_eng)
                {
                        cr_utils_trace_info 
                                ("Could not create selection engine") ;
                        cr_utils_trace_info 
                                ("System may be out of memory") ;
                        return NULL ;
                }
        }

        for (cur = a_root_node ; cur ; cur = cur->next)
        {
                CRStyle *style = NULL, *parent_style = NULL ;                

                if (cur->type != XML_ELEMENT_NODE
                    && cur->type != XML_TEXT_NODE)
                        continue ;

                /*build here the node annotation*/
                if (cur->parent && a_parent_box && a_parent_box->style)
                        parent_style = a_parent_box->style ;

                if (cur->type == XML_ELEMENT_NODE)
                {
                        status = 
                                cr_sel_eng_get_matched_style 
                                (PRIVATE (a_this)->sel_eng, 
                                 PRIVATE (a_this)->cascade,
                                 cur, parent_style, &style) ;

                        if (status != CR_OK
                            || (style && style->display == DISPLAY_NONE))
                        {
                                continue ;
                        }

                        /*here, build the box,
                         *append it to the box tree
                         *and update all it attributes but
                         *the positioning. The positioning will
                         *be updated later via the cr_box_reflow() method.
                         */
                        cur_box = cr_box_new (style) ;
                        if (!cur_box)
                        {
                                cr_utils_trace_info
                                        ("Could not create a box") ;
                                cr_utils_trace_info
                                        ("The system may be out of memory") ;
                                return NULL ;
                        }

                        if (a_parent_box)
                                cr_box_append_child (a_parent_box,
                                                     cur_box) ;
                        style = NULL ;

                        /*
                         *store a pointer to the node that generated
                         *the current box into that current box.
                         */
                        box_data = cr_box_data_new (cur) ;
                        if (!box_data)
                        {
                                cr_utils_trace_info ("Out of memory") ;
                                goto error ;
                        }
                        cur_box->croco_data = (gpointer)box_data ;
                        box_data = NULL ;

                        if (style)
                        {
                                cr_style_destroy (style) ;
                                style = NULL ;
                        }
                }
                else if (cur->type == XML_TEXT_NODE)
                {
                        CRBoxContent *box_content = NULL ;
                        xmlChar *node_text = NULL ;

                        if (xmlIsBlankNode (cur))
                                continue ;

                        node_text = xmlNodeGetContent (cur) ;
                        if (node_text)
                        {
                                box_content = 
                                        cr_box_content_new_from_text 
                                        (node_text) ;
                                xmlFree (node_text) ;
                                node_text = NULL ;
                        }
                        if (box_content)
                        {
                                /*
                                 *create here an anonymous box
                                 *which style inherits the style
                                 *of the parent box.
                                 */
                                cur_box = cr_box_new (parent_style) ;
                                if (!cur_box)
                                {
                                        cr_utils_trace_info
                                                ("could not create "
                                                 "anonymous box") ;
                                        goto error ;
                                }
                                cur_box->content = box_content ;
                                box_content = NULL ;
                                /*
                                 *the anonymous box
                                 *must have no margin, 
                                 *no padding, no border,
                                 *no border style, no offset
                                 */
                                init_anonymous_text_box (cur_box) ;
                                cr_box_append_child (a_parent_box,
                                                     cur_box) ;
                                cur_box = NULL ;
                        }
                }
                else 
                {
                        cr_utils_trace_info 
                                ("xml node type neither element nor text") ;
                        cr_utils_trace_info 
                                ("this should not happen. This is a bug") ;
                }

                /*walk through what remains from the tree*/
                if (cur->children)
                {
                        cr_lay_eng_create_box_tree_real
                                (a_this, cur->children, cur_box) ;
                }
        }

        return cur_box ;

 error:
        if (cur_box)
        {
                cr_box_destroy (cur_box) ;
                cur_box = NULL ;                
        }
        if (box_data)
        {
                cr_box_data_destroy (box_data) ;
                box_data = NULL ;
        }

        return NULL ;
}


/**********************
 *Public methods.
 **********************/

CRLayEng *
cr_lay_eng_new (void)
{
	CRLayEng *result = NULL;

	result = g_try_malloc (sizeof (CRLayEng)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRLayEng)) ;

	PRIVATE (result) = g_try_malloc (sizeof (CRLayEngPriv)) ;
	if (!PRIVATE (result))
	{
		cr_utils_trace_info ("Out of memory") ;
		g_free (result) ;
		result = NULL ;
		return NULL ;
	}
	memset (PRIVATE (result), 0, sizeof (CRLayEngPriv)) ;

	return result ;
}

/**
 *Creates the box model from an xml document.
 *@param a_this the current instance of #CRLayEng.
 *@param a_doc the current xml document.
 *@param a_cascade the css2 stylesheet cascade.
 *@param a_box_model out parameter. The returned
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_lay_eng_build_box_tree (CRLayEng *a_this,
                           xmlDoc *a_doc,
                           CRCascade *a_cascade,
                           CRBox **a_box_model)
{
        xmlNode *root_node = NULL ;

        g_return_val_if_fail (a_this && a_doc && a_cascade,
                              CR_BAD_PARAM_ERROR) ;

        root_node = xmlDocGetRootElement (a_doc) ;
        if (!root_node)
                return CR_NO_ROOT_NODE_ERROR ;

        PRIVATE (a_this)->cascade = a_cascade ;

        *a_box_model = 
                cr_lay_eng_create_box_tree_real (a_this, root_node,
                                                 NULL) ;

        return CR_OK ;
}


/**
 *Destuctor of #CRLayEng.
 *@param a_this the current instance of #CRLayEng.
 */
void
cr_lay_eng_destroy (CRLayEng *a_this)
{
	g_return_if_fail (a_this) ;

        if (PRIVATE (a_this)->sel_eng)
        {
                cr_sel_eng_destroy (PRIVATE (a_this)->sel_eng) ;
                PRIVATE (a_this)->sel_eng = NULL ;
        }

	if (PRIVATE (a_this))
	{
		g_free (PRIVATE (a_this)) ;
		PRIVATE (a_this) = NULL ;
	}

	if (a_this)
	{
		g_free (a_this) ;
	}
}
