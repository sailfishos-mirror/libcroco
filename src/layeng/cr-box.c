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
#include "cr-box.h"

/**
 *@file
 *The definition file of the #CRBox class.
 */

static enum CRStatus
cr_box_edge_to_string (CRBoxEdge *a_this,
                       gulong a_nb_indent,
                       GString **a_string) ;

static enum CRBoxType
cr_box_guess_type (CRStyle *a_style) ;


/******************************
 *Private methods
 ******************************/

/**
 *Guess the type of a box from the 'position' rule
 *contained in its style data structure.
 *@param a_style the style data structure associated to the box.
 */
static enum CRBoxType
cr_box_guess_type (CRStyle *a_style)
{
        enum CRBoxType box_type = BOX_TYPE_INLINE ;

        if (!a_style)
                return box_type ;

        switch (a_style->display)
        {
        case DISPLAY_NONE:
                break ;

        case DISPLAY_INLINE:
        case DISPLAY_MARKER:
                box_type = BOX_TYPE_INLINE ;
                break ;

        case DISPLAY_BLOCK:
        case DISPLAY_LIST_ITEM:
        case DISPLAY_TABLE:
        case DISPLAY_INLINE_TABLE:
        case DISPLAY_TABLE_ROW_GROUP:
        case DISPLAY_TABLE_HEADER_GROUP:
        case DISPLAY_TABLE_FOOTER_GROUP:
        case DISPLAY_TABLE_ROW:
        case DISPLAY_TABLE_COLUMN_GROUP:
        case DISPLAY_TABLE_COLUMN:
        case DISPLAY_TABLE_CELL:
        case DISPLAY_TABLE_CAPTION:
                box_type = BOX_TYPE_BLOCK ;
                break ;

        case DISPLAY_COMPACT:
                box_type = BOX_TYPE_COMPACT ;
                break ;

        case DISPLAY_RUN_IN:
                box_type = BOX_TYPE_RUN_IN ;
                break ;

        case DISPLAY_INHERIT:
                if (a_style->parent_style)
                        box_type =  
                                cr_box_guess_type (a_style->parent_style) ;
                break ;

        default:
                box_type = BOX_TYPE_INLINE ;
                break ;
        }

        return box_type ;
}


static enum CRStatus
cr_box_edge_to_string (CRBoxEdge *a_this,
                       gulong a_nb_indent,
                       GString **a_string)
{
        GString *result = NULL ;

        if (*a_string)
        {
                result = *a_string ;
        }
        else
        {
                result = g_string_new (NULL) ;
                if (!result)
                {
                        cr_utils_trace_info ("Out of memory") ;
                        return CR_ERROR ;
                }
        }

        cr_utils_dump_n_chars2 (' ', result, 
                                a_nb_indent) ;
        g_string_append_printf (result, "(%ld, %ld)\n", 
                                (long int)a_this->x,
                                (long int) a_this->y) ;
        cr_utils_dump_n_chars2 (' ', result, 
                                a_nb_indent) ;
        g_string_append_printf (result, "width: %ld\n", 
                                (long int)a_this->width) ;
        cr_utils_dump_n_chars2 (' ', 
                                result, a_nb_indent) ;
        g_string_append_printf (result, "height: %ld\n", 
                                (long int)a_this->height) ;
        cr_utils_dump_n_chars2 (' ', 
                                result, a_nb_indent) ;
        g_string_append_printf (result, "x_offset: %ld\n", 
                                (long int)a_this->x_offset) ;
        cr_utils_dump_n_chars2 (' ', result, 
                                a_nb_indent) ;
        g_string_append_printf (result, "y_offset: %ld\n", 
                                (long int)a_this->y_offset) ;

        return CR_OK ;
}

/*******************************
 *Public methods
 *******************************/

/**
 *Instanciates a new #CRBoxData.
 *@param a_node the xml node to store in the box.
 *@return the newly built #CRBoxData, or null if an error arises.
 */
CRBoxData *
cr_box_data_new (xmlNode *a_node)
{
        CRBoxData *result = NULL ;

        result = g_try_malloc (sizeof (CRBoxData)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL;
        }
        memset (result, 0, sizeof (CRBoxData)) ;
        result->xml_node = a_node ;
        return result ;
}


/**
 *Destructor of #CRBoxData.
 *@param a_this the current instance 
 *of #CRBoxData to be destroyed.
 */
