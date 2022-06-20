/***************************************************************************
 *            functions.c
 *
 *  Fri Mar  4 15:43:49 2005
 *  Copyright  2005  Pravin Paratey
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

#if HAVE_CONFIG_H
 #include <config.h>
#endif

#include <string.h>

#include <gtk/gtk.h>
#include <glib.h>

#include <yahoo2.h>
#include <yahoo2_callbacks.h>
#include "yahoo.h"
#include "support.h"
#include "functions.h"
#include "interface.h"
#include "yahoo_utils.h"

extern GtkWidget *winMain;
extern YList *buddies;
extern yahoo_local_account *ylad;

YList *bwr = NULL; // buddy-window-reln
//static int sbim_timeout=0;

typedef struct
{
	char *smiley;
	int num;
}_SmileyList;

// This stores the filename (.gif)
// End with "", -1
_SmileyList SmileyList[] =
{
	{":)", 1},
	{":-)", 1},
	{"", -1} 
};

// Returns the filename for a smiley
// 0 if not found
int GetSmileyNum (char *s)
{
	int i=0;
	
	for(i=0;SmileyList[i].num != -1; i++)
		if (strcmp(SmileyList[i].smiley, s) == 0)
			return SmileyList[i].num;
	return 0;
}


void SetMainWidgetsEnabled(gboolean state)
{
	GtkWidget *w;
	w = lookup_widget (winMain,"tbMain");
	gtk_widget_set_sensitive (w, state);
	w = lookup_widget (winMain,"cboStatus");
	gtk_widget_set_sensitive (w, state);
}

void SetMainStatusbar(char *text)
{
	GtkStatusbar *w;
	guint context_id;
	w = GTK_STATUSBAR(lookup_widget (winMain, "sbMain"));
	context_id = gtk_statusbar_get_context_id(w, "msg");
	gtk_statusbar_pop(w, context_id);
	gtk_statusbar_push(w, context_id, text);
}
/*
void sbim_timeout_func(GtkStatusbar *sb)
{
	guint context_id;
	context_id = gtk_statusbar_get_context_id(sb, "msg");
	gtk_statusbar_pop(sb, context_id);
	gtk_statusbar_push(sb, context_id, "");
	g_source_remove(sbim_timeout);
	sbim_timeout = 0;
}
*/
void SetIMStatusbar(GtkStatusbar *sb, char *text)
{
	guint context_id;
	YList *lookup;
//	GtkWidget *parent;
//	time_t cur_time;
	
	context_id = gtk_statusbar_get_context_id(sb, "msg");
	gtk_statusbar_pop(sb, context_id);
	gtk_statusbar_push(sb, context_id, text);
	
/*	
	parent = gtk_widget_get_toplevel(sb);
	
	lookup = bwr;
	while(lookup)
	{
		struct buddy_im_window_reln *growl = lookup->data; 
		if(growl->window == parent)
		{
			
			time(&cur_time);
			if(cur_time - growl->timeout > 2000) // 2 secs (i think)
			{
				growl->timeout = cur_time;
				g_print("2 secs UP\n");
			}
			else
			{
				g_print("2 secs NOT UP-%i:%i\n", cur_time, growl->timeout);
			}
			break;
		}		
		lookup = lookup->next;
	}

	// Add a timeout of 2 secs to erase the last message
	if(sbim_timeout)
	{
		g_source_remove(sbim_timeout);
		sbim_timeout = 0;
	}
	sbim_timeout = g_timeout_add(2*1000, sbim_timeout_func, sb);
*/
}

GtkTreeStore *CreateBuddyListModel(YList *buds)
{
	GtkTreeStore 	*model;
	GtkTreeIter 	parent, child;
	char 			cur_group[255];
	GdkPixbuf 		*icon, *m_group;
	
	// TODO: Making assumption here that all friends in one group appear
	// consecutively
	model = gtk_tree_store_new(BUDDY_LIST_NUM, G_TYPE_STRING, 
		G_TYPE_BOOLEAN, G_TYPE_STRING, GDK_TYPE_PIXBUF);
	
	icon = create_pixbuf("offline.png");
//	m_group = create_pixbuf("group.png");
	
	for(; buds; buds = buds->next)
	{
		yahoo_account *bud = buds->data;
		if(strncmp(cur_group, bud->group, 255) != 0) // Create new group
		{
			strncpy(cur_group, bud->group, 255);
			// Add group
			gtk_tree_store_append(model, &parent, NULL);
			gtk_tree_store_set(model, &parent,
				BUDDY_LIST_NAME, bud->group, 
				BUDDY_LIST_BOLD, TRUE, 
				BUDDY_LIST_STAT, "", -1);
		}
		// Add buddy
		gtk_tree_store_append(model, &child, &parent);
		gtk_tree_store_set(model, &child,
			BUDDY_LIST_NAME, bud->yahoo_id, 
			BUDDY_LIST_BOLD, FALSE, 
			BUDDY_LIST_STAT, "Offline", 
			BUDDY_LIST_ICON, icon,-1);
	}
	g_object_unref(icon);
//	g_object_unref(m_group);
	return model;
}


