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

#include <string.h>
#include "cr-utils.h"
#include "cr-om-parser.h"

/**
 *@file
 *The definition of the CSS Object Model Parser.
 *This parser uses (and sits) the SAC api of libcroco defined
 *in cr-parser.h and cr-doc-handler.h
 */

struct _CROMParserPriv
{
	CRParser *parser ;
} ;

#define PRIVATE(a_this) ((a_this)->priv)

/*
 *Forward declaration of a type defined later
 *in this file.
 */
struct _ParsingContext ;
typedef struct _ParsingContext ParsingContext ;

static ParsingContext *
new_parsing_context (void) ;

static void
destroy_context (ParsingContext *a_ctxt) ;

static void
unrecoverable_error (CRDocHandler *a_this) ;

static void
error (CRDocHandler *a_this) ;

static void
property (CRDocHandler *a_this,
          GString *a_name,
          CRTerm *a_expression) ;

static void
end_selector (CRDocHandler *a_this,
              CRSelector *a_selector_list) ;

static void
start_selector (CRDocHandler *a_this,
                CRSelector *a_selector_list) ;

static void
start_font_face (CRDocHandler *a_this) ;

static void
end_font_face (CRDocHandler *a_this) ;

static void 
end_document (CRDocHandler *a_this) ;

static void
start_document (CRDocHandler *a_this) ;

static void
charset (CRDocHandler *a_this, GString *a_charset) ;

static void
start_page (CRDocHandler *a_this, GString *a_page,
            GString *a_pseudo_page) ;

static void
end_page (CRDocHandler *a_this, GString *a_page,
          GString *a_pseudo_page) ;

static void
start_media (CRDocHandler *a_this, GList *a_media_list) ;

static void
end_media (CRDocHandler *a_this, GList *a_media_list) ;

static void
import_style (CRDocHandler *a_this, GList *a_media_list,
              GString *a_uri, GString *a_uri_default_ns) ;


struct _ParsingContext
{
        CRStyleSheet *stylesheet ;
        CRStatement *cur_stmt ;
        CRStatement *cur_media_stmt ;
} ;


/********************************************
 *Private methods
 ********************************************/

static ParsingContext *
new_parsing_context (void)
{
        ParsingContext *result = NULL ;

        result = g_try_malloc (sizeof (ParsingContext)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of Memory") ;
                return NULL ;
        }
        memset (result, 0, sizeof (ParsingContext)) ;
        return result ;
}

static void
destroy_context (ParsingContext *a_ctxt)
{
        g_return_if_fail (a_ctxt) ;
        
        if (a_ctxt->stylesheet)
        {
                cr_stylesheet_destroy (a_ctxt->stylesheet) ;
                a_ctxt->stylesheet = NULL ;
        }
        if (a_ctxt->cur_stmt)
        {
                cr_statement_destroy (a_ctxt->cur_stmt) ;
                a_ctxt->cur_stmt = NULL ;
        }
        g_free (a_ctxt) ;        
}


static enum CRStatus
cr_om_parser_init_default_sac_handler (CROMParser *a_this)
{
        CRDocHandler *sac_handler = NULL ;
        gboolean free_hdlr_if_error = FALSE ;
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->parser,
                              CR_BAD_PARAM_ERROR) ;
        
        status = cr_parser_get_sac_handler (PRIVATE (a_this)->parser,
                                            &sac_handler) ;
        g_return_val_if_fail (status == CR_OK, status) ;

        if (!sac_handler)
        {
                sac_handler = cr_doc_handler_new ();
                free_hdlr_if_error = TRUE ;
        }

        /*
         *initialyze here the sac handler.
         */
        sac_handler->start_document = start_document ;
        sac_handler->end_document = end_document ;
        sac_handler->start_selector = start_selector ;
        sac_handler->end_selector = end_selector;
        sac_handler->property = property ;
        sac_handler->start_font_face = start_font_face ;
        sac_handler->end_font_face = end_font_face ;
        sac_handler->error = error ;
        sac_handler->unrecoverable_error = unrecoverable_error ;
        sac_handler->charset = charset ;
        sac_handler->start_page = start_page ;
        sac_handler->end_page = end_page ;
        sac_handler->start_media = start_media ;
        sac_handler->end_media = end_media ;        
        sac_handler->import_style = import_style ;

        status = cr_parser_set_sac_handler (PRIVATE (a_this)->parser,
                                            sac_handler) ;
        if (status == CR_OK) 
        {
                return CR_OK;
        }

        if (sac_handler && free_hdlr_if_error == TRUE)
        {
                cr_doc_handler_destroy (sac_handler) ;
                sac_handler = NULL ;
        }

        return status ;

}

