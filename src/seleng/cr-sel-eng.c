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
 * You should have received a copy of the GNU Lesser 
 * General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */


#include <string.h>
#include "cr-sel-eng.h"

/**
 *@file:
 *The definition of the  #CRSelEng class.
 *The #CRSelEng is actually the "Selection Engine"
 *class. This is highly experimental for at the moment and
 *its api is very likely to change in a near future.
 */

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

static enum CRStatus
sel_matches_node_real (CRSelEng *a_this, CRSimpleSel *a_sel,
                       xmlNode *a_node, gboolean *a_result,
                       gboolean a_recurse) ;

static enum CRStatus
cr_sel_eng_get_matched_rulesets_real (CRSelEng *a_this, 
                                      CRStyleSheet *a_stylesheet,
                                      xmlNode *a_node,
                                      CRStatement **a_rulesets, 
                                      gulong *a_len) ;

static enum CRStatus
put_css_properties_in_hashtable (GHashTable **a_props_hashtable,
                                 CRStatement *a_ruleset) ;

static void
set_style_from_props_hash_hr_func (gpointer a_prop, gpointer a_decl,
                                   gpointer a_style) ;

struct _CRSelEngPriv
{
        /*not used yet*/
        gboolean case_sensitive ;

        CRStyleSheet *sheet ;

        /**
         *where to store the next statement
         *to be visited so that we can remember
         *it from one method call to another.
         */
        CRStatement *cur_stmt ;
};

/**
 *@param a_add_sel the class additional selector to consider.
 *@param a_node the xml node to consider.
 *@return TRUE if the class additional selector matches
 *the xml node given in argument, FALSE otherwise.
 */
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

