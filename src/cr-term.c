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
#include <string.h>
#include "cr-term.h"
#include "cr-num.h"

/**
 *@file
 *Definition of the #CRTem class.
 */


static void
cr_term_clear (CRTerm *a_this)
{
        g_return_if_fail (a_this) ;

        switch (a_this->type)
        {
        case NO_TYPE:
                break ;

        case NUMBER:
        case PERCENTAGE :
        case EMS:
        case EXS:
        case LENGTH:
        case ANGLE:
        case TIME:
        case FREQ:
                if (a_this->content.num)
                {
                        cr_num_destroy (a_this->content.num) ;
                        a_this->content.num = NULL ;
                }
                break ;
        
        case FUNCTION:
                if (a_this->ext_content.func_param)
                {
                        cr_term_destroy (a_this->ext_content.func_param) ;
                        a_this->ext_content.func_param = NULL ;
                }
        case STRING:
        case IDENT:
        case URI:
        case HASH:
                if (a_this->content.str)
                {
                        g_string_free (a_this->content.str, TRUE) ;
                        a_this->content.str = NULL ;
                }
                break ;
        case RGB:
                if (a_this->content.rgb)
                {
                        cr_rgb_destroy (a_this->content.rgb) ;
                        a_this->content.rgb = NULL ;
                }
                break ;
        case UNICODERANGE:        
        default :                
                break ;
        }

        a_this->unit = NO_UNIT ;
        a_this->type = NO_TYPE ;
}

/**
 *Instanciate a #CRTerm.
 *@return the newly build instance
 *of #CRTerm.
 */
CRTerm *
cr_term_new (void)
{
	CRTerm *result = NULL ;

	result = g_try_malloc (sizeof (CRTerm)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }
        memset (result, 0, sizeof (CRTerm)) ;
	return result ;
}


enum CRStatus
cr_term_set_number (CRTerm *a_this, CRNum *a_num)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = NUMBER ;
        a_this->content.num = a_num ;
        return CR_OK ;
}
        
enum CRStatus
cr_term_set_percentage (CRTerm *a_this, CRNum *a_num)
{
      g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = PERCENTAGE ;
        a_this->content.num = a_num ;
        return CR_OK ;
}

enum CRStatus
cr_term_set_length (CRTerm *a_this, enum TermUnit a_unit,
                    CRNum *a_num)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = LENGTH ;
        a_this->unit = a_unit ;
        a_this->content.num = a_num ;
        return CR_OK ;
}

enum CRStatus
cr_term_set_ems (CRTerm *a_this, CRNum *a_num)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = EMS ;
        a_this->content.num = a_num ;
        return CR_OK ;
}

enum CRStatus
cr_term_set_exs (CRTerm *a_this, CRNum * a_num)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = EXS ;
        a_this->content.num = a_num ;
        return CR_OK ;
}

enum CRStatus
cr_term_set_angle (CRTerm *a_this, enum TermUnit a_unit,
                   CRNum *a_num)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = ANGLE ;
        a_this->unit = a_unit ;
        a_this->content.num = a_num ;
        return CR_OK ;
}

enum CRStatus
cr_term_set_time (CRTerm *a_this, enum TermUnit a_unit,
                  CRNum *a_num)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = TIME ;
        a_this->unit = a_unit ;
        a_this->content.num = a_num ;
        return CR_OK ;
}

enum CRStatus
cr_term_set_freq (CRTerm *a_this, enum TermUnit a_unit,
                  CRNum *a_num)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = FREQ ;
        a_this->unit = a_unit ;
        a_this->content.num = a_num ;
        return CR_OK ;
}

enum CRStatus
cr_term_set_function (CRTerm *a_this, GString *a_func_name,
                      CRTerm *a_func_param)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = FUNCTION ;        
        a_this->content.str = a_func_name ;
        a_this->ext_content.func_param = a_func_param ;
        return CR_OK ;
}