static void
start_document (CRDocHandler *a_this)
{
        ParsingContext *ctxt = NULL ;
        CRStyleSheet * stylesheet = NULL ;

        g_return_if_fail (a_this) ;
        
        ctxt = new_parsing_context () ;
        g_return_if_fail (ctxt) ;
        
        stylesheet = cr_stylesheet_new (NULL) ;
        ctxt->stylesheet = stylesheet ;
        a_this->context = ctxt ;
}

static void
start_font_face (CRDocHandler *a_this)
{
        ParsingContext *ctxt = NULL ;
        g_return_if_fail (a_this) ;

        g_return_if_fail (a_this && a_this->context) ;
        ctxt = a_this->context ;
        g_return_if_fail (ctxt->cur_stmt == NULL) ;

        ctxt->cur_stmt = cr_statement_new_at_font_face_rule (NULL) ;
        g_return_if_fail (ctxt->cur_stmt) ;
}

static void
end_font_face (CRDocHandler *a_this)
{
        ParsingContext *ctxt = NULL ;
        CRStatement *stmts = NULL ;

        g_return_if_fail (a_this) ;

        g_return_if_fail (a_this && a_this->context) ;
        ctxt = a_this->context ;
        g_return_if_fail 
                (ctxt->cur_stmt 
                 && ctxt->cur_stmt->type == AT_FONT_FACE_RULE_STMT
                 && ctxt->stylesheet) ;
        
        stmts = cr_statement_append (ctxt->stylesheet->statements,
                                     ctxt->cur_stmt) ;
        if (!stmts)
                goto error ;

        ctxt->stylesheet->statements = stmts ;
        stmts = NULL ;
        ctxt->cur_stmt = NULL ;

        return ;
        
 error:

        if (ctxt->cur_stmt)
        {
                cr_statement_destroy (ctxt->cur_stmt) ;
                ctxt->cur_stmt = NULL ;
        }

        if (!stmts)
        {
                cr_statement_destroy (stmts) ;
                stmts = NULL;
        }                
}


static void 
end_document (CRDocHandler *a_this)
{
        ParsingContext *ctxt = NULL ;
        g_return_if_fail (a_this) ;

        g_return_if_fail (a_this && a_this->context) ;
        ctxt = a_this->context ;

        if (!ctxt->stylesheet || ctxt->cur_stmt)
                goto error ;

        a_this->result = ctxt->stylesheet ;
        ctxt->stylesheet = NULL ;

        destroy_context (ctxt) ;

        a_this->context = NULL ;
        return ;

 error:
        if (ctxt)
        {
                destroy_context (ctxt) ;
        }
}

