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
#include "cr-attr-sel.h"

/**
 *@file
 *The class that abstracts an attribute selector.
 *Attributes selectors are described in the css2 spec [5.8].
 *There are more generally used in the css2 selectors described in
 *css2 spec [5] .
 */


/**
 *The constructor of #CRAttrSel.
 *@return the newly allocated instance
 *of #CRAttrSel.
 */
CRAttrSel *
cr_attr_sel_new (void)
{
	CRAttrSel *result = NULL ;
        result = g_malloc0 (sizeof (CRAttrSel)) ;

        return result ;
}

/**
 *Appends an attribute selector to the current list of
 *attribute selectors represented by a_this.
 *
 *@param a_this the this pointer of the current instance of
 *#CRAttrSel.
 *@param a_attr_sel selector to append.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_attr_sel_append_attr_sel (CRAttrSel *a_this, CRAttrSel *a_attr_sel)
{
        CRAttrSel * cur_sel = NULL ;

        g_return_val_if_fail (a_this && a_attr_sel, 
                              CR_BAD_PARAM_ERROR) ;
        
        for (cur_sel = a_this ; cur_sel->next ; cur_sel = cur_sel->next) ;
        
        cur_sel->next = a_attr_sel ;
        a_attr_sel->prev = cur_sel ;
        
        return CR_OK ;
}

/**
 *Prepends an attribute selector to the list of
 *attributes selector represented by a_this.
 *
 *@param a_this the "this pointer" of the current instance
 *of #CRAttrSel.
 *@param a_attr_sel the attribute selector to append.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_attr_sel_prepend_attr_sel (CRAttrSel *a_this, CRAttrSel *a_attr_sel)
{
        g_return_val_if_fail (a_this && a_attr_sel, CR_BAD_PARAM_ERROR) ;

        a_attr_sel->next = a_this ;
        a_this->prev = a_attr_sel ;

        return CR_OK ;
}

/**
 *Dumps the current instance of #CRAttrSel to a file.
 *@param a_this the "this pointer" of the current instance of
 *#CRAttrSel.
 *@param a_fp the destination file.
 */
void
cr_attr_sel_dump (CRAttrSel *a_this, FILE *a_fp)
{
        CRAttrSel *cur = NULL ;

        g_return_if_fail (a_this) ;


        for (cur = a_this ; cur ; cur = cur->next)
        {
                if (cur->prev)
                {
                        fprintf (a_fp," ") ;
                }

                if (cur->name)
                {
                        guchar *name = NULL ;

                        name = g_strndup (cur->name->str,
                                          cur->name->len) ;
                        if (name)
                        {
                                fprintf (a_fp,name) ;
                                g_free (name) ;
                                name = NULL ;
                        }
                }
        
                if (cur->value)
                {
                        guchar *value = NULL ;

                        value = g_strndup (cur->value->str,
                                           cur->value->len) ;
                        if (value)
                        {
                                switch (cur->match_way)
                                {
                                case SET:
                                        break ;

                                case EQUALS:
                                        fprintf (a_fp,"=") ;
                                        break ;

                                case INCLUDES:
                                        fprintf (a_fp,"~=") ;
                                        break ;

                                case DASHMATCH:
                                        fprintf (a_fp,"|=") ;
                                                break ;

                                default:
                                        break ;
                                }
                        
                                fprintf (a_fp,value) ;

                                g_free (value) ;
                                value = NULL ;
                        }
                }
        }
}


/**
 *Destroys the current instance of #CRAttrSel.
 *Frees all the fields if they are non null.
 *@param a_this the "this pointer" of the current
 *instance of #CRAttrSel.
 */
void
cr_attr_sel_destroy (CRAttrSel *a_this)
{
        g_return_if_fail (a_this) ;

        if (a_this->name)
        {
                g_string_free (a_this->name, TRUE) ;
                a_this->name = NULL ;
        }

        if (a_this->value)
        {
                g_string_free (a_this->value, TRUE) ;
                a_this->value = NULL ;
        }

        if (a_this->next)
        {
                cr_attr_sel_destroy (a_this->next) ;
                a_this->next = NULL ;
        }

        if (a_this)
        {
                g_free (a_this) ;
                a_this = NULL ;
        }
}
