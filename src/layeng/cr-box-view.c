/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of 
 * the GNU Lesser General Public
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

#include "cr-box-view.h"
#include "cr-lay-eng.h"
#include "cr-om-parser.h"
#include <libxml/tree.h>

#define PRIVATE(a_this) ((a_this)->priv)

struct _CRBoxViewPriv
{
	CRBoxModel *box_model ;
        /*
         *The default graphical context
         *Function willing to modify this gc
         *should save it firts, then modify it,
         *draw what they have to draw and then restore it !!
         */
        GdkGC *gc ;

        CRLayEng *layeng ;

        /**
         * a boolean used by some drawing functions.
         *greping PRIVATE (a_this)->draw should tell you who uses it :)
         */
        gboolean draw ;
} ;


static GtkLayoutClass *gv_parent_class = NULL ;


static void
cr_box_view_class_init (CRBoxViewClass *a_klass) ;


static void
cr_box_view_init (CRBoxView *a_this) ;


static gboolean
expose_event_cb (GtkWidget *a_this,
                 GdkEventExpose *a_event,
                 gpointer a_user_data) ;

static enum CRStatus
draw_box (CRBoxView *a_this,
          CRBox *a_box,
          GdkRectangle *a_region_to_draw) ;

static enum CRStatus
draw_borders (CRBoxView *a_bv,
              CRBox *a_box) ;

static enum CRStatus
draw_paddings (CRBoxView *a_bv,
               CRBox *a_box) ;

static enum CRStatus
draw_inner_box (CRBoxView *a_bv,
                CRBox *a_box) ;

static enum CRStatus
set_border_line_attrs (CRBoxView *a_this,
                       CRBox *a_box,
                       enum CRBorderStyleProp a_style_prop) ;

static enum CRStatus
set_color (CRBoxView *a_this, CRRgb *a_rgb_color,
           gboolean a_foreground) ;

static void
cr_box_view_class_init (CRBoxViewClass *a_klass)
{
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (a_klass) ;

	gv_parent_class = 
		g_type_class_peek_parent (a_klass) ;
        
        g_return_if_fail (gv_parent_class) ;
        g_return_if_fail (gtk_object_class) ;
	gtk_object_class->destroy = cr_box_view_destroy ;
}

static void
cr_box_view_init (CRBoxView *a_this)
{
	g_return_if_fail (a_this) ;

	PRIVATE (a_this) = g_try_malloc (sizeof (CRBoxView)) ; 
	if (!PRIVATE (a_this))
	{
		cr_utils_trace_info ("Out of memory") ;
		return ;
	}
	memset (PRIVATE (a_this), 0, sizeof (CRBoxView)) ;
}


static gboolean
expose_event_cb (GtkWidget *a_this,
                 GdkEventExpose *a_event,
                 gpointer a_user_data)
{
        g_return_val_if_fail (a_event
                              && a_this 
                              && GTK_IS_LAYOUT (a_this)
                              && CR_IS_BOX_VIEW (a_this),
                              CR_BAD_PARAM_ERROR) ;

        switch (a_event->type)
        {
        case GDK_EXPOSE:
                g_return_val_if_fail 
                        (PRIVATE (CR_BOX_VIEW (a_this))->box_model
                         && ((CRBox*)PRIVATE (CR_BOX_VIEW (a_this))->
                         box_model)->children,
                         FALSE) ;

                cr_box_view_layout (CR_BOX_VIEW (a_this)) ;

                draw_box (CR_BOX_VIEW (a_this),
                          ((CRBox*)PRIVATE (CR_BOX_VIEW (a_this))->box_model)->
                          children,
                          &a_event->area) ;
                break ;

        default:
                cr_utils_trace_info ("Unexpected event received, "
                                     "Only GDK_EXPOSE was expected.") ;
                return FALSE ;
                break ;
        }

        return FALSE ;
        
}

