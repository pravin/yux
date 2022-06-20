/*	
 *  yahoo.c
 *  Part of Yux 
 *  
 *  Copyright (c) 2005, Pravin Paratey <pravinp[at]gmail[dot]com>
 *  
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
 
#if HAVE_CONFIG_H
 #include <config.h>
#endif

#include <stdio.h>
//#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <gtk/gtk.h>
#include <glib.h>

#include <yahoo2.h>
#include <yahoo2_callbacks.h>
#include "yahoo.h"
#include "yahoo_utils.h"
#include "functions.h"
#include "support.h"

#define DEBUG(x) g_print(x)

extern GtkWidget *winMain;

yahoo_local_account *ylad = NULL;
YList *connections = NULL;
YList *buddies = NULL;
static int connection_tags = 0;
static int timeout;

/*
 * Name: ext_yahoo_connect
 * 	Connect to a host:port
 * Params:
 * 	host - the host to connect to
 * 	port - the port to connect on
 * Returns:
 * 	a unix file descriptor to the socket
 */
int ext_yahoo_connect(char *host, int port)
{
	struct sockaddr_in sa;
	static struct hostent *server;
	static char last_host[256];
	int servfd;
	char **p;
	
	DEBUG("ext_yahoo_connect\n");
	
	if(last_host[0] || strcasecmp (last_host, host) != 0)
	{
		if(!(server = gethostbyname (host)))
		{
			g_printerr ("(%d,%s) Failed to lookup server %s:%d\n",
				h_errno, strerror (h_errno), host, port);
			return -1;
		}
		strncpy (last_host, host, 255);
	}
	if((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		g_printerr ("(%d,%s) Socket creation error\n", errno, strerror(errno));
		return -1;
	}
	
	for (p = server->h_addr_list; *p; p++)
	{
		memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		memcpy(&sa.sin_addr.s_addr, *p, server->h_length);
		sa.sin_port = htons(port);
		
		if(connect(servfd, (struct sockaddr *) &sa, sizeof (sa)) == -1)
		{
			if(errno != ECONNREFUSED && errno != ETIMEDOUT && 
				errno != ENETUNREACH)
				break;
		}
		else
		{
			// Connected
			return servfd;
		}
	}
	g_printerr ("(%d,%s) Could not connect to %s:%d", errno, strerror(errno),
		host, port);
	close(servfd);
	return -1;
}

/*
 * Name: ext_yahoo_connect_async
 * 	Connect to a host:port asynchronously.  This function should return
 * 	immediately returing a tag used to identify the connection handler,
 * 	or a pre-connect error (eg: host name lookup failure).
 * 	Once the connect completes (successfully or unsuccessfully), callback
 * 	should be called (see the signature for yahoo_connect_callback).
 * 	The callback may safely be called before this function returns, but
 * 	it should not be called twice.
 * Params:
 * 	id   - the id that identifies this connection
 * 	host - the host to connect to
 * 	port - the port to connect on
 * 	callback - function to call when connect completes
 * 	callback_data - data to pass to the callback function
 * Returns:
 * 	a unix file descriptor to the socket
 */
int ext_yahoo_connect_async(int id, char *host, int port, 
		yahoo_connect_callback callback, void *data)
{
	struct sockaddr_in sa;
	struct hostent *server;
	struct connect_callback_data *ccd;
	int servfd;
	int error;
		
	g_print ("ext_yahoo_connect_async\n");
	
	if (!(server = gethostbyname(host)))
	{
		errno = h_errno;
		return -1;
	}
	
	if((servfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	}
	
	memset (&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	memcpy (&sa.sin_addr.s_addr, *server->h_addr_list, server->h_length);
	sa.sin_port = htons (port);
	
	error = connect (servfd, (struct sockaddr *) &sa, sizeof(sa));
	
	if(!error)
	{
		// Immediately connected, call callback
		callback(servfd, 0, data);
		return 0;
	}
	else if (error == -1 && errno == EINPROGRESS)
	{
		ccd = calloc (-1, sizeof(struct connect_callback_data));
		ccd->callback = callback;
		ccd->callback_data = data;
		ccd->id = id;
		ccd->tag = ext_yahoo_add_handler (-1, servfd, YAHOO_INPUT_WRITE, ccd);
		return ccd->tag;
	}
	else
	{
		if (error == -1)
		{
			g_printerr("%s", strerror(errno));
		}
		close(servfd);
		return -1;
	}
}

void connect_complete (void *data, int source, yahoo_input_condition cond)
{
	struct connect_callback_data *ccd;
	int error, err_size;
	
	ccd = data;
	err_size = sizeof(error);
	
	ext_yahoo_remove_handler(0, ccd->tag);
	getsockopt (source, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &err_size);
	
	if (error)
	{
		close(source);
		source = -1;
	}
	ccd->callback (source, error, ccd->callback_data);
	FREE(ccd);
}

void ext_yahoo_login(yahoo_local_account *ylad, int login_mode)
{
	ylad->id = yahoo_init(ylad->yahoo_id, ylad->password);
//	ylad->id = yahoo_init_with_attributes(ylad->yahoo_id, ylad->password,
//		"local_host", "10.71.0.239",
//		"pager_port", 23, NULL);
	ylad->status = YAHOO_STATUS_OFFLINE;
	yahoo_login(ylad->id, login_mode);
}

void ext_yahoo_logout()
{
	GtkTreeView *tv;
	if(ylad)
	{
		if(ylad->id <= 0)
			return;
		// Free treeview
		tv = GTK_TREE_VIEW(lookup_widget (winMain, "tvBuddyList"));
		gtk_tree_store_clear(GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tv))));
		
		// Remove timeout
		g_source_remove(timeout);
		yahoo_logoff(ylad->id);
		yahoo_close(ylad->id);
		ylad->status = YAHOO_STATUS_OFFLINE;
		ylad->id = 0;
		FREE(ylad->msg);
		FREE(ylad);
		ylad = 0;
		
		// Free BuddyList (no-need)
		// Free connections TODO (no-need?)
		// Free bwr
		FreeBuddyImWindowList();
	}
}