/**
 *@return TRUE if the additional attribute selector matches
 *the current xml node given in argument, FALSE otherwise.
 *@param a_add_sel the additional attribute selector to consider.
 *@param a_node the xml node to consider.
 */
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
                                
                                if (g_strstr_len (ptr1, ptr2 - ptr1 + 1,
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

/**
 *Evaluate a selector (a simple selectors list) and says
 *if it matches the xml node given in parameter.
 *The algorithm used here is the following:
 *Walk the combinator separated list of simple selectors backward, starting
 *from the end of the list. For each simple selector, looks if
 *if matches the current node.
 *
 *@param a_this the selection engine.
 *@param a_sel the simple selection list.
 *@param a_node the xml node.
 *@param a_result out parameter. Set to true if the
 *selector matches the xml node, FALSE otherwise.
 *@param a_recurse if set to TRUE, the function will walk to
 *the next simple selector (after the evaluation of the current one) 
 *and recursively evaluate it. Must be usually set to TRUE unless you
 *know what you are doing.
 */
static enum CRStatus
sel_matches_node_real (CRSelEng *a_this, CRSimpleSel *a_sel,
                       xmlNode *a_node, gboolean *a_result,
                       gboolean a_recurse)
{
        CRSimpleSel *cur_sel = NULL ;
	xmlNode *cur_node = NULL ;
        
	g_return_val_if_fail (a_this && PRIVATE (a_this)
			      && a_this && a_node 
			      && a_result,
			      CR_BAD_PARAM_ERROR) ;

        *a_result = FALSE ;

        if (a_node->type != XML_ELEMENT_NODE)
                return CR_OK ;

	
        if (a_recurse == TRUE)
        {
                /*go and get the last simple selector of the list*/
                for (cur_sel = a_sel ; 
                     cur_sel && cur_sel->next ; 
                     cur_sel = cur_sel->next) ;
        }
        else
        {
                cur_sel = a_sel ;
        }        

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
                                        /*
                                         *this simple selector
                                         *matches the current xml node
                                         *Let's see if the preceding
                                         *simple selectors also match
                                         *their xml node counterpart.
                                         */
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
		 *choose the axis of the xml tree traversal
		 *and walk one step in the xml tree.
		 */
                if (!cur_sel->prev)
                        break ;

                switch (cur_sel->combinator)
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
        
        /*
         *if we reached this point, it means the selector matches
         *the xml node.
         */
        *a_result = TRUE ;
        
 done:
        return CR_OK ;
}


/**
 *Returns  array of the ruleset statements that matches the
 *given xml node.
 *The engine keeps in memory the last statement he
 *visited during the match. So, the next call
 *to this function will eventually return a rulesets list starting
 *from the last ruleset statement visited during the previous call.
 *The enable users to get matching rulesets in an incremental way.
 *Note that for each statement returned, 
 *the engine calculates the specificity of the selector
 *that matched the xml node and stores it in the "specifity" field
 *of the statement structure.
 *
 *@param a_sel_eng the current selection engine
 *@param a_node the xml node for which the request
 *is being made.
 *@param a_sel_list the list of selectors to perform the search in.
 *@param a_rulesets in/out parameter. A pointer to the
 *returned array of rulesets statements that match the xml node
 *given in parameter. The caller allocates the array before calling this
 *function.
 *@param a_len in/out parameter the length (in sizeof (#CRStatement*)) 
 *of the returned array.
 *(the length of a_rulesets, more precisely).
 *The caller must set it to the length of a_ruleset prior to calling this
 *function. In return, the function sets it to the length 
 *(in sizeof (#CRStatement)) of the actually returned CRStatement array.
 *@return CR_OUTPUT_TOO_SHORT_ERROR if found more rulesets than the size
 *of the a_rulesets array. In this case, the first *a_len rulesets found
 *are put in a_rulesets, and a further call will return the following
 *ruleset(s) following the same principle.
 *@return CR_OK if all the rulesets found have been returned. In this
 *case, *a_len is set to the actual number of ruleset found.
 *@return CR_BAD_PARAM_ERROR in case any of the given parameter are
 *bad (e.g null pointer).
 *@return CR_ERROR if any other error occured.
 */
static enum CRStatus
cr_sel_eng_get_matched_rulesets_real (CRSelEng *a_this, 
                                      CRStyleSheet *a_stylesheet,
                                      xmlNode *a_node,
                                      CRStatement **a_rulesets, 
                                      gulong *a_len)
{
        CRStatement *cur_stmt = NULL ;
        CRSelector *sel_list = NULL, *cur_sel = NULL ;
        gboolean matches = FALSE ;
        enum CRStatus status = CR_OK ;
        gulong i = 0;

        g_return_val_if_fail (a_this
                              && a_stylesheet
                              && a_stylesheet->statements
                              && a_node
                              && a_rulesets,
                              CR_BAD_PARAM_ERROR) ;

        /*
         *if this stylesheet is "new one"
         *let's remember it for subsequent calls.
         */
        if (PRIVATE (a_this)->sheet != a_stylesheet)
        {
                PRIVATE (a_this)->sheet = a_stylesheet ;
                PRIVATE (a_this)->cur_stmt =  a_stylesheet->statements ;
        }

        /*
         *walk through the list of statements and,
         *get the selectors list inside the statements that
         *contain some, and try to match our xml node in these
         *selectors lists.
         */
        for (cur_stmt = PRIVATE (a_this)->cur_stmt, i = 0 ;
             (PRIVATE (a_this)->cur_stmt = cur_stmt); 
             cur_stmt = cur_stmt->next)
        {
                /*
                 *initialyze the selector list in which we will
                 *really perform the search.
                 */
                sel_list = NULL ;

                /*
                 *get the the damn selector list in 
                 *which we have to look
                 */
                switch (cur_stmt->type)
                {
                case RULESET_STMT:
                        if (cur_stmt->kind.ruleset 
                            && cur_stmt->kind.ruleset->sel_list)
                        {
                                sel_list = cur_stmt->kind.ruleset->sel_list ;
                        }
                        break ;
                
                case AT_MEDIA_RULE_STMT:
                        if (cur_stmt->kind.media_rule
                            && cur_stmt->kind.media_rule->rulesets
                            && cur_stmt->kind.media_rule->rulesets->
                            kind.ruleset
                            &&cur_stmt->kind.media_rule->rulesets->
                                kind.ruleset->sel_list)
                        {
                                sel_list = 
                                        cur_stmt->kind.media_rule->
                                        rulesets->kind.ruleset->sel_list ;
                        }
                        break ;

                case AT_IMPORT_RULE_STMT:
                        /*
                         *some recursivity may be needed here.
                         *I don't like this :(
                         */
                        break ;
                default:
                        break ;
                }

                if (!sel_list)
                        continue ;

                /*
                 *now, we have a comma separated selector list to look in.
                 *let's walk it and try to match the xml_node
                 *on each item of the list.
                 */
                for (cur_sel = sel_list ; cur_sel ; cur_sel = cur_sel->next)
                {
                        if (!cur_sel->simple_sel)
                                continue ;

                        status = cr_sel_eng_matches_node 
                                (a_this, cur_sel->simple_sel,
                                 a_node, &matches) ;

                        if (status == CR_OK && matches == TRUE)
                        {
                                /*
                                 *bingo!!! we found one ruleset that
                                 *matches that fucking node.
                                 *lets put it in the out array.
                                 */

                                if (i < *a_len)
                                {
                                        a_rulesets[i] = cur_stmt ;
                                        i++ ;

                                        /*
                                         *For the cascade computing algorithm
                                         *(which is gonna take place later)
                                         *we must compute the specificity
                                         *(css2 spec chap 6.4.1) of the selector
                                         *that matched the current xml node
                                         *and store it in the css2 statement
                                         *(statement == ruleset here).
                                         */
                                        status = 
                                                cr_simple_sel_compute_specificity
                                                (cur_sel->simple_sel) ;

                                        g_return_val_if_fail (status == CR_OK,
                                                              CR_ERROR) ;
                                        cur_stmt->specificity = 
                                                cur_sel->simple_sel->specificity;
                                }
                                else
                                        
                                {
                                        *a_len = i ;
                                        return CR_OUTPUT_TOO_SHORT_ERROR ;
                                }
                        }
                }
        }

        /*
         *if we reached this point, it means
         *we reached the end of stylesheet.
         *no need to store any info about the stylesheet
         *anymore.
         */
        g_return_val_if_fail (!PRIVATE (a_this)->cur_stmt, CR_ERROR) ;
        PRIVATE (a_this)->sheet = NULL ;
        *a_len = i ;
        return CR_OK ;
}

/**
 *Walks through the property/value pairs of a ruleset
 *statement and put the properties found into a hashtable.
 *Each key of the hashtable is a css property. The
 *associated value is a pointer to the current #CRStatement.
 *This function is where the cascading property sorting is done.
 *
 *@param a_props_hashtable in/out parameter. The hashtable into
 *which the the property/statement pairs will be added.
 *Note that each hashtable key (a statement property) is a null terminated 
 *instance of guchar *.
 *Each value associated to a key is an instance of #CRStatement. The statement
 *is actually the css ruleset that contains the property (the key).
 *@param a_ruleset the ruleset from wich the properties are gathered.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
put_css_properties_in_hashtable (GHashTable **a_props_hashtable,
                                 CRStatement *a_stmt)
{
        GHashTable *props_hash = NULL ;
        CRDeclaration *cur_decl = NULL ;

        g_return_val_if_fail (a_props_hashtable && a_stmt
                              && a_stmt->type == RULESET_STMT
                              && a_stmt->kind.ruleset,
                              CR_BAD_PARAM_ERROR) ;

        if (!*a_props_hashtable)
        {
                *a_props_hashtable = g_hash_table_new (g_str_hash,
                                                       g_str_equal) ;
        }
        props_hash = *a_props_hashtable ;

        for (cur_decl = a_stmt->kind.ruleset->decl_list ; 
             cur_decl ; cur_decl = cur_decl->next)
        {
                if (!cur_decl->property || !cur_decl->property->str)
                        continue ;

                CRDeclaration *decl = NULL ;

                /*
                 *First, test if the property is not
                 *already present in our properties hashtable.
                 *If yes, apply the cascading rules to
                 *compute the precedence. If not, insert
                 *the property into the hashtable.
                 */
                decl = g_hash_table_lookup 
                        (props_hash, cur_decl->property->str) ;

                if (!decl)
                {
                        g_hash_table_replace 
                                (props_hash,
                                 cur_decl->property->str,
                                 cur_decl) ;
                        continue ;
                }

                /*
                 *A property with the same name already exists.
                 *We must apply here 
                 *some cascading rules
                 *to compute the precedence.
                 */

                /*
                 *first, look at the origin.
                 *6.4.1 says: 
                 *"for normal declarations, 
                 *author style sheets override user 
                 *style sheets which override 
                 *the default style sheet."
                 */
                if (decl->parent_statement 
                    && decl->parent_statement->parent_sheet
                    && (decl->parent_statement->parent_sheet->origin 
                        <
                        a_stmt->parent_sheet->origin))
                {
                        g_hash_table_insert 
                                (props_hash,
                                 cur_decl->property->str,
                                 cur_decl) ;
                        continue ;
                }
                else if (decl->parent_statement 
                         && decl->parent_statement->parent_sheet
                         && (decl->parent_statement->
                             parent_sheet->origin 
                             >
                             a_stmt->parent_sheet->origin))
                {
                        /*TODO: support !important rule.*/
                        continue ;
                }

                /*
                 *A property with the same
                 *name and the same origin already exist.
                 *shit. This is lasting longer than expected ...
                 *Luckily, the spec says in 6.4.1:
                 *"more specific selectors will override 
                 *more general ones"
                 *and
                 *"if two rules have the same weight, 
                 *origin and specificity, 
                 *the latter specified wins"
                 */
                if (a_stmt->specificity
                    >= decl->parent_statement->specificity)
                {
                        g_hash_table_insert
                                (props_hash,
                                 cur_decl->property->str,
                                 cur_decl) ;
                }
        }


        return CR_OK ;
}



