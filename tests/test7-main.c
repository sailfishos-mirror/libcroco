/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset:8 -*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms 
 * of version 2.1 of the GNU Lesser General Public
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

#include <string.h>
#include "libcroco.h"
#include "cr-test-utils.h"

/*
const guchar *gv_xmlbuf =
"<ARTICLE>"
"  <HEADLINE>Fredrick the Great meets Bach</HEADLINE>"
"  <AUTHOR>Johann Nikolaus Forkel</AUTHOR>"
"  <PARA>"
"    One evening, just as he was getting his"
"    <INSTRUMENT>flute</INSTRUMENT> ready and his"
"    musicians were assembled, an officer brought him a list of"
"    the strangers who had arrived."
"  </PARA>"
"</ARTICLE>"
*/
;

/*
const guchar *gv_xmlbuf =
"<ARTICLE>"
"  <HEADLINE>This is a first headline</HEADLINE>"
"   <PARA>A paragraph</PARA>"
"  <AUTHOR>Author: Dodji</AUTHOR>"
"  <HEADLINE>This is another headline</HEADLINE>"
"   <PARA>A second paragraph</PARA>"
"  <AUTHOR>Author: Dodji.</AUTHOR>"
"</ARTICLE>"
;

const guchar *gv_cssbuf =
"INSTRUMENT { display: inline}"
"ARTICLE, HEADLINE, AUTHOR, PARA"
"{"
" display: block ;"
" position: static; "
"}"

"ARTICLE"
"{"
" background-color: white;"
"}"

"HEADLINE"
"{"
" background-color: blue;"
" border: solid black thin;"
"}"

"PARA"
"{"
" background-color: pink;"
" margin-left: 20px; " 
"}"
;
*/

const guchar *gv_xmlbuf =
"<?xml version=\"1.0\"?>"
"<rss version=\"0.91\">"
"  <channel>"
"    <title>scottandrew.com JavaScript and DHTML Channel</title>"
"<link>http://www.scottandrew.com</link>"
"<description>DHTML, DOM and JavaScript snippets from scottandrew.com</description>"
"    <language>en-us</language>"
"    <item>"
"      <title>DHTML Animation Array Generator</title>"
"<description>Robert points us to the first third-party tool for the DomAPI: The Animation Array Generator, a visual tool for creating...</description>"
"<link>http://www.scottandrew.com/weblog/2002_06#a000395</link>"
"    </item>"
"    <item>"
"      <title>DOM and Extended Entries</title>"
"<description>Aarondot: A Better Way To Display Extended Entries. Very cool, and uses the DOM and JavaScript to reveal the extended...</description>"
"<link>http://www.scottandrew.com/weblog/2002_06#a000373</link>"
"    </item>"
"    <item>"
"      <title>cellspacing and the DOM</title>"
"<description>By the way, if you're using the DOM to generate TABLE elements, you have to use setAttribute() to set the...</description>"
"      <link>http://www.scottandrew.com/weblog/2002_05#a000365</link>"
"    </item>"
"    <item>"
"      <title>contenteditable for Mozilla</title>"
"      <description>The folks art Q42, creator of Quek (cute little avatar/chat) and Xopus (browser-based WYSIWYG XML-editor) have released code that simulates...</description>"
"      <link>http://www.scottandrew.com/weblog/2002_05#a000361</link>"
"    </item>"
"  </channel>"
"</rss>"
;

const char * gv_cssbuf=
"rss"
"{                         "
"    display:block;        "
"    margin:10px;          "
"}                         "
"                          "
"channel                   "
"{                         "
" display:block;           "
" /*height:300px;*/        "
" width:200px;             "
" border:1px solid #000;   "
" overflow:auto;           "
" background-color:#eee;   "
" font: 12px verdana;      "
"}                         "
"                          "
"item                      "
"{                         "
" display: block;          "
" padding:10px;            "
" margin-bottom:10px;      "
" border:1px solid #ccc;   "
" /*border-top:1px solid #c"
" border-bottom:1px solid #"
" border-left:1px solid #cc"
" border-right:1px solid #c"
" */                       "
" background-color:#fff;   "
"}                         "
"                          "
"                          "
"channel>title, channel>des"
"{                         "
"        display: block;   "
"        margin-left:10px; "
"        margin-top:10px;  "
"        background-color:#"
"        font-weight:bold; "
"}                         "
"                          "
"channel>title             "
"{                         "
"        font-size:16px;   "
"}                         "
"                          "
"channel>description       "
"{                         "
"        display: block ;  "
"        font-size:10px;   "
"        margin-bottom:10px"
"}                         "
"                          "
"item>title                "
"{                         "
"        font-weight:bold; "
"}                         "
"                          "
"item>link, channel>link,  "
"{                         "
"        display: none;    "
"}                         "  
;

/*
" margin-top: 10px ;"
" margin-left: 10px ;"
" margin-bottom: 15px ;"
" margin-right: 15px ;"
" border-top-width: thick ;"
" border-top-style: solid ;"
" border-right-width: thin ;"
" border-right-style: solid ;"
" border-bottom-width: thin ;"
" border-bottom-style: solid ;"
" border-left-width: thick ;"
" border-left-style: solid ;"
*/



static enum CRStatus
test_layout_box (void)  ;

static gboolean
delete_event_cb (GtkWidget *a_widget, GdkEvent *a_event,
                 gpointer *a_user_data) ;


static gboolean
delete_event_cb (GtkWidget *a_widget, GdkEvent *a_event,
                 gpointer *a_user_data)
{
        gtk_main_quit () ;
        return FALSE ;
}

static enum CRStatus
test_layout_box (void)
{
        enum CRStatus status = CR_OK ;
        CRBoxView *box_view = NULL ;
        GtkWidget *window = NULL, *scroll = NULL ;

        box_view = cr_box_view_new_from_xml_css_bufs 
                (gv_xmlbuf, gv_cssbuf) ;
        g_return_val_if_fail (box_view, CR_BAD_PARAM_ERROR) ;

        window = gtk_window_new (GTK_WINDOW_TOPLEVEL) ;
        g_return_val_if_fail (window, CR_BAD_PARAM_ERROR) ;

        gtk_window_set_title (GTK_WINDOW (window), 
                              "Croco Renderer Test") ;
        gtk_window_set_policy (GTK_WINDOW (window), TRUE, TRUE, TRUE) ;
        gtk_widget_set_size_request (window, 800, 600) ;

        g_signal_connect (G_OBJECT (window),
                          "delete-event",
                          G_CALLBACK (delete_event_cb),
                          NULL) ;

        scroll = gtk_scrolled_window_new (NULL, NULL) ;
        g_return_val_if_fail (scroll, CR_BAD_PARAM_ERROR) ;

        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                        GTK_POLICY_ALWAYS,
                                        GTK_POLICY_ALWAYS) ;
        gtk_container_add (GTK_CONTAINER (window), scroll) ;
        gtk_container_add
                (GTK_CONTAINER (scroll), 
                 GTK_WIDGET (box_view)) ;
        gtk_widget_show_all (window) ;
        gtk_main () ;

        return CR_OK ;
}



int
main (int argc, char **argv)
{
	enum CRStatus status = CR_OK ;

        cr_lay_eng_init (argc, argv) ;

	status = test_layout_box () ;

	if (status != CR_OK)
	{
		g_print ("\nKO\n") ;
	}
	return 0 ;
}
