#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <time.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "functions.h"

#include <yahoo2.h>
#include <yahoo2_callbacks.h>
#include "yahoo.h"
#include "yahoo_utils.h"

extern GtkWidget *winMain;
extern yahoo_local_account *ylad;


void
on_winMain_show                        (GtkWidget       *widget,
                                        gpointer         user_data)
{
	// Register yahoo callbacks
	register_callbacks();
	SetMainWidgetsEnabled(FALSE);
	on_mnuMessengerConnect_activate (NULL, NULL);
}


void
on_mnuMessengerConnect_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *login, *w;
	int retval;
	gchar *ptr;
	gboolean invi;
	
	login = create_dlgLogin();
	retval = gtk_dialog_run(GTK_DIALOG(login));
	
	if(retval == GTK_RESPONSE_OK)
	{
		ylad = y_new0(yahoo_local_account, 1);
		
		w = lookup_widget(login, "txtUsername");
		ptr = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);
		strncpy(ylad->yahoo_id, ptr, 255);
		g_free(ptr);
		w = lookup_widget(login, "txtPassword");
		ptr = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);
		strncpy(ylad->password, ptr, 255);
		g_free(ptr);
		w = lookup_widget(login, "chkInvisible");
		
		invi = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
		
		// Call yahoo_login
		ext_yahoo_login(ylad, invi ?
			YAHOO_STATUS_INVISIBLE : YAHOO_STATUS_AVAILABLE);
		SetMainStatusbar("Connecting to Yahoo ...");
		w = lookup_widget(winMain, "cboStatus");
		gtk_combo_box_set_active(GTK_COMBO_BOX(w), invi? 5 : 0);
	}
	gtk_widget_destroy(login);
}


void
on_mnuDisconnect_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ext_yahoo_logout();
	SetMainStatusbar("Disconnected");
	SetMainWidgetsEnabled(FALSE);
}


void
on_mnuPreferences_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_mnuMessengerQuit_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ext_yahoo_logout();
	gtk_main_quit();
}


void
on_mnuHelpAbout_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *about;
	about = create_dlgAbout();
	gtk_dialog_run(GTK_DIALOG(about));
	gtk_widget_destroy(about);
}


void
on_butSendIM_clicked                   (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
	GtkTreeView *treeview;
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	GtkWidget *d_sendim, *im, *edit;
	gchar *bud;
	int retval;
	
	treeview = GTK_TREE_VIEW(lookup_widget(winMain, "tvBuddyList"));
	selection = gtk_tree_view_get_selection(treeview);
	if(selection)
	{
		gtk_tree_selection_get_selected(selection, &model, &iter);
	
		if(!gtk_tree_model_iter_has_child(model,&iter))
		{
			path = gtk_tree_model_get_path(model, &iter);
			on_tvBuddyList_row_activated(treeview, path, NULL, NULL);
			return;
		}
	}

	// Show the SendIM dialog	
	d_sendim = create_dlgSendIM();
	retval = gtk_dialog_run(GTK_DIALOG(d_sendim));
	
	if(retval == GTK_RESPONSE_OK)
	{
		// Get name of person to send msg to
		edit = lookup_widget(d_sendim, "txtTargetID");
		bud = gtk_editable_get_chars(GTK_EDITABLE(edit), 0, -1);
		// Create the IM window
		im = GetBuddyImWindow(bud);
		gtk_widget_show(im);
		g_free(bud);
	}
	gtk_widget_destroy(d_sendim);
}


void
on_tbAddBuddy_clicked                  (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
	GtkWidget *dlg, *w;
	int retval;
	gchar *bud, *group, *msg;
	GtkTreeIter iter;
	GtkTreeModel *model;
	
	dlg = create_dlgAddBuddy();
	retval = gtk_dialog_run(GTK_DIALOG(dlg));
	if (retval == GTK_RESPONSE_OK)
	{
		w = lookup_widget(dlg, "txtYahooID");
		bud = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);
		w = lookup_widget(dlg, "cboeGroup");
		gtk_combo_box_get_active_iter(GTK_COMBO_BOX(w), &iter);
		model = gtk_combo_box_get_model(GTK_COMBO_BOX(w));
		group = gtk_tree_model_get_string_from_iter(&model, &iter);
		w = lookup_widget(dlg, "txtMessage");
		msg = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);
		yahoo_add_buddy(ylad->id, bud, group, msg);
		g_free(bud); g_free(group); g_free(msg);
	}
	gtk_widget_destroy(dlg);
}


void
on_cboStatus_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
	int sel, retval;
	GtkWidget *dlg, *w;
	char *msg;
	int m_status;
	
	sel = gtk_combo_box_get_active(combobox);
	switch(sel)
	{
		case 0: // Available
			yahoo_set_away(ylad->id, YAHOO_STATUS_AVAILABLE, NULL, 0);
			break;
		case 1: // Be right back
			yahoo_set_away(ylad->id, YAHOO_STATUS_BRB, NULL, 0);
			break;
		case 2: // Busy
			yahoo_set_away(ylad->id, YAHOO_STATUS_BUSY, NULL, 0);
			break;
		case 3: // Not at desk
			yahoo_set_away(ylad->id, YAHOO_STATUS_NOTATDESK, NULL, 0);
			break;
		case 4: // On the phone
			yahoo_set_away(ylad->id, YAHOO_STATUS_ONPHONE, NULL, 0);
			break;
		case 5: // Invisible
			yahoo_set_away(ylad->id, YAHOO_STATUS_INVISIBLE, NULL, 0);
			break;
		case 6: // Custom
			dlg = create_dlgCustomAway();
			retval = gtk_dialog_run(GTK_DIALOG(dlg));
			if(retval == GTK_RESPONSE_OK)
			{
				w = lookup_widget(dlg, "txtAwayMessage");
				msg = gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);
				w = lookup_widget(dlg, "chkShowBusy");
				m_status = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
				yahoo_set_away(ylad->id, YAHOO_STATUS_CUSTOM, msg, m_status);
			}
			g_free(msg);
			gtk_widget_destroy(dlg);
			break;
	}
}