static void
set_style_from_props_hash_hr_func (gpointer a_prop, gpointer a_decl,
                                   gpointer a_style)
{
        CRDeclaration *decl = a_decl ;
        CRStyle *style = a_style ;

        g_return_if_fail (a_decl && a_prop && a_style) ;

        cr_style_set_style_from_decl (style, decl) ;
}


/****************************************
 *PUBLIC METHODS
 ****************************************/

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

        PRIVATE (result) = g_try_malloc (sizeof (CRSelEngPriv)) ;
        if (!PRIVATE (result))
	{
		cr_utils_trace_info ("Out of memory") ;
                g_free (result) ;
		return NULL ;
	}
        memset (PRIVATE (result), 0, sizeof (CRSelEngPriv)) ;

	return result ;
}


/**
 *Evaluates a chained list of simple selectors (known as a css2 selector).
 *Says wheter if this selector matches the xml node given in parameter or
 *not.
 *@param a_this the selection engine.
 *@param a_sel the simple selector against which the xml node 
 *is going to be matched.
 *@param a_node the node against which the selector is going to be matched.
 *@param a_result out parameter. The result of the match. Is set to
 *TRUE if the selector matches the node, FALSE otherwise. This value
 *is considered if and only if this functions returns CR_OK.
 *@return the CR_OK if the selection ran correctly, an error code otherwise.
 */
