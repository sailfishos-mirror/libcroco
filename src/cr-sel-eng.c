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

#include <string.h>
#include "cr-sel-eng.h"

/**
 *@file:
 *The definition of the  #CRSelEng class.
 *The #CRSelEng is actually the "Selection Engine"
 *class.
 */
#ifdef WITH_SELENG

#define PRIVATE(a_this) (a_this)->priv

static gboolean
class_add_sel_matches_node (CRAdditionalSel *a_add_sel,
                            xmlNode *a_node) ;

static gboolean
id_add_sel_matches_node (CRAdditionalSel *a_add_sel,
                         xmlNode *a_node) ;

static gboolean
attr_add_sel_matches_node (CRAdditionalSel *a_add_sel,
                           xmlNode *a_node) ;

static gboolean
class_add_sel_matches_node (CRAdditionalSel *a_add_sel,
                            xmlNode *a_node)
{
        gboolean result = FALSE ;
        xmlChar *class = NULL ;

        g_return_val_if_fail (a_add_sel
                              && a_add_sel->type == CLASS_ADD_SELECTOR
                              && a_add_sel->content.class_name
                              && a_add_sel->content.class_name->str
                              && a_node, FALSE) ;

        if (xmlHasProp (a_node, "class"))
        {
                class = xmlGetProp (a_node, "class") ;
                if (!strncmp (class, a_add_sel->content.class_name->str,
                             a_add_sel->content.class_name->len))
                {
                        result = TRUE ;
                }
        }

        if (class)
        {
                xmlFree (class) ;
                class = NULL ;
        }
        return result ;
        
}

static gboolean
id_add_sel_matches_node (CRAdditionalSel *a_add_sel,
                         xmlNode *a_node)
{
        g_return_val_if_fail (a_add_sel 
                              && a_add_sel->type == ID_ADD_SELECTOR
                              && a_add_sel->content.id_name
                              && a_add_sel->content.id_name->str
                              && a_node, FALSE) ;

        gboolean result = FALSE ;
        xmlChar *id = NULL ;

        g_return_val_if_fail (a_add_sel
                              && a_add_sel->type == ID_ADD_SELECTOR
                              && a_node, FALSE) ;

        if (xmlHasProp (a_node, "id"))
        {
                id = xmlGetProp (a_node, "id") ;
                if (!strncmp (id, a_add_sel->content.id_name->str,
                              a_add_sel->content.id_name->len))
                {
                        result = TRUE ;
                }
        }

        if (id)
        {
                xmlFree (id) ;
                id = NULL ;
        }
        return result ;
}


static gboolean
attr_add_sel_matches_node (CRAdditionalSel *a_add_sel,
                           xmlNode *a_node) 
{        
        CRAttrSel *cur_sel = NULL ;

        g_return_val_if_fail (a_add_sel 
                              && a_add_sel->type == ATTRIBUTE_ADD_SELECTOR
                              && a_node, FALSE) ;
        
        for (cur_sel = a_add_sel->content.attr_sel ; 
             cur_sel ; cur_sel = cur_sel->next)
        {
                switch (cur_sel->match_way)
                {
                case SET:
                        if (!cur_sel->name || !cur_sel->name->str)
                                return FALSE ;

                        if (!xmlHasProp (a_node, cur_sel->name->str))
                                return FALSE ;
                        break ;

                case EQUALS:
                {
                        xmlChar *value = NULL ;

                        if (!cur_sel->name || !cur_sel->name->str
                            || !cur_sel->value || !cur_sel->value->str)
                                return FALSE ;

                        if (!xmlHasProp (a_node, cur_sel->name->str))
                                return FALSE ;
                        
                        value = xmlGetProp (a_node, cur_sel->name->str) ;

                        if (value && strncmp (value, cur_sel->value->str,
                                              cur_sel->value->len))
                        {
                                xmlFree (value) ;
                                return FALSE ;
                        }
                        xmlFree (value);
                }
                break ;

                case INCLUDES:
                 {
                        xmlChar *value = NULL ;
                        
                        if (!xmlHasProp (a_node, cur_sel->name->str))
                                return FALSE ;
                        value = xmlGetProp (a_node, cur_sel->name->str) ;
                        
                        /*
                         *here, make sure value is a space
                         *separated list of "words", where one
                         *value is exactly cur_sel->value->str
                         */
                        
                        xmlFree (value) ;
                }       
                 break ;
                case DASHMATCH:
                {
                        xmlChar *value = NULL ;
                        
                        if (!xmlHasProp (a_node, cur_sel->name->str))
                                return FALSE ;
                        value = xmlGetProp (a_node, cur_sel->name->str) ;
                        
                        /*
                         *here, make sure value is an hyphen
                         *separated list of "words", each of which
                         *starting with "cur_sel->value->str"
                         */

                        
                        xmlFree (value) ;
                }
                break ;
                default:
                        return FALSE ;
                }
        }

        return TRUE ;
}

