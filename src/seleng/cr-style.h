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

#include <pango/pango.h>
#include "cr-utils.h"
#include "cr-statement.h"
#include "cr-fonts.h"

/**
 *@file
 *The declaration of the #CRStyle class.
 */
G_BEGIN_DECLS

typedef struct _CRStyle CRStyle ;

enum CRBorderStyle
{
        BORDER_STYLE_NONE = 0,
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
        POSITION_INHERIT,
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
 *A numerical css property value.
 *This data type is actually split in 3 parts:
 *1/the specified value
 *2/the computed value
 *3/the actual value.
 *To understand the semantic of these three parts,
 *see css2 spec chap 6.1 ("Specified, computed and actual values.").
 */
typedef struct _CRNumPropVal CRNumPropVal ;
struct _CRNumPropVal
{
        /**specified value*/
        CRNum sv ;
        /**computed value*/
        CRNum cv ;
        /**actual value*/
        CRNum av ;
} ;

/**
 *An rgb css property value.
 *This data type is actually split in 3 parts:
 *1/the specified value
 *2/the computed value
 *3/the actual value.
 *To understand the semantic of these three parts,
 *see css2 spec chap 6.1 ("Specified, computed and actual values.").
 */
typedef struct _CRRgbPropVal CRRgbPropVal ;
struct _CRRgbPropVal
{        
        /**specified value*/
        CRRgb sv ;
        /**computed value*/
        CRRgb cv ;
        /**actual value*/
        CRRgb av ;
} ;


enum CRNumProp
{        
        NUM_PROP_TOP=0,
        NUM_PROP_RIGHT,
        NUM_PROP_BOTTOM,
        NUM_PROP_LEFT,/*3*/

        NUM_PROP_PADDING_TOP,
        NUM_PROP_PADDING_RIGHT,
        NUM_PROP_PADDING_BOTTOM,
        NUM_PROP_PADDING_LEFT,/*7*/

        NUM_PROP_BORDER_TOP,
        NUM_PROP_BORDER_RIGHT,
        NUM_PROP_BORDER_BOTTOM,
        NUM_PROP_BORDER_LEFT,/*11*/

        NUM_PROP_MARGIN_TOP,
        NUM_PROP_MARGIN_RIGHT,
        NUM_PROP_MARGIN_BOTTOM,
        NUM_PROP_MARGIN_LEFT,/*15*/

        NUM_PROP_WIDTH,

        /*must be last*/
        NB_NUM_PROPS
} ;

enum CRRgbProp
{
        RGB_PROP_BORDER_TOP_COLOR = 0,
        RGB_PROP_BORDER_RIGHT_COLOR,
        RGB_PROP_BORDER_BOTTOM_COLOR,
        RGB_PROP_BORDER_LEFT_COLOR,
        RGB_PROP_COLOR,
        RGB_PROP_BACKGROUND_COLOR,

        /*must be last*/
        NB_RGB_PROPS
} ;


enum CRBorderStyleProp
{
        BORDER_STYLE_PROP_TOP = 0,
        BORDER_STYLE_PROP_RIGHT,
        BORDER_STYLE_PROP_BOTTOM,
        BORDER_STYLE_PROP_LEFT,
        
        /*must be last*/
        NB_BORDER_STYLE_PROPS
} ;

enum CRBoxOffsetProp
{
        BOX_OFFSET_PROP_TOP = 0,
        BOX_OFFSET_PROP_RIGHT,
        BOX_OFFSET_PROP_BOTTOM,
        BOX_OFFSET_PROP_LEFT,

        /*must be last*/
        NB_BOX_OFFSET_PROPS
} ;




/**
 *The css2 style class.
 *Contains computed and actual values
 *of inferred from the declarations found
 *in the stylesheets.
 *See css2 spec chapter 6.
 */
struct _CRStyle
{
        /**
         *numerical properties.
         *the properties are indexed by
         *enum #CRNumProp. 
         */
        CRNumPropVal num_props[NB_NUM_PROPS] ;

        /**
         *color properties.
         *They are indexed by enum #CRRgbProp .
         */
        CRRgbPropVal rgb_props[NB_RGB_PROPS] ;

        /**
         *border style properties.
         *They are indexed by enum #CRBorderStyleProp .
         */
        enum CRBorderStyle border_style_props[NB_BORDER_STYLE_PROPS] ;

        /**box display type*/
        enum CRDisplayType display ;

        /**the positioning scheme*/
        enum CRPositionType position ;

        /**the float property*/
        enum CRFloatType float_type ;

        /*
         *the 'font-family' property.
         */
        CRFontFamily *font_family ;

        /**
         *the 'font-size' property.
         */
        CRFontSize *font_size ;
        CRFontSizeAdjust *font_size_adjust ;
        enum CRFontStyle font_style ;
        enum CRFontVariant font_variant ;
        enum CRFontWeight font_weight ;
        enum CRFontStretch font_stretch ;

        CRStyle *parent_style ;
        gulong ref_count ;
} ;


CRStyle *
cr_style_new (void) ;


enum CRStatus
cr_style_set_style_from_decl (CRStyle *a_this, CRDeclaration *a_decl) ;

enum CRStatus
cr_style_to_pango_font_attributes (CRStyle *a_style,
                                   PangoAttrList *a_pgo_attrs,
                                   gulong text_len) ;
enum CRStatus
cr_style_ref (CRStyle *a_this) ;

gboolean
cr_style_unref (CRStyle *a_this) ;

void
cr_style_destroy (CRStyle *a_this) ;

CRStyle *
cr_style_dup (CRStyle *a_this) ;

G_END_DECLS

#endif /*__CR_STYLE_H__*/