void CreateBuddyList(YList *buds)
{
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkTreeView *tv;
	GtkTreeStore *model;
	
	model = CreateBuddyListModel(buds);
	tv = GTK_TREE_VIEW(lookup_widget(winMain,"tvBuddyList"));
	
	// Column 0 - PIXMAP
	col = gtk_tree_view_column_new();
	gtk_tree_view_append_column(tv, col);
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_set_attributes(col, renderer, 
		"pixbuf", BUDDY_LIST_ICON, NULL);
//	g_object_set(renderer, "xpad", 0.0, "ypad", 1, NULL);
	
	// Column 1 - NAME
	col = gtk_tree_view_column_new();
	gtk_tree_view_append_column(tv, col);
	
	renderer = gtk_cell_renderer_text_new();
	
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", BUDDY_LIST_NAME);
	gtk_tree_view_column_add_attribute(col, renderer, "weight-set", BUDDY_LIST_BOLD);
	g_object_set(renderer, "weight", PANGO_WEIGHT_BOLD, 
//		"xpad", 4, "ypad", 0, 
		NULL); 
	
	// Column 2 - STATUS
	col = gtk_tree_view_column_new();
	gtk_tree_view_append_column(tv, col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", BUDDY_LIST_STAT);
	gtk_tree_view_column_add_attribute(col, renderer, "weight-set", BUDDY_LIST_BOLD);
	g_object_set(renderer, "weight", PANGO_WEIGHT_BOLD,
//		"ypad", 0, "yalign", 0.5,
		NULL);
	
	gtk_tree_view_set_model(tv, GTK_TREE_MODEL(model));
	gtk_tree_view_expand_all(tv);
	g_object_unref(model);
}


// Gets the winIM window for bud
// If window not present, creates a new one
GtkWidget *GetBuddyImWindow(char *bud)
{
	struct buddy_im_window_reln *b;
	GtkWidget *winIM, *w;
	char title[256];
	YList *lookup;
	GtkTextBuffer *buf;
	
	lookup = bwr;
	
	for(; lookup; lookup = lookup->next)
	{
		// [PP] I cant think up cool variable names :( Sun Mar 6 17:50:25 2005
		struct buddy_im_window_reln *growl = lookup->data; 
		if(strncmp(growl->yahoo_id, bud, 255) == 0)
		{
			// [PP] Yay! got window. duh. ya, i've been working on this real
			// long without much of a break. and i keep getting emails about
			// prospective russian brides. now, _that's_ a thought >:)
			return growl->window;
		}
	}
	b = y_new0(struct buddy_im_window_reln, 1);
	strncpy(b->yahoo_id, bud, 255);
	winIM = create_winIM();
	sprintf(title, "IM - %s", bud);
	gtk_window_set_title(GTK_WINDOW(winIM), title);
	
	// Create color tags
	w = lookup_widget(winIM, "txtChat");
	buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w));
	gtk_text_buffer_create_tag(buf, "date", "foreground", "grey", NULL);
	gtk_text_buffer_create_tag(buf, "me-user", "foreground", "#336699",
		"weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(buf, "other-user", "foreground", "#993333", 
		"weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(buf, "error", "background", "red", 
		"foreground", "white", NULL);
	gtk_text_buffer_create_tag(buf, "buzz", "foreground", "#339933", 
		"weight", PANGO_WEIGHT_BOLD, NULL);
//	gtk_text_buffer_create_tag(buf, "smiley", "invisible", TRUE, 
//		"invisible-set", TRUE, NULL);
	
	b->window = winIM; // [PP] whoops, almost added w instead of winIM
	bwr = y_list_append(bwr, b);
	return winIM;
}