static enum CRStatus
set_border_line_attrs (CRBoxView *a_this,
                       CRBox *a_box,
                       enum CRBorderStyleProp a_style_prop)
{
        enum CRNumProp border_width_dir ;
        GdkGCValues gc_values  ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_box
                              && a_style_prop < NB_BORDER_STYLE_PROPS,
                              CR_BAD_PARAM_ERROR) ;

        memset (&gc_values, 0, sizeof (GdkGCValues)) ;

        gdk_gc_get_values (PRIVATE (a_this)->gc, &gc_values) ;        

        switch (a_style_prop)
        {
        case BORDER_STYLE_PROP_TOP:
                border_width_dir = NUM_PROP_BORDER_TOP ;
                set_color 
                        (a_this, 
                         &a_box->style->rgb_props[RGB_PROP_BORDER_TOP_COLOR].cv,
                           TRUE /*foreground*/) ;
                break ;

        case BORDER_STYLE_PROP_RIGHT:
                border_width_dir = NUM_PROP_BORDER_RIGHT ;
                set_color (a_this, 
                           &a_box->style->
                           rgb_props[RGB_PROP_BORDER_RIGHT_COLOR].cv,
                           TRUE /*foreground*/) ;
                break ;

        case BORDER_STYLE_PROP_BOTTOM:
                border_width_dir = NUM_PROP_BORDER_BOTTOM ;
                set_color (a_this, 
                           &a_box->style->
                           rgb_props[RGB_PROP_BORDER_BOTTOM_COLOR].cv,
                           TRUE /*foreground*/) ;
                break ;

        case BORDER_STYLE_PROP_LEFT:
                border_width_dir = NUM_PROP_BORDER_LEFT ;
                set_color (a_this, 
                           &a_box->style->
                           rgb_props[RGB_PROP_BORDER_LEFT_COLOR].cv,
                           TRUE /*foreground*/) ;
                break ;

        default:
                cr_utils_trace_info ("Bad value of enum CRBorderStyleProp " 
                                     "given in parameter") ;
                return CR_BAD_PARAM_ERROR ;

        }

        switch (a_box->style->border_style_props[a_style_prop])
        {
        case BORDER_STYLE_NONE :
        case BORDER_STYLE_HIDDEN:
                PRIVATE (a_this)->draw = FALSE ;
                break ;
                        
        case BORDER_STYLE_DOTTED:
                gdk_gc_set_line_attributes 
                        (PRIVATE (a_this)->gc,
                         a_box->style->num_props[border_width_dir].cv.val,
                         GDK_LINE_ON_OFF_DASH,
                         gc_values.cap_style,
                         gc_values.join_style) ;
                PRIVATE (a_this)->draw = TRUE ;
                break ;

        case BORDER_STYLE_DASHED:
                gdk_gc_set_line_attributes 
                        (PRIVATE (a_this)->gc,
                         a_box->style->num_props[border_width_dir].cv.val,
                         GDK_LINE_ON_OFF_DASH,
                         gc_values.cap_style,
                         gc_values.join_style) ;
                PRIVATE (a_this)->draw = TRUE ;
                break ;

        case BORDER_STYLE_SOLID:
        case BORDER_STYLE_DOUBLE:
        case BORDER_STYLE_GROOVE:
        case BORDER_STYLE_RIDGE:
        case BORDER_STYLE_INSET:
        case BORDER_STYLE_OUTSET:
                gdk_gc_set_line_attributes 
                        (PRIVATE (a_this)->gc,
                         a_box->style->num_props[border_width_dir].cv.val,
                         GDK_LINE_SOLID,
                         gc_values.cap_style,
                         gc_values.join_style) ;
                PRIVATE (a_this)->draw = TRUE ;
                break ;
        }

        return CR_OK ;
}