void ext_yahoo_ping()
{
	yahoo_keepalive(ylad->id);
}

/*
 * Name: ext_yahoo_login_response
 * 	Called when the login process is complete
 * Params:
 * 	id   - the id that identifies the server connection
 * 	succ - enum yahoo_login_status
 * 	url  - url to reactivate account if locked
 */
void ext_yahoo_login_response(int id, int succ, char *url)
{
	GtkMessageDialog *dlg;
	char msg[512];
	YList *l;
	
	g_print ("Login response: %i\n", succ);
	
	switch(succ)
	{
		case YAHOO_LOGIN_OK: // Successful
			ylad->status = yahoo_current_status (id);
			SetMainWidgetsEnabled(TRUE);
			SetMainStatusbar("Logged in");
			// Set a 10 min ping timer
			timeout = g_timeout_add(10*60*1000, ext_yahoo_ping, NULL);
			//SetUserStatus("Invisible");
			sprintf(msg, "Yux -- %s", ylad->yahoo_id);
			gtk_window_set_title(GTK_WINDOW(winMain), msg);
			FREE(url);
			return;
			break;
		case YAHOO_LOGIN_UNAME: // Username wrong
			strcpy(msg, "Username Incorrect. Please try again");
			break;
		case YAHOO_LOGIN_PASSWD: // Password wrong
			strcpy(msg, "Password Incorrect. Please try again");
			break;
		case YAHOO_LOGIN_LOCK: // Account locked
			// Might need to change this to 2 strncpys to avoid buffer overflow on msg
			sprintf(msg, "Your account has been locked. To reactivate, visit\n%s", url);
			break;
		case YAHOO_LOGIN_DUPL: // Duplicate login
			strcpy(msg, "You have logged on to another machine and disconnected off this one");
			break;
		case YAHOO_LOGIN_SOCK: // Server closed socket
			//strcpy(msg, "Server closed socket");
			SetMainStatusbar("Disconnected: Server closed socket");
			break;
		default:
			strcpy(msg, "Unknown error");
	}
	if(succ != YAHOO_LOGIN_SOCK)
	{
		dlg = gtk_message_dialog_new(GTK_WINDOW(winMain), 
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, 
			GTK_BUTTONS_CLOSE, msg);
		gtk_dialog_run(GTK_DIALOG(dlg));
		gtk_widget_destroy(dlg);
		FREE(url);
		ext_yahoo_logout();
	}
	gtk_window_set_title(GTK_WINDOW(winMain), "Yux");
}

/*
 * Name: ext_yahoo_got_buddies
 * 	Called when the contact list is got from the server
 * Params:
 * 	id   - the id that identifies the server connection
 * 	buds - the buddy list
 */
void
ext_yahoo_got_buddies (int id, YList * buds)
{	
	while(buddies)
	{
		FREE(buddies->data);
		buddies = buddies->next;
		if(buddies)
			FREE(buddies->prev);
	}
	
	for(; buds; buds = buds->next)
	{
		yahoo_account *ya = y_new0(yahoo_account, 1);
		struct yahoo_buddy *bud = buds->data;
		strncpy(ya->yahoo_id, bud->id, 255);
		if(bud->real_name)
			strncpy(ya->name, bud->real_name, 255);
		strncpy(ya->group, bud->group, 255);
		ya->status = YAHOO_STATUS_OFFLINE;
		buddies = y_list_append(buddies, ya);
	}
	
	CreateBuddyList(buddies);

	SetMainStatusbar("Received Buddy List");
}