static void
charset (CRDocHandler *a_this, GString *a_charset)
{
        CRStatement *stmt = NULL, *stmt2 = NULL ;
        GString * charset = NULL ;

        ParsingContext *ctxt = NULL ;
        g_return_if_fail (a_this && a_this->context) ;

        ctxt = a_this->context ;
        g_return_if_fail (ctxt->stylesheet) ;

        charset = g_string_new_len (a_charset->str, 
                                    a_charset->len) ;

        stmt = cr_statement_new_at_charset_rule (charset) ;
        g_return_if_fail (stmt) ;

        stmt2 = cr_statement_append (ctxt->stylesheet->statements,
                                     stmt) ;
        if (!stmt2)
        {
                if (stmt)
                {
                        cr_statement_destroy (stmt) ;
                        stmt = NULL ;
                }

                if (charset)
                {
                        g_string_free (charset, TRUE) ;
                }
                return ;
        }

        ctxt->stylesheet->statements = stmt2 ;
        stmt2 = NULL ;
}

static void
start_page (CRDocHandler *a_this, GString *a_page,
            GString *a_pseudo)
{
        ParsingContext *ctxt = NULL ;
        g_return_if_fail (a_this && a_this->context) ;

        ctxt = a_this->context ;
        g_return_if_fail (ctxt->cur_stmt == NULL) ;

        ctxt->cur_stmt = cr_statement_new_at_page_rule (NULL, NULL, NULL) ;

        if (a_page)
        {
                 ctxt->cur_stmt->kind.page_rule->name = 
                         g_string_new_len (a_page->str, a_page->len) ;

                 if (!ctxt->cur_stmt->kind.page_rule->name)
                 {
                         goto error ;
                 }
        }
        
        if (a_pseudo)
        {
                ctxt->cur_stmt->kind.page_rule->pseudo =
                        g_string_new_len (a_pseudo->str, a_pseudo->len) ;

                if (!ctxt->cur_stmt->kind.page_rule->pseudo)
                {
                        goto error ;
                }
        }

        return ;

 error:
        if (ctxt->cur_stmt)
        {
                cr_statement_destroy (ctxt->cur_stmt) ;
                ctxt->cur_stmt = NULL ;
        }
}


static void
end_page (CRDocHandler *a_this, GString *a_page,
          GString *a_pseudo_page)
{
        ParsingContext *ctxt = NULL ;
        CRStatement *stmt = NULL ;

        g_return_if_fail (a_this && a_this->context) ;

        ctxt = a_this->context ;
        g_return_if_fail (ctxt->cur_stmt 
                          && ctxt->cur_stmt->type == AT_PAGE_RULE_STMT
                          && ctxt->stylesheet) ;

        stmt = cr_statement_append (ctxt->stylesheet->statements,
                                     ctxt->cur_stmt) ;

        if (stmt)
        {
                ctxt->stylesheet->statements = stmt ;
                stmt = NULL ;
                ctxt->cur_stmt = NULL ;
        }
                                
        if (ctxt->cur_stmt)
        {
                cr_statement_destroy (ctxt->cur_stmt) ;
                ctxt->cur_stmt = NULL ;
        }
        a_page = NULL ; /*keep compiler happy*/
        a_pseudo_page = NULL ; /*keep compiler happy*/
}

static void
start_media (CRDocHandler *a_this, GList *a_media_list)
{
        ParsingContext *ctxt = NULL ;
        GList * media_list = NULL ;

        g_return_if_fail (a_this && a_this->context) ;
        ctxt = a_this->context ;

        g_return_if_fail (ctxt 
                          && ctxt->cur_stmt == NULL
                          && ctxt->cur_media_stmt == NULL
                          && ctxt->stylesheet) ;

        if (a_media_list)
        {
                GList *cur = NULL ;

                /*duplicate the media_list*/
                for (cur = a_media_list; cur ; cur = cur->next)
                {
                        GString *str = NULL ;
                        
                        str = g_string_new_len (((GString*)cur->data)->str,
                                                ((GString*)cur->data)->len) ;
                        if (str)                                
                                media_list = g_list_append (media_list, 
                                                            str) ;
                }
        }

        ctxt->cur_media_stmt = 
                cr_statement_new_at_media_rule (NULL, media_list) ;
        
}

