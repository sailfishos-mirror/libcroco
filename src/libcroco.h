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

#ifndef __LIBCROCO_H__
#define __LIBCROCO_H__

#include "libcroco-config.h"

#include "parser/cr-utils.h"
#include "parser/cr-pseudo.h"
#include "parser/cr-term.h"
#include "parser/cr-attr-sel.h"
#include "parser/cr-simple-sel.h"
#include "parser/cr-selector.h"
#include "parser/cr-enc-handler.h"
#include "parser/cr-doc-handler.h"
#include "parser/cr-parser-input.h"
#include "parser/cr-input.h"
#include "parser/cr-parser.h"
#include "parser/cr-statement.h"
#include "parser/cr-stylesheet.h"
#include "parser/cr-om-parser.h"

#ifdef WITH_SELENG /*The selection engine headers*/
#include "seleng/cr-sel-eng.h"
#endif /*WITH_SELENG*/

#ifdef WITH_LAYENG /*the layout engine headers*/
#include "layeng/cr-style.h"
#include "layeng/cr-box.h"
#include "layeng/cr-box-view.h"
#include "layeng/cr-lay-eng.h"
#endif

#endif /*__LIBCROCO_H__*/
