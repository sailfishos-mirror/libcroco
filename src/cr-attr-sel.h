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

/*
 *$Id$
 */

#include <stdio.h>
#include <glib.h>
#include "cr-utils.h"

#ifndef __CR_ATTR_SEL_H__
#define __CR_ATTR_SEL_H__

#ifdef __cplusplus
extern "C" {
#endif

	struct _CRAttrSel ;
	typedef struct _CRAttrSel CRAttrSel ;

	enum AttrMatchWay
	{
                NO_MATCH = 0,
		SET,
		EQUALS,
		INCLUDES,
		DASHMATCH
	} ;

	struct _CRAttrSel
	{
		GString             *name ;
		GString             *value ;
		enum AttrMatchWay  match_way ;
		CRAttrSel          *next ;
		CRAttrSel          *prev ;
	} ;

	CRAttrSel *
	cr_attr_sel_new (void) ;

        enum CRStatus
	cr_attr_sel_append_attr_sel (CRAttrSel * a_this, 
                                     CRAttrSel *a_new) ;
        enum CRStatus
        cr_attr_sel_prepend_attr_sel (CRAttrSel *a_this, 
                                      CRAttrSel *a_attr_sel) ;
        void
        cr_attr_sel_dump (CRAttrSel *a_this, FILE *a_fp) ;

        void
	cr_attr_sel_destroy (CRAttrSel *a_this) ;

#ifdef __cplusplus
} /*extern C*/
#endif


#endif /*__CR_ATTR_SEL_H__*/
