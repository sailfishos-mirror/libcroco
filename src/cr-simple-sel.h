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

#ifndef __CR_SEL_H__
#define __CR_SEL_H__

#include <stdio.h>
#include <glib.h>
#include "cr-additional-sel.h"

G_BEGIN_DECLS

/**
 *@file
 *the declaration of the #CRSimpleSel class.
 *
 */
enum Combinator
{
        NO_COMBINATOR,
        COMB_WS,/*whitesape*/
        COMB_PLUS,
        COMB_GT/*greater than*/
} ;

enum SimpleSelectorType
{
        NO_SELECTOR_TYPE = 0,
        UNIVERSAL_SELECTOR = 1,
        TYPE_SELECTOR = 1 << 1,
        DESCENDANT_SELECTOR = 1 << 2,
        CHILD_SELECTOR = 1 << 3
} ;

typedef struct _CRSimpleSel CRSimpleSel ;

/**
 *The abstraction of a css2 simple selection list
 *as defined by the right part of the "selector" production in the 
 *appendix D.1 of the css2 spec.
 *It is basically a list of simple selector, each
 *simple selector being separated by a combinator.
 *
 *In the libcroco's implementation, each simple selector 
 *is made of at most two parts:
 *
 *1/An element name or 'type selector' (which can hold a '*' and
 *then been called 'universal selector')
 *
 *2/An additional selector that "specializes" the preceding type or
 *universal selector. The additionnal selector can be either
 *an id selector, or a class selector, or an attribute selector.
 */
struct _CRSimpleSel
{
        enum SimpleSelectorType type_mask ;
        gboolean is_case_sentive ;
        GString * name ;

        /**
         *The combinator that separates
         *this simple selector from the previous
         *one.
         */
        enum Combinator combinator ;

        /**
         *The additional selector list of the
         *current simple selector.
         *An additional selector may
         *be a class selector, an id selector,
         *or an attribute selector.
         *Note that this field is a linked list.
         */
        CRAdditionalSel *add_sel ;

        CRSimpleSel *next ;
        CRSimpleSel *prev ;
} ;

CRSimpleSel *
cr_simple_sel_new (void) ;

CRSimpleSel *
cr_simple_sel_append_simple_sel (CRSimpleSel *a_this, 
                                 CRSimpleSel *a_sel) ;

CRSimpleSel *
cr_simple_sel_prepend_simple_sel (CRSimpleSel *a_this, 
                                  CRSimpleSel *a_sel) ;

enum CRStatus
cr_simple_sel_dump (CRSimpleSel *a_this, FILE *a_fp) ;

enum CRStatus
cr_simple_sel_dump_attr_sel_list (CRSimpleSel *a_this) ;

void
cr_simple_sel_destroy (CRSimpleSel *a_this) ;

G_END_DECLS


#endif /*__CR_SIMPLE_SEL_H__*/