gboolean
on_txtIMSend_key_press_event           (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
	GtkWidget *parent;
	GtkButton *button;
	
	parent = gtk_widget_get_toplevel(GTK_WIDGET(widget));
	
	// multiline entry
	if(event->keyval == GDK_Return && event->state != GDK_CONTROL_MASK)
	{
		button = lookup_widget(parent, "butIMSend");
		on_butIMSend_clicked(button, NULL);
		return TRUE;
	}
	if(event->keyval == GDK_G && event->state == GDK_CONTROL_MASK) // BUZZ
	{	
		on_butIMBuzz_clicked(GTK_BUTTON(lookup_widget(parent,"butIMBuzz")),
			NULL);
		return TRUE;
	}
	return FALSE;
}

void
on_tvBuddyList_row_activated           (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{
	GtkWidget *im;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *bud;
	
	model = gtk_tree_view_get_model(treeview);
	if(gtk_tree_model_get_iter(model, &iter, path))
	{
		if(!gtk_tree_model_iter_has_child(model,&iter)) // If not a group
		{
			gtk_tree_model_get(model, &iter, BUDDY_LIST_NAME, &bud, -1);
			im = GetBuddyImWindow(bud);
			gtk_widget_show(im);
			g_free(bud);
		}
	}
}

void
on_butIMSend_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *dest, *src, *parent;
	GtkTextIter iter, start, end;
	GtkTextBuffer *buf_dest, *buf_src;
	GtkTextMark *mark;
	gchar *ptr, *target;
	struct tm mytime;
	int hour, min, sec;
	gchar timestamp[128];
	gchar *u_timestamp;
	time_t tm;
	gchar *title1, title2[256];

	parent = gtk_widget_get_toplevel(GTK_WIDGET(button));
	dest = lookup_widget(parent, "txtChat");
	buf_dest = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dest));
	gtk_text_buffer_get_end_iter(buf_dest, &iter);
	
	// Look up the bottom textview and clear it
	src = lookup_widget(parent, "txtIMSend");
	buf_src = gtk_text_view_get_buffer(GTK_TEXT_VIEW(src));
	gtk_text_buffer_get_start_iter(buf_src, &start);
	gtk_text_buffer_get_end_iter(buf_src, &end);
	ptr = gtk_text_buffer_get_text(buf_src, &start, &end, FALSE);
	gtk_text_buffer_set_text(buf_src, "", -1);
	
	// Add timestamp to chat area
	time(&tm);
	mytime = *localtime(&tm);
	sec = mytime.tm_sec;
	min = mytime.tm_min;
	hour = mytime.tm_hour;
	sprintf(timestamp, "(%2.2i:%2.2i:%2.2i) ", hour, min, sec);
	
	u_timestamp = y_str_to_utf8(timestamp);
	gtk_text_buffer_insert_with_tags_by_name(buf_dest, &iter, u_timestamp, -1, 
		"me-user", NULL); // [PP] me-user. hrrmph! cant i think of cool names?
	g_free(u_timestamp);
	
	// Add the contents to the chat area
	gtk_text_buffer_insert_with_tags_by_name(buf_dest, &iter, ylad->yahoo_id, -1, 
		"me-user" , NULL);
	gtk_text_buffer_insert_with_tags_by_name(buf_dest, &iter, ": ", -1, "me-user" , NULL);
	
	// TODO: Parse this string for smileys and replace smiley with pixbuf
	gtk_text_buffer_insert(buf_dest, &iter, ptr, -1);
	
	gtk_text_buffer_insert(buf_dest, &iter, "\n", -1);
	// Scroll chat area
	mark = gtk_text_buffer_create_mark(buf_dest, mark, &iter, FALSE);
	gtk_text_view_scroll_mark_onscreen(dest, mark);
	gtk_text_buffer_delete_mark(buf_dest, mark);
	
	// Send to yahoo
	target = GetBuddyFromWindow(parent);
	yahoo_send_im(ylad->id, NULL, target, ptr, TRUE);
	g_free (ptr);
	
	// Remove the flash
	title1 = gtk_window_get_title(parent);
	if(title1[0] == '*')
	{
		strncpy(title2, title1+1,255);
		gtk_window_set_title(parent,title2);
	}
}

gboolean
on_winIM_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	RemoveBuddyIMRelation(widget);
	return FALSE;
}

gboolean
on_winMain_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	on_mnuMessengerQuit_activate(NULL, NULL);
	return FALSE;
}

void
on_butIMSendFile_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_butImInvite_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_butSmiley_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_butIMFont_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_butIMBuzz_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkStatusbar *sb;
	GtkWidget *parent;
	gchar *target;
	
	parent = gtk_widget_get_toplevel(GTK_WIDGET(button));
	sb = lookup_widget(parent, "sbIM");
	SetIMStatusbar(sb, "Sent BUZZ!!!");
	target = GetBuddyFromWindow(parent);
	yahoo_send_im(ylad->id, NULL, target, "<ding>", TRUE);
}


void
on_butIMAddBuddy_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	// If already buddy, then dont add
	
}

void
on_mnuConversationSave_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	
}

void
on_mnuConversationQuit_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_signal_emit_by_name(
		GTK_OBJECT(gtk_widget_get_parent(GTK_WIDGET(menuitem))), 
		"delete_event");
}

void
on_mnuMessengerRefresh_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	SetMainStatusbar("Refreshing Buddylist");
	yahoo_refresh(ylad->id);
}
