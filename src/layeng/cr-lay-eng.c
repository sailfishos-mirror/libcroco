/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the 
 * GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the 
 * GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

/*
 *$Id$
 */

#include <gnome.h>
#include <string.h>
#include "cr-lay-eng.h"
#include "cr-sel-eng.h"

#define PRIVATE(a_this) ((a_this)->priv)

/**
 *@file
 *The definition of the #CRLayEng class.
 *Highly unstable and experimental so far.
 *This is in developement so the api is *really* gonna change ...
 */

struct _CRLayEngPriv
{
	CRCascade *cascade ;
        CRSelEng *sel_eng ;
} ;

static CRBox *
cr_lay_eng_create_box_tree_real (CRLayEng * a_this,
                                 xmlNode *a_root_node,
                                 CRBox *a_parent_box) ;

static enum CRStatus
cr_lay_eng_layout_box (CRBox *a_this) ;

static glong
cr_lay_eng_get_box_bottommost_y (CRBox *a_this) ;

static glong
cr_lay_eng_get_box_rightmost_x (CRBox *a_this) ;

static enum CRStatus
cr_lay_eng_layout_box_normal (CRBox *a_this) ;

static enum CRStatus
compute_text_box_inner_edge_size (CRBox *a_this) ;


/**********************
 *Private methods.
 **********************/

/**
 *Sets the box style values so that
 *it has no padding, no border, no margin.
 *The other style values are left as is cause
 *they must have been set prior to calling this
 *function.
 */
static void
init_anonymous_text_box (CRBox *a_box)
{
        g_return_if_fail (a_box && a_box->style) ;
        
        cr_num_set (&a_box->style->padding_top, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->padding_right, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->padding_bottom, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->padding_left, 0, NUM_LENGTH_PX) ;
        
        cr_num_set (&a_box->style->border_top_width, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->border_right_width, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->border_bottom_width, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->border_left_width, 0, NUM_LENGTH_PX) ;

        cr_num_set (&a_box->style->margin_top, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->margin_right, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->margin_bottom, 0, NUM_LENGTH_PX) ;
        cr_num_set (&a_box->style->margin_left, 0, NUM_LENGTH_PX) ;
        
        a_box->style->border_top_style = BORDER_STYLE_NONE ;
        a_box->style->border_right_style = BORDER_STYLE_NONE ;
        a_box->style->border_bottom_style = BORDER_STYLE_NONE ;
        a_box->style->border_left_style = BORDER_STYLE_NONE ;

        cr_num_set (&a_box->style->top.num, 0, NUM_LENGTH_PX) ;
        a_box->style->top.type = OFFSET_DEFINED ;
        cr_num_set (&a_box->style->right.num, 0, NUM_LENGTH_PX) ;
        a_box->style->right.type = OFFSET_DEFINED ;
        cr_num_set (&a_box->style->bottom.num, 0, NUM_LENGTH_PX) ;
        a_box->style->bottom.type = OFFSET_DEFINED ;
        cr_num_set (&a_box->style->left.num, 0, NUM_LENGTH_PX) ;
        a_box->style->left.type = OFFSET_DEFINED ;
        
        a_box->style->float_type = FLOAT_NONE ;
}

/**
 *Creates a box sub tree from an xml node tree.
 *@param a_this the current instance of #CRLayEng.
 *@param a_root_node the root node of the xml tree.
 *@param a_parent_box the root of the box tree to build.
 *@return the newly built box tree, or NULL if an error
 *happens.
 */
