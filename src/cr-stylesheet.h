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

#ifndef __CR_STYLESHEET_H__
#define __CR_STYLESHEET_H__

#include "cr-utils.h"
#include "cr-statement.h"

/**
 *@file
 *The declaration of the #CRStyleSheet class.
 */

/**
 *An abstraction of a css stylesheet as defined
 *by the css2 spec in chapter 4.
 */
struct _CRStyleSheet
{
	/**The css statements list*/
	CRStatement *statements ;
} ;


CRStyleSheet *
cr_stylesheet_new (CRStatement *a_stmts) ;

void
cr_stylesheet_dump (CRStyleSheet *a_this, FILE *a_fp) ;

void
cr_stylesheet_destroy (CRStyleSheet *a_this) ;

#endif /*__CR_STYLESHEET_H__*/