void
cr_box_data_destroy (CRBoxData *a_this)
{
        if (!a_this)
                return ;

        g_free (a_this) ;
}

/**
 *Instanciates a new #CRBoxContent and set the
 *content to text content.
 *@param a_text the text content.
 */
CRBoxContent *
cr_box_content_new_from_text (guchar *a_text)
{
        CRBoxContent *result = NULL ;

        g_return_val_if_fail (a_text, NULL) ;

        result = g_try_malloc (sizeof (CRBoxContent)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }
        memset (result, 0, sizeof (CRBoxContent)) ;
        result->u.text = g_strdup (a_text) ;
        result->type = TEXT_CONTENT_TYPE ;
        return result ;
}

/**
 *Destructor of #CRBoxContent.
 *@param a_this the current instance of #CRBoxContent
 *to be destroyed.
 */
void
cr_box_content_destroy (CRBoxContent *a_this)
{
        if (!a_this)
                return ;

        switch (a_this->type)
        {
        case TEXT_CONTENT_TYPE:
                if (a_this->u.text)
                {
                        g_free (a_this->u.text) ;
                        a_this->u.text = NULL ;
                }
                break ;

        default:
                cr_utils_trace_info ("Unrecognized box content type") ;
                cr_utils_trace_info ("This seems to be a mem leak") ;
                break ;
        }
        g_free (a_this) ;
        return ;
}

/**
 *Creates a new box model.
 *This box model contains an empty box tree.
 *Box tree may be added by calling cr_box_append_child().
 *@return the newly built instance of #CRBoxModel, or NULL if an
 *error arises.
 */
CRBoxModel *
cr_box_model_new (void)
{
        CRBoxModel *result = NULL ;

        result = g_try_malloc (sizeof (CRBoxModel)) ;
        if (!result)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }

        memset (result, 0, sizeof (CRBoxModel)) ;

        ((CRBox*)result)->type = BOX_TYPE_BOX_MODEL ;
        ((CRBox*)result)->box_model = result ;

        return result ;
}


/**
 *Instanciates a new box.
 *Everything is initialized to zero in it.
 *@return the newly created box.
 */
CRBox *
cr_box_new (CRStyle *a_style)
{
	CRBox *result = NULL ;
        CRStyle *style = a_style ;

	result = g_try_malloc (sizeof (CRBox)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		goto error ;
	}
	memset (result, 0, sizeof (CRBox)) ;

        if (!style)
        {
                style = cr_style_new () ;
                if (!style)
                {
                        cr_utils_trace_info 
                                ("Could not create style object") ;
                        cr_utils_trace_info ("System may be out of memory") ;
                        goto error ;
                }
        }
        result->style = style ;
        cr_style_ref (result->style) ;
        result->type = cr_box_guess_type (result->style) ;
	return result ;

 error:
        if (result)
        {
                cr_box_destroy (result) ;
                result = NULL ;
        }

        return NULL ;
} 

/**
 *Appends a child box to at the end of the current box's children.
 *@param a_this the current box.
 *@param a_to_append, the box to append.
 *@retrurn CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_box_append_child (CRBox *a_this, CRBox *a_to_append)
{
	CRBox * cur = NULL ;

	g_return_val_if_fail (a_this 
                              && a_this->box_model
                              && a_to_append, CR_BAD_PARAM_ERROR) ;

	if (!a_this->children)
	{
		a_this->children = a_to_append ;
                a_to_append->prev = NULL ;
                a_to_append->parent = a_this ;
                a_to_append->box_model = a_this->box_model ;
		return CR_OK ;
	}

	for (cur = a_this->children ; cur && cur->next ; cur = cur->next)
		;

	cur->next = a_to_append ;
        a_to_append->prev = cur ;
        a_to_append->parent = cur ;
        a_to_append->box_model = a_this->box_model ;

	return CR_OK ;
}


/**
 *Inserts a sibling box between two adjacent sibling nodes.
 *@param a_prev the box after which we have to insert a new one.
 *@param a_next the box before which we have to insert a new one.
 *@param a_to_insert the node to insert.
 */