enum CRStatus
cr_term_set_string (CRTerm *a_this, GString *a_str)
{
       g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

       cr_term_clear (a_this) ;
        
       a_this->type = STRING ;        
       a_this->content.str = a_str ;
       return CR_OK ;
}

enum CRStatus
cr_term_set_ident (CRTerm *a_this, GString *a_str)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = IDENT ;        
        a_this->content.str = a_str ;
        return CR_OK ;
}

enum CRStatus
cr_term_set_uri (CRTerm *a_this, GString *a_str)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = URI ;
        a_this->content.str = a_str ;
        return CR_OK ;
}
        
enum CRStatus
cr_term_set_rgb (CRTerm *a_this, CRRgb *a_rgb)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

       cr_term_clear (a_this) ;
        
       a_this->type = RGB ;
       a_this->content.rgb = a_rgb ;
       return CR_OK ;
}
        
enum CRStatus
cr_term_set_hash (CRTerm *a_this, GString *a_str)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR) ;

        cr_term_clear (a_this) ;
        
        a_this->type = HASH ;
        a_this->content.str = a_str ;
        return CR_OK ;
}

/**
 *Appends a new term to the current list of #CRTerm.
 *
 *@param a_this the "this pointer" of the current instance
 *of #CRTerm .
 *@param a_new_term the term to append.
 *@return the list of terms with the a_new_term appended to it.
 */
CRTerm *
cr_term_append_term (CRTerm *a_this, CRTerm *a_new_term)
{
	CRTerm *cur = NULL ;

	g_return_val_if_fail (a_new_term, 
			      NULL) ;
	
        if (a_this == NULL)
                return a_new_term ;

	for (cur = a_this ;cur->next ; cur = cur->next) ;
	
	cur->next = a_new_term ;
	a_new_term->prev = cur ;

	return a_this ;
}


/**
 *Prepends a term to the list of terms represented by a_this.
 *
 *@param a_this the "this pointer" of the current instance of
 *#CRTerm .
 *@param a_new_term the term to prepend.
 *@return the head of the new list.
 */
CRTerm *
cr_term_prepend_term (CRTerm *a_this, CRTerm *a_new_term)
{
	g_return_val_if_fail (a_this && a_new_term, 
			      NULL) ;
	
	a_new_term->next = a_this ;
	a_this->prev = a_new_term ;
	
        return a_new_term ;
}


/**
 *Dumps the expression (a list of terms connected by operators)
 *to a file.
 *TODO: finish the dump. The dump of some type of terms have not yet been
 *implemented.
 *@param a_this the current instance of #CRTerm.
 *@param a_fp the destination file pointer.
 */
