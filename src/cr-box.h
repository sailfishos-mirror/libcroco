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

#ifndef __CR_BOX_H__
#define __CR_BOX_H__

#include "cr-style.h"

G_BEGIN_DECLS

/**
 *@file 
 *the declaration of the #CRBox class.
 */
enum BoxContentType
{
	NO_CONTENT_TYPE ;
	TEXT_CONTENT_TYPE ;
	IMAGE_CONTENT_TYPE ;
} ;

typedef struct _ImageContentDesc ImageContentDesc ;
struct _ImageContentDesc
{
	guchar *img_buf ;
	gulong *len ;
	guchar *img_path ;
} ;

typedef struct _BoxContent BoxContent ;
struct _BoxContent
{
	enum BoxContentType type ;
	union 
	{
		guchar *text ;
		ImageContentDesc *img_desc ;
	} kind ;
} ;

typedef struct _BoxEdge BoxEdge ;

/**
 *An internal data structure
 *used by #CRBox.
 *An edge is an rectangular area
 *defined by the coordinates of the two points
 *that delimit its bias (in french: bias = diagonale).
 *(x0,y0) is the top left delimiter point of the bias, and
 *(x1,y1) is the bottom right delimiter point of the bias.
 */
struct _BoxEdge
{
	gulong x0,y0,x1,y1 ;
} ;


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
	BoxEdge inner_edge ;

	/**
	 *The padding edge.
	 *It includes the inner_edge plus a
	 *a surrounding area called "padding".
	 *When the padding is inexistent, the
	 *padding edge equals the inner or content edge.
	 */
	BoxEdge padding_edge ;

	/**
	 *The border edge.
	 *It includes the padding edge plus
	 *a surrounding area called "border".
	 *If the border is inexistant, the
	 *border edge equals the padding edge.
	 *
	 */
	BoxEdge border_edge ;

	/**
	 *The outer edge aka margin edge.
	 *It includes the border edge plus
	 *a surrounding area called "margin".
	 *If the border is inexistant, the outer
	 *edge equals the border_edge.
	 */
	BoxEdge outer_edge ;

	/**
	 *The values of all the preceding
	 *edges are infered from the value
	 *of the 'style' field.
	 */

	/**
	 *The content (text or image) of this
	 *box
	 */
	BoxContent content ;

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
	 *
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
};

G_END_DECLS

#endif __CR_BOX_H__
