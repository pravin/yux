/***************************************************************************
 *            functions.h
 *
 *  Fri Mar  4 15:46:45 2005
 *  Copyright  2005  Pravin Paratey
 *  pravin[at]gmail[dot]com
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

#include <gtk/gtk.h>
#include <sys/time.h>

enum
{
	BUDDY_LIST_NAME=0,
	BUDDY_LIST_BOLD,
	BUDDY_LIST_STAT,
	BUDDY_LIST_ICON,
	BUDDY_LIST_NUM
};

struct buddy_im_window_reln
{
	char yahoo_id[256];
	GtkWidget *window;
};

#endif /* _FUNCTIONS_H */