static void
end_media (CRDocHandler *a_this, GList *a_media_list)
{
        ParsingContext *ctxt = NULL ;
        CRStatement * stmts = NULL ;

        g_return_if_fail (a_this && a_this->context) ;
        ctxt = a_this->context ;

        g_return_if_fail (ctxt 
                          && ctxt->cur_media_stmt
                          && ctxt->cur_media_stmt->type == AT_MEDIA_RULE_STMT
                          && ctxt->stylesheet) ;

        stmts = cr_statement_append (ctxt->stylesheet->statements,
                                     ctxt->cur_media_stmt) ;
        if (!stmts)
        {
                cr_statement_destroy (ctxt->cur_media_stmt) ;
                ctxt->cur_media_stmt = NULL ;
        }
        
        ctxt->stylesheet->statements = stmts ;
        stmts = NULL ;
        
        a_media_list = NULL ; /*compiler happy*/
}


static void
import_style (CRDocHandler *a_this, GList *a_media_list,
              GString *a_uri, GString *a_uri_default_ns)
{
        GString *uri = NULL ;
        CRStatement *stmt = NULL, *stmt2 = NULL ;
        ParsingContext *ctxt = NULL ;
        GList *media_list = NULL, *cur = NULL ;

        g_return_if_fail (a_this && a_this->context) ;

        ctxt = a_this->context ;
        g_return_if_fail (ctxt->stylesheet) ;

        uri = g_string_new_len (a_uri->str, a_uri->len) ;
        
        for (cur = a_media_list ; cur; cur = cur->next)
        {
                if (cur->data)
                {
                        GString *str1 = NULL, *str2 = NULL ;
                        str1 = (GString*)cur->data ;
                        str2 = g_string_new_len (str1->str, str1->len) ;
                                                
                        media_list = g_list_append (media_list,
                                                   str2);
                }
        }

        stmt = cr_statement_new_at_import_rule (uri,
                                                media_list,
                                                NULL) ;    
        if (!stmt) 
                goto error ;
        
        if (ctxt->cur_stmt)
        {
                stmt2 = cr_statement_append (ctxt->cur_stmt, stmt) ;
                if (!stmt2)
                        goto error ;
                ctxt->cur_stmt = stmt2 ;
                stmt2 = NULL ;
                stmt = NULL ;
        }
        else
        {
                stmt2 = cr_statement_append (ctxt->stylesheet->statements,
                                             stmt) ;
                if (!stmt2)
                        goto error ;
                ctxt->stylesheet->statements = stmt2 ;
                stmt2 = NULL ;
                stmt = NULL ;
        }

        return ;

 error:
        if (uri)
        {
                g_string_free (uri, TRUE) ;
        }

        if (stmt)
        {
                cr_statement_destroy (stmt) ;
                stmt = NULL ;
        }
        a_uri_default_ns = NULL ; /*keep compiler happy*/
}

static void
start_selector (CRDocHandler *a_this,
                CRSelector *a_selector_list)
{
        ParsingContext *ctxt = NULL ;
        g_return_if_fail (a_this && a_this->context) ;

        ctxt = a_this->context ;
        if (ctxt->cur_stmt)
        {
                /*hmm, this should be NULL so free it*/
                cr_statement_destroy (ctxt->cur_stmt) ;
                ctxt->cur_stmt = NULL ;
        }

        ctxt->cur_stmt =cr_statement_new_ruleset 
                (a_selector_list,NULL, NULL) ;
}


