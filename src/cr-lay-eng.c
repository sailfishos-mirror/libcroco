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
                if (cur_box)
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
