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
#include <gdk/gdk.h>
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
        gboolean update_parent_box_size ;
	CRCascade *cascade ;
        CRSelEng *sel_eng ;
        gulong xdpi ;/*x resolution*/
        gulong ydpi ; /*y resolution*/
} ;


enum CRDirection
{
        DIR_UNKNOWN = 0,
        DIR_VERTICAL,
        DIR_HORIZONTAL
} ;

static gboolean gv_layeng_initialized = FALSE ;

static void
init_anonymous_text_box (CRBox *a_box) ;

static enum CRStatus
style_specified_2_computed_values (CRLayEng *a_this,
                                   CRStyle *a_style, 
                                   CRBox *a_parent_box) ;

static CRBox *
create_box_tree_real (CRLayEng * a_this,
                      xmlNode *a_root_node,
                      CRBox *a_parent_box) ;

static glong
get_box_bottommost_y (CRBox *a_this) ;

static glong
get_box_rightmost_x (CRBox *a_this) ;

static enum CRStatus
compute_box_size (CRLayEng *a_this,
                  CRBox *a_cur_box) ;

static enum CRStatus
layout_inline_box (CRLayEng *a_this,
                   CRBox *a_cur_box) ;

static enum CRStatus
layout_block_box (CRLayEng *a_this,
                  CRBox *a_cur_box) ;

static enum CRStatus
layout_box_in_normal_flow (CRLayEng *a_this,
                           CRBox *a_cur_box) ;

static enum CRStatus
layout_box (CRLayEng *a_this, CRBox *a_cur_box) ;

static enum CRStatus
compute_text_box_inner_edge_size (CRBox *a_this) ;

static enum CRStatus
adjust_parent_inner_edge_size (CRLayEng *a_this,
                               CRBox *a_cur_box) ;

static enum CRStatus
normalize_num (CRLayEng *a_this,
               CRNum *a_dest_num,
               CRNum *a_src_num,
               enum CRDirection a_dir) ;

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
        glong i = 0 ;

        g_return_if_fail (a_box && a_box->style) ;

        for (i = 0 ; i< NB_NUM_PROPS ; i++)
        {
                switch (i)
                {
                case NUM_PROP_PADDING_TOP:
                case NUM_PROP_PADDING_RIGHT:
                case NUM_PROP_PADDING_BOTTOM:
                case NUM_PROP_PADDING_LEFT:
                case NUM_PROP_BORDER_TOP:
                case NUM_PROP_BORDER_RIGHT:
                case NUM_PROP_BORDER_BOTTOM:
                case NUM_PROP_BORDER_LEFT:
                case NUM_PROP_MARGIN_TOP:
                case NUM_PROP_MARGIN_RIGHT:
                case NUM_PROP_MARGIN_BOTTOM:
                case NUM_PROP_MARGIN_LEFT:
                        cr_num_set (&a_box->style->num_props[i].sv, 
                                    0, NUM_LENGTH_PX) ;
                        break ;

                default:
                        break ;
                }
                
        }
        
        for (i = 0 ; i< NB_BORDER_STYLE_PROPS ; i++)
        {
                a_box->style->border_style_props[i] = BORDER_STYLE_NONE ;
        }
        
        a_box->style->float_type = FLOAT_NONE ;
}


static enum CRStatus
normalize_num (CRLayEng *a_this,
               CRNum *a_dest_num,
               CRNum *a_src_num,
               enum CRDirection a_dir)
{
        g_return_val_if_fail (a_this && a_dest_num 
                              && a_src_num,
                              CR_BAD_PARAM_ERROR) ;

