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
draw_box (CRBoxView *a_this) ;


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
                break ;
                
        default:
                cr_utils_trace_info ("Unexpected event received, "
                                     "Only GDK_EXPOSE was expected.") ;
                return FALSE ;
        }

        return FALSE ;
        
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
cr_box_view_new (CRBox *a_box)
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
