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

#include "cr-additional-sel.h"
#include "string.h"

/**
 *Default constructor of #CRAdditionalSel.
 *@return the newly build instance of #CRAdditionalSel.
 */
CRAdditionalSel *
cr_additional_sel_new (void)
{
	CRAdditionalSel *result = NULL ;

	result = g_try_malloc (sizeof (CRAdditionalSel)) ;

	if (result == NULL)
	{
		cr_utils_trace_debug ("Out of memory") ;
		return NULL ;
	}

	memset (result, 0, sizeof (CRAdditionalSel)) ;

	return result ;
}


/**
 *Constructor of #CRAdditionalSel.
 *@param a_sel_type the type of the newly built instance 
 *of #CRAdditionalSel.
 *@return the newly built instance of #CRAdditionalSel.
 */
CRAdditionalSel *
cr_additional_sel_new_with_type (enum AddSelectorType a_sel_type)
{
        CRAdditionalSel * result = NULL ;

        result = cr_additional_sel_new () ;

        g_return_val_if_fail (result, NULL) ;

        result->type = a_sel_type ;

        return result ;
}

/**
 *Sets a new class name to a
 *CLASS additional selector.
 *@param a_this the "this pointer" of the current instance
 *of #CRAdditionalSel .
 *@param a_class_name the new class name to set.
 *
 */
void
cr_additional_sel_set_class_name (CRAdditionalSel *a_this,
				  GString *a_class_name)
{
	g_return_if_fail (a_this 
			  && a_this->type == CLASS_ADD_SELECTOR) ;
	
	if (a_this->content.class_name)
	{
		g_string_free (a_this->content.class_name, TRUE) ;
	}

	a_this->content.class_name = a_class_name ;
}

/**
 *Sets a new id name to an
 *ID additional selector.
 *@param a_this the "this pointer" of the current instance
 *of #CRAdditionalSel .
 *@param a_id the new id to set.
 */
void
cr_additional_sel_set_id_name (CRAdditionalSel *a_this,
			       GString *a_id)
{
	g_return_if_fail (a_this
			  && a_this->type == ID_ADD_SELECTOR) ;
	
	if (a_this->content.id_name)
	{
		g_string_free (a_this->content.id_name, TRUE) ;
	}

	a_this->content.id_name = a_id ;
}

/**
 *Sets a new pseudo to a
 *PSEUDO additional selector.
 *@param a_this the "this pointer" of the current instance
 *of #CRAdditionalSel .
 *@param a_pseudo the new pseudo to set.
 */
void
cr_additional_sel_set_pseudo (CRAdditionalSel *a_this,
			      CRPseudo *a_pseudo)
{
	g_return_if_fail (a_this 
			  && a_this->type == PSEUDO_CLASS_ADD_SELECTOR) ;

	if (a_this->content.pseudo)
	{
		cr_pseudo_destroy (a_this->content.pseudo) ;
	}

	a_this->content.pseudo = a_pseudo ;
}

/**
 *Sets a new instance of #CRAttrSel to 
 *a ATTRIBUTE additional selector.
 *@param a_this the "this pointer" of the current instance
 *of #CRAdditionalSel .
 *@param a_sel the new instance of #CRAttrSel to set.
 */
void
cr_additional_sel_set_attr_sel (CRAdditionalSel *a_this,
				CRAttrSel *a_sel)
{
	g_return_if_fail (a_this 
			  && a_this->type == ATTRIBUTE_ADD_SELECTOR) ;

	if (a_this->content.attr_sel)
	{
		cr_attr_sel_destroy (a_this->content.attr_sel) ;
	}
	
	a_this->content.attr_sel = a_sel ;
}

/**
 *Appends a new instance of #CRAdditional to the
 *current list of #CRAdditional.
 *@param a_this the "this pointer" of the current instance
 *of #CRAdditionalSel .
 *@param a_sel the new instance to #CRAdditional to append.
 *@return the new list of CRAdditionalSel or NULL if an error arises.
 */
