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
sel_matches_node_real (CRSelEng *a_this, CRSimpleSel *a_sel,
                       xmlNode *a_node, gboolean *a_result,
                       gboolean a_recurse) ;

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

/**
 *Returns TRUE if the instance of #CRAdditional selector matches
 *the node given in parameter, FALSE otherwise.
 *@param a_add_sel the additional selector to evaluate.
 *@param a_node the xml node against whitch the selector is to
 *be evaluated
 *return TRUE if the additional selector matches the current xml node
 *FALSE otherwise.
 */
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
                         xmlChar *value = NULL, *ptr1 = NULL, *ptr2 = NULL,
                                 *cur = NULL;
                         gboolean found = FALSE ;

                        if (!xmlHasProp (a_node, cur_sel->name->str))
                                return FALSE ;
                        value = xmlGetProp (a_node, cur_sel->name->str) ;
                        
                        if (!value)                        
                                return FALSE;
                        
                        /*
                         *here, make sure value is a space
                         *separated list of "words", where one
                         *value is exactly cur_sel->value->str
                         */
                        for (cur = value ; *cur ; cur++)
                        {
                                /*
                                 *set ptr1 to the first non white space
                                 *char addr.
                                 */
                                while (cr_utils_is_white_space 
                                       (*cur) == TRUE && *cur)
                                        cur ++ ;
                                if (!*cur)
                                        break ;
                                ptr1 = cur ;

                                /*
                                 *set ptr2 to the end the word.
                                 */
                                while (cr_utils_is_white_space 
                                       (*cur) == FALSE && *cur)
                                        cur++ ;
                                if (!*cur)
                                        break ;
                                cur-- ;
                                ptr2 = cur ;

                                if (!strncmp (ptr1, cur_sel->value->str,
                                              ptr2 - ptr1 + 1))
                                {
                                        found = TRUE ;
                                        break ;
                                }
                                ptr1 = ptr2 = NULL ;
                        }

                        if (found == FALSE)
                        {
                                xmlFree (value) ;
                                return FALSE ;
                        }
                        xmlFree (value) ;
                }
                 break ;

                case DASHMATCH:
                {
                        xmlChar *value = NULL, *ptr1 = NULL, *ptr2 = NULL,
                                 *cur = NULL;
                         gboolean found = FALSE ;
                        
                        if (!xmlHasProp (a_node, cur_sel->name->str))
                                return FALSE ;
                        value = xmlGetProp (a_node, cur_sel->name->str) ;

                        /*
                         *here, make sure value is an hyphen
                         *separated list of "words", each of which
                         *starting with "cur_sel->value->str"
                         */
                        for (cur = value ; *cur ; cur++)
                        {
                                if (*cur == '-')
                                        cur ++ ;
                                ptr1 = cur ;
                                
                                while (*cur != '-' && *cur)
                                        cur ++ ;
                                if (!*cur)
                                        break ;
                                cur-- ;
                                ptr2 = cur ;
                                
                                if (g_strstr_len (ptr1, ptr1 - ptr2 + 1,
                                                  cur_sel->value->str)
                                        == (gchar*)ptr1)
                                {
                                        found = TRUE ;
                                        break ;
                                }
                        }

                        if (found == FALSE)
                        {
                                xmlFree (value) ;
                                return FALSE ;
                        }
                        xmlFree (value) ;
                }
                break ;
                default:
                        return FALSE ;
                }
        }

        return TRUE ;
}


