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

#include <string.h>
#include <glib.h>
#include "cr-simple-sel.h"


/**
 *The constructor of #CRSimpleSel.
 *
 *@return the new instance of #CRSimpleSel.
 */
CRSimpleSel *
cr_simple_sel_new (void)
{
        CRSimpleSel *result = NULL ;

        result = g_try_malloc (sizeof (CRSimpleSel)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }
        memset (result, 0, sizeof (CRSimpleSel)) ;

        return result ;
}

/**
 *Appends a simpe selector to the current list of simple selector.
 *
 *@param a_this the this pointer of the current instance of #CRSimpleSel.
 *@param a_sel the simple selector to append.
 *@return the new list upon successfull completion, an error code otherwise.
 */
CRSimpleSel *
cr_simple_sel_append_simple_sel (CRSimpleSel *a_this, CRSimpleSel *a_sel)
{
        CRSimpleSel *cur = NULL ;

        g_return_val_if_fail (a_sel, NULL) ;

        if (a_this == NULL)
                return a_sel ;

        for (cur = a_this ; cur->next ; cur = cur->next) ;

        cur->next = a_sel ;
        a_sel->prev = cur ;

        return a_this ;
}

/**
 *Prepends a simple selector to the current list of simple selectors.
 *@param a_this the this pointer of the current instance of #CRSimpleSel.
 *@param a_sel the simple selector to prepend.
 *@return the new list upon successfull completion, an error code otherwise.
 */
CRSimpleSel *
cr_simple_sel_prepend_simple_sel (CRSimpleSel *a_this, CRSimpleSel *a_sel)
{
        g_return_val_if_fail (a_sel, NULL) ;

        if (a_this == NULL)
                return a_sel ;

        a_sel->next = a_this ;
        a_this->prev = a_sel ;

        return a_sel ;
}


/**
 *Dumps the selector to a file.
 *TODO: add the support of unicode in the dump.
 *
 *@param a_this the current instance of #CRSimpleSel.
 *@param a_fp the destination file pointer.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_simple_sel_dump (CRSimpleSel *a_this, FILE *a_fp)
{
        CRSimpleSel *cur = NULL ;

        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        for (cur = a_this ; cur ; cur = cur->next)
        {
                if (cur->name)
                {
                        guchar * str = g_strndup (cur->name->str, 
                                                   cur->name->len) ;
                        if (str)
                        {
                                switch (cur->combinator)
                                {
                                case COMB_WS:
                                        fprintf (a_fp," ") ;
                                        break ;
                                case COMB_PLUS:
                                        fprintf (a_fp,"+") ;
                                        break ;
                                case COMB_GT:
                                        fprintf (a_fp,">") ;
                                        break ;
                                default:
                                        break ;
                                }
                       
                                fprintf (a_fp,str) ;

                                g_free (str) ;
                                str = NULL ;
                        }                        
                }

                if (cur->add_sel)
                {
                        cr_additional_sel_dump (cur->add_sel, a_fp) ;
                }
        }

        return CR_OK ;
}


/**
 *The destructor of the current instance of
 *#CRSimpleSel.
 *@param a_this the this pointer of the current instance of #CRSimpleSel.
 *
 */
void
cr_simple_sel_destroy (CRSimpleSel *a_this)
{
        g_return_if_fail (a_this) ;

        if (a_this->name)
        {
                g_string_free (a_this->name, TRUE) ;
                a_this->name = NULL ;
        }

        if (a_this->add_sel)
        {
                cr_additional_sel_destroy (a_this->add_sel) ;
                a_this->add_sel = NULL ;
        }

        if (a_this->next)
        {
                cr_simple_sel_destroy (a_this->next) ;
        }

        if (a_this)
        {
                g_free (a_this) ;
        }
}