CRAdditionalSel *
cr_additional_sel_append (CRAdditionalSel *a_this, 
			  CRAdditionalSel *a_sel)
{
	CRAdditionalSel *cur_sel = NULL ;

	g_return_val_if_fail (a_sel, NULL) ;

	if (a_this == NULL)
	{
		return a_sel ;
	}

	if (a_sel == NULL)
		return NULL ;
	
	for (cur_sel = a_this ; 
	     cur_sel && cur_sel->next ;
	     cur_sel = cur_sel->next) ;

	g_return_val_if_fail (cur_sel != NULL, NULL) ;

	cur_sel->next = a_sel ;
	a_sel->prev = cur_sel ;
	
	return a_this ;
}

/**
 *Preppends a new instance of #CRAdditional to the
 *current list of #CRAdditional.
 *@param a_this the "this pointer" of the current instance
 *of #CRAdditionalSel .
 *@param a_sel the new instance to #CRAdditional to preappend.
 *@return the new list of CRAdditionalSel or NULL if an error arises.
 */
CRAdditionalSel *
cr_additional_sel_prepend (CRAdditionalSel *a_this, 
			   CRAdditionalSel *a_sel)
{
	g_return_val_if_fail (a_sel, NULL) ;

	if (a_this == NULL)
	{
		return a_sel ;
	}

	a_sel->next = a_this ;
	a_this->prev = a_sel ;

	return a_sel ;	
}


/**
 *Dumps the current instance of #CRAdditionalSel to a file
 *@param a_this the "this pointer" of the current instance of
 *#CRAdditionalSel.
 *@param a_fp the destination file.
 */
void
cr_additional_sel_dump (CRAdditionalSel *a_this, FILE *a_fp)
{
        CRAdditionalSel *cur = NULL ;

        g_return_if_fail (a_this) ;

        for (cur = a_this ;cur ; cur = cur->next) 
        {
                switch (cur->type)
                {
                case CLASS_ADD_SELECTOR:
                {
                        guchar * name = NULL ;
                        if (cur->content.class_name)
                        {
                                name = g_strndup 
                                        (cur->content.class_name->str,
                                         cur->content.class_name->len);

                                if (name)
                                {
                                        fprintf (a_fp, ".%s", name) ;
                                        g_free (name) ;
                                        name = NULL ;
                                }
                        }
                }
                        break ;

                case ID_ADD_SELECTOR:
                {
                        guchar * name = NULL ;
                        if (cur->content.class_name)
                        {
                                name = g_strndup 
                                        (cur->content.id_name->str,
                                         cur->content.id_name->len);

                                if (name)
                                {
                                        fprintf (a_fp, "#%s", name) ;
                                        g_free (name) ;
                                        name = NULL ;
                                }
                        }
                }
                        
                        break ;

                case PSEUDO_CLASS_ADD_SELECTOR:
                {
                        if (cur->content.pseudo)
                        {
                                fprintf (a_fp, ":") ;
                                cr_pseudo_dump (cur->content.pseudo, a_fp) ;
                        }
                }
                        break ;
                
                case ATTRIBUTE_ADD_SELECTOR:
                        if (cur->content.attr_sel)
                        {
                                fprintf (a_fp,"[") ;

                                cr_attr_sel_dump (cur->content.attr_sel, 
                                                  a_fp) ;

                                fprintf (a_fp,"]") ;
                        }

                        break ;

                default:
                        break ;
                }
        }        
}

/**
 *Destroys an instance of #CRAdditional.
 *@param a_this the "this pointer" of the current instance
 *of #CRAdditionalSel .
 */
void
cr_additional_sel_destroy (CRAdditionalSel *a_this)
{
	g_return_if_fail (a_this) ;

	switch (a_this->type)
	{
	case CLASS_ADD_SELECTOR:
		g_string_free (a_this->content.class_name, TRUE) ;
		a_this->content.class_name = NULL ;
		break ;

	case PSEUDO_CLASS_ADD_SELECTOR:
		cr_pseudo_destroy (a_this->content.pseudo) ;
		a_this->content.pseudo = NULL ;
		break ;
		
	case ID_ADD_SELECTOR:
		g_string_free (a_this->content.id_name, TRUE) ;
		a_this->content.id_name = NULL ;
		break ;

	case ATTRIBUTE_ADD_SELECTOR:
		cr_attr_sel_destroy (a_this->content.attr_sel) ;
		a_this->content.attr_sel = NULL ;
		break ;

	default :
		break ;
	}

	if (a_this->next)
	{
		cr_additional_sel_destroy (a_this->next) ;
	}

	g_free (a_this) ;
}
