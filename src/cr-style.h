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

#ifndef __CR_STYLE_H__
#define __CR_STYLE_H__

#include "cr-utils.h"
#include "cr-statement.h"

/**
 *@file
 *The declaration of the #CRStyle class.
 */
G_BEGIN_DECLS

typedef struct _CRStyle CRStyle ;

enum CRBorderStyle
{
        BORDER_STYLE_NONE,
        BORDER_STYLE_HIDDEN,
        BORDER_STYLE_DOTTED,
        BORDER_STYLE_DASHED,
        BORDER_STYLE_SOLID,
        BORDER_STYLE_DOUBLE,
        BORDER_STYLE_GROOVE,
        BORDER_STYLE_RIDGE,
        BORDER_STYLE_INSET,
        BORDER_STYLE_OUTSET
} ;

enum CRDisplayType
{
        DISPLAY_NONE,
        DISPLAY_INLINE,
        DISPLAY_BLOCK,
        DISPLAY_LIST_ITEM,
        DISPLAY_RUN_IN,
        DISPLAY_COMPACT,
        DISPLAY_MARKER,
        DISPLAY_TABLE,
        DISPLAY_INLINE_TABLE,
        DISPLAY_TABLE_ROW_GROUP,
        DISPLAY_TABLE_HEADER_GROUP,
        DISPLAY_TABLE_FOOTER_GROUP,
        DISPLAY_TABLE_ROW,
        DISPLAY_TABLE_COLUMN_GROUP,
        DISPLAY_TABLE_COLUMN,
        DISPLAY_TABLE_CELL,
        DISPLAY_TABLE_CAPTION,
        DISPLAY_INHERIT
} ;

enum CRPositionType
{
        POSITION_STATIC,
        POSITION_RELATIVE,
        POSITION_ABSOLUTE,
        POSITION_FIXED,
} ;

enum CRFloatType
{
        FLOAT_NONE,
        FLOAT_LEFT,
        FLOAT_RIGHT,
        FLOAT_INHERIT
} ;


#define BORDER_THIN 2
#define BORDER_MEDIUM 4
#define BORDER_THICK 6

/**
 *The css2 style class.
 *Contains computed and actual values
 *of inferred from the declarations found
 *in the stylesheets.
 *See css2 spec chapter 6.
 */
struct _CRStyle
{

	/**padding properties, in pixel*/

	CRNum padding_top ;
	CRNum padding_right ;
	CRNum padding_bottom ;
	CRNum padding_left ;

	/**border properties*/

	CRNum border_top_width ;
	CRNum border_right_width ;
	CRNum border_bottom_width ;
	CRNum border_left_width ;

	CRRgb border_top_color ;
	CRRgb border_right_color ;
	CRRgb border_bottom_color ;
	CRRgb border_left_color ;

        enum CRBorderStyle border_top_style ;
        enum CRBorderStyle border_right_style ;
        enum CRBorderStyle border_bottom_style ;
        enum CRBorderStyle border_left_style ;

	/**margin properties, in pixel*/

	CRNum margin_top ;
	CRNum margin_right ;
	CRNum margin_bottom;
	CRNum margin_left ;

        /**box display type*/
        enum CRDisplayType display ;

        /**the positioning scheme*/
        enum CRPositionType position ;
        /**box offset*/
        CRNum top ;
        CRNum right ;
        CRNum bottom ;
        CRNum left ;

        /**the float property*/
        enum CRFloatType float_type ;

        CRNum width ;
        CRStyle *parent_style ;
} ;


CRStyle *
cr_style_new (void) ;


enum CRStatus
cr_style_new_from_ruleset (CRStatement *a_stmt, 
                           CRStyle *a_parent_style,
			   CRStyle **a_style) ;
enum CRStatus
cr_style_set_style_from_decl (CRStyle *a_this, CRDeclaration *a_decl,
                              CRStyle *a_parent_style) ;

void
cr_style_destroy (CRStyle *a_this) ;

G_END_DECLS

#endif /*__CR_STYLE_H__*/