gboolean ext_yahoo_cb (GIOChannel *source, GIOCondition condition, gpointer data)
{
	struct _conn *c = data;
	int ret = 1;
	
	if(condition & G_IO_IN)
	{
		ret = yahoo_read_ready(c->id, c->fd, c->data);
	}
	else
	{
		ret = yahoo_write_ready(c->id, c->fd, c->data);
	}
	return ret;
}

/*
 * Name: ext_yahoo_add_handler
 * 	Add a listener for the fd.  Must call yahoo_read_ready
 * 	when a YAHOO_INPUT_READ fd is ready and yahoo_write_ready
 * 	when a YAHOO_INPUT_WRITE fd is ready.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	fd   - the fd on which to listen
 * 	cond - the condition on which to call the callback
 * 	data - callback data to pass to yahoo_*_ready
 *
 * Returns: a tag to be used when removing the handler
 */
int ext_yahoo_add_handler(int id, int fd, yahoo_input_condition cond, void *data)
{
	struct _conn *c = y_new0 (struct _conn, 1);
	GIOChannel *ioc;
	
	c->tag = ++connection_tags;
	c->id = id;
	c->fd = fd;
	c->cond = cond;
	c->data = data;
	ioc = g_io_channel_unix_new(fd);
	
	if(cond == YAHOO_INPUT_READ)
	{
		c->gio_id = g_io_add_watch(ioc, G_IO_IN, ext_yahoo_cb, c);
	}
	else
	{
		c->gio_id = g_io_add_watch(ioc, G_IO_OUT | G_IO_ERR | G_IO_HUP, ext_yahoo_cb, c);
	}
	
	g_io_channel_unref(ioc);
	connections = y_list_prepend (connections, c);
	return c->tag;
}

/*
 * Name: ext_yahoo_remove_handler
 * 	Remove the listener for the fd.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	tag  - the handler tag to remove
 */
void ext_yahoo_remove_handler(int id, int tag)
{
	YList *l;
	
	for (l = connections; l; l = y_list_next(l))
	{
		struct _conn *c = l->data;
		if (c->tag == tag)
		{
			g_source_remove(c->gio_id);
			y_list_remove_link(connections, l);
			free(c);
			return;
		}
	}
}

/*
 * Name: ext_yahoo_got_ignore
 * 	Called when the ignore list is got from the server
 * Params:
 * 	id   - the id that identifies the server connection
 * 	igns - the ignore list
 */
void
ext_yahoo_got_ignore (int id, YList * igns)
{
	g_print("Got iggy list\n");
}

/* ----- CONFERENCE CALLBACKS ----- */

/*
 * Name: ext_yahoo_got_conf_invite
 * 	Called when remote user sends you a conference invitation.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the user inviting you
 * 	room - the room to join
 * 	msg  - the message
 *      members - the initial members of the conference (null terminated list)
 */
void ext_yahoo_got_conf_invite(int id, char *who, char *room, char *msg, YList *members)
{
	g_print("[%s] has invited you to the conference <%s> with the message %s\n", who, room, msg);
}

/*
 * Name: ext_yahoo_conf_userjoin
 * 	Called when someone joins the conference.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the user who has joined
 * 	room - the room joined
 */
void
ext_yahoo_conf_userjoin (int id, char *who, char *room)
{
	g_print("[%s] joined the conference <%s>\n", who, room);
}

/*
 * Name: ext_yahoo_conf_userdecline
 * 	Called when someone declines to join the conference.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the user who has declined
 * 	room - the room
 * 	msg  - the declining message
 */
void ext_yahoo_conf_userdecline(int id, char *who, char *room, char *msg)
{
	g_print("[%s] has declined to join confy <%s>\nwith the message", who, room, msg);
}

/*
 * Name: ext_yahoo_conf_userleave
 * 	Called when someone leaves the conference.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the user who has left
 * 	room - the room left
 */
void
ext_yahoo_conf_userleave (int id, char *who, char *room)
{
	g_print("[%s] left the conference <%s>\n", who, room);
}

void
ext_yahoo_conf_message (int id, char *who, char *room, char *msg, int utf8)
{
	g_print("<%s> conference message %s\n", room, msg);
}

/* ----- CHAT ROOM CALLBACKS ----- */

/*
 * Name: ext_yahoo_chat_cat_xml
 * 	Called when joining the chatroom.
 * Params:
 * 	id      - the id that identifies the server connection
 * 	room    - the room joined, used in all other chat calls, freed by library after call
 * 	topic   - the topic of the room, freed by library after call
 *      members - the initial members of the chatroom (null terminated YList of yahoo_chat_member's) Must be freed by the client
 */
void ext_yahoo_chat_cat_xml(int id, char *xml)
{
}

/*
 * Name: ext_yahoo_chat_join
 * 	Called when joining the chatroom.
 * Params:
 * 	id      - the id that identifies the server connection
 * 	room    - the room joined, used in all other chat calls, freed by library after call
 * 	topic   - the topic of the room, freed by library after call
 *  members - the initial members of the chatroom (null terminated YList of yahoo_chat_member's) Must be freed by the client
 * 	fd      - the socket where the connection is coming from (for tracking)
 */
