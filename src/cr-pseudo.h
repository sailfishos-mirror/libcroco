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

#ifndef __CR_PSEUDO_H__
#define __CR_PSEUDO_H__

#include <stdio.h>
#include <glib.h>
#include "cr-attr-sel.h"

#ifdef __cplusplus
extern "C" {
#endif

        enum CRPseudoType
        {
                IDENT_PSEUDO = 0,
                FUNCTION_PSEUDO
        } ;

        typedef struct _CRPseudo CRPseudo ;

	/**
	 *The CRPseudo Class.
	 *Abstract a "pseudo" as defined by the css2 spec
	 *in appendix D.1 .
	 */
        struct _CRPseudo
        {
                enum CRPseudoType type ;
                GString *name ;
                GString *extra ;
        } ;

	CRPseudo *
	cr_pseudo_new (void) ;

        void
        cr_pseudo_dump (CRPseudo *a_this, FILE *a_fp) ;

	void
	cr_pseudo_destroy (CRPseudo *a_this) ;

#ifdef __cplusplus
} /*extern "C" {*/
#endif

#endif /*__CR_PSEUDO_H__*/