static void
end_selector (CRDocHandler *a_this,
              CRSelector *a_selector_list)
{
        ParsingContext *ctxt = NULL ;
        g_return_if_fail (a_this && a_this->context) ;

        ctxt = a_this->context ;
        g_return_if_fail (ctxt->cur_stmt 
                          && ctxt->stylesheet) ;

        if (ctxt->cur_stmt)
        {
                CRStatement *stmts = NULL ;

                if (ctxt->cur_media_stmt)
                {
                        CRAtMediaRule *media_rule = NULL ;

                        media_rule = ctxt->cur_media_stmt->kind.media_rule ;

                        stmts = cr_statement_append 
                                (media_rule->rulesets, ctxt->cur_stmt) ;

                        if (!stmts)
                        {
                                cr_utils_trace_info 
                                        ("Could not append a new statement");
                                cr_statement_destroy 
                                        (media_rule->rulesets) ;
                                ctxt->cur_media_stmt->
                                        kind.media_rule->rulesets = NULL ;
                                return ;
                        }
                        media_rule->rulesets = stmts ;
                        ctxt->cur_stmt = NULL ;
                }
                else
                {
                        stmts = cr_statement_append 
                                (ctxt->stylesheet->statements,
                                 ctxt->cur_stmt) ;
                        if (!stmts)
                        {
                                cr_utils_trace_info 
                                        ("Could not append a new statement");
                                cr_statement_destroy (ctxt->cur_stmt) ;
                                ctxt->cur_stmt = NULL ;
                                return ;
                        }
                        ctxt->stylesheet->statements = stmts ;
                        ctxt->cur_stmt = NULL ;
                }
                
        }
        a_selector_list = NULL ; /*keep compiler happy*/
}

static void
property (CRDocHandler *a_this,
          GString *a_name,
          CRTerm *a_expression)
{
        ParsingContext *ctxt = NULL ;
        CRDeclaration *decl = NULL, *decl2 = NULL ;
        GString *str = NULL ;

        g_return_if_fail (a_this && a_this->context) ;
        ctxt = a_this->context ;

        /*
         *make sure a current ruleset statement has been allocated
         *already.
         */
        g_return_if_fail 
                (ctxt->cur_stmt 
                 && 
                 (ctxt->cur_stmt->type == RULESET_STMT
                  || ctxt->cur_stmt->type == AT_FONT_FACE_RULE_STMT
                  || ctxt->cur_stmt->type == AT_PAGE_RULE_STMT));

        if (a_name)
	{
		str = g_string_new_len (a_name->str,
					a_name->len) ;
		g_return_if_fail (str) ;
	}

        /*instanciates a new declaration*/
        decl = cr_declaration_new (str, a_expression) ;
        g_return_if_fail (decl) ;
        str = NULL ;

        /*
         *add the new declaration to the current statement
         *being build.
         */
        switch (ctxt->cur_stmt->type)
        {
        case RULESET_STMT:
                decl2 = cr_declaration_append 
                        (ctxt->cur_stmt->kind.ruleset->decl_list,
                         decl) ;
                if (!decl2)
                {
                        cr_declaration_destroy (decl) ;
                        cr_utils_trace_info 
                                ("Could not append decl to ruleset");
                        goto error ;
                }
                ctxt->cur_stmt->kind.ruleset->decl_list = decl2 ;
                decl = NULL ; decl2 = NULL ;
                break ;

        case AT_FONT_FACE_RULE_STMT:
                decl2 = cr_declaration_append 
                        (ctxt->cur_stmt->kind.font_face_rule->decls_list,
                         decl) ;
                if (!decl2)
                {
                        cr_declaration_destroy (decl) ;
                        cr_utils_trace_info 
                                ("Could not append decl to ruleset");
                        goto error ;
                }
                ctxt->cur_stmt->kind.font_face_rule->decls_list = decl2 ;
                decl = NULL ; decl2 = NULL ;
                break ;
        case AT_PAGE_RULE_STMT:
                decl2 = cr_declaration_append 
                        (ctxt->cur_stmt->kind.page_rule->decls_list,
                         decl) ;
                if (!decl2)
                {
                        cr_declaration_destroy (decl) ;
                        cr_utils_trace_info 
                                ("Could not append decl to ruleset");
                        goto error ;
                }
                ctxt->cur_stmt->kind.page_rule->decls_list = decl2 ;
                decl = NULL ; decl2 = NULL ;
                break ;

        default:
                goto error ;
                break ;
        }

        return ;

 error:
        if (str)
        {
                g_free (str) ;
                str = NULL ;
        }

        if (decl)
        {
                cr_declaration_destroy (decl) ;
                decl = NULL ;
        }
}