void FreeBuddyImWindowList()
{
	while(bwr)
	{
		FREE(bwr->data);
		bwr = bwr->next;
		if(bwr)
			FREE(bwr->prev);
	}
}

// Returns buddy from a window pointer
char *GetBuddyFromWindow(GtkWidget *window)
{
	YList *lookup;
	
	lookup = bwr;
	for(; lookup; lookup = lookup->next)
	{
		struct buddy_im_window_reln *growl = lookup->data;
		if(growl->window == window)
		{
			return growl->yahoo_id;
		}
	}
	return NULL;
}

/* Removes the buddy IM relation from bwr
*/
void RemoveBuddyIMRelation(GtkWidget *window)
{
	YList *lookup;
	
	lookup = bwr;
	while(lookup)
	{
		struct buddy_im_window_reln *growl = lookup->data; 
		if(growl->window == window)
		{
			bwr = y_list_remove(bwr, lookup->data);
			break;
		}		
		lookup = lookup->next;
	}
}

void AddSmiley(char *sm, GtkButton *b)
{
	GtkWidget *toplevel;
	GtkTextView *edit;
	GtkTextBuffer *buf;
	GtkTextIter iter;
	
	// TODO: Below code Wrong. how do i get which IM window called winSmiley?
	// Better idea - use a multicolumn list as a dropdown, or a multicolumn menu
/*
	toplevel = gtk_widget_get_toplevel(GTK_WIDGET(b));
	edit = lookup_widget(toplevel, "txtIMSend");
	buf = gtk_text_view_get_buffer(edit);
	gtk_text_buffer_get_end_iter(buf, &iter);
	gtk_text_buffer_insert(buf, &iter, sm, -1);
*/
}


// The parse functions have been copied from freehoo
char *
parse_color_code (char *string)
{
  char *parsedstring;
  int i, j;
  int startflag = 0;
  int size;

  size = (strlen (string) + 1) * sizeof (char);
  if (!(parsedstring = (char *) malloc (size)))
    return NULL;

  for (i = 0, j = 0; string[i]; i++)
    {
      if (startflag)
        {
          if (string[i] == 'm')
            startflag = 0;
        }
      else
        {
          if (string[i] == 27) //27 is equ to ESC code
            startflag = 1;
          else
            parsedstring[j++] = string[i];
        }
    }
  parsedstring[j] = '\0';
  return parsedstring;
}

char *
parse_font_tag (char *string)
{
  char *parsedstring;
  int i, j;
  int startflag = 0;
  int size;

  size = (strlen (string) + 1) * sizeof (char);
  if (!(parsedstring = (char *) malloc (size)))
    return NULL;

  for (i = 0, j = 0; string[i]; i++)
    {
      if (startflag)
        {
          if (string[i] == '>')
            startflag = 0;
        }
      else
        {
          if (string[i] == '<' && strncasecmp (string + i, "<font", 5) == 0)
            startflag = 1;
          else
            parsedstring[j++] = string[i];
        }
    }
  parsedstring[j] = '\0';
  return parsedstring;
}

char *
parse_control_characters (char *string)
{
  char *parsedstring;
  int i, j;
  int size;

  size = (strlen (string) + 1) * sizeof (char);
  if (!(parsedstring = (char *) malloc (size)))
    return NULL;

  for (i = 0, j = 0; string[i]; i++)
    {
          if (!((string[i] > 0) && (string[i] < 32))) // 1 - 31 are control chars
            parsedstring[j++] = string[i];
    }
  parsedstring[j] = '\0';
  return parsedstring;

}

char *
filter_message (char *message)
{
  char *color_parsedstring = NULL;
  char *font_parsedstring = NULL;
  char *control_char_parsedstring = NULL;

  color_parsedstring = parse_color_code (message);

  font_parsedstring = (color_parsedstring) ?
    parse_font_tag (color_parsedstring) : parse_font_tag (message);

  if (color_parsedstring)
    free (color_parsedstring);

  control_char_parsedstring = (font_parsedstring) ?
    parse_control_characters (font_parsedstring) : parse_control_characters (message);

  if (font_parsedstring)
    free (font_parsedstring);

  return ((control_char_parsedstring) ? control_char_parsedstring : message);
}
