/***************************************************************************
 *            yahoo.h
 *
 *  Wed Mar  2 22:34:58 2005
 *  Copyright  2005  pravin
 *  pravinp[at]gmail[dot]com
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
 
#ifndef _YAHOO_H
#define _YAHOO_H

#include <gtk/gtk.h>
#include <glib.h>
#include <yahoo2.h>
#include <yahoo2_callbacks.h>

typedef struct
{
	char yahoo_id[255];
	char password[255];
	int id;
	int fd;
	int status;
	char *msg;
} yahoo_local_account;

typedef struct
{
	char yahoo_id[255];
	char name[255];
	int status;
	int away;
	char *msg;
	char group[255];
} yahoo_account;

typedef struct
{
	int id;
	char *label;
} yahoo_idlabel;

typedef struct
{
	int id;
	char *who;
} yahoo_authorize_data;

typedef struct
{
	int id;
	char * room_name;
	char * host;
	YList * members;
	int joined;
} conf_room;

struct connect_callback_data
{
	yahoo_connect_callback callback;
	void * callback_data;
	int id;
	int tag;
};

struct _conn
{
	int tag;
	int fd;
	int id;
	yahoo_input_condition cond;
	void *data;
	unsigned int gio_id;
};

void register_callbacks();
void ext_yahoo_login(yahoo_local_account *ylad, int login_mode);
#endif /* _YAHOO_H */