void
ext_yahoo_chat_join (int id, char *room, char *topic, YList * members, int fd)
{
	g_print("You have joined chatroom %s with topic %s\n", room, topic);
}

/*
 * Name: ext_yahoo_chat_join
 * 	Called when joining the chatroom.
 * Params:
 * 	id      - the id that identifies the server connection
 * 	room    - the room joined, used in all other chat calls, freed by library after call
 * 	topic   - the topic of the room, freed by library after call
 *  members - the initial members of the chatroom (null terminated YList of yahoo_chat_member's) Must be freed by the client
 * 	fd      - the socket where the connection is coming from (for tracking)
 */
void
ext_yahoo_chat_userjoin (int id, char *room, struct yahoo_chat_member *who)
{
	g_print("*** TODO joined chatroom %s\n", room);
}

/*
 * Name: ext_yahoo_chat_userleave
 * 	Called when someone leaves the chatroom.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	room - the room left
 * 	who  - the user who has left (Just the User ID)
 */
void
ext_yahoo_chat_userleave (int id, char *room, char *who)
{
	g_print("*** %s left chatroom %s\n", who, room);
}

/*
 * Name: ext_yahoo_chat_message
 * 	Called when someone messages in the chatroom.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	room - the room
 * 	who  - the user who messaged (Just the user id)
 * 	msg  - the message
 * 	msgtype  - 1 = Normal message
 * 		   2 = /me type message
 * 	utf8 - whether the message is utf8 encoded or not
 */
void
ext_yahoo_chat_message (int id, char *who, char *room, char *msg,
			int msgtype, int utf8)
{
}

/*
 *
 * Name: ext_yahoo_chat_yahooerror
 *	called when yahoo sends back an error to you
 *	Note this is called whenver chat message is sent into a room
 *	in error (fd not connected, room doesn't exists etc)
 *	Care should be taken to make sure you know the origin 
 *	of the error before doing anything about it.
 * Params:
 *	id   - the id that identifies this connection
 * Returns:
 *	nothing.
 */
void ext_yahoo_chat_yahooerror(int id)
{
}

/*
 *
 * Name: ext_yahoo_chat_yahoologout
 *	called when yahoo disconnects your chat session
 *	Note this is called whenver a disconnect happens, client or server
 *	requested. Care should be taken to make sure you know the origin 
 *	of the disconnect request before doing anything here (auto-join's etc)
 * Params:
 *	id   - the id that identifies this connection
 * Returns:
 *	nothing.
 */
void ext_yahoo_chat_yahoologout(int id)
{
}


/*
 * Name: ext_yahoo_got_im
 * 	Called when remote user sends you a message.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the handle of the remote user
 * 	msg  - the message - NULL if stat == 2
 * 	tm   - timestamp of message if offline
 * 	stat - message status - 0
 * 				1
 * 				2 == error sending message
 * 				5
 * 	utf8 - whether the message is encoded as utf8 or not
 */