static CRBox *
cr_lay_eng_create_box_tree_real (CRLayEng * a_this,
                                 xmlNode *a_root_node,
                                 CRBox *a_parent_box)
{
        enum CRStatus status = CR_OK ;
        xmlNode *cur = NULL ;
        CRBox *cur_box = NULL ;
        CRBoxData *box_data = NULL ;

        g_return_val_if_fail (a_this
                              && PRIVATE (a_this)
                              && PRIVATE (a_this)->cascade
                              && a_root_node, NULL) ;

        if (!PRIVATE (a_this)->sel_eng)
        {
                PRIVATE (a_this)->sel_eng = cr_sel_eng_new () ;
                if (!PRIVATE (a_this)->sel_eng)
                {
                        cr_utils_trace_info 
                                ("Could not create selection engine") ;
                        cr_utils_trace_info 
                                ("System may be out of memory") ;
                        return NULL ;
                }
        }

        for (cur = a_root_node ; cur ; cur = cur->next)
        {
                CRStyle *style = NULL, *parent_style = NULL ;

                if (cur->type != XML_ELEMENT_NODE
                    && cur->type != XML_TEXT_NODE)
                        continue ;

                /*build here the node annotation*/
                if (cur->parent && a_parent_box && a_parent_box->style)
                        parent_style = a_parent_box->style ;

                if (cur->type == XML_ELEMENT_NODE)
                {
                        status =
                                cr_lay_eng_get_matched_style
                                (a_this,
                                 PRIVATE (a_this)->cascade,
                                 cur, parent_style, &style) ;

                        if (status != CR_OK
                            || (style && style->display == DISPLAY_NONE))
                        {
                                continue ;
                        }

                        /*here, build the box,
                         *append it to the box tree
                         *and update all it attributes but
                         *the positioning. The positioning will
                         *be updated later via the cr_box_reflow() method.
                         */
                        cur_box = cr_box_new (style) ;
                        if (!cur_box)
                        {
                                cr_utils_trace_info
                                        ("Could not create a box") ;
                                cr_utils_trace_info
                                        ("The system may be out of memory") ;
                                return NULL ;
                        }

                        if (a_parent_box)
                                cr_box_append_child (a_parent_box,
                                                     cur_box) ;
                        style = NULL ;

                        /*
                         *store a pointer to the node that generated
                         *the current box into that current box.
                         */
                        box_data = cr_box_data_new (cur) ;
                        if (!box_data)
                        {
                                cr_utils_trace_info ("Out of memory") ;
                                goto error ;
                        }
                        cur_box->box_data = box_data ;
                        box_data = NULL ;

                        if (style)
                        {
                                cr_style_destroy (style) ;
                                style = NULL ;
                        }
                }
                else if (cur->type == XML_TEXT_NODE)
                {
                        CRBoxContent *box_content = NULL ;
                        xmlChar *node_text = NULL ;

                        if (xmlIsBlankNode (cur))
                                continue ;

                        node_text = xmlNodeGetContent (cur) ;
                        if (node_text)
                        {
                                box_content = 
                                        cr_box_content_new_from_text
                                        (node_text) ;
                                xmlFree (node_text) ;
                                node_text = NULL ;
                        }
                        if (box_content)
                        {
                                /*
                                 *create here an anonymous box
                                 *which style inherits the style
                                 *of the parent box.
                                 */
                                cur_box = cr_box_new (parent_style) ;
                                if (!cur_box)
                                {
                                        cr_utils_trace_info
                                                ("could not create "
                                                 "anonymous box") ;
                                        goto error ;
                                }
                                cur_box->content = box_content ;
                                box_content = NULL ;
                                /*
                                 *the anonymous box
                                 *must have no margin,
                                 *no padding, no border,
                                 *no border style, no offset
                                 */
                                init_anonymous_text_box (cur_box) ;

                                /*
                                 *store a pointer to the node that generated
                                 *the current box into that current box.
                                 */
                                box_data = cr_box_data_new (cur) ;
                                if (!box_data)
                                {
                                        cr_utils_trace_info 
                                                ("Out of memory") ;
                                        goto error ;
                                }
                                cur_box->box_data = box_data ;
                                box_data = NULL ;

                                cr_box_append_child (a_parent_box,
                                                     cur_box) ;
                                cur_box = NULL ;
                        }
                }
                else
                {
                        cr_utils_trace_info 
                                ("xml node type neither element nor text") ;
                        cr_utils_trace_info 
                                ("this should not happen. This is a bug") ;
                }

                /*walk through what remains from the tree*/
                if (cur->children)
                {
                        cr_lay_eng_create_box_tree_real
                                (a_this, cur->children, cur_box) ;
                }
        }

        return cur_box ;

 error:
        if (cur_box)
        {
                cr_box_destroy (cur_box) ;
                cur_box = NULL ;                
        }
        if (box_data)
        {
                cr_box_data_destroy (box_data) ;
                box_data = NULL ;
        }

        return NULL ;
}


static glong
cr_lay_eng_get_box_bottommost_y (CRBox *a_this)
{
        return (a_this->outer_edge.y
                +
                a_this->outer_edge.y_offset
                +
                a_this->outer_edge.height) ;
}

/**
 *Computes the abscissa of the rightmost side
 *of the current box.
 *@param a_box the current box.
 *@return a positve or 0 number if the computation went well,
 *-1 otherwise.
 */
static glong
cr_lay_eng_get_box_rightmost_x (CRBox *a_this)
{       
        if (!a_this)
                return 0 ;

        return (a_this->outer_edge.x 
                +
                a_this->outer_edge.x_offset
                +
                a_this->outer_edge.width) ;
}