enum CRStatus
cr_sel_eng_matches_node (CRSelEng *a_this, CRSimpleSel *a_sel,
                         xmlNode *a_node, gboolean *a_result)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this)
			      && a_this && a_node 
			      && a_result,
			      CR_BAD_PARAM_ERROR) ;

        if (a_node->type != XML_ELEMENT_NODE)
        {
                *a_result = FALSE ;
                return CR_OK ;
        }

        return sel_matches_node_real (a_this, a_sel, a_node,
                                      a_result, TRUE) ;
}

/**
 *Returns an array of pointers to selectors that matches
 *the xml node given in parameter.
 *
 *@param a_this the current instance of the selection engine.
 *@param a_sheet the stylesheet that holds the selectors.
 *@param a_node the xml node to consider during the walk thru
 *the stylesheet.
 *@param a_rulesets out parameter. A pointer to an array of
 *rulesets statement pointers. *a_rulesets is allocated by
 *this function and must be freed by the caller. However, the caller
 *must not alter the rulesets statements pointer because they
 *point to statements that are still in the css stylesheet.
 *@param a_len the length of *a_ruleset.
 *@return CR_OK upon sucessfull completion, an error code otherwise.
 */
enum CRStatus
cr_sel_eng_get_matched_rulesets (CRSelEng *a_this,
                                 CRStyleSheet *a_sheet,
                                 xmlNode *a_node,
                                 CRStatement ***a_rulesets,
                                 gulong *a_len)
{
        CRStatement ** stmts_tab = NULL ;
        enum CRStatus status = CR_OK ;
        gulong tab_size = 0, tab_len = 0, index = 0 ;
        gushort stmts_chunck_size = 8 ;

        g_return_val_if_fail (a_this
                              && a_sheet
                              && a_node
                              && a_rulesets && *a_rulesets == NULL
                              && a_len,
                              CR_BAD_PARAM_ERROR) ;

        stmts_tab = g_try_malloc (stmts_chunck_size *
                                  sizeof (CRStatement *)) ;

        if (!stmts_tab)
        {
                cr_utils_trace_info ("Out of memory") ;
                status = CR_ERROR ;
                goto error ;
        }
        memset (stmts_tab, 0, stmts_chunck_size * sizeof (CRStatement*)) ;

        tab_size = stmts_chunck_size ;
        tab_len = tab_size ;

        while ((status = cr_sel_eng_get_matched_rulesets_real 
                (a_this, a_sheet, a_node, stmts_tab + index, &tab_len))
               == CR_OUTPUT_TOO_SHORT_ERROR)
        {
                stmts_tab = g_try_realloc (stmts_tab,
                                           (tab_size + stmts_chunck_size)
                                           * sizeof (CRStatement*)) ;
                if (!stmts_tab)
                {
                        cr_utils_trace_info ("Out of memory") ;
                        status = CR_ERROR ;
                        goto error ;
                }
                tab_size +=  stmts_chunck_size ;
                index += tab_len ;
                tab_len = tab_size - index ;
        }


        tab_len = tab_size - stmts_chunck_size +tab_len ;
        *a_rulesets = stmts_tab ;
        *a_len = tab_len ;

        return CR_OK ;

 error:

        if (stmts_tab)
        {
                g_free (stmts_tab) ;
                stmts_tab = NULL ;
                
        }

        *a_len = 0 ;
        return status ;
}