static enum CRStatus
set_color (CRBoxView *a_this, CRRgb *a_rgb_color,
           gboolean a_foreground)
{
        GdkColor gdk_color = {0} ;
        g_return_val_if_fail (a_this && a_rgb_color, 
                              CR_BAD_PARAM_ERROR) ;

        gdk_color.red = (a_rgb_color->red << 8)     | a_rgb_color->red ;
        gdk_color.green = (a_rgb_color->green << 8) | a_rgb_color->green;
        gdk_color.blue = (a_rgb_color->blue << 8)   | a_rgb_color->blue ;

        gdk_rgb_find_color
                (gdk_drawable_get_colormap
                 (GDK_DRAWABLE (GTK_LAYOUT (a_this)->bin_window)),
                 &gdk_color) ;

        if (a_foreground == FALSE)
        {
                gdk_gc_set_background (PRIVATE (a_this)->gc, 
                                       &gdk_color) ;
        }
        else
        {
                gdk_gc_set_foreground (PRIVATE (a_this)->gc, 
                                       &gdk_color) ;
        }


        return CR_OK ;
}

static enum CRStatus
draw_borders (CRBoxView *a_this,
              CRBox *a_box)
{
        GdkWindow * window = NULL ;
        GtkWidget *widget = NULL ;
        CRBox *box = NULL ;

        gulong x0=0, y0=0, x1=0, y1=0 ;
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this
                              && CR_IS_BOX_VIEW (a_this)
                              && a_box,
                              CR_BAD_PARAM_ERROR) ;

        widget = GTK_WIDGET (a_this) ;
        window = GTK_LAYOUT (a_this)->bin_window ;
        g_return_val_if_fail (window, CR_ERROR) ;

        box = a_box ;
        g_return_val_if_fail (box, CR_ERROR) ;

        /*
         *Draw left border.
         */
        x0 = box->border_edge.x + (box->padding_edge.x - box->border_edge.x)/2 ;
        /*x0 = box->border_edge.x ;*/
        y0 = box->border_edge.y ;
        x1 = x0;
        y1 = y0 + box->border_edge.height ;
        status = set_border_line_attrs (a_this, a_box,
                                        BORDER_STYLE_PROP_LEFT) ;
        g_return_val_if_fail (status == CR_OK, status) ;

        if (PRIVATE (a_this)->draw == TRUE)
                gdk_draw_line
                        (window,
                         PRIVATE (a_this)->gc,
                         x0, y0, x1, y1);

        /*
         *draw right border
         */
        x0 = box->padding_edge.x + box->padding_edge.width +
                (box->border_edge.x + box->border_edge.width 
                 - box->padding_edge.x - box->padding_edge.width)/2 ;
        y0 = box->border_edge.y ;
        x1 = x0 ;
        /*y1 remains the same as y0*/
        status = set_border_line_attrs (a_this, a_box,
                                        BORDER_STYLE_PROP_RIGHT) ;
        g_return_val_if_fail (status == CR_OK, status) ;

        if (PRIVATE (a_this)->draw == TRUE)
                gdk_draw_line (window, PRIVATE (a_this)->gc,
                               x0, y0, x1, y1) ;

        /*
         *draw top border.
         */
        x0 = box->border_edge.x ;
        y0 = box->border_edge.y + (box->padding_edge.y - box->border_edge.y)/2 ;
        /*y0 = box->border_edge.y ;*/
        x1 = x0 + box->border_edge.width ;
        y1 = y0 ;
        status = set_border_line_attrs (a_this, a_box,
                                              BORDER_STYLE_PROP_TOP) ;
        g_return_val_if_fail (status == CR_OK, status) ;

        if (PRIVATE (a_this)->draw == TRUE)
                gdk_draw_line (window, PRIVATE (a_this)->gc,
                               x0, y0, x1, y1) ;

        /*
         *draw bottom border
         */
        /*x0 remains the same as previous x0 ;*/
        
        y0 = box->padding_edge.y + box->padding_edge.height +
                (box->border_edge.y + box->border_edge.height 
                 - box->padding_edge.y - box->padding_edge.height)/2 ;

        /*y0 = box->padding_edge.y + box->padding_edge.height ;*/
        x1 = x0 + box->border_edge.width ;
        y1  = y0 ;
        status = set_border_line_attrs (a_this, a_box,
                                              BORDER_STYLE_PROP_BOTTOM) ;
        g_return_val_if_fail (status == CR_OK, status) ;

        if (PRIVATE (a_this)->draw == TRUE)
                gdk_draw_line (window, PRIVATE (a_this)->gc,
                               x0, y0, x1, y1) ;

        PRIVATE (a_this)->draw = TRUE ;

        return CR_OK ;
}