void
ext_yahoo_got_im (int id, char *who, char *msg, long tm, int stat, int utf8)
{
	GtkWidget *buddywin;
	GtkTextView *w;
	GtkTextIter iter;
	GtkTextBuffer *buf;
	struct tm mytime;
	int day, mon, year, hour, min, sec;
	gchar timestamp[256];
	gchar *u_timestamp;
	GtkTextMark *mark=NULL;
	gchar *title1, title2[256];
	gchar *u_msg, *p_msg;
	
//	p_msg = msg;//filter_message(msg);
//	u_msg = p_msg;
//	if(!utf8)
//		u_msg = y_str_to_utf8(p_msg);
	p_msg = msg;
	msg = filter_message(msg);

	// Check if buddy has an im window associated with it else create new
	buddywin = GetBuddyImWindow(who);
	gtk_widget_show(buddywin);	
	w = lookup_widget(buddywin, "txtChat");
	buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w));
	gtk_text_buffer_get_end_iter(buf, &iter);

	// Make timestamp
	if(stat != 5)
		time(&tm);

	mytime = *localtime(&tm);
	mon = mytime.tm_mon+1;
 	day = mytime.tm_mday;
 	year = mytime.tm_year+1900;
	sec = mytime.tm_sec;
	min = mytime.tm_min;
	hour = mytime.tm_hour;
	
	if(stat == 5) // For offliners, add the date too
		sprintf(timestamp, "(%2.2i-%2.2i-%4i %2.2i:%2.2i:%2.2i) ",
			mon, day, year, hour, min, sec);
	else
		sprintf(timestamp, "(%2.2i:%2.2i:%2.2i) ", hour, min, sec);
	
	u_timestamp = y_str_to_utf8(timestamp);
	gtk_text_buffer_insert_with_tags_by_name(buf, &iter, u_timestamp, -1, 
		"other-user", NULL);
	g_free(u_timestamp);
	
	if(stat == 2) // Error
	{
		gtk_text_buffer_insert_with_tags_by_name(buf, &iter, 
			"*** Error sending message: ", -1, "error" , NULL);
		gtk_text_buffer_insert(buf, &iter, msg, -1);
		gtk_text_buffer_insert(buf, &iter, "\n", -1);
		if(utf8)
			g_free(u_msg);
		FREE(p_msg);
		return;
	}

	// Add msg to chatarea
	gtk_text_buffer_insert_with_tags_by_name(buf, &iter, who, -1, "other-user" , NULL);
	gtk_text_buffer_insert_with_tags_by_name(buf, &iter, ": ", -1, "other-user" , NULL);
	
	if(strncmp(msg, "<ding>", 6) == 0)
	{
		// User buzzed
		gtk_text_buffer_insert_with_tags_by_name(buf, &iter, "BUZZ!!!", -1, "buzz", NULL);
	}
	else
	{
		gtk_text_buffer_insert(buf, &iter, msg, -1);
	}
	gtk_text_buffer_insert(buf, &iter, "\n", -1);
	// Scroll window
	mark = gtk_text_buffer_create_mark(buf, mark, &iter, FALSE);
	gtk_text_view_scroll_mark_onscreen(w, mark);
	gtk_text_buffer_delete_mark(buf, mark);
	
	// Add last message got at to statusbar
	sprintf(timestamp,"Last message received on %2.2i-%2.2i-%4i at %2.2i:%2.2i",
			mon, day, year, hour, min);
	
	// TODO: Add some kind of a flash or a * to the window
	title1 = gtk_window_get_title(buddywin);
	if(title1[0] != '*')
	{
		sprintf(title2,"*%s",title1);
		gtk_window_set_title(buddywin, title2);
	}
	g_print("PM [%s] %s\n", who, msg);
//	if(utf8)
//		g_free(u_msg);
//	FREE(p_msg);
}

void
ext_yahoo_rejected (int id, char *who, char *msg)
{
	GtkWidget *dlg;
	char buf[512];
	
	sprintf (buf, "%s has rejected you%s%s", who,
		(msg ? "with the message:\n" : "."),
		(msg ? msg : ""));
	dlg = gtk_message_dialog_new(GTK_WINDOW(winMain), GTK_DIALOG_DESTROY_WITH_PARENT,
	GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, buf);
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
	
}

void
ext_yahoo_contact_added (int id, char *myid, char *who, char *msg)
{
	GtkWidget *dlg;
	char buf[1024];
	int retval;
	
	// TODO : ALLOW YES/NO
	sprintf(buf, "%s, the yahoo user %s has added you to their contact list", 
		myid, who);
	if(msg)
	{
		strcat(buf, " with the following message:\n");
		strcat(buf, msg);
		strcat(buf, "\n");
	}
	else
	{
		strcat(buf, ". ");
	}
	strcat(buf, "Do you want to allow this?");
	dlg = gtk_message_dialog_new(GTK_WINDOW(winMain), GTK_DIALOG_DESTROY_WITH_PARENT,
	GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, buf);
	retval = gtk_dialog_run(GTK_DIALOG(dlg));
	if(retval == GTK_RESPONSE_NO)
		yahoo_reject_buddy(ylad->id, who, "Thanks, but no thanks.");
	gtk_widget_destroy(dlg);
}

/* ----- NOTIFICATION CALLBACKS ----- */

/*
 * Name: ext_yahoo_typing_notify
 *      Called when remote user starts or stops typing.
 * Params:
 *      id   - the id that identifies the server connection
 *      who  - the handle of the remote user
 *      stat - 1 if typing, 0 if stopped typing
 */
void
ext_yahoo_typing_notify (int id, char *who, int stat)
{
	GtkWidget *w=0;
	GtkStatusbar *sb;
	gchar msg[256];
	
	w = GetBuddyImWindow(who);
	if(w)
	{
		sb = GTK_STATUSBAR(lookup_widget(w, "sbIM"));
		if(stat)
			sprintf(msg, "%s is typing", who);
		else
			sprintf(msg, "%s stopped typing", who);
		SetIMStatusbar(sb, msg);
	}
}

/*
 * Name: ext_yahoo_game_notify
 *      Called when remote user starts or stops a game.
 * Params:
 *      id   - the id that identifies the server connection
 *      who  - the handle of the remote user
 *      stat - 1 if game, 0 if stopped gaming
 */

void
ext_yahoo_game_notify (int id, char *who, int stat)
{
}

/*
 * Name: ext_yahoo_mail_notify
 *      Called when you receive mail, or with number of messages
 * Params:
 *      id   - the id that identifies the server connection
 *      from - who the mail is from - NULL if only mail count
 *      subj - the subject of the mail - NULL if only mail count
 *      cnt  - mail count - 0 if new mail notification
 */