struct _CRSelEngPriv
{

};



/**
 *Creates a new instance of #CRSelEng.
 *@return the newly built instance of #CRSelEng of
 *NULL if an error occurs.
 */
CRSelEng *
cr_sel_eng_new (void)
{
	CRSelEng *result ;
	
	result = g_try_malloc (sizeof (CRSelEng)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRSelEng)) ;
	return result ;
}


/**
 *Evaluates a chained list of simple selectors (known as a css2 selector).
 *Says wheter if this selector matches the xml node given in parameter or
 *not.
 */
enum CRStatus
cr_sel_eng_sel_matches_node (CRSelEng *a_this, CRSimpleSel *a_sel,
			     xmlNode *a_node, gboolean *a_result)
{
	CRSimpleSel *cur_sel = NULL ;
	xmlNode *cur_node = NULL ;

	g_return_val_if_fail (a_this && PRIVATE (a_this)
			      && a_this && a_node 
			      && a_node->type == XML_ELEMENT_NODE
			      && a_result,
			      CR_BAD_PARAM_ERROR) ;

	/*go and get the last simple selector of the list*/
	for (cur_sel = a_sel ; 
	     cur_sel && cur_sel->next ; 
	     cur_sel = cur_sel->next) ;

	for (; cur_sel ; cur_sel = cur_sel->prev)
	{

		if (cur_sel->type_mask & UNIVERSAL_SELECTOR)
		{
			return TRUE ;
		}
		else if (cur_sel->type_mask & TYPE_SELECTOR)
		{
			if (cur_sel && cur_sel->name && cur_sel->name->str)
			{
				if (!strcmp (cur_sel->name->str,
					    cur_node->name))
				{
					return TRUE ;
				}
				goto walk_a_step_in_expr ;
			}
			else
			{
				return FALSE ;
			}
		}

		if (!cur_sel->add_sel)
			return FALSE ;

		if (cur_sel->add_sel->type == NO_ADD_SELECTOR)
			return FALSE ;
		
		if (cur_sel->add_sel->type == CLASS_ADD_SELECTOR
		    && cur_sel->add_sel->content.class_name
		    && cur_sel->add_sel->content.class_name->str)
		{
                        if (class_add_sel_matches_node 
                            (cur_sel->add_sel, a_node) == FALSE)
                                return FALSE ;
                        goto walk_a_step_in_expr ;
		}
		else if (cur_sel->add_sel->type == ID_ADD_SELECTOR
			 && cur_sel->add_sel->content.id_name
			 && cur_sel->add_sel->content.id_name->str)
		{
                        if (id_add_sel_matches_node 
                            (cur_sel->add_sel, a_node) == FALSE)
                                return FALSE ;
                        goto walk_a_step_in_expr ;

		}
		else if (cur_sel->add_sel->type == ATTRIBUTE_ADD_SELECTOR
			 && cur_sel->add_sel->content.attr_sel)
		{
                        
			/*
			 *here, call a function that does the match
			 *against an attribute additionnal selector
			 *and a list of attribute xml node.
			 */
		}

	walk_a_step_in_expr:		
                continue ;
		/*
		 *here, depending on the combinator of cur_sel
		 *choose the axis of the xml tree traversing
		 *and walk one step in the xml tree.
		 */
	}

	return TRUE ;
}

/**
 *The destructor of #CRSelEng
 *@param a_this the current instance of the selection engine.
 */
void
cr_sel_eng_destroy (CRSelEng *a_this)
{
	g_return_if_fail (a_this) ;

	if (PRIVATE (a_this))
	{
		g_free (PRIVATE (a_this)) ;
		PRIVATE (a_this) = NULL ;
	}

	if (a_this)
	{
		g_free (a_this) ;
	}
}

#endif /*WITH_SELENG*/
