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

#define PRIVATE(a_this) ((a_this)->priv)

struct _CRBoxViewPriv
{
	CRBox *box ;
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
draw_margins (CRBoxView *a_bv,
              CRBox *a_box) ;

static enum CRStatus
draw_borders (CRBoxView *a_bv,
              CRBox *a_box) ;

static enum CRStatus
draw_paddings (CRBoxView *a_bv,
               CRBox *a_box) ;

static enum CRStatus
draw_inner_box (CRBoxView *a_bv,
                CRBox *a_box) ;

static void
cr_box_view_class_init (CRBoxViewClass *a_klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (a_klass) ;
	GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (a_klass) ;

	gv_parent_class =  (GtkLayoutClass *)
		g_type_class_peek_parent (gobject_class) ;

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
                draw_box (CR_BOX_VIEW (a_this),
                          PRIVATE (CR_BOX_VIEW (a_this))->box,
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
draw_margins (CRBoxView *a_bv,
              CRBox *a_box)
{
        GdkWindow * window = NULL ;
        GtkWidget *widget = NULL ;
        CRBox *box = NULL ;
        GdkRectangle rect ;

        g_return_val_if_fail (a_bv
                              && CR_IS_BOX_VIEW (a_bv)
                              && a_box,
                              CR_BAD_PARAM_ERROR) ;
        
        widget = GTK_WIDGET (a_bv) ;
        window = GTK_LAYOUT (a_bv)->bin_window ;
        g_return_val_if_fail (window, CR_ERROR) ;

        box = a_box ;
        g_return_val_if_fail (box, CR_ERROR) ;

        /*
         *draw left margin rectangle
         */
        rect.x = box->outer_edge.x ;
        rect.y = box->border_edge.y ;
        rect.width = box->border_edge.x - box->outer_edge.x ;
        rect.height = box->border_edge.height ;
        gdk_draw_rectangle 
                (window,
                 widget->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;

        /*
         *draw right margin rectangle
         *Note: this uses the previous left margin rectangle calculation result.
         */
        rect.x = box->border_edge.x + box->border_edge.width;
        /*rect.y remains the same*/
        rect.width = box->outer_edge.width - 
                (box->border_edge.width + rect.width) ;
        /*rect.height remains the same;*/        
        gdk_draw_rectangle 
                (window,
                 GTK_WIDGET (a_bv)->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;


        /*
         *draw top margin rectangle.
         */
        rect.x = box->outer_edge.x ;
        rect.y = box->outer_edge.y ;
        rect.width = box->outer_edge.width ;
        rect.height = box->border_edge.y - box->outer_edge.y ;
        
        gdk_draw_rectangle 
                (window,
                 GTK_WIDGET (a_bv)->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;

        /*
         *draw bottom margin rectangle.
         */
        /*rect.x remains the same*/
        rect.y = box->border_edge.y + box->border_edge.height ;
        /*rect.width remains the same as for top margin rect.*/
        rect.height = box->outer_edge.height -
                (box->border_edge.height + rect.height) ;

        gdk_draw_rectangle 
                (window,
                 GTK_WIDGET (a_bv)->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;

        return CR_OK ;
}

static enum CRStatus
draw_borders (CRBoxView *a_bv,
              CRBox *a_box)
{
        GdkWindow * window = NULL ;
        GtkWidget *widget = NULL ;
        CRBox *box = NULL ;
        GdkRectangle rect ;

        g_return_val_if_fail (a_bv
                              && CR_IS_BOX_VIEW (a_bv)
                              && a_box,
                              CR_BAD_PARAM_ERROR) ;

        widget = GTK_WIDGET (a_bv) ;
        window = GTK_LAYOUT (a_bv)->bin_window ;
        g_return_val_if_fail (window, CR_ERROR) ;

        box = a_box ;
        g_return_val_if_fail (box, CR_ERROR) ;

        /*
         *draw left border rectangle
         */
        rect.x = box->border_edge.x ;
        rect.y = box->padding_edge.y ;
        rect.width = box->padding_edge.x - box->border_edge.x ;
        rect.height = box->padding_edge.height ;
        gdk_draw_rectangle 
                (window,
                 widget->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;
        /*
         *draw right border rectangle
         */
        rect.x = box->padding_edge.x + box->padding_edge.width ;
        rect.y = box->padding_edge.y ;
        rect.width = box->border_edge.width -
                (rect.width + box->padding_edge.width) ;
        /*rect.height remains the same*/
        gdk_draw_rectangle 
                (window,
                 widget->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;

        /*
         *draw top border rectangle
         */
        rect.x = box->border_edge.x ;
        rect.y = box->border_edge.y ;
        rect.width = box->border_edge.width ;
        rect.height = box->padding_edge.y - box->border_edge.y ;
        gdk_draw_rectangle 
                (window,
                 widget->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;

        /*
         *draw bottom border rectangle
         */
        /*rect.x remains the same*/
        rect.y = box->padding_edge.x + box->padding_edge.height;
        /*rect.width remains the same as the top border rect width*/
        rect.height = box->border_edge.height -
                (rect.height + box->padding_edge.height) ;
        gdk_draw_rectangle 
                (window,
                 widget->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;

        return CR_OK ;
}

static enum CRStatus
draw_paddings (CRBoxView *a_bv,
               CRBox *a_box)
{
        GdkWindow * window = NULL ;
        GtkWidget *widget = NULL ;
        CRBox *box = NULL ;
        GdkRectangle rect ;
        
        g_return_val_if_fail (a_bv
                              && CR_IS_BOX_VIEW (a_bv)
                              && a_box,
                              CR_BAD_PARAM_ERROR) ;

        widget = GTK_WIDGET (a_bv) ;
        window = GTK_LAYOUT (a_bv)->bin_window ;
        g_return_val_if_fail (window, CR_ERROR) ;

        box = a_box ;
        g_return_val_if_fail (box, CR_ERROR) ;

        /*
         *draw the left padding
         */
        rect.x = box->padding_edge.x ;
        rect.y = box->inner_edge.y ;
        rect.width = box->inner_edge.x - box->padding_edge.x ;
        rect.height = box->inner_edge.height ;
        gdk_draw_rectangle 
                (window,
                 widget->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;

        /*
         *draw the right padding rectangle.
         */
        rect.x = box->inner_edge.x + box->inner_edge.width ;
        /*rect.y remains the same*/
        rect.width = box->padding_edge.width -
                (rect.width + box->inner_edge.width) ;
        /*rect.height remains the same*/
        gdk_draw_rectangle 
                (window,
                 widget->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;

        /*
         *draw the top padding rectangle.
         */
        rect.x = box->padding_edge.x ;
        rect.y = box->padding_edge.y ;
        rect.width = box->padding_edge.width ;
        rect.height = box->inner_edge.y - box->padding_edge.y ;
        gdk_draw_rectangle 
                (window,
                 widget->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;

        /*
         *draw the the bottom padding rectangle.
         */
        /*rect.x remains the same*/
        rect.y = box->inner_edge.y + box->inner_edge.height ;
        /*rect.width remains the same*/
        rect.height = box->padding_edge.height -
                (rect.height + box->inner_edge.height) ;
        gdk_draw_rectangle 
                (window,
                 widget->style->base_gc[widget->state],
                 FALSE,
                 rect.x, rect.y, rect.width, rect.height) ;

        return CR_OK ;
}


static enum CRStatus
draw_inner_box (CRBoxView *a_bv,
                CRBox *a_box)
{
        GtkWidget *widget = NULL, *label = NULL ;
        CRBox *box = NULL ;
   
        g_return_val_if_fail (a_bv
                              && CR_IS_BOX_VIEW (a_bv)
                              && a_box,
                              CR_BAD_PARAM_ERROR) ;

        widget = GTK_WIDGET (a_bv) ;        
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
                gtk_layout_put (GTK_LAYOUT (a_bv), label,
                                box->inner_edge.x, 
                                box->inner_edge.y) ;
        else
                gtk_layout_move (GTK_LAYOUT (a_bv), label,
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

        for (cur_box = a_box; cur_box ; cur_box = cur_box->next)
        {
                draw_margins (a_this, cur_box) ;
                draw_borders (a_this, cur_box) ;
                draw_paddings (a_this, cur_box) ;
                draw_inner_box (a_this, cur_box) ;

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
cr_box_view_new (CRBox *a_box, CRBoxView *a_parent)
{
	CRBoxView *result = NULL ;


        result = g_object_new (CR_TYPE_BOX_VIEW, NULL) ;
        g_return_val_if_fail (result, NULL) ;

        cr_box_view_set_box (result, a_box) ;

        g_signal_connect (G_OBJECT (result),
                          "expose-event",
                          (GCallback)expose_event_cb,
                          NULL) ;

	return result ;
}


enum CRStatus
cr_box_view_set_box (CRBoxView *a_this,
                     CRBox *a_box)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        if (PRIVATE (a_this)->box)
        {
                if (cr_box_unref (PRIVATE (a_this)->box) == TRUE)
                        PRIVATE (a_this) = NULL ;
        }

        PRIVATE (a_this)->box = a_box ;
        if (a_box)
                cr_box_ref (a_box) ;

        return TRUE ;
}


enum CRStatus
cr_box_view_get_box (CRBoxView *a_this, CRBox **a_box)
{
        g_return_val_if_fail (a_this
                              && PRIVATE (a_this), 
                              CR_BAD_PARAM_ERROR) ;

        *a_box = PRIVATE (a_this)->box ;
        return TRUE ;
}

void
cr_box_view_destroy (GtkObject *a_this)
{	
	CRBoxView *self = NULL ;

	g_return_if_fail (a_this && CR_IS_BOX_VIEW (a_this)) ;

        self = CR_BOX_VIEW (a_this) ;
        g_return_if_fail (self) ;

        if (PRIVATE (self) && PRIVATE (self)->box)
        {
                cr_box_unref (PRIVATE (self)->box) ;
                PRIVATE (self)->box = NULL ;
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