enum CRStatus
cr_sel_eng_get_matched_properties_from_cascade  (CRSelEng *a_this,
                                                 CRCascade *a_cascade,
                                                 xmlNode *a_node,
                                                 GHashTable **a_props_hashtable)
{
        CRStatement ** stmts_tab = NULL ;
        enum CRStatus status = CR_OK ;
        gulong tab_size = 0, tab_len = 0, index = 0, i = 0  ;
        enum CRStyleOrigin origin = 0 ;
        gushort stmts_chunck_size = 8 ;
        CRStyleSheet *sheet = NULL ;

        g_return_val_if_fail (a_this
                              && a_cascade
                              && a_node
                              && a_props_hashtable,
                              CR_BAD_PARAM_ERROR) ;

        stmts_tab = g_try_malloc (stmts_chunck_size *
                                  sizeof (CRStatement *)) ;

        if (!stmts_tab)
        {
                cr_utils_trace_info ("Out of memory") ;
                status = CR_ERROR ;
                goto error ;
        }
        memset (stmts_tab, 0, stmts_chunck_size * sizeof (CRStatement*)) ;
        tab_size = stmts_chunck_size ;
                tab_len = tab_size ;

        for (origin = ORIGIN_UA ; origin < NB_ORIGINS ; origin++)
        {
                sheet = cr_cascade_get_sheet (a_cascade, origin) ;
                if (!sheet)
                        continue ;                

                while ((status = cr_sel_eng_get_matched_rulesets_real 
                        (a_this, sheet, a_node, stmts_tab + index, &tab_len))
                       == CR_OUTPUT_TOO_SHORT_ERROR)
                {
                        stmts_tab = g_try_realloc 
                                (stmts_tab,
                                 (tab_size + stmts_chunck_size)
                                 * sizeof (CRStatement*)) ;
                        if (!stmts_tab)
                        {
                                cr_utils_trace_info ("Out of memory") ;
                                status = CR_ERROR ;
                                goto error ;
                        }
                        tab_size +=  stmts_chunck_size ;
                        index += tab_len ;
                        tab_len = tab_size - index ;
                }
                if (status != CR_OK)
                {
                        cr_utils_trace_info ("Error while running "
                                             "selector engine") ;
                        goto error ;
                }                
                
        }

        /*
         *TODO, walk down the stmts_tab and build the
         *property_name/declaration hashtable.
         *Make sure one can walk from the declaration to
         *the stylesheet.
         */
        for (i = 0 ; i < tab_len ; i ++)
        {
                CRStatement *stmt = stmts_tab[i] ;

                if (!stmt)
                        continue ;
                
                switch (stmt->type)
                {
                case RULESET_STMT:
                        if (!stmt->parent_sheet)
                                continue ;
                        status = put_css_properties_in_hashtable
                                (a_props_hashtable,
                                 stmt) ;
                        break ;

                default:
                        break ;
                }
                
        }

        return CR_OK ;
 error:

        if (stmts_tab)
        {
                g_free (stmts_tab) ;
                stmts_tab = NULL ;
                
        }

        return status ;
}


enum CRStatus
cr_sel_eng_get_matched_style (CRSelEng *a_this,
                              CRCascade *a_cascade,
                              xmlNode *a_node,
                              CRStyle *a_parent_style,
                              CRStyle **a_style)
{
        enum CRStatus status = CR_OK ;
        GHashTable *props_hash = NULL ;

        g_return_val_if_fail (a_this && a_cascade
                              && a_node && a_style
                              && (*a_style == NULL),
                              CR_BAD_PARAM_ERROR) ;
        
        status = cr_sel_eng_get_matched_properties_from_cascade 
                (a_this, a_cascade, a_node, &props_hash) ;
        g_return_val_if_fail (status == CR_OK, status) ;

        if (props_hash && g_hash_table_size (props_hash))
        {
                
                if (!*a_style)
                {
                        *a_style = cr_style_new () ;
                        g_return_val_if_fail (*a_style, CR_ERROR) ;
                }
                (*a_style)->parent_style = a_parent_style ;
                
                g_hash_table_foreach (props_hash,
                                      ((GHFunc)
                                       set_style_from_props_hash_hr_func),
                                      *a_style) ;                
        }

        if (props_hash)
        {
                g_hash_table_destroy (props_hash) ;
                props_hash = NULL ;
        }

        return CR_OK ;
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