static enum CRStatus
compute_text_box_inner_edge_size (CRBox *a_this)
{
        enum CRStatus status = CR_OK ;
        GtkWidget *label = NULL ;
        PangoLayout *pgo_layout = NULL ;
        PangoRectangle ink_rect = {0}, logical_rect = {0} ;
        guchar *text = NULL ;

        g_return_val_if_fail (a_this 
                              && a_this->content
                              && a_this->content->type == TEXT_CONTENT_TYPE,
                              CR_BAD_PARAM_ERROR) ;
        
        if (a_this->content->u.text == NULL 
            || strlen (a_this->content->u.text) == 0)
        {
                a_this->inner_edge.width = 0 ;
                a_this->inner_edge.height = 0 ;
                return CR_OK ;
        }

        text = a_this->content->u.text ;

        label = gtk_label_new (NULL) ;        
        g_return_val_if_fail (label, CR_ERROR) ;

        pgo_layout = gtk_widget_create_pango_layout (label, text) ;
        pango_layout_get_pixel_extents (pgo_layout, &ink_rect,
                                        &logical_rect) ;

        a_this->inner_edge.width = logical_rect.width ;
        a_this->inner_edge.height = logical_rect.height ;

 cleanup:

        if (label)
        {
                gtk_widget_destroy (label) ;
                label = NULL ;
        }

        if (pgo_layout)
        {
                g_object_unref (G_OBJECT (pgo_layout)) ;
                pgo_layout = NULL ;
        }

        return status ;
}

/**
 *Lay the box out according to "Normal flow"
 *as decribed in css2 spec chap 9.4.
 *In normal flow, a box belongs to a formating context
 *that may be block or inline. In block formating context,
 *boxes are laid out verticaly, one under an other.
 *In inline formatting context, boxes are laid out horizontally,
 *usually from the left to the right, unless we support bidi.
 *@param a_this the current box.
 */