enum CRStatus
cr_box_insert_sibling (CRBox *a_prev,
                       CRBox *a_next,
                       CRBox *a_to_insert)
{
	g_return_val_if_fail (a_prev && a_prev->parent
			      && a_next && a_prev->next == a_next
			      && a_next->parent == a_prev->parent
                              && a_prev->box_model
                              && a_prev->box_model == a_next->box_model
			      && a_to_insert
			      && a_to_insert->parent != a_prev->parent,
			      CR_BAD_PARAM_ERROR) ;

	a_prev->next = a_to_insert ;
	a_to_insert->prev = a_prev ;
	a_to_insert->next = a_next ;
	a_next->prev = a_to_insert ;
        a_to_insert->parent = a_prev->parent ;
        a_to_insert->box_model = a_prev->box_model ;

	return CR_OK ;
}


/**
 *This is for debug purposes ...
 *Gives a string representation of the box tree.
 *@return the build string of NULL in case of an error.
 */
enum CRStatus
cr_box_to_string (CRBox *a_this, 
                  gulong a_nb_indent,
                  GString **a_string)
{
        GString *result = NULL ;
        CRBox *cur_box = NULL ;

        g_return_val_if_fail (a_this && a_string, 
                              CR_BAD_PARAM_ERROR) ;

        if (*a_string)
        {
                result = *a_string ;
        }
        else
        {
                result = g_string_new (NULL) ;
                if (!result)
                {
                        cr_utils_trace_info ("Out of memory") ;
                        return CR_ERROR ;
                }
                *a_string = result ;
        }        

        for (cur_box = a_this ; cur_box ; cur_box = cur_box->next)
        {
                if (cur_box->prev || cur_box->parent)
                        g_string_append_printf (result, "\n\n") ;

                cr_utils_dump_n_chars2 (' ', result, a_nb_indent) ;

                switch (cur_box->type)
                {                        
                case BOX_TYPE_BLOCK:
                        g_string_append_printf (result, "BLOCK") ;
                        break ;

                case BOX_TYPE_ANONYMOUS_BLOCK:
                        g_string_append_printf (result, "ANONYMOUS BLOCK") ;
                        break ;

                case BOX_TYPE_INLINE:
                        g_string_append_printf (result, "INLINE") ;
                        break ;

                case BOX_TYPE_ANONYMOUS_INLINE:
                        g_string_append_printf (result, "ANONYMOUS INLINE") ;
                        break ;

                case BOX_TYPE_COMPACT:
                        g_string_append_printf (result, "COMPACT") ;
                        break ;

                case BOX_TYPE_RUN_IN:
                        g_string_append_printf (result, "RUN IN") ;
                        break ;

                case BOX_TYPE_BOX_MODEL:
                        g_string_append_printf (result, "Root") ;
                        break ;

                default:
                        g_string_append_printf (result, "UNKNOWN") ;
                        break ;
                }
                
                g_string_append_printf (result, " box\n") ;
                cr_utils_dump_n_chars2 (' ', result, a_nb_indent) ;
                g_string_append_printf (result, "{") ;
        
                if (cur_box->box_data && cur_box->box_data->xml_node)
                {
                        switch (cur_box->box_data->xml_node->type)
                        {
                        case XML_ELEMENT_NODE:
                                cr_utils_dump_n_chars2 
                                        (' ', result, a_nb_indent) ;
                                g_string_append_printf 
                                        (result, "xml-node-name: %s\n", 
                                         cur_box->box_data->xml_node->name) ;
                                break ;

                        case XML_TEXT_NODE:
                                cr_utils_dump_n_chars2 
                                        (' ', result, a_nb_indent) ;
                                g_string_append_printf 
                                        (result, "xml-text-node\n") ;
                                break ;

                        default:                                
                                break ;
                        }
                }

                cr_utils_dump_n_chars2 (' ', result, a_nb_indent + 2) ;
                g_string_append_printf (result, "/*****%s begin*****/\n", 
                                        "outer_edge") ;
                cr_box_edge_to_string (&cur_box->outer_edge,
                                       a_nb_indent + 2, &result) ;
                cr_utils_dump_n_chars2 (' ', result, a_nb_indent + 2) ;
                g_string_append_printf (result, "/*****%s end*****/\n", 
                                        "outer_edge") ;

                cr_utils_dump_n_chars2 (' ', result, a_nb_indent + 2) ;
                g_string_append_printf (result, "/*****%s begin*****/\n", 
                                        "border_edge") ;
                cr_box_edge_to_string (&cur_box->border_edge, 
                                       a_nb_indent + 2, &result) ;
                cr_utils_dump_n_chars2 (' ', result, a_nb_indent + 2) ;
                g_string_append_printf (result, "/*****%s end*****/\n", 
                                        "border_edge") ;

                cr_utils_dump_n_chars2 (' ', result, a_nb_indent + 2) ;
                g_string_append_printf (result, "/*****%s begin*****/\n", 
                                        "padding_edge") ;
                cr_box_edge_to_string (&cur_box->padding_edge,
                                       a_nb_indent + 2, &result) ;
                cr_utils_dump_n_chars2 (' ', result, a_nb_indent + 2) ;
                g_string_append_printf (result, "/*****%s end*****/\n", 
                                        "padding_edge") ;

                cr_utils_dump_n_chars2 (' ', result, a_nb_indent + 2) ;
                g_string_append_printf (result, "/*****%s begin*****/\n", 
                                        "inner_edge") ;
                cr_box_edge_to_string (&cur_box->inner_edge,
                                       a_nb_indent + 2, &result) ;
                cr_utils_dump_n_chars2 (' ', result, a_nb_indent + 2) ;
                g_string_append_printf (result, "/*****%s end*****/\n", 
                                        "inner_edge") ;

                if (cur_box->children)
                {
                        g_string_append_printf (result, "\n") ;
                        cr_box_to_string (cur_box->children,
                                          a_nb_indent + 2, &result) ;
                }

                g_string_append_printf (result, "\n") ;
                cr_utils_dump_n_chars2 (' ', result, a_nb_indent) ;
                g_string_append_printf (result, "}\n") ;
        }

        return CR_OK ;
}