void
cr_term_dump (CRTerm *a_this, FILE *a_fp)
{
        guchar *content=NULL ;
        CRTerm *cur = NULL ;

        g_return_if_fail (a_this) ;

        for (cur = a_this ; cur ; cur = cur->next)
        {
                if ((cur->content.str == NULL)
                    && (cur->content.num == NULL)
                    && (cur->content.str == NULL)
                    && (cur->content.rgb == NULL))
                        continue ;
                
                switch (cur->operator)
                {
                case DIVIDE:
                        fprintf (a_fp, " / ") ;
                        break ;
                case COMMA:
                        fprintf (a_fp, ", ") ;
                        break ;

                case NO_OP:
                        if (cur->prev)
                        {
                                fprintf (a_fp, " ") ;
                        }
                        break ;
                default:
                        
                        break ;
                }

                switch (cur->unary_op)
                {
                case PLUS_UOP:
                        fprintf (a_fp, "+") ;
                        break ;

                case MINUS_UOP:
                        fprintf (a_fp, "-") ;
                        break ;

                default :
                        break ;
                }

                switch (cur->type)
                {
                case NUMBER:
                        if (cur->content.num)
                        {
                                content = 
                                        cr_num_to_string 
                                        (cur->content.num) ;
                        }

                        if (content)
                        {
                                fprintf (a_fp, content) ;
                                g_free (content) ;
                                content = NULL ;
                        }

                        break ;

                case PERCENTAGE:
                        if (cur->content.num)
                        {
                                content = cr_num_to_string 
                                        (cur->content.num) ;
                        }

                        if (content)
                        {
                                fprintf (a_fp, "%s", content) ;
                                putchar ('%') ;
                                g_free (content) ;
                                content = NULL ;
                        }
                        break ;

                case LENGTH:
                        if (cur->content.num)
                        {
                                content = cr_num_to_string
                                        (cur->content.num) ;
                        }

                        g_return_if_fail (content) ;

                        switch (cur->unit)
                        {
                        case UNIT_PX:                   
                                fprintf (a_fp, "%spx", content) ;
                                break ;

                        case UNIT_CM:
                                fprintf (a_fp, "%scm", content) ;
                                break ;

                        case UNIT_MM:
                                fprintf (a_fp, "%smm", content) ;
                                break ;

                        case UNIT_IN:
                                fprintf (a_fp, "%sin", content) ;
                                break ;

                        case UNIT_PT:
                                fprintf (a_fp, "%spt", content) ;
                                break ;

                        case UNIT_PC:
                                fprintf (a_fp, "%spc", content) ;
                                break ;

                        default:
                                fprintf (a_fp, "%s",content) ;
                        }

                        if (content)
                        {
                                g_free (content) ;
                                content = NULL ;
                        }
                        break ;

                case EMS:
                        if (cur->content.num)
                        {
                                content = cr_num_to_string
                                        (cur->content.num) ;
                        }
                        if (content)
                        {
                                fprintf (a_fp, "%sem", content) ;
                                g_free (content) ;
                                content = NULL ;
                        }
                        break ;
                
                case EXS:
                        if (cur->content.num)
                        {
                                content = cr_num_to_string
                                        (cur->content.num) ;
                        }

                        if (content)
                        {
                                fprintf (a_fp, "%sex", content) ;
                                g_free (content) ;
                                content = NULL ;
                        }                        
                        break;

                case ANGLE:
                        if (cur->content.num)
                        {
                                content = cr_num_to_string
                                        (cur->content.num) ;
                        }

                        switch (cur->unit)
                        {
                        case UNIT_DEG:
                                if (content) 
                                {
                                        fprintf (a_fp, "%sdeg",content) ;
                                        g_free (content) ;
                                        content = NULL ;
                                }
                                break ;

                        case UNIT_RAD:
                                if (content)
                                {
                                        fprintf (a_fp, "%srad",content) ;
                                        g_free (content) ;
                                        content = NULL ;
                                }
                                break ;

                        case UNIT_GRAD:
                                if (content)
                                {
                                        fprintf (a_fp, "%sgrad",content) ;
                                        g_free (content) ;
                                        content = NULL ;
                                }
                                break ;
                        
                        default :
                                if (content)
                                {
                                        fprintf (a_fp, "%s", content) ;
                                        g_free (content) ;
                                        content = NULL ;
                                }
                                break ;
                        }

                        break ;

                case TIME:
                        if (cur->content.num)
                        {
                                content = cr_num_to_string
                                        (cur->content.num) ;
                        }
                        
                        g_return_if_fail (content) ;

                        switch (cur->unit)
                        {
                        case UNIT_MS:
                                fprintf (a_fp, "%sms", content) ;
                                break ;

                        case UNIT_S:
                                fprintf (a_fp, "%ss",content) ;
                                break ;

                        default :
                                break ;
                        }

                        if (content)
                        {
                                g_free (content) ;
                                content = NULL ;
                        }
                        break ;

                case FREQ:
                        if (cur->content.num)
                        {
                                content = cr_num_to_string
                                        (cur->content.num) ;
                        }

                        g_return_if_fail (content) ;

                        switch (cur->unit)
                        {
                        case UNIT_HZ:
                                fprintf (a_fp, "%sHz", content) ;
                                break ;

                        case UNIT_KHZ:
                                fprintf (a_fp, "%sKHz",content) ;
                                break ;

                        default:
                                fprintf (a_fp, "%s",content) ;
                                break ;
                        }

                        if (content)
                        {
                                g_free (content) ;
                                content = NULL ;
                        }
                        break ;

                case FUNCTION:
                        if (cur->content.str)
                        {
                                content = g_strndup 
                                        (cur->content.str->str,
                                         cur->content.str->len) ;
                        }                        

                        if (content)
                        {
                                fprintf (a_fp, "%s(", content) ;

                                if (a_this->ext_content.func_param)
                                {
                                        cr_term_dump 
                                                (a_this->
                                                 ext_content.func_param,
                                                 a_fp) ;
                                }

                                fprintf (a_fp, ")") ;
                                g_free (content) ;
                                content = NULL ;
                        }
                        break ;

                case STRING:
                        if (cur->content.str)
                        {
                                content = g_strndup 
                                        (cur->content.str->str,
                                         cur->content.str->len) ;
                        }

                        if (content)
                        {
                                fprintf (a_fp, "\"%s\"", content) ;
                                g_free (content) ;
                                content = NULL ;
                        }                        
                        break ;

                case IDENT:
                        if (cur->content.str)
                        {
                                content = g_strndup 
                                (cur->content.str->str,
                                 cur->content.str->len) ;
                        }
                        
                        if (content)
                        {
                                fprintf (a_fp, "%s",content) ;
                                g_free (content) ;
                                content = NULL ;
                        }                        
                        break ;

                case URI:
                        if (cur->content.str)
                        {
                                content = g_strndup 
                                (cur->content.str->str,
                                 cur->content.str->len) ;
                        }

                        if (content)
                        {
                                fprintf (a_fp, "url(%s)",content) ;
                                g_free (content) ;
                                content = NULL ;
                        }                        
                        break ;

                case RGB:
                        if (cur->content.rgb)
                        {
                                fprintf (a_fp, "rgb(") ;
                                cr_rgb_dump (cur->content.rgb, a_fp) ;
                                fprintf (a_fp, ")") ;
                        }

                        break ;

                case UNICODERANGE:
                       fprintf
                                (a_fp, 
                                 "?found unicoderange: dump not supported yet?") ;
                        break ;

                case HASH:
                        if (cur->content.str)
                        {
                                content = g_strndup 
                                        (cur->content.str->str,
                                         cur->content.str->len) ;
                        }
                        g_return_if_fail (content) ;

                        fprintf (a_fp, "#%s", content) ;

                        break ;

                default:
                        fprintf (a_fp, "%s", "Unrecognized Term type") ;
                        break ;
                }
                                
                if (content)
                {
                        g_free (content) ;
                        content = NULL ;
                }
        }
}