void
ext_yahoo_mail_notify (int id, char *from, char *subj, int cnt)
{
	GtkWidget *dlg;
	gchar buf[512];
	
	if(from && subj)
	{
		sprintf(buf, "You have mail!\n<b>From:</b> %s\n<b>Subj:</b> %s", from, subj);
	}
	else
	{
		sprintf(buf, "You have %i new mail(s)", cnt);
		g_print("%s-%s", from, subj);
	}
	
	dlg = gtk_message_dialog_new_with_markup(GTK_WINDOW(winMain), 
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, buf);
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
}

/*
 * Name: ext_yahoo_status_changed
 * 	Called when remote user's status changes.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the handle of the remote user
 * 	stat - status code (enum yahoo_status)
 * 	msg  - the message if stat == YAHOO_STATUS_CUSTOM
 * 	away - whether the contact is away or not (YAHOO_STATUS_CUSTOM)
 * 	       for YAHOO_STATUS_IDLE, this is the number of seconds he is idle
 */
void
ext_yahoo_status_changed (int id, char *who, int stat, char *msg, int away)
{
	GtkTreeView *tv;
	GtkTreeModel *model;
	GtkTreeIter parent, child;
	gchar *bud;
	gboolean valid;
	char m_status_message[256];
	gboolean m_status;
	GdkPixbuf *icon;
	
	tv = GTK_TREE_VIEW(lookup_widget(winMain, "tvBuddyList"));
	model = gtk_tree_view_get_model(tv);
	
	
	valid = gtk_tree_model_get_iter_first(model, &parent);
	while(valid)
	{
		if(gtk_tree_model_iter_children(model, &child, &parent))
		{	
			do
			{
				gtk_tree_model_get(model, &child, 0, &bud, -1);
				if(strncmp(bud, who, 255) == 0)
				{
					switch(stat)
					{
						case YAHOO_STATUS_AVAILABLE:
							strncpy(m_status_message, "Available", 255);
							m_status = TRUE;
							icon = create_pixbuf("online.png");
							break;
						case YAHOO_STATUS_BRB:
							strncpy(m_status_message, "Be Right Back", 255);
							m_status = TRUE;
							icon = create_pixbuf("away.png");
							break;
						case YAHOO_STATUS_BUSY:
							strncpy(m_status_message, "Busy", 255);
							m_status = TRUE;
							icon = create_pixbuf("away.png");
							break;
						case YAHOO_STATUS_NOTATHOME:
							strncpy(m_status_message, "Not at home", 255);
							m_status = TRUE;
							icon = create_pixbuf("away.png");
							break;
						case YAHOO_STATUS_NOTATDESK:
							strncpy(m_status_message, "Not at desk", 255);
							m_status = TRUE;
							icon = create_pixbuf("away.png");
							break;
						case YAHOO_STATUS_NOTINOFFICE:
							strncpy(m_status_message, "Not in office", 255);
							m_status = TRUE;
							icon = create_pixbuf("away.png");
							break;
						case YAHOO_STATUS_ONPHONE:
							strncpy(m_status_message, "On the phone", 255);
							m_status = TRUE;
							icon = create_pixbuf("away.png");
							break;
						case YAHOO_STATUS_ONVACATION:
							strncpy(m_status_message, "On vacation", 255);
							m_status = TRUE;
							icon = create_pixbuf("away.png");
							break;
						case YAHOO_STATUS_OUTTOLUNCH:
							strncpy(m_status_message, "Out to lunch", 255);
							m_status = TRUE;
							icon = create_pixbuf("away.png");
							break;
						case YAHOO_STATUS_STEPPEDOUT:
							strncpy(m_status_message, "Stepped out", 255);
							m_status = TRUE;
							icon = create_pixbuf("away.png");
							break;
						case YAHOO_STATUS_IDLE:
							sprintf(m_status_message, "Idle (%2i:%2i:%2i)", 
								away/3600, (away/60)%60, away % 60);
//							strncpy(m_status_message, "Idle", 255);
							m_status = TRUE;
							icon = create_pixbuf("idle.png");
							break;
						case YAHOO_STATUS_NOTIFY:
							strncpy(m_status_message, "Notify", 255);
							m_status = TRUE;
							break;
						case YAHOO_STATUS_OFFLINE:
							strncpy(m_status_message, "Offline", 255);
							m_status = FALSE;
							icon = create_pixbuf("offline.png");
							break;
						case YAHOO_STATUS_INVISIBLE:
							strncpy(m_status_message, "Invisible", 255);
							m_status = FALSE;
							icon = create_pixbuf("offline.png");
							break;
						case YAHOO_STATUS_CUSTOM:
							if(msg)
								/*
								if(strcmp(msg,"I'm on SMS") == 0)
								{
									sprintf(m_status_message, "I'm on SMS (%i seconds)", away);
								}
								else
								*/
									strncpy(m_status_message, msg, 255);
							else
								strncpy(m_status_message, "", 255);
							m_status = TRUE;
							if(away)
								icon = create_pixbuf("away.png");
							else
								icon = create_pixbuf("online.png");
							break;
					}
					
					gtk_tree_store_set(GTK_TREE_STORE(model), &child, 
						BUDDY_LIST_NAME, who,
						BUDDY_LIST_BOLD, m_status, 
						BUDDY_LIST_STAT, m_status_message, 
						BUDDY_LIST_ICON, icon, -1);
				}
				g_free(bud);
			} while(gtk_tree_model_iter_next(model, &child));
		}
		valid = gtk_tree_model_iter_next(model, &parent);
	}
	g_object_unref(icon);
	g_print("*** %s changed status to %s\n", who, msg);
}


