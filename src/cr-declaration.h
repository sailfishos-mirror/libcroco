/* -*- Mode: C; indent-tabs-mode: ni; c-basic-offset: 8 -*- */

/*
 *This file is part of the Croco Library
 *
 *The Croco Library is free software; 
 *you can redistribute it and/or modify it under the terms of 
 *the GNU General Public License as 
 *published by the Free Software Foundation; either version 2, 
 *or (at your option) any later version.
 *
 *The Croco Library is distributed in the hope 
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

#ifndef __CR_DECLARATION_H__
#define __CR_DECLARATION_H__

#include <stdio.h>
#include "cr-utils.h"
#include "cr-term.h"

G_BEGIN_DECLS

/**
 *@file
 *The declaration of the #CRDeclaration class.
 */

/**
 *The abstraction of a css declaration defined by the
 *css2 spec in chapter 4.
 */
typedef struct _CRDeclaration CRDeclaration ;
struct _CRDeclaration
{
	/**The property.*/
	GString *property ;

	/**The value of the property.*/
	CRTerm *value ;
	
	CRDeclaration *next ;
	CRDeclaration *prev ;
	glong ref_count ;
} ;


CRDeclaration *
cr_declaration_new (GString *a_property, CRTerm *a_value) ;

CRDeclaration *
cr_declaration_append (CRDeclaration *a_this, CRDeclaration *a_new) ;

CRDeclaration *
cr_declaration_prepend (CRDeclaration *a_this, CRDeclaration *a_new) ;

CRDeclaration *
cr_declaration_append2 (CRDeclaration *a_this, GString *a_prop,
			CRTerm *a_value) ;

void
cr_declaration_dump (CRDeclaration *a_this, FILE *a_fp, glong a_indent,
		     gboolean a_one_per_line) ;

void 
cr_declaration_ref (CRDeclaration *a_this) ;

gboolean
cr_declaration_unref (CRDeclaration *a_this) ;

void
cr_declaration_destroy (CRDeclaration *a_this) ;

G_END_DECLS

#endif /*__CR_DECLARATION_H__*/