        switch (a_src_num->type)
        {
        case NUM_LENGTH_PX:
                cr_num_copy (a_dest_num, a_src_num) ;
                /*a_dest_num->type = NUM_LENGTH_PX ;*/
                break ;

        case NUM_LENGTH_EM:
        case NUM_LENGTH_EX:                
                break ;

        case NUM_LENGTH_IN:
                if (a_dir == DIR_HORIZONTAL)
                {
                        a_dest_num->val = a_src_num->val 
                                * PRIVATE (a_this)->xdpi ;
                }
                else if (a_dir == DIR_VERTICAL)
                {
                        a_dest_num->val = a_src_num->val 
                                * PRIVATE (a_this)->ydpi ;
                }
                else
                {
                        cr_utils_trace_info ("Bad direction given") ;
                        return CR_BAD_PARAM_ERROR ;
                }
                a_dest_num->type = NUM_LENGTH_PX ;
                break ;

        case NUM_LENGTH_CM:
                /*1 inch == 25.4 mm*/
                if (a_dir == DIR_HORIZONTAL)
                {
                        a_dest_num->val = a_src_num->val / 2.54 *
                                PRIVATE (a_this)->xdpi ;
                }
                else if (a_dir == DIR_VERTICAL)
                {
                        a_dest_num->val = a_src_num->val / 2.54 *
                                PRIVATE (a_this)->ydpi ;
                }
                else
                {
                        cr_utils_trace_info ("Bad direction given") ;
                        return CR_BAD_PARAM_ERROR ;
                }

                a_dest_num->type = NUM_LENGTH_PX ;
                break ;

        case NUM_LENGTH_MM:
                /*1 inch == 25.4 mm*/
                if (a_dir == DIR_HORIZONTAL)
                {
                        a_dest_num->val = a_src_num->val / 25.4 *
                                PRIVATE (a_this)->xdpi ;
                }
                else if (a_dir == DIR_VERTICAL)
                {
                        a_dest_num->val = a_src_num->val / 25.4 *
                                PRIVATE (a_this)->ydpi ;
                }
                else
                {
                        cr_utils_trace_info ("Bad direction given") ;
                        return CR_BAD_PARAM_ERROR ;
                }
                a_dest_num->type = NUM_LENGTH_PX ;
                break ;

        case NUM_LENGTH_PT:
                /*1 point == 1/72 inch*/
                if (a_dir == DIR_HORIZONTAL)
                {
                        a_dest_num->val = a_src_num->val *
                                PRIVATE (a_this)->xdpi / 72 ;
                }
                else if (a_dir == DIR_VERTICAL)
                {
                        a_dest_num->val = a_src_num->val *
                                PRIVATE (a_this)->ydpi / 72 ;
                }
                else
                {
                        cr_utils_trace_info ("Bad direction given") ;
                        return CR_BAD_PARAM_ERROR ;
                }

                a_dest_num->type = NUM_LENGTH_PX ;
                break ;

        case NUM_LENGTH_PC:
                /*1 pica == 12 points*/
                if (a_dir == DIR_HORIZONTAL)
                {
                        a_dest_num->val = a_src_num->val *
                                PRIVATE (a_this)->xdpi / 72 * 12 ;
                }
                else if (a_dir == DIR_VERTICAL)
                {
                        a_dest_num->val = a_src_num->val *
                                PRIVATE (a_this)->ydpi / 72 * 12 ;
                }
                else
                {
                        cr_utils_trace_info ("Bad direction given") ;
                        return CR_BAD_PARAM_ERROR ;
                }

                a_dest_num->type = NUM_LENGTH_PX ;
                break ;

        case NUM_ANGLE_DEG:
                a_dest_num->val = a_src_num->val ;
                
                break ;
        case NUM_ANGLE_RAD:
                a_dest_num->val = a_src_num->val * 180 / 3.1415 ;
                a_dest_num->type = NUM_ANGLE_DEG ;
                break ;

        case NUM_ANGLE_GRAD:
                a_dest_num->val = a_src_num->val * 90 / 100 ;
                a_dest_num->type = NUM_ANGLE_DEG ;
                break ;

        case NUM_TIME_MS:
                a_dest_num->val = a_src_num->val ;
                
                break ;

        case NUM_TIME_S:
                a_dest_num->val = a_src_num->val * 1000 ;
                a_dest_num->type = NUM_TIME_MS ;
                break ;

        case NUM_FREQ_HZ:
                a_dest_num->val = a_src_num->val ;
                break ;

        case NUM_FREQ_KHZ:
                a_dest_num->val = a_src_num->val * 1000 ;
                a_dest_num->type = NUM_FREQ_HZ ;
                break ;

        case NUM_PERCENTAGE:
                cr_utils_trace_info ("a PERCENTAGE cannot be normalized") ;
                return CR_BAD_PARAM_ERROR ;

        default:

                cr_num_copy (a_dest_num, a_src_num) ;
                break ;
        }

        return CR_OK ;
}