enum CRStatus
cr_box_dump_to_file (CRBox *a_this, 
                     gulong a_nb_indent,
                     FILE *a_filep)
{
        GString *str = NULL ;
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && a_filep,
                              CR_BAD_PARAM_ERROR) ;

        status = cr_box_to_string (a_this, a_nb_indent, &str) ;

        if (status != CR_OK)
        {
                cr_utils_trace_info ("An error occured "
                                     "during in memory serialisation") ;
                goto cleanup ;
        }

        if (!str || !str->str)
        {
                cr_utils_trace_info ("Error: Box could not be serialised") ;
                goto cleanup ;
        }

        if (!fwrite (str->str, 1, str->len, a_filep))
        {
                cr_utils_trace_info ("An error occured during"
                                     "serialisation into file") ;
                status = CR_ERROR ;
                goto cleanup ;
        }

        status = CR_OK ;

 cleanup:

        if (str)
        {
                g_string_free (str, TRUE) ;
                str = NULL ;
        }
        return status ;
}

/**
 *Increments the reference count of
 *the current instance of #CRBox.
 */
enum CRStatus
cr_box_ref (CRBox *a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;
        
        a_this->ref_count ++ ;

        return TRUE ;
}


/**
 *Decrements the current instance's ref count.
 *If the ref count reaches zero, the instance is destroyed.
 *@param a_this the current instance.
 *@return TRUE if the ref count reached zero and the instance has been
 *destroyed, FALSE otherwise.
 */
gboolean
cr_box_unref (CRBox *a_this)
{
        g_return_val_if_fail (a_this, FALSE) ;

        if (a_this->ref_count)
                a_this->ref_count -- ;

        if (a_this->ref_count == 0)
        {
                cr_box_destroy (a_this) ;
                return TRUE ;
        }

        return FALSE ;
}

/**
 *Destructor of #CRBox.
 *recursively destroys all
 *the children nodes of the current node.
 *@param a_this the current box to destroy.
 */
void
cr_box_destroy (CRBox *a_this)

{
	g_return_if_fail (a_this) ;

        if (a_this->content)
        {
                cr_box_content_destroy (a_this->content) ;
                a_this->content = NULL ;
        }

        if (a_this->style)
        {
                cr_style_unref (a_this->style) ;
                a_this->style = NULL ;
        }

	if (a_this->children)
	{
		CRBox *cur = NULL;

		for (cur = a_this->children ; cur ; cur = cur->next)
		{
			cr_box_destroy (cur) ;
		}
		a_this->children = NULL ;
	}

	g_free (a_this) ;
}