/**
 *Increments the reference counter of the current instance
 *of #CRTerm.*
 *@param a_this the current instance of #CRTerm.
 */
void
cr_term_ref (CRTerm *a_this)
{
        g_return_if_fail (a_this) ;

        a_this->ref_count ++ ;
}


/**
 *Decrements the ref count of the current instance of
 *#CRTerm. If the ref count reaches zero, the instance is
 *destroyed.
 *@param a_this the current instance of #CRTerm.
 *@return TRUE if the current instance has been destroyed, FALSE otherwise.
 */
gboolean
cr_term_unref (CRTerm *a_this)
{
        g_return_val_if_fail (a_this, FALSE) ;

        if (a_this->ref_count)
        {
                a_this->ref_count -- ;
        }

        if (a_this->ref_count == 0)
        {
                cr_term_destroy (a_this) ;
                return TRUE ;
        }

        return FALSE ;
}

/**
 *The destructor of the the #CRTerm class.
 *@param a_this the "this pointer" of the current instance
 *of #CRTerm.
 */
void
cr_term_destroy (CRTerm *a_this)
{
	g_return_if_fail (a_this) ;

        cr_term_clear (a_this) ;

	if (a_this->next)
	{
		cr_term_destroy (a_this->next) ;
		a_this->next = NULL ;
	}

        if (a_this)
        {
                g_free (a_this) ;
        }

}