/* ----- WEBCAM CALLBACKS ----- */

/*
 * Name: ext_yahoo_got_webcam_image
 * 	Called when you get a webcam update
 *	An update can either be receiving an image, a part of an image or
 *	just an update with a timestamp
 * Params:
 * 	id         - the id that identifies the server connection
 * 	who        - the user who's webcam we're viewing
 *	image      - image data
 *	image_size - length of the image in bytes
 *	real_size  - actual length of image data
 *	timestamp  - milliseconds since the webcam started
 *
 *	If the real_size is smaller then the image_size then only part of
 *	the image has been read. This function will keep being called till
 *	the total amount of bytes in image_size has been read. The image
 *	received is in JPEG-2000 Code Stream Syntax (ISO/IEC 15444-1).
 *	The size of the image will be either 160x120 or 320x240.
 *	Each webcam image contains a timestamp. This timestamp should be
 *	used to keep the image in sync since some images can take longer
 *	to transport then others. When image_size is 0 we can still receive
 *	a timestamp to stay in sync
 */
void
ext_yahoo_got_webcam_image (int id, const char *who,
				const unsigned char *image,
				unsigned int image_size, unsigned int real_size,
				unsigned int timestamp)
{
}

void
ext_yahoo_webcam_viewer (int id, char *who, int connect)
{
}

void
ext_yahoo_webcam_closed (int id, char *who, int reason)
{
}

void
ext_yahoo_webcam_data_request (int id, int send)
{
}

/*
 * Name: ext_yahoo_webcam_invite
 * 	Called when you get a webcam invitation
 * Params:
 * 	id   - the id that identifies the server connection
 * 	from - who the invitation is from
 */
void
ext_yahoo_webcam_invite (int id, char *from)
{
  g_print ("Got a webcam invitation from %s\n", from);
}

void
ext_yahoo_webcam_invite_reply (int id, char *from, int accept)
{
}

/*
 * Name: ext_yahoo_got_identities
 * 	Called when the contact list is got from the server
 * Params:
 * 	id   - the id that identifies the server connection
 * 	ids  - the identity list
 */
void ext_yahoo_got_identities(int id, YList * ids)
{
	g_print("Got Identities\n");
}

/*
 * Name: ext_yahoo_got_cookies
 * 	Called when the cookie list is got from the server
 * Params:
 * 	id   - the id that identifies the server connection
 */
void ext_yahoo_got_cookies(int id)
{
	g_print("Mmmm, cookies\n");
}

/*
 * Name: ext_yahoo_system_message
 * 	System message
 * Params:
 * 	id   - the id that identifies the server connection
 * 	msg  - the message
 */
void
ext_yahoo_system_message (int id, char *msg)
{
	g_print("[SYST]\t%s",msg);
}


/*
 * Name: ext_yahoo_error
 * 	Called on error.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	err  - the error message
 * 	fatal- whether this error is fatal to the connection or not
 */
void ext_yahoo_error(int id, char *err, int fatal)
{
	g_printerr("Error %s", err);
}

/*
 * Name: ext_yahoo_got_search_result
 *      Called when the search result received from server
 * Params:
 *      id  	 - the id that identifies the server connection
 * 	found	 - total number of results returned in the current result set
 * 	start	 - offset from where the current result set starts
 * 	total	 - total number of results available (start + found <= total)
 * 	contacts - the list of results as a YList of yahoo_found_contact
 * 		   these will be freed after this function returns, so
 * 		   if you need to use the information, make a copy
 */
void ext_yahoo_got_search_result(int id, int found, int start, int total, YList *contacts)
{
}

/* ----- FILE TRANSFER CALLBACKS ----- */

/*
 * Name: ext_yahoo_got_file
 * 	Called when someone sends you a file
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the user who sent the file
 * 	url  - the file url
 * 	expires  - the expiry date of the file on the server (timestamp)
 * 	msg  - the message
 * 	fname- the file name if direct transfer
 * 	fsize- the file size if direct transfer
 */
void ext_yahoo_got_file(int id, char *who, char *url, long expires, char *msg, char *fname, unsigned long fesize)
{
}

