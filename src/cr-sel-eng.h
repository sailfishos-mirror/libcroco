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

#ifndef __CR_SEL_ENG_H__
#define __CR_SEL_ENG_H__

#include "cr-utils.h"
#include "cr-stylesheet.h"
#include "cr-cascade.h"
#include "cr-style.h"

#ifdef HAVE_LIBXML2
 #include <libxml/tree.h>
#endif

#ifdef WITH_SELENG


/**
 *@file:
 *The declaration of the  #CRSelEng class.
 *The #CRSelEng is actually the "Selection Engine"
 *class.
 */

G_BEGIN_DECLS

typedef struct _CRSelEng CRSelEng ;
typedef struct _CRSelEngPriv CRSelEngPriv ;

/**
 *The Selection engine class.
 *The main service provided by this class, is
 *the ability to interpret a libcroco implementation
 *of css2 selectors, and given an xml node, say if
 *the selector matches the node or not.
 */
struct _CRSelEng
{
	CRSelEngPriv *priv ;
} ;

CRSelEng *
cr_sel_eng_new (void) ;

enum CRStatus
cr_sel_eng_matches_node (CRSelEng *a_this, CRSimpleSel *a_sel,
                         xmlNode *a_node, gboolean *a_result) ;

enum CRStatus
cr_sel_eng_get_matched_rulesets (CRSelEng *a_this,
                                 CRStyleSheet *a_sheet,
                                 xmlNode *a_node,
                                 CRStatement ***a_rulesets,
                                 gulong *a_len) ;

enum CRStatus
cr_sel_eng_get_matched_style (CRSelEng *a_this,
                              CRCascade *a_cascade,
                              xmlNode *a_node,
                              CRStyle **a_style) ;

void
cr_sel_eng_destroy (CRSelEng *a_this) ;

G_END_DECLS

#endif /*WITH_SELENG*/

#endif/*__CR_SEL_ENG_H__*/