static enum CRStatus
draw_paddings (CRBoxView *a_this,
               CRBox *a_box)
{
        GdkWindow * window = NULL ;
        GtkWidget *widget = NULL ;
        CRBox *box = NULL ;

        g_return_val_if_fail (a_this
                              && CR_IS_BOX_VIEW (a_this)
                              && a_box,
                              CR_BAD_PARAM_ERROR) ;

        widget = GTK_WIDGET (a_this) ;
        window = GTK_LAYOUT (a_this)->bin_window ;
        g_return_val_if_fail (window, CR_ERROR) ;

        box = a_box ;
        g_return_val_if_fail (box, CR_ERROR) ;

        set_color (a_this, 
                   &a_box->style->rgb_props[RGB_PROP_BACKGROUND_COLOR].cv,
                   TRUE/*foreground*/) ;

        gdk_draw_rectangle 
                (window,
                 PRIVATE (a_this)->gc,
                 TRUE,
                 box->padding_edge.x, box->padding_edge.y,
                 box->padding_edge.width, box->padding_edge.height) ;

        return CR_OK ;
}


static enum CRStatus
draw_inner_box (CRBoxView *a_this,
                CRBox *a_box)
{
        GtkWidget *widget = NULL, *label = NULL ;
        CRBox *box = NULL ;
   
        g_return_val_if_fail (a_this
                              && CR_IS_BOX_VIEW (a_this)
                              && a_box,
                              CR_BAD_PARAM_ERROR) ;

        widget = GTK_WIDGET (a_this) ;        
        g_return_val_if_fail (widget, CR_ERROR) ;

        box = a_box ;
        g_return_val_if_fail (box, CR_ERROR) ;

        if (!box->content)
                return CR_OK ;

        if (box->content->content_cache)
        {
                label = GTK_WIDGET (box->content->content_cache) ;
                g_return_val_if_fail (label, CR_ERROR) ;
        }

        g_return_val_if_fail (label, CR_ERROR) ;

        if (label->parent == NULL)                
                gtk_layout_put (GTK_LAYOUT (a_this), label,
                                box->inner_edge.x, 
                                box->inner_edge.y) ;
        else
                gtk_layout_move (GTK_LAYOUT (a_this), label,
                                 box->inner_edge.x, 
                                 box->inner_edge.y) ;

        gtk_widget_show_all (widget) ;

        return CR_OK ;
}


static enum CRStatus
draw_box (CRBoxView *a_this,
          CRBox *a_box,
          GdkRectangle *a_region_to_draw)
{
        CRBox *cur_box = NULL ;
        GtkWidget *widget = NULL;

        g_return_val_if_fail (a_this
                              && CR_IS_BOX_VIEW (a_this)
                              && a_box,                              
                              CR_BAD_PARAM_ERROR) ;

        widget = GTK_WIDGET (a_this) ;
        g_return_val_if_fail (widget, CR_ERROR) ;

        if (!PRIVATE (a_this)->gc)
        {
                PRIVATE (a_this)->gc = gdk_gc_new 
                        (GDK_DRAWABLE (GTK_LAYOUT (a_this)->bin_window)) ;
                g_return_val_if_fail (PRIVATE (a_this)->gc,
                                      CR_ERROR) ;

                gdk_gc_copy (PRIVATE (a_this)->gc,
                             GTK_WIDGET (a_this)->style->base_gc[widget->state]);
        }

        for (cur_box = a_box; cur_box ; cur_box = cur_box->next)
        {
                /*draw_margins (a_this, cur_box) ;*/
                draw_paddings (a_this, cur_box) ;
                draw_inner_box (a_this, cur_box) ;
                draw_borders (a_this, cur_box) ;                                

                if (cur_box->children)
                {
                        draw_box (a_this,
                                  cur_box->children,
                                  a_region_to_draw) ;
                }
        }

        return CR_OK ;
}