static enum CRStatus
cr_lay_eng_layout_box_normal (CRBox *a_this)
{
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && a_this->style,
                              CR_BAD_PARAM_ERROR) ;

        /*
         *Only boxes that have
         *the position rule set to 'static' or 'relative'
         *can be part of a normal formatting context.
         */
        if (a_this->style->position != POSITION_STATIC
            && a_this->style->position != POSITION_RELATIVE)
        {
                return CR_UNEXPECTED_POSITION_SCHEME ;
        }

        /*
         *TODO
         *compute the "computed values" of the style data structure.
         */
        switch (a_this->type)
        {
        case BOX_TYPE_BLOCK:
        case BOX_TYPE_ANONYMOUS_BLOCK:
        {
                CRBox *cont_box = a_this->parent ;
                /************************************
                 *We are in a block formating context 
                 ************************************/

                /*
                 *position the 'x' of the top
                 *leftmost corner of this box
                 *at the leftmost abscissa of it's 
                 *containing box.
                 *Position the 'y' of 
                 *the top left corner of this
                 *just under the previous box.
                 */                
                if (!cont_box)
                        a_this->outer_edge.x = 0 ;
                else
                        a_this->outer_edge.x = 
                                cont_box->inner_edge.x ;
                
                a_this->outer_edge.y =
                        cr_lay_eng_get_box_bottommost_y (a_this->prev) ;

                a_this->outer_edge.x =
                        cr_lay_eng_get_box_rightmost_x (a_this->prev) ;

                /*******************************************
                 *Now, compute the inner edge of this box;
                 *which means 
                 *1/set the left border and 
                 *left padding edges.
                 *2/compute the left most x and topmost y of
                 *the inner box and.
                 *3/Compute the outer edge of the containing
                 *box; this is recursive.
                 *******************************************/

                /*
                 *1/ => left side of outer edge is separated from
                 *left side of border edge by "margin-left"... same
                 *principle applies for padding edge and inner edge.
                 */

                /*
                 *TODO: collapse this margin !!!. 
                 *See css2 chap 8.3.1 to see what "collapsing" means.
                 */
                a_this->border_edge.x =
                        a_this->outer_edge.x
                        - 
                        a_this->style->margin_left.val ;
                a_this->border_edge.y =
                        a_this->outer_edge.y
                        -
                        a_this->style->margin_top.val ;

                a_this->padding_edge.x =
                        a_this->border_edge.x 
                        -
                        a_this->style->border_left_width.val ;
                a_this->padding_edge.y =
                        a_this->border_edge.y
                        -
                        a_this->style->border_top_width.val ;

                /*
                 *Now 2/
                 */
                a_this->inner_edge.x =
                        a_this->padding_edge.x
                        -
                        a_this->style->padding_left.val ;
                a_this->inner_edge.y =
                        a_this->padding_edge.y
                        -
                        a_this->style->padding_left.val ;

                /*
                 *And now, 3/
                 */
                if (a_this->children)
                {
                        cr_lay_eng_layout_box (a_this->children) ;
                }
                else
                {
                        /*
                         *this box may have a content.
                         *TODO: compute it's width and height.
                         *then, when computed, update the
                         *children max width size in the parent box.
                         */
                        if (a_this->content)
                        {
                                switch (a_this->content->type)
                                {
                                case TEXT_CONTENT_TYPE:
                                        compute_text_box_inner_edge_size
                                                (a_this) ;
                                        break ;

                                case IMAGE_CONTENT_TYPE:
                                        cr_utils_trace_info 
                                                ("image content not "
                                                 "supported yet") ;
                                        break ;
                                case NO_CONTENT_TYPE:
                                        cr_utils_trace_info
                                                ("incoherent box model. "
                                                 "We should have either "
                                                 "image or text here. "
                                                 "found NO_CONTENT_TYPE "
                                                 "intead") ;
                                        break ;
                                default:
                                        cr_utils_trace_info 
                                                ("Unknown content type") ;
                                        break ;
                                }
                        }
                }

                /*
                 *now that we have the width/height of the inner box,
                 *let's compute the width/height of the padding box,
                 *border box and outer box.
                 */
                a_this->padding_edge.width = a_this->inner_edge.width +
                        a_this->style->padding_right.val +
                        a_this->style->padding_left.val ;

                a_this->padding_edge.height = a_this->inner_edge.height +
                        a_this->style->padding_top.val +
                        a_this->style->padding_bottom.val ;

                /*TODO continue the calculus of the widths/height*/

                /*******************************************
                 *Inner edge position (x,y) computing is 
                 *finished. (we have it's width).
                 *So now, we can compute the widths of the
                 *remaining three other boxes 
                 *(padding edge, border edge and outer edge)
                 ******************************************/
                break ;
        }

        case BOX_TYPE_COMPACT:
        case BOX_TYPE_RUN_IN:
        case BOX_TYPE_INLINE:
        case BOX_TYPE_ANONYMOUS_INLINE:
        {
                CRBox *cont_box = NULL, *prev_box = NULL ;
                
                cont_box = a_this->parent ;
                prev_box = a_this->prev ;

                /************************************
                 *We are in an inline formating context 
                 ************************************/

                /*
                 *position the 'x' of the top
                 *leftmost corner of this box
                 *one pixel right after the rightmost x
                 *of the preceding box.
                 *Position the 'y' of this box to
                 *the y of the previous box.
                 */
                if (!prev_box)
                {
                        a_this->outer_edge.x = 0 ;
                        a_this->outer_edge.y = 0 ;
                }
                else
                {
                        a_this->outer_edge.x = 
                                cr_lay_eng_get_box_rightmost_x
                                (prev_box) + 1 ;
                        a_this->outer_edge.y = prev_box->outer_edge.y ;
                }

                /*TODO*/
                /*******************************************
                 *Now, compute the inner edge of this box;
                 *which means 
                 *1/set the left border and 
                 *left padding edges.
                 *2/compute the left most x and topmost y of
                 *the inner box and.
                 *3/Compute the outer edge of the containing
                 *box; this is recursive.
                 *******************************************/
        }
                break ;

        default:
                break ;
        }

        return status ;
}

static enum CRStatus
cr_lay_eng_layout_box (CRBox *a_this)
{
        CRBox *cur_box = NULL ;

        g_return_val_if_fail (a_this && a_this->style,
                              CR_BAD_PARAM_ERROR) ;

        for (cur_box = a_this ; cur_box ; 
             cur_box = cur_box->next)
        {
                switch (cur_box->style->position)
                {
                case POSITION_STATIC:
                case POSITION_RELATIVE:
                        cr_lay_eng_layout_box_normal (cur_box) ;
                        break ;

                case POSITION_ABSOLUTE:
                case POSITION_FIXED:
                        /*cr_box_layout_absolute (a_this) ;*/
                        break ;

                case POSITION_INHERIT:
                        break ;
                }
        }
        return CR_OK ;
}

/**********************
 *Public methods.
 **********************/