static enum CRStatus
style_specified_2_computed_values (CRLayEng *a_this,
                                   CRStyle *a_style, 
                                   CRBox *a_parent_box)
{
        glong i = 0 ;
        CRBoxEdge *parent_inner_edge = NULL;

        g_return_val_if_fail (a_style && a_this,
                              CR_BAD_PARAM_ERROR) ;

        /*
         *walk thru the numerical properties (num_props) and 
         *compute their computed value.
         */
        for (i = 0 ; i < NB_NUM_PROPS ; i++)
        {
                switch (i)
                {
                case NUM_PROP_TOP:
                case NUM_PROP_BOTTOM:
                case NUM_PROP_PADDING_TOP:
                case NUM_PROP_PADDING_BOTTOM:
                case NUM_PROP_BORDER_TOP:
                case NUM_PROP_BORDER_BOTTOM:
                case NUM_PROP_MARGIN_TOP:
                case NUM_PROP_MARGIN_BOTTOM:                
                        if (a_style->num_props[i].sv.type == NUM_PERCENTAGE)
                        {
                                /*
                                 *TODO: compute the computed value
                                 *using the parent box size.
                                 */
                                if (a_parent_box)
                                {
                                        parent_inner_edge = 
                                                &a_parent_box->inner_edge ;
                                }

                                g_return_val_if_fail (parent_inner_edge,
                                                      CR_BAD_PARAM_ERROR) ;

                                a_style->num_props[i].cv.val =
                                        parent_inner_edge->height * 
                                        a_style->num_props[i].sv.val / 100 ;
                        }
                        else
                        {
                                normalize_num (a_this,
                                               &a_style->num_props[i].cv,
                                               &a_style->num_props[i].sv,
                                               DIR_VERTICAL) ;
                        }

                        break ;

                case NUM_PROP_WIDTH:
                case NUM_PROP_RIGHT:
                case NUM_PROP_LEFT:
                case NUM_PROP_PADDING_LEFT:
                case NUM_PROP_PADDING_RIGHT:
                case NUM_PROP_BORDER_LEFT:
                case NUM_PROP_BORDER_RIGHT:
                case NUM_PROP_MARGIN_LEFT:
                case NUM_PROP_MARGIN_RIGHT:
                        if (a_style->num_props[i].sv.type == NUM_PERCENTAGE)
                        {
                                /*
                                 *TODO: compute the computed value
                                 *using the parent box size.
                                 */
                                if (a_parent_box)
                                {
                                        parent_inner_edge = 
                                                &a_parent_box->inner_edge ;
                                }

                                g_return_val_if_fail (parent_inner_edge,
                                                      CR_BAD_PARAM_ERROR) ;

                                a_style->num_props[i].cv.val =
                                        parent_inner_edge->width * 
                                        a_style->num_props[i].sv.val / 100 ;
                        }
                        else
                        {
                                normalize_num (a_this,
                                               &a_style->num_props[i].cv,
                                               &a_style->num_props[i].sv,
                                               DIR_HORIZONTAL) ;
                        }
                        break ;

                default:
                        normalize_num (a_this,
                                       &a_style->num_props[i].cv,
                                       &a_style->num_props[i].sv,
                                       DIR_UNKNOWN) ;
                        break ;
                }
        }

        for (i = 0 ; i < NB_RGB_PROPS; i++)
        {
                cr_rgb_set_from_rgb (&a_style->rgb_props[i].cv, 
                                     &a_style->rgb_props[i].sv) ;
        }

        /*************************************
         *Now compute the specific css2 specification recommendations.
         *This can seem ugly, but it needs to be done. I do it here untill
         *I find a better place for it.
         ***************************************/

        /*
         *css2 spec chap 8.5.3 says that if border-style-x is set to
         *'none', it forces border-x to have a width of zero.
         *For the time being, we consider the 'none' and 'hidden' values
         *being equal untill be implement the support of table.
         */
        if ((a_style->border_style_props[BORDER_STYLE_PROP_TOP] == 
             BORDER_STYLE_NONE)
            || (a_style->border_style_props[BORDER_STYLE_PROP_TOP] == 
                BORDER_STYLE_HIDDEN))
        {
                cr_num_set (&a_style->num_props[NUM_PROP_BORDER_TOP].cv,
                            0, NUM_LENGTH_PX) ;
        }

        if ((a_style->border_style_props[BORDER_STYLE_PROP_RIGHT] == 
            BORDER_STYLE_NONE)
            || (a_style->border_style_props[BORDER_STYLE_PROP_RIGHT] == 
                BORDER_STYLE_HIDDEN))
        {
                cr_num_set (&a_style->num_props[NUM_PROP_BORDER_RIGHT].cv, 
                            0, NUM_LENGTH_PX) ;
        }

        if ((a_style->border_style_props[BORDER_STYLE_PROP_BOTTOM] == 
            BORDER_STYLE_NONE)
            || (a_style->border_style_props[BORDER_STYLE_PROP_BOTTOM] == 
                BORDER_STYLE_HIDDEN))
        {
                cr_num_set (&a_style->num_props[NUM_PROP_BORDER_BOTTOM].cv,
                            0, NUM_LENGTH_PX) ;
        }

        if ((a_style->border_style_props[BORDER_STYLE_PROP_LEFT] == 
             BORDER_STYLE_NONE)
            || (a_style->border_style_props[BORDER_STYLE_PROP_LEFT] == 
                BORDER_STYLE_HIDDEN))
        {
                cr_num_set (&a_style->num_props[NUM_PROP_BORDER_LEFT].cv,
                            0, NUM_LENGTH_PX) ;
        }

        
        return CR_OK ;
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
create_box_tree_real (CRLayEng * a_this,
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
                                cr_sel_eng_get_matched_style
                                (PRIVATE (a_this)->sel_eng,
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
                         *be updated later via the cr_box_layout() method.
                         */
                        style_specified_2_computed_values 
                                (a_this, style, a_parent_box) ;

                        cur_box = cr_box_new (style, TRUE) ;
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
                        CRStyle * style_dup = NULL ;

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
                                style_dup = cr_style_dup (parent_style) ;
                                if (!style_dup)
                                {
                                        cr_utils_trace_info 
                                                ("cr_style_dup "
                                                 "Maybe out of memory") ;
                                        goto error ;
                                }

                                cur_box = cr_box_new (style_dup, TRUE) ;
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
                                 *by default, text/img boxes are inline.
                                 */
                                cur_box->type = BOX_TYPE_INLINE ;
                                cur_box->style->display = DISPLAY_INLINE ;

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

                                /*
                                 *the anonymous box
                                 *must have no margin,
                                 *no padding, no border,
                                 *no border style, no offset
                                 */
                                init_anonymous_text_box (cur_box) ;

                                style_specified_2_computed_values 
                                        (a_this, cur_box->style, a_parent_box) ;

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
                        create_box_tree_real
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
get_box_bottommost_y (CRBox *a_this)
{
        if (!a_this)
                return 0 ;

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
get_box_rightmost_x (CRBox *a_this)
{       
        if (!a_this)
                return 0 ;

        return (a_this->outer_edge.x 
                +
                a_this->outer_edge.x_offset
                +
                a_this->outer_edge.width) ;
}


/**
 *computes the inner edge size of a box which
 *contents text only.
 *This fonction uses pango to compute the size
 *of the box. Note that layout_text_in_box must have
 *been called prior to this function.
 *Note that this is highly experimental for the time being.
 *It more a design sketch than a real working code.
 *@param a_this in/out parameter the current box which inner edge is to
 *be computed.
 *@return TRUE if the inner edge has been computed, FALSE otherwise.
 */
static enum CRStatus
compute_text_box_inner_edge_size (CRBox *a_this)
{
        enum CRStatus status = CR_OK ;
        GtkWidget *label = NULL ;
        PangoLayout *pgo_layout = NULL ;
        PangoRectangle ink_rect = {0}, logical_rect = {0} ;

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

        g_return_val_if_fail (a_this->content->content_cache,
                              CR_BAD_PARAM_ERROR) ;

        label = a_this->content->content_cache ;
        gtk_misc_set_alignment (GTK_MISC (label),0, 0) ;
        gtk_misc_set_padding (GTK_MISC (label), 0, 0) ;

        pgo_layout = gtk_label_get_layout (GTK_LABEL (label)) ;

        pango_layout_get_pixel_extents (pgo_layout, &ink_rect,
                                        &logical_rect) ;

        a_this->inner_edge.width = logical_rect.width ;
        a_this->inner_edge.height = logical_rect.height ;
        
        return status ;
}

static enum CRStatus
layout_text_in_box (CRLayEng *a_this, CRBox *a_text_box)
{
        GtkWidget *label = NULL ;
        PangoLayout * pgo_layout = NULL ;
        PangoAttrList *pgo_attr_list = NULL ;
        PangoAttribute *pgo_attr = NULL ;
        PangoFontDescription *pgo_font_desc = NULL ;
        PangoRectangle ink_rect, logical_rect ;
        glong wrap_width = 0 ;

        g_return_val_if_fail (a_this && a_text_box
                              && a_text_box->content
                              && (a_text_box->content->type 
                                  == TEXT_CONTENT_TYPE)
                              && a_text_box->content->u.text,
                              CR_BAD_PARAM_ERROR) ;

        g_return_val_if_fail ((a_text_box->parent->inner_edge.max_width
                               + a_text_box->parent->inner_edge.x)
                              >= a_text_box->inner_edge.x,
                              CR_BAD_PARAM_ERROR) ;

        if (!a_text_box->content->content_cache)
        {
                a_text_box->content->content_cache = 
                        gtk_label_new (NULL) ;
                g_return_val_if_fail (a_text_box->content->content_cache, 
                                      CR_ERROR) ;
        }

        label = a_text_box->content->content_cache ;
        g_return_val_if_fail (GTK_IS_LABEL (label), CR_ERROR) ;

        gtk_label_set_text (GTK_LABEL (label), 
                            a_text_box->content->u.text) ;
        gtk_label_set_use_markup (GTK_LABEL (label),
				  FALSE) ;
	gtk_label_set_use_underline (GTK_LABEL (label),
				     FALSE) ;
        pgo_layout = gtk_label_get_layout (GTK_LABEL (label)) ;

        /*
         *set the wrap width if necessary.
         */
        if (cr_num_is_fixed_length 
            (&a_text_box->style->num_props[NUM_PROP_WIDTH].cv)
            == TRUE)
        {
                wrap_width = 
                        a_text_box->style->num_props[NUM_PROP_WIDTH].cv.val ;

                gtk_widget_set_size_request (label, wrap_width,
                                     -1) ;
        }
       
/*        else 
        {                
                wrap_width = a_text_box->parent->inner_edge.max_width 
                        + a_text_box->parent->inner_edge.x -
                        a_text_box->inner_edge.x ;
        }
*/
        /*
         *TODO: set the font description attributes.
         */                

        return CR_OK ;
}

/**
 *Computes the size and positions of border edge, pading edge,
 *and inner edge. Also compute the size of the outer edge (aka margin edge).
 *All these calculations are done relatively to the position of the outer edge.
 *Which means that the position (x,y) of the outer edge *must* be set prior
 *to calling this function.
 *Note that this function calls layout_box() to compute the size of
 *the inner edge if it contains non terminal boxes.
 *@param a_this the layout engine.
 *@param a_cur_box the box.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
compute_box_size (CRLayEng *a_this,
                  CRBox *a_cur_box)
{
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_cur_box && a_cur_box->style,
                              CR_BAD_PARAM_ERROR) ;

        /*******************************************
         *1/set the left border and 
         *left padding edges.
         *2/compute the left most x and topmost y of
         *the inner box.
         *3/Compute the outer edge of the contained
         *box; this is recursive.
         *******************************************/

        /*
         *step 1/
         */

        /*
         *TODO: collapse this margin !!!.
         *See css2 chap 8.3.1 to see what "collapsing" means.
         */
        a_cur_box->border_edge.x =
                a_cur_box->outer_edge.x
                + 
                a_cur_box->style->num_props[NUM_PROP_MARGIN_LEFT].cv.val ;
        a_cur_box->border_edge.y =
                a_cur_box->outer_edge.y
                +
                a_cur_box->style->num_props[NUM_PROP_MARGIN_TOP].cv.val ;

        a_cur_box->padding_edge.x =
                a_cur_box->border_edge.x 
                +
                a_cur_box->style->num_props[NUM_PROP_BORDER_LEFT].cv.val ;

        a_cur_box->padding_edge.y =
                a_cur_box->border_edge.y
                +
                a_cur_box->style->num_props[NUM_PROP_BORDER_TOP].cv.val ;

        /*
         *Step 2/
         */
        a_cur_box->inner_edge.x =
                a_cur_box->padding_edge.x
                +
                a_cur_box->style->num_props[NUM_PROP_PADDING_LEFT].cv.val ;
        a_cur_box->inner_edge.y =
                a_cur_box->padding_edge.y
                +
                a_cur_box->style->num_props[NUM_PROP_PADDING_LEFT].cv.val ;

        a_cur_box->inner_edge.max_width =
                a_cur_box->outer_edge.max_width +
                a_cur_box->outer_edge.x -
                a_cur_box->inner_edge.x ;

        /*
         *Step 3.
         */
        if (cr_num_is_fixed_length 
            (&a_cur_box->style->num_props[NUM_PROP_WIDTH].cv)
            == TRUE)
        {
                a_cur_box->inner_edge.width = 
                        a_cur_box->style->num_props[NUM_PROP_WIDTH].cv.val ;
                a_cur_box->inner_edge.max_width = a_cur_box->inner_edge.width ;
        }

        if (a_cur_box->children)
        {
                /*
                 *layout the children boxes. This function call will
                 *also update the current inner_edge size.
                 */
                status = layout_box (a_this, a_cur_box->children) ;
                g_return_val_if_fail (status == CR_OK, status) ;

        }
        else
        {
                /*
                 *this box may have a content.
                 *TODO: compute it's width and height.
                 *then, when computed, update the
                 *children max width size in the parent box.
                 */
                if (a_cur_box->content)
                {
                        switch (a_cur_box->content->type)
                        {
                        case TEXT_CONTENT_TYPE:
                                layout_text_in_box (a_this, a_cur_box) ;
                                compute_text_box_inner_edge_size
                                        (a_cur_box) ;
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
        
        
        /*******************************************
         *Inner edge position (x,y) and size computing is 
         *finished.
         *Now, we can compute the widths of the
         *remaining three other boxes 
         *(padding edge, border edge and outer edge)
         ******************************************/
        a_cur_box->padding_edge.width = a_cur_box->inner_edge.width +
                a_cur_box->style->num_props[NUM_PROP_PADDING_RIGHT].cv.val +
                a_cur_box->style->num_props[NUM_PROP_PADDING_LEFT].cv.val ;
        a_cur_box->padding_edge.height = a_cur_box->inner_edge.height +
                a_cur_box->style->num_props[NUM_PROP_PADDING_TOP].cv.val +
                a_cur_box->style->num_props[NUM_PROP_PADDING_BOTTOM].cv.val ;

        a_cur_box->border_edge.width = a_cur_box->padding_edge.width +
                a_cur_box->style->num_props[NUM_PROP_BORDER_RIGHT].cv.val +
                a_cur_box->style->num_props[NUM_PROP_BORDER_LEFT].cv.val ;
        a_cur_box->border_edge.height = a_cur_box->padding_edge.height +
                a_cur_box->style->num_props[NUM_PROP_BORDER_TOP].cv.val +
                a_cur_box->style->num_props[NUM_PROP_BORDER_BOTTOM].cv.val ;

        a_cur_box->outer_edge.width = a_cur_box->border_edge.width +
                a_cur_box->style->num_props[NUM_PROP_MARGIN_LEFT].cv.val +
                a_cur_box->style->num_props[NUM_PROP_MARGIN_RIGHT].cv.val ;
        a_cur_box->outer_edge.height = a_cur_box->border_edge.height +
                a_cur_box->style->num_props[NUM_PROP_MARGIN_TOP].cv.val +
                a_cur_box->style->num_props[NUM_PROP_MARGIN_BOTTOM].cv.val ;

        return CR_OK ;
}

/**
 *Adjusts the size of the inner edge of this box's parent.
 *That is, increases (if needed) the parent inner edge's width/height.
 *@param a_this the current instance of #CRBox.
 */
static enum CRStatus
adjust_parent_inner_edge_size (CRLayEng *a_this,
                               CRBox *a_cur_box)
{        
        g_return_val_if_fail (a_cur_box 
                              && a_this
                              && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        if (PRIVATE (a_this)->update_parent_box_size == TRUE
            && a_cur_box->parent)
        {
                gulong parent_inner_edge_right_bound = 
                        a_cur_box->parent->inner_edge.x +
                        a_cur_box->parent->inner_edge.width ;
                gulong outer_edge_right_bound = 
                        a_cur_box->outer_edge.x + 
                        a_cur_box->outer_edge.width ;
   
                if (parent_inner_edge_right_bound
                    <
                    outer_edge_right_bound)
                {
                        /*
                         *parent inner edge is too short to
                         *contain this box outer edge.
                         *So, we just enlarge it.
                         */
                        a_cur_box->parent->inner_edge.width = 
                                outer_edge_right_bound - 
                                a_cur_box->parent->inner_edge.x ;
                }
        }

        /*
         *Make sure the parent inner_edge.heigth is big enough
         *to contain the current box.
         */
        if (a_cur_box->parent)
        {
                gulong parent_inner_edge_bottom_bound =
                        a_cur_box->parent->inner_edge.y +
                        a_cur_box->parent->inner_edge.height ;
                gulong outer_edge_bottom_bound =
                        a_cur_box->outer_edge.y +
                        a_cur_box->outer_edge.height ;

                if (parent_inner_edge_bottom_bound <
                    outer_edge_bottom_bound)
                {
                        a_cur_box->parent->inner_edge.height =
                                outer_edge_bottom_bound -
                                a_cur_box->parent->inner_edge.y ;
                }
        }

        return CR_OK ;
}

/**
 *Layout a box in block formating context.
 *See css2 spec in chapters 9.2.
 *@param a_this the current instance of CRLayEng.
 *@param a_cur_box the current box to layout.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
layout_block_box (CRLayEng *a_this,
                  CRBox *a_cur_box)
{
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_cur_box && a_cur_box->style,
                              CR_BAD_PARAM_ERROR) ;
        
        CRBox *cont_box = a_cur_box->parent ;

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
        {
                a_cur_box->outer_edge.x = 0 ;
                a_cur_box->outer_edge.y = 0 ;
                a_cur_box->inner_edge.width = 800 ;
                a_cur_box->inner_edge.height = 600 ;
                a_cur_box->inner_edge.max_width = 800 ;
        }
        else
        {
                a_cur_box->outer_edge.x = 
                        cont_box->inner_edge.x ;
                if (a_cur_box->prev)
                {
                        a_cur_box->outer_edge.y =
                                get_box_bottommost_y (a_cur_box->prev) ;
                }
                else
                {
                        a_cur_box->outer_edge.y = 
                                cont_box->inner_edge.y ;
                }
        }

        g_return_val_if_fail (a_cur_box->parent->inner_edge.max_width 
                              + a_cur_box->parent->inner_edge.x
                              > a_cur_box->outer_edge.x,
                              CR_ERROR) ;

        a_cur_box->outer_edge.max_width = 
                a_cur_box->parent->inner_edge.max_width 
                + a_cur_box->parent->inner_edge.x - 
                a_cur_box->outer_edge.x ;

        status = compute_box_size (a_this,
                                   a_cur_box) ;
        return status ;
}


/**
 *Layout a box in an inline formating context.
 *See css2 spec in chapters 9.2.
 *@param a_this the layout engine.
 *@param a_cur_box the current box to layout.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
layout_inline_box (CRLayEng *a_this,
                   CRBox *a_cur_box)
{
        CRBox *cont_box = NULL, *prev_box = NULL ;
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_cur_box && a_cur_box->style,
                              CR_BAD_PARAM_ERROR) ;

        cont_box = a_cur_box->parent ;
        prev_box = a_cur_box->prev ;

        /************************************
         *We are in an inline formating context 
         ************************************/

        /********************************************
         *position the 'x' of the top
         *leftmost corner of this box
         *one pixel right after the rightmost x
         *of the preceding box.
         *Position the 'y' of this box to
         *the y of the previous box.
         ********************************************/

        if (!prev_box)
        {
                /*
                 *this box is the leftmost box contained in its containing
                 *box.
                 */
                if (cont_box)
                {
                        a_cur_box->outer_edge.x = cont_box->inner_edge.x ;
                        a_cur_box->outer_edge.y = cont_box->inner_edge.y ;
                }
                else
                {
                        /*this box does not have any containing box*/
                        a_cur_box->outer_edge.x = 0 ;
                        a_cur_box->outer_edge.y = 0 ;
                }
        }
        else
        {
                a_cur_box->outer_edge.x = 
                        get_box_rightmost_x (prev_box) + 1 ;
                a_cur_box->outer_edge.y = prev_box->outer_edge.y ;
        }

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

        a_cur_box->outer_edge.max_width = 
                a_cur_box->parent->inner_edge.max_width 
                + a_cur_box->parent->inner_edge.x - 
                a_cur_box->outer_edge.x ;

        status = compute_box_size (a_this,
                                   a_cur_box) ;

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
 *@param a_this the layout engine.
 *@param a_cur_box the current box.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
layout_box_in_normal_flow (CRLayEng *a_this,
                           CRBox *a_cur_box)
{
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_cur_box && a_cur_box->style,
                              CR_BAD_PARAM_ERROR) ;

        /*
         *Only boxes that have
         *the position rule set to 'static' or 'relative'
         *can be part of a normal formatting context.
         */
        if (a_cur_box->style->position != POSITION_STATIC
            && a_cur_box->style->position != POSITION_RELATIVE)
        {
                return CR_UNEXPECTED_POSITION_SCHEME ;
        }

        /*
         *TODO
         *compute the "computed values" of the style data structure.
         */
        switch (a_cur_box->type)
        {
        case BOX_TYPE_BLOCK:
        case BOX_TYPE_ANONYMOUS_BLOCK:
                layout_block_box (a_this, a_cur_box) ;
                break ;

        case BOX_TYPE_COMPACT:
        case BOX_TYPE_RUN_IN:
        case BOX_TYPE_INLINE:
        case BOX_TYPE_ANONYMOUS_INLINE:
                layout_inline_box (a_this, a_cur_box) ;
                break ;

        default:
                break ;
        }


        return status ;
}


/**
 *Layout a box.
 *This function determine the  flow scheme (e.g: normal flow etc ...)
 *and call the right specialized function that knows how to perform
 *the layout according to that flow scheme.
 *Note that a flow scheme is local to a box. A child box can have a 
 *different flow scheme for example. So the lower level function called
 *by layout_box() can also call layout_box() to perform the layout of their
 *children boxes.
 *@param a_this the layout engine.
 *@param a_cur_box the current box.
 */
static enum CRStatus
layout_box (CRLayEng *a_this,
            CRBox *a_cur_box)
{
        CRBox *cur_box = NULL ;

        g_return_val_if_fail (a_cur_box && a_cur_box->style,
                              CR_BAD_PARAM_ERROR) ;

        PRIVATE (a_this)->update_parent_box_size = TRUE ;

        for (cur_box = a_cur_box ; cur_box ; 
             cur_box = cur_box->next)
        {
                switch (cur_box->style->position)
                {
                case POSITION_STATIC:
                case POSITION_RELATIVE:
                        layout_box_in_normal_flow 
                                (a_this, cur_box) ;
                        break ;

                case POSITION_ABSOLUTE:
                case POSITION_FIXED:
                        /*cr_box_layout_absolute (a_cur_box) ;*/
                        break ;

                case POSITION_INHERIT:
                        break ;
                }
                /*
                 *make sure the parent inner_edge is big enough to contain
                 *the current box.
                 */
                adjust_parent_inner_edge_size (a_this,
                                               cur_box) ;
        }
        
        return CR_OK ;
}

/**********************
 *Public methods.
 **********************/

/**
 *The first function to call prior to any other
 *method of the layout engine.
 *@param a_argc the argc parameter passed to the standard C main entry point.
 *@param a_argv the argv parameter passed to the standard C main entry point.
 */
void
cr_lay_eng_init (glong a_argc, gchar ** a_argv)
{        

        if (gv_layeng_initialized == FALSE)
        {
                gtk_init ((int*)&a_argc, &a_argv) ;
                gv_layeng_initialized = TRUE ;
        }
}


/**
 *Instanciates a new Layout Engine.
 *return the new instance of #CRLayEng or NULL if
 *an error occured.
 */
CRLayEng *
cr_lay_eng_new (void)
{
	CRLayEng *result = NULL ;

        if (gv_layeng_initialized == FALSE)
        {
                cr_utils_trace_info ("Layout Engine must be initialized "
                                     "by calling cr_lay_eng_init() first") ;
                return NULL ;
        }

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

        

        PRIVATE (result)->xdpi = gdk_screen_width () / 
                gdk_screen_width_mm () * 25.4 ;
        PRIVATE (result)->ydpi = gdk_screen_height () /
                gdk_screen_height_mm () * 25.4 ;

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
cr_lay_eng_create_box_model (CRLayEng *a_this,
                             xmlDoc *a_doc,
                             CRCascade *a_cascade,
                             CRBoxModel **a_box_model)
{
        xmlNode *root_node = NULL ;
        CRBox *box_tree = NULL;

        g_return_val_if_fail (a_this && a_doc && a_cascade,
                              CR_BAD_PARAM_ERROR) ;

        root_node = xmlDocGetRootElement (a_doc) ;

        if (!root_node)
                return CR_NO_ROOT_NODE_ERROR ;

        PRIVATE (a_this)->cascade = a_cascade ;

        if (!*a_box_model)
        {
                *a_box_model = cr_box_model_new () ;
        }

        box_tree = 
                create_box_tree_real (a_this, root_node,
                                      (CRBox*)*a_box_model) ;

        if (box_tree)
                return CR_OK ;
        else
                return CR_ERROR ;
}



/**
 *Recursively computes the sizes and positions of each
 *box in the box tree.
 *@param a_this
 *@param a_box_tree
 *@return
 */
enum CRStatus
cr_lay_eng_layout_box_tree (CRLayEng *a_this,
                            CRBox *a_box_tree)
{
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && a_box_tree,
                              CR_BAD_PARAM_ERROR) ;

        status =  layout_box (a_this, a_box_tree) ;

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