/**********************************
 *Public funtions
 **********************************/

GType
cr_box_view_get_type (void)
{
	static GType type = 0 ;

	if (type == 0)
	{
		static const GTypeInfo type_info = 
			{
				sizeof (CRBoxViewClass),
				NULL,NULL,
				(GClassInitFunc)cr_box_view_class_init,
				NULL, NULL,
				sizeof (CRBoxView),
				0,
				(GInstanceInitFunc)cr_box_view_init
			} ;

		type = g_type_register_static (GTK_TYPE_LAYOUT,
					     "CRBoxView", &type_info, 0) ;
	}

	return type ;
}


CRBoxView *
cr_box_view_new_from_xml_css_bufs (const guchar *a_xml_buf,
                                   const guchar *a_css_buf)
{
        enum CRStatus status = CR_OK ;
        CRStyleSheet *sheet = NULL;
        xmlDoc * xml_doc = NULL;
        CRBoxView *result = NULL ;
        gulong len = 0 ;
        CRCascade *cascade = NULL ;
        CRBoxModel *box_model = NULL ;
        
        result = g_object_new (CR_TYPE_BOX_VIEW, NULL) ;
        g_return_val_if_fail (result, NULL) ;

        len = strlen (a_css_buf) ;
        status = cr_om_parser_simply_parse_buf (a_css_buf, len, CR_UTF_8,
                                                &sheet) ;
        
        if (status != CR_OK || !sheet)
	{
		cr_utils_trace_info ("Could not parse css buf") ;
		status = CR_ERROR ;
		goto cleanup ;
	}
                
        len = strlen (a_xml_buf) ;
	xml_doc = xmlParseMemory (a_xml_buf, len) ;
	if (!xml_doc)
	{
		cr_utils_trace_info ("Could not parse xml buf") ;
		status = CR_ERROR ;
		goto cleanup ;
	}

        PRIVATE (result)->layeng = cr_lay_eng_new (GTK_LAYOUT (result)) ;
        if (!PRIVATE (result)->layeng)
        {       
                cr_utils_trace_info ("Could not instanciate the layout engine. "
                                     "The system may be out of memory") ;
                cr_box_view_destroy (GTK_OBJECT (result)) ;
                return NULL ;
        }

        cascade = cr_cascade_new (sheet, NULL, NULL) ;
        
        if (!cascade)
	{
		cr_utils_trace_info ("could not create the cascade") ;
		cr_utils_trace_info 
			("The system is possibly out of memory") ;
		goto cleanup ;
	}
        sheet = NULL ;

        status = cr_lay_eng_create_box_model (PRIVATE (result)->layeng,
                                              xml_doc, cascade,
                                              &box_model) ;
	if (status != CR_OK)
	{
		cr_utils_trace_info ("could not build the annotated doc") ;
		goto cleanup ;
	}
        
        if (box_model)
        {
                box_model->box.inner_edge.width = 800 ;
                box_model->box.inner_edge.max_width = 800 ;
                box_model->box.inner_edge.width = 600 ;

                cr_box_view_set_box_model (result, box_model) ;
                gtk_layout_set_size (GTK_LAYOUT (result), 1024, 768) ;
                g_signal_connect (G_OBJECT (result),
                                  "expose-event",
                                  (GCallback)expose_event_cb,
                                  NULL) ;

                return result ;
        }

 cleanup:
        if (sheet)
        {
                cr_stylesheet_destroy (sheet) ;
                sheet = NULL ;
        }
        
        if (xml_doc)
        {
                xmlFreeDoc (xml_doc) ;
                xml_doc = NULL ;
        }

        if (cascade)
        {
                cr_cascade_destroy (cascade) ;
                cascade = NULL ;
        }

        if (box_model)
        {
                cr_box_destroy ((CRBox*)box_model) ;
                box_model = NULL ;
        }
        
        if (result)
        {
                gtk_object_destroy (GTK_OBJECT (result)) ;
        }

        return NULL ;
}