static void
error (CRDocHandler *a_this)
{
        ParsingContext *ctxt = NULL ;
        g_return_if_fail (a_this && a_this->context) ;
        
        ctxt = a_this->context ;

        if (ctxt->cur_stmt)
        {
                cr_statement_destroy (ctxt->cur_stmt) ;
                ctxt->cur_stmt = NULL ;
        }
}



static void
unrecoverable_error (CRDocHandler *a_this)
{
        if (a_this->context)
        {
                if (((ParsingContext*)a_this->context)->stylesheet) 
                {
                        a_this->result = 
                                ((ParsingContext*)a_this->context)->
                                stylesheet ;
                }
                g_free (a_this->context) ;
                a_this->context = NULL ;
        }
}


/********************************************
 *Public methods
 ********************************************/

/**
 *Constructor of the CROMParser.
 *@param a_input the input stream.
 *@return the newly built instance of #CROMParser.
 */
CROMParser *
cr_om_parser_new (CRInput *a_input)
{
	CROMParser *result = NULL  ;
        enum CRStatus status = CR_OK ;

	result = g_try_malloc (sizeof (CROMParser)) ;

	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}

	memset (result, 0, sizeof (CROMParser)) ;
	PRIVATE (result) = g_try_malloc (sizeof (CROMParserPriv)) ;

        if (!PRIVATE (result))
        {
                cr_utils_trace_info ("Out of memory") ;
                goto error ;
        }

        memset (PRIVATE (result), 0, sizeof (CROMParserPriv)) ;

        PRIVATE (result)->parser = cr_parser_new_from_input (a_input);

        if (!PRIVATE (result)->parser)
        {
                cr_utils_trace_info ("parsing instanciation failed") ;
                goto error ;
        }

        status = cr_om_parser_init_default_sac_handler (result) ;

        if (status != CR_OK)
        {
                goto error ;
        }

        return result ;

 error:

        if (result)
        {
                cr_om_parser_destroy (result) ;
        }

        return NULL ;
}


enum CRStatus
cr_om_parser_parse_file (CROMParser *a_this,
                         guchar *a_file_uri,
                         enum CREncoding a_enc,
                         CRStyleSheet **a_result)
{
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && a_file_uri && a_result, 
                              CR_BAD_PARAM_ERROR) ;

        if (!PRIVATE (a_this)->parser)
        {
                PRIVATE (a_this)->parser = cr_parser_new_from_file 
                        (a_file_uri, a_enc) ;
        }

        status = cr_parser_parse_from_file (PRIVATE (a_this)->parser,
                                            a_file_uri, a_enc) ;

        if (status == CR_OK)
        {
                CRDocHandler *sac_handler = NULL ;
                cr_parser_get_sac_handler (PRIVATE (a_this)->parser,
                                           &sac_handler) ;
                g_return_val_if_fail (sac_handler, CR_ERROR) ;

                if (sac_handler->result)
                        *a_result = sac_handler->result ;
        }

        return status ;
}

/**
 *Destructor of the #CROMParser.
 *@param a_this the current instance of #CROMParser.
 */
void
cr_om_parser_destroy (CROMParser *a_this)
{
	g_return_if_fail (a_this && PRIVATE (a_this)) ;

        if (PRIVATE (a_this)->parser)
        {
                cr_parser_destroy (PRIVATE (a_this)->parser) ;
                PRIVATE (a_this)->parser = NULL ;
        }

	if (PRIVATE (a_this))
	{
		g_free (PRIVATE (a_this)) ;
		PRIVATE (a_this) = NULL ;
	}

	if (a_this)
	{
		g_free (a_this) ;
		a_this = NULL ;
	}
}

