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
#include "cr-rgb.h"
#include "cr-num.h"

#ifndef __CR_TERM_H__
#define __CR_TERM_H__

G_END_DECLS

/**
 *@file
 *Declaration of the #CRTem class.
 */

enum CRTermType
{        
        TERM_NO_TYPE = 0,
        TERM_NUMBER,
        TERM_FUNCTION,
        TERM_STRING,
        TERM_IDENT,
        TERM_URI,
        TERM_RGB,
        TERM_UNICODERANGE,
        TERM_HASH,
} ;


enum UnaryOperator
{
        NO_UNARY_UOP = 0,
        PLUS_UOP,
        MINUS_UOP,
        EMPTY_UNARY_UOP
} ;

enum Operator
{
        NO_OP = 0,
        DIVIDE,
        COMMA		
} ;

struct _CRTerm ;
typedef struct _CRTerm CRTerm ;

/**
 *An abstraction of a css2 term as
 *defined in the CSS2 spec in appendix D.1:
 *term ::=
 *[ NUMBER S* | PERCENTAGE S* | LENGTH S* | EMS S* | EXS S* 
 *| ANGLE S* | TIME S* | FREQ S* | function ]
 * | STRING S* | IDENT S* | URI S* | RGB S* 
 *| UNICODERANGE S* | hexcolor
 */
struct _CRTerm
{
        /**
         *The type of the term.
         */
        enum CRTermType type ;
                
        /**
         *The unary operator associated to
         *the current term.
         */
        enum UnaryOperator unary_op ;

        /**
         *The operator associated to the current term.
         */
        enum Operator operator ;


        /**
         *The content of the term.
         *Depending of the type of the term,
         *this holds either a number, a percentage ...
         */
        union
        {
                CRNum *num ;
                GString * str ;
                CRRgb * rgb ;
        } content ;

        /**
         *If the term is of type UNICODERANGE, 
         *this field holds the upper bound of the range.
         *if the term is of type FUNCTION, this holds
         *an instance of CRTerm that represents
         * the expression which is the argument of the function.
         */
        union
        {
                CRTerm *func_param ;                        
        } ext_content ;

        /**
         *A spare pointer, just in case.
         *Can be used by the application.
         */
        void * custom_data ;

        glong ref_count ;

        /**
         *A pointer to the next term, 
         *just in case this term is part of
         *an expression.
         */
        CRTerm *next ;

        /**
         *A pointer to the previous
         *term.
         */
        CRTerm *prev ;
} ;

CRTerm *
cr_term_new (void) ;

enum CRStatus
cr_term_set_number (CRTerm *a_this, CRNum *a_num) ;
        
enum CRStatus
cr_term_set_function (CRTerm *a_this, GString *a_func_name,
                      CRTerm *a_func_param) ;

enum CRStatus
cr_term_set_string (CRTerm *a_this, GString *a_str) ;

enum CRStatus
cr_term_set_ident (CRTerm *a_this, GString *a_str) ;

enum CRStatus
cr_term_set_uri (CRTerm *a_this, GString *a_str) ;
        
enum CRStatus
cr_term_set_rgb (CRTerm *a_this, CRRgb *a_rgb) ;                
        
enum CRStatus
cr_term_set_hash (CRTerm *a_this, GString *a_str) ;
        
CRTerm *
cr_term_append_term (CRTerm *a_this, CRTerm *a_new_term) ;

CRTerm *
cr_term_prepend_term (CRTerm *a_this, CRTerm *a_new_term) ;

void
cr_term_dump (CRTerm *a_this, FILE *a_fp) ;

void
cr_term_ref (CRTerm *a_this) ;

gboolean
cr_term_unref (CRTerm *a_this) ;

void
cr_term_destroy (CRTerm * a_term) ;

G_END_DECLS

#endif /*__CR_TERM_H__*/