CRBoxView *
cr_box_view_new_from_bm (CRBoxModel *a_box_root)
{
	CRBoxView *result = NULL ;

        result = g_object_new (CR_TYPE_BOX_VIEW, NULL) ;
        g_return_val_if_fail (result, NULL) ;

        cr_box_view_set_box_model (result, a_box_root) ;
        gtk_layout_set_size (GTK_LAYOUT (result), 1024, 768) ;
        g_signal_connect (G_OBJECT (result),
                          "expose-event",
                          (GCallback)expose_event_cb,
                          NULL) ;

        PRIVATE (result)->layeng = cr_lay_eng_new (GTK_LAYOUT (result)) ;
        if (!PRIVATE (result)->layeng)
        {       
                cr_utils_trace_info ("Could not instanciate the layout engine. "
                                     "The system may be out of memory") ;
                cr_box_view_destroy (GTK_OBJECT (result)) ;

                return NULL ;
        }
	return result ;
}


enum CRStatus
cr_box_view_set_box_model (CRBoxView *a_this,
                           CRBoxModel *a_box_model)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        if (PRIVATE (a_this)->box_model)
        {
                if (cr_box_unref ((CRBox*)PRIVATE (a_this)->box_model) == TRUE)
                        PRIVATE (a_this) = NULL ;
        }

        PRIVATE (a_this)->box_model = a_box_model ;
        if (a_box_model)
                cr_box_ref ((CRBox*)a_box_model) ;

        return TRUE ;
}

enum CRStatus
cr_box_view_layout (CRBoxView *a_this)
{
        
        g_return_val_if_fail (a_this 
                              && CR_IS_BOX_VIEW (a_this)
                              && PRIVATE (a_this)->box_model, 
                              CR_BAD_PARAM_ERROR) ;

        cr_lay_eng_layout_box_tree (PRIVATE (a_this)->layeng,
                                    PRIVATE (a_this)->box_model->box.children) ;
        cr_box_dump_to_file (PRIVATE (a_this)->box_model->box.children,
                             0, stdout) ;

        return CR_OK ;
}

enum CRStatus
cr_box_view_get_box_model (CRBoxView *a_this, CRBoxModel **a_box_model)
{
        g_return_val_if_fail (a_this
                              && PRIVATE (a_this), 
                              CR_BAD_PARAM_ERROR) ;

        *a_box_model = PRIVATE (a_this)->box_model ;
        return TRUE ;
}

void
cr_box_view_destroy (GtkObject *a_this)
{
	CRBoxView *self = NULL ;

	g_return_if_fail (a_this && CR_IS_BOX_VIEW (a_this)) ;

        self = CR_BOX_VIEW (a_this) ;
        g_return_if_fail (self && PRIVATE (self) ) ;

        if (PRIVATE (self)->box_model)
        {
                cr_box_unref ((CRBox*)PRIVATE (self)->box_model) ;
                PRIVATE (self)->box_model = NULL ;
        }

        if (PRIVATE (self)->gc)
        {
                gdk_gc_unref (PRIVATE (self)->gc) ;
                PRIVATE (self)->gc = NULL ;
        }

        if (PRIVATE (self))
        {
                g_free (PRIVATE (self)) ;
                PRIVATE (self) = NULL ;
        }

        if (gv_parent_class
            && GTK_OBJECT_CLASS (gv_parent_class)->destroy)
        {
                GTK_OBJECT_CLASS (gv_parent_class)->destroy (a_this) ;
        }
}
