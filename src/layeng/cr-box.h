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

#ifndef __CR_BOX_H__
#define __CR_BOX_H__

#include "cr-style.h"
#include "libxml/tree.h"

/*
 *$Id$
 */

G_BEGIN_DECLS

/**
 *@file 
 *the declaration of the #CRBox class.
 */
enum CRBoxContentType
{
	NO_CONTENT_TYPE,
	TEXT_CONTENT_TYPE,
	IMAGE_CONTENT_TYPE
} ;

typedef struct _CRImageContentDesc CRImageContentDesc ;
struct _CRImageContentDesc
{
	guchar *img_buf ;
	gulong *len ;
	guchar *img_path ;
} ;

typedef struct _CRBoxContent CRBoxContent ;
struct _CRBoxContent
{
	enum CRBoxContentType type ;
	union 
	{
		guchar *text ;
		CRImageContentDesc *img_desc ;
	} u ;

        /*
         *a place where the rendered content can be cached.
         *This can be usefull because sometime, during the layout,
         *calculating the size of the content is better achieve by
         *rendering it. In this case, the rendered content is just cached
         *here so that the rendering stage can just pick it.
         */
        gpointer content_cache ;
} ;

typedef struct _CRBoxEdge CRBoxEdge ;

/**
 *An internal data structure
 *used by #CRBox.
 *An edge is an rectangular area
 *defined by the coordinates of the it top left corner,
 *its width and height.
 */
struct _CRBoxEdge
{
	gulong x, y, width, height, x_offset, y_offset ;
} ;


enum CRBoxType
{
        BOX_TYPE_UNDEFINED,
        BOX_TYPE_BOX_MODEL,
        BOX_TYPE_BLOCK,
        BOX_TYPE_ANONYMOUS_BLOCK,
        BOX_TYPE_INLINE,
        BOX_TYPE_ANONYMOUS_INLINE,
        BOX_TYPE_COMPACT,
        BOX_TYPE_RUN_IN
} ;

typedef struct _CRBoxData CRBoxData ;

/**
 *Some data stored in the box.
 *these data are about the node which
 *generated the current box.
 */
struct _CRBoxData
{
        /**
         *The xml node which generated
         *the css box. If NULL, it means
         *that this node is an anonymous node
         */
        xmlNode *xml_node ;
} ;

CRBoxData *
cr_box_data_new (xmlNode *a_node)  ;

void
cr_box_data_destroy (CRBoxData *a_this) ;

typedef struct _CRBoxModel CRBoxModel ;
typedef struct _CRBox CRBox ;

/**
 *The CRBox class.
 *Abstracts the css2 box as defined in the
 *css2 spec in chapter 8.
  *It is actually a tree of boxes, each being "generated"
 *by an xml document tree node.
 */
struct _CRBox
{
        enum CRBoxType type ;

	/**
	 *The inner edge, or content edge.
	 *Is the one that immediately wraps
	 *the content of the box.
	 *The content of the box may be
	 *a text, an image, or a set of boxes.
	 *If the content is a set of boxes, then
	 *this set is inevitably the set of children
	 *boxes of this current box.
	 */
	CRBoxEdge inner_edge ;

	/**
	 *The padding edge.
	 *It includes the inner_edge plus a
	 *a surrounding area called "padding".
	 *When the padding is inexistent, the
	 *padding edge equals the inner or content edge.
	 */
	CRBoxEdge padding_edge ;

	/**
	 *The border edge.
	 *It includes the padding edge plus
	 *a surrounding area called "border".
	 *If the border is inexistant, the
	 *border edge equals the padding edge.
	 */
	CRBoxEdge border_edge ;

	/**
	 *The outer edge aka margin edge.
	 *It includes the border edge plus
	 *a surrounding area called "margin".
	 *If the border is inexistant, the outer
	 *edge equals the border_edge.
	 */
	CRBoxEdge outer_edge ;

	/**
	 *The values of all the preceding
	 *edges are infered from the value
	 *of the 'style' field.
	 */

	/**
	 *The content (text or image) of this
	 *box
	 */
	CRBoxContent *content ;

	/**
	 *if TRUE, it means that this box has
	 *a simple content. Simple content means
	 *either a text or image content.
	 *Normally, to be logic, if this is
	 *set to TRUE, the children pointer should
	 *be NULL ... see what I mean ?
	 */
	gboolean as_simple_content ;

	/**
	 *The value infered from what has been found
	 *in the css stylesheet.
	 */
	CRStyle *style ;

	/**the containing box*/
	CRBox *parent ;

	/**the next box in the flow*/
	CRBox *next ;

	/**the previous box in the flow*/
	CRBox *prev ;

	/**the children (contained) boxes*/
	CRBox *children ;

        CRBoxModel * box_model ;

        /**some custom data used by libcroco*/
        CRBoxData *box_data ;
        /**some application data that will never 
         *be used by libcroco. Applications
         *are free to use it.
         */
        gpointer *app_data ;
        
        gulong ref_count ;
} ;

struct _CRBoxModel
{
        CRBox box ;
        gulong viewport_width ;
        gulong viewport_height ;
        gulong ref_count ;
} ;


CRBoxModel *
cr_box_model_new (void) ;

void
cr_box_model_destroy (CRBoxModel *a_this) ;

void
cr_box_model_ref (CRBoxModel *a_this) ;

gboolean
cr_box_model_unref (CRBoxModel *a_this) ;

CRBoxContent *
cr_box_content_new_from_text (guchar *a_text) ;

void
cr_box_content_destroy (CRBoxContent *a_this) ;

CRBox *
cr_box_new (CRStyle *a_this, gboolean a_set_default_style) ;

enum CRStatus
cr_box_insert_sibling (CRBox *a_prev,
                       CRBox *a_next,
                       CRBox *a_to_insert) ;
enum CRStatus
cr_box_to_string (CRBox *a_this, 
                  gulong a_nb_indent,
                  GString **a_string) ;

enum CRStatus
cr_box_dump_to_file (CRBox *a_this, 
                     gulong a_nb_indent,
                     FILE *a_filep) ;

enum CRStatus
cr_box_ref (CRBox *a_this) ;


gboolean
cr_box_unref (CRBox *a_this) ;

enum CRStatus
cr_box_append_child (CRBox *a_this, CRBox *a_to_append) ;

void
cr_box_destroy (CRBox *a_this) ;


G_END_DECLS

#endif /*__CR_BOX_H__*/