CRLayEng *
cr_lay_eng_new (void)
{
	CRLayEng *result = NULL;

	result = g_try_malloc (sizeof (CRLayEng)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRLayEng)) ;

	PRIVATE (result) = g_try_malloc (sizeof (CRLayEngPriv)) ;
	if (!PRIVATE (result))
	{
		cr_utils_trace_info ("Out of memory") ;
		g_free (result) ;
		result = NULL ;
		return NULL ;
	}
	memset (PRIVATE (result), 0, sizeof (CRLayEngPriv)) ;

	return result ;
}

/**
 *Creates the box model from an xml document.
 *@param a_this the current instance of #CRLayEng.
 *@param a_doc the current xml document.
 *@param a_cascade the css2 stylesheet cascade.
 *@param a_box_model out parameter. The returned
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_lay_eng_build_box_tree (CRLayEng *a_this,
                           xmlDoc *a_doc,
                           CRCascade *a_cascade,
                           CRBox **a_box_model)
{
        xmlNode *root_node = NULL ;

        g_return_val_if_fail (a_this && a_doc && a_cascade,
                              CR_BAD_PARAM_ERROR) ;

        root_node = xmlDocGetRootElement (a_doc) ;
        if (!root_node)
                return CR_NO_ROOT_NODE_ERROR ;

        PRIVATE (a_this)->cascade = a_cascade ;

        *a_box_model = 
                cr_lay_eng_create_box_tree_real (a_this, root_node,
                                                 NULL) ;

        return CR_OK ;
}


/**
 *Retrieves the style structure that matches the xml node
 *from the cascade.
 *NOTE: this does not implement the complex cascade algorithms
 *described in the css2 spec from chapter 6.4 on, but instead,
 *is just an empty design sketch so that other hackers (yeah, we can dream) 
 *can come an implement it. I don't have the time for this right now.
 *@param a_this the current instance of #CRLayEng.
 *@param a_cascade the cascade from which the request is to be made.
 *@param a_node the xml node to match
 *@param a_parent_style the style of the parent xml node.
 *@param a_style out parameter. a pointer to the style 
 *structure to be returned. *a_style must be set to NULL, otherwise
 *a CR_BAD_PARAM_ERROR is returned. The caller must free the
 *the returned *a_style using cr_style_destroy().
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_lay_eng_get_matched_style (CRLayEng *a_this,
                              CRCascade *a_cascade,
                              xmlNode *a_node,
                              CRStyle *a_parent_style,
                              CRStyle **a_style)
{       
        CRStatement **rulesets = NULL ;
        CRStyleSheet *author_sheet = NULL ;
        gulong len = 0 ;
        CRStyle *result_style = NULL ;
        CRSelEng * sel_eng = NULL ;
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && a_cascade
                              && a_node && a_style
                              && (*a_style == NULL),
                              CR_BAD_PARAM_ERROR) ;

        author_sheet = cr_cascade_get_sheet (a_cascade, 
                                             ORIGIN_AUTHOR) ;
        if (!author_sheet)
        {
                cr_utils_trace_info ("Could not get author sheet "
                                     "from cascade") ;
                return CR_ERROR ;
        }

        sel_eng = cr_sel_eng_new () ;
        if (!sel_eng)
        {
                cr_utils_trace_info ("Failed to instanciate "
                                     "The Selection Engine") ;
                return CR_ERROR ;
        }

        status = cr_sel_eng_get_matched_rulesets (sel_eng, author_sheet,
                                                  a_node, &rulesets,
                                                  &len) ;
        if (len && rulesets[len - 1])
        {
                status = cr_style_new_from_ruleset 
                        (rulesets[len - 1], a_parent_style,
                         &result_style) ;
                
        }

        if (result_style)
        {
                *a_style = result_style ;
                result_style = NULL ;
        }

        if (rulesets)
        {
                g_free (rulesets) ;
                rulesets = NULL ;
        }

/* cleanup:*/

        if (sel_eng)
        {
                cr_sel_eng_destroy (sel_eng) ;
                sel_eng = NULL ;
        }

        return status ;
}

/**
 *Destuctor of #CRLayEng.
 *@param a_this the current instance of #CRLayEng.
 */
void
cr_lay_eng_destroy (CRLayEng *a_this)
{
	g_return_if_fail (a_this) ;

        if (PRIVATE (a_this)->sel_eng)
        {
                cr_sel_eng_destroy (PRIVATE (a_this)->sel_eng) ;
                PRIVATE (a_this)->sel_eng = NULL ;
        }

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