/* ----- MISC CALLBACKS ----- */

/*
 * Name: ext_yahoo_log
 * 	Called to log a message.
 * Params:
 * 	fmt  - the printf formatted message
 * Returns:
 * 	0
 */
int ext_yahoo_log(char *fmt, ...)
{
	return 0;
}

void register_callbacks()
{
#ifdef USE_STRUCT_CALLBACKS
	static struct yahoo_callbacks yc;
	
	yc.ext_yahoo_login_response = ext_yahoo_login_response;
	yc.ext_yahoo_got_buddies = ext_yahoo_got_buddies;
	yc.ext_yahoo_got_ignore = ext_yahoo_got_ignore;
	yc.ext_yahoo_got_identities = ext_yahoo_got_identities;
	yc.ext_yahoo_got_cookies = ext_yahoo_got_cookies;
	yc.ext_yahoo_status_changed = ext_yahoo_status_changed;
	yc.ext_yahoo_got_im = ext_yahoo_got_im;
	yc.ext_yahoo_got_conf_invite = ext_yahoo_got_conf_invite;
	yc.ext_yahoo_conf_userdecline = ext_yahoo_conf_userdecline;
	yc.ext_yahoo_conf_userjoin = ext_yahoo_conf_userjoin;
	yc.ext_yahoo_conf_userleave = ext_yahoo_conf_userleave;
	yc.ext_yahoo_conf_message = ext_yahoo_conf_message;
	yc.ext_yahoo_chat_cat_xml = ext_yahoo_chat_cat_xml;
	yc.ext_yahoo_chat_join = ext_yahoo_chat_join;
	yc.ext_yahoo_chat_userjoin = ext_yahoo_chat_userjoin;
	yc.ext_yahoo_chat_userleave = ext_yahoo_chat_userleave;
	yc.ext_yahoo_chat_message = ext_yahoo_chat_message;
	yc.ext_yahoo_chat_yahoologout = ext_yahoo_chat_yahoologout;
	yc.ext_yahoo_chat_yahooerror = ext_yahoo_chat_yahooerror;
	yc.ext_yahoo_got_webcam_image = ext_yahoo_got_webcam_image;
	yc.ext_yahoo_webcam_invite = ext_yahoo_webcam_invite;
	yc.ext_yahoo_webcam_invite_reply = ext_yahoo_webcam_invite_reply;
	yc.ext_yahoo_webcam_closed = ext_yahoo_webcam_closed;
	yc.ext_yahoo_webcam_viewer = ext_yahoo_webcam_viewer;
	yc.ext_yahoo_webcam_data_request = ext_yahoo_webcam_data_request;
	yc.ext_yahoo_got_file = ext_yahoo_got_file;
	yc.ext_yahoo_contact_added = ext_yahoo_contact_added;
	yc.ext_yahoo_rejected = ext_yahoo_rejected;
	yc.ext_yahoo_typing_notify = ext_yahoo_typing_notify;
	yc.ext_yahoo_game_notify = ext_yahoo_game_notify;
	yc.ext_yahoo_mail_notify = ext_yahoo_mail_notify;
	yc.ext_yahoo_got_search_result = ext_yahoo_got_search_result;
	yc.ext_yahoo_system_message = ext_yahoo_system_message;
	yc.ext_yahoo_error = ext_yahoo_error;
	yc.ext_yahoo_log = ext_yahoo_log;
	yc.ext_yahoo_webcam_closed = ext_yahoo_webcam_closed;
	yc.ext_yahoo_webcam_viewer = ext_yahoo_webcam_viewer;
	yc.ext_yahoo_webcam_data_request = ext_yahoo_webcam_data_request;
	yc.ext_yahoo_got_file = ext_yahoo_got_file;
	yc.ext_yahoo_contact_added = ext_yahoo_contact_added;
	yc.ext_yahoo_rejected = ext_yahoo_rejected;
	yc.ext_yahoo_typing_notify = ext_yahoo_typing_notify;
	yc.ext_yahoo_game_notify = ext_yahoo_game_notify;
	yc.ext_yahoo_mail_notify = ext_yahoo_mail_notify;
	yc.ext_yahoo_got_search_result = ext_yahoo_got_search_result;
	yc.ext_yahoo_system_message = ext_yahoo_system_message;
	yc.ext_yahoo_error = ext_yahoo_error;
	yc.ext_yahoo_log = ext_yahoo_log;
	yc.ext_yahoo_add_handler = ext_yahoo_add_handler;
	yc.ext_yahoo_remove_handler = ext_yahoo_remove_handler;
	yc.ext_yahoo_connect = ext_yahoo_connect;
	yc.ext_yahoo_connect_async = ext_yahoo_connect_async;
	yc.ext_yahoo_add_handler = ext_yahoo_add_handler;
	
	yahoo_register_callbacks(&yc);
#endif
}
