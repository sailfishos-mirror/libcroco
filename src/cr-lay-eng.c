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
} ;

typedef struct
{
        CRStyle *style ;
} CRNodeAnnotation ;

static enum CRStatus
cr_lay_eng_annotate_tree_real (CRLayEng *a_this,
                               xmlNode *a_root_node) ;

static enum CRStatus
cr_lay_eng_destroy_tree_annotation_real (xmlNode *a_node) ;

static CRNodeAnnotation *
cr_node_annotation_new (CRStyle *a_style) ;

static void
cr_node_annotation_destroy (CRNodeAnnotation *a_this) ;


/**********************
 *Private methods.
 **********************/

static CRNodeAnnotation *
cr_node_annotation_new (CRStyle *a_style)
{
        CRNodeAnnotation *result = NULL ;

        result = g_try_malloc (sizeof (CRNodeAnnotation)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }
        memset (result, 0, sizeof (CRNodeAnnotation)) ;

        if (a_style)
        {
                result->style = a_style ;
                cr_style_ref (a_style) ;
        }

        return result ;
}

static void
cr_node_annotation_destroy (CRNodeAnnotation *a_this)
{
        g_return_if_fail (a_this) ;

        if (a_this->style)
        {
                cr_style_unref (a_this->style) ;
                a_this->style = NULL ;
        }

        if (a_this)
        {
                g_free (a_this) ;
                a_this = NULL ;
        }
}

static enum CRStatus
cr_lay_eng_annotate_tree_real (CRLayEng * a_this,
                               xmlNode *a_root_node)
{
        enum CRStatus status = CR_OK ;
        xmlNode *cur = NULL ;
        CRSelEng * sel_eng = NULL ;/*selection engine*/

        g_return_val_if_fail (a_this
                              && PRIVATE (a_this)
                              && PRIVATE (a_this)->cascade
                              && a_root_node,
                              CR_BAD_PARAM_ERROR) ;

        sel_eng = cr_sel_eng_new () ;

        g_return_val_if_fail (sel_eng, CR_ERROR) ;

        for (cur = a_root_node ; cur ; cur = cur->next)
        {
                CRStyle *style = NULL, *parent_style = NULL ;
                CRNodeAnnotation *annotation = NULL ;

                /*build here the node annotation*/
                if (cur->parent
                    && cur->parent->_private)
                {
                        parent_style = 
                                ((CRNodeAnnotation*)
                                 cur->parent->_private)->style ;
                }

                status = 
                        cr_sel_eng_get_matched_style 
                        (sel_eng, PRIVATE (a_this)->cascade,
                         cur, parent_style, &style) ;

                if (status == CR_OK
                    && style)
                {
                        annotation =
                                cr_node_annotation_new (style) ;
                        if (annotation)
                        {
                                cur->_private = annotation ;
                        }
                        annotation = NULL ;
                        style = NULL ;
                }

                if (style)
                {
                        cr_style_destroy (style) ;
                        style = NULL ;
                }

                /*walk through what remains from the tree*/
                if (cur->children)
                {
                        status = 
                                cr_lay_eng_annotate_tree_real 
                                (a_this, cur->children) ;

                        if (status != CR_OK)
                                return status ;
                }
        }

        if (sel_eng)
        {
                cr_sel_eng_destroy (sel_eng) ;
                sel_eng = NULL ;
        }

        return CR_OK ;
}

static enum CRStatus
cr_lay_eng_destroy_tree_annotation_real (xmlNode *a_node)
{
        xmlNode *cur = NULL ;

        for (cur = a_node ; cur ; cur = cur->next)
        {
                if (cur->_private)
                {
                        cr_node_annotation_destroy (cur->_private) ;
                        cur->_private = NULL ;
                }
                if (cur->children)
                {
                        cr_lay_eng_destroy_tree_annotation_real 
                                (cur->children) ;
                }
        }
        return CR_OK ;
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
cr_lay_eng_build_annotated_doc (CRLayEng *a_this,
                                xmlDoc *a_doc,
                                CRCascade *a_cascade)
{
        xmlNode *root_node = NULL ;

        g_return_val_if_fail (a_this && a_doc && a_cascade,
                              CR_BAD_PARAM_ERROR) ;

        root_node = xmlDocGetRootElement (a_doc) ;
        if (!root_node)
                return CR_NO_ROOT_NODE_ERROR ;

        PRIVATE (a_this)->cascade = a_cascade ;
        return cr_lay_eng_annotate_tree_real (a_this, root_node) ;

        return CR_OK ;
}

enum CRStatus
cr_lay_eng_destroy_doc_annotation (xmlDoc *a_xml_doc)
{
        xmlNode *root_node = NULL ;

        root_node = xmlDocGetRootElement (a_xml_doc) ;
        if (!root_node)
                return CR_OK ;

        return cr_lay_eng_destroy_tree_annotation_real (root_node) ;
}

void
cr_lay_eng_destroy (CRLayEng *a_this)
{
	g_return_if_fail (a_this) ;

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