static gboolean
sel_matches_node_real (CRSelEng *a_this, CRSimpleSel *a_sel,
                       xmlNode *a_node, gboolean *a_result,
                       gboolean a_recurse)
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

	for (cur_node = a_node ; cur_sel ; cur_sel = cur_sel->prev)
	{
		if (cur_sel->type_mask & UNIVERSAL_SELECTOR)
		{
                        goto walk_a_step_in_expr ;
		}
		else if (cur_sel->type_mask & TYPE_SELECTOR)
		{
			if (cur_sel && cur_sel->name && cur_sel->name->str)
			{
				if (!strcmp (cur_sel->name->str,
                                             cur_node->name))
				{
					goto walk_a_step_in_expr ;
				}
                                goto done ;
			}
			else
			{
                                goto done ;
			}
		}

		if (!cur_sel->add_sel)
                {
                        goto done ;
                }

		if (cur_sel->add_sel->type == NO_ADD_SELECTOR)
                {
			goto done ;
                }
		
		if (cur_sel->add_sel->type == CLASS_ADD_SELECTOR
		    && cur_sel->add_sel->content.class_name
		    && cur_sel->add_sel->content.class_name->str)
		{
                        if (class_add_sel_matches_node 
                            (cur_sel->add_sel, cur_node) == FALSE)
                        {
                                goto done ;
                        }
                        goto walk_a_step_in_expr ;
		}
		else if (cur_sel->add_sel->type == ID_ADD_SELECTOR
			 && cur_sel->add_sel->content.id_name
			 && cur_sel->add_sel->content.id_name->str)
		{
                        if (id_add_sel_matches_node 
                            (cur_sel->add_sel, cur_node) == FALSE)
                        {
                               goto done;
                        }
                        goto walk_a_step_in_expr ;

		}
		else if (cur_sel->add_sel->type == ATTRIBUTE_ADD_SELECTOR
			 && cur_sel->add_sel->content.attr_sel)
		{
                        /*
			 *here, call a function that does the match
			 *against an attribute additionnal selector
			 *and an xml node.
			 */
                        if (attr_add_sel_matches_node 
                            (cur_sel->add_sel, cur_node)
                            == FALSE)
                        {
                                goto done ;
                        }
                        goto walk_a_step_in_expr ;
		}

	walk_a_step_in_expr:
                if (a_recurse == FALSE)
                {
                        *a_result = TRUE ;
                        goto done ;
                }

                /*
		 *here, depending on the combinator of cur_sel
		 *choose the axis of the xml tree traversing
		 *and walk one step in the xml tree.
		 */
                if (!cur_sel->prev)
                        break ;

                switch (cur_sel->prev->combinator)
                {
                case NO_COMBINATOR:
                        break ;

                case COMB_WS:/*descendant selector*/
                {
                        xmlNode *n = NULL ;
                        enum CRStatus status = CR_OK ;
                        gboolean matches= FALSE ;

                        /*
                         *walk the xml tree upward looking for a parent
                         *node that matches the preceding selector.
                         */
                        for (n = cur_node->parent ; n ; n = n->parent)
                        {
                                status = 
                                        sel_matches_node_real (a_this,
                                                               cur_sel->prev,
                                                               n,
                                                               &matches,
                                                               FALSE) ;
                                if (status != CR_OK)
                                        goto done ;

                                if (matches == TRUE)
                                {
                                        cur_node = n ;
                                        break ;
                                }
                        }

                        if (!n)
                        {
                                /*
                                 *didn't find any ancestor that matches
                                 *the previous simple selector.
                                 */
                                goto done ;
                        }
                        /*
                         *in this case, the preceding simple sel
                         *will have been interpreted twice, which
                         *is a cpu and mem waste ... I need to find
                         *another way to do this. Anyway, this is
                         *my first attempt to write this function and
                         *I am a bit clueless.
                         */
                        break ;
                }
     
                case COMB_PLUS:
                {
                        if (!cur_node->prev)
                                goto done ;
                        cur_node = cur_node->prev ;
                }
                break ;

                case COMB_GT:
                        if (!cur_node->parent)
                                goto done ;
                        cur_node = cur_node->parent ;
                        break ;

                default:
                        goto done ;
                }
                continue ;
		
	}

        *a_result = TRUE ;

 done:
        return CR_OK ;
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
	CRSelEng *result = NULL;

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
	g_return_val_if_fail (a_this && PRIVATE (a_this)
			      && a_this && a_node 
			      && a_node->type == XML_ELEMENT_NODE
			      && a_result,
			      CR_BAD_PARAM_ERROR) ;

        return sel_matches_node_real (a_this, a_sel, a_node,
                                      a_result, TRUE) ;
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
