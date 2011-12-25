/*  Copyright (C) 2010 Roberto Guido <madbob@users.barberaware.org>
 *
 *  This file is part of Maintainr
 *
 *  Maintainr is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "maintainr-projectbox.h"
#include "maintainr-todo.h"
#include "maintainr-icons.h"

#define MAINTAINR_PROJECTBOX_GET_PRIVATE(obj)       (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MAINTAINR_PROJECTBOX_TYPE, MaintainrProjectboxPrivate))

struct _MaintainrProjectboxPrivate {
	MaintainrProjectconf *conf;

	GtkWidget *label;
	GtkWidget *todos;
	GtkWidget *services_buttons;

	GtkWidget *priority_icon;
	GtkWidget *project_name;
	GtkWidget *priority;
	GtkWidget *services_confs;
};

enum {
	MOVE_TOP,
	CONF_CHANGE,
	DELETE_PROJECT,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL];

G_DEFINE_TYPE (MaintainrProjectbox, maintainr_projectbox, GTK_TYPE_NOTEBOOK);

static void require_top (GtkButton *button, MaintainrProjectbox *item)
{
	g_signal_emit (item, signals [MOVE_TOP], 0, NULL);
}

static void show_config (MaintainrProjectbox *item)
{
	gtk_notebook_set_current_page (GTK_NOTEBOOK (item), 1);
}

static void show_main (MaintainrProjectbox *item)
{
	gtk_notebook_set_current_page (GTK_NOTEBOOK (item), 0);
}

static void delete_project (GtkButton *button, MaintainrProjectbox *item)
{
	g_signal_emit (item, signals [DELETE_PROJECT], 0, NULL);
}

static void hide_and_show_services (MaintainrProjectbox *item)
{
	GList *buttons;
	GList *services;
	GList *iter_b;
	GList *iter_s;

	buttons = gtk_container_get_children (GTK_CONTAINER (item->priv->services_buttons));
	services = maintainr_projectconf_get_services (item->priv->conf);

	for (iter_b = buttons, iter_s = services; iter_b && iter_s; iter_b = iter_b->next, iter_s = iter_s->next) {
		if (maintainr_service_get_active (iter_s->data))
			gtk_widget_show (iter_b->data);
		else
			gtk_widget_hide (iter_b->data);
	}
}

static void set_priority_icon (MaintainrProjectbox *box, PROJECT_PRIORITY priority)
{
	switch (priority) {
		case PROJECT_PRIORITY_HIGH:
			gtk_image_set_from_pixbuf (GTK_IMAGE (box->priv->priority_icon),
						   gdk_pixbuf_new_from_xpm_data (red_xpm));
			break;

		case PROJECT_PRIORITY_MEDIUM:
			gtk_image_set_from_pixbuf (GTK_IMAGE (box->priv->priority_icon),
						   gdk_pixbuf_new_from_xpm_data (yellow_xpm));
			break;

		case PROJECT_PRIORITY_LOW:
		default:
			gtk_image_set_from_pixbuf (GTK_IMAGE (box->priv->priority_icon),
						   gdk_pixbuf_new_from_xpm_data (green_xpm));
			break;
	}
}

static void save_conf (GtkButton *button, MaintainrProjectbox *item)
{
	GList *iter;

	gtk_label_set_text (GTK_LABEL (item->priv->label), gtk_entry_get_text (GTK_ENTRY (item->priv->project_name)));
	set_priority_icon (item, gtk_combo_box_get_active (GTK_COMBO_BOX (item->priv->priority)));
	show_main (item);

	for (iter = maintainr_projectconf_get_services (item->priv->conf); iter; iter = iter->next)
		maintainr_service_config_saved (iter->data);

	hide_and_show_services (item);
	g_signal_emit (item, signals [CONF_CHANGE], 0, NULL);
}

static void todo_check_changed (GtkCellRendererToggle *cell_renderer, gchar *path, MaintainrProjectbox *item)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (item->priv->todos));
	gtk_tree_model_get_iter_from_string (model, &iter, path);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, !gtk_cell_renderer_toggle_get_active (cell_renderer), -1);

	g_signal_emit (item, signals [CONF_CHANGE], 0, NULL);
}

static void todo_string_changed (GtkCellRendererText *renderer, gchar *path, gchar *new_text, MaintainrProjectbox *item)
{
	int pos;
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (item->priv->todos));
	gtk_tree_model_get_iter_from_string (model, &iter, path);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, 1, new_text, -1);

	gtk_tree_model_get (model, &iter, 2, &pos, -1);
	if (pos == -1)
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2, G_MAXINT, -1);

	g_signal_emit (item, signals [CONF_CHANGE], 0, NULL);
}

static void add_todo (MaintainrProjectbox *item)
{
	GtkTreeModel *model;
	GtkTreeView *todos;
	GtkTreeIter iter;
	GtkTreePath *path;

	todos = GTK_TREE_VIEW (item->priv->todos);
	model = gtk_tree_view_get_model (todos);
	gtk_list_store_insert_with_values (GTK_LIST_STORE (model), &iter, G_MAXINT, 0, FALSE, 1, "Edit me!", 2, -1, -1);

	path = gtk_tree_model_get_path (model, &iter);
	gtk_widget_grab_focus (item->priv->todos);
	gtk_tree_view_set_cursor (todos, path, gtk_tree_view_get_column (todos, 1), TRUE);
	gtk_tree_path_free (path);
}

static void remove_todo (MaintainrProjectbox *item)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (item->priv->todos));
	if (gtk_tree_selection_get_selected (selection, &model, &iter) == TRUE) {
		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
		g_signal_emit (item, signals [CONF_CHANGE], 0, NULL);
	}
}

static void activate_service (GtkButton *button, MaintainrProjectbox *item)
{
	int index;
	GList *buttons;
	GList *iter;

	buttons = gtk_container_get_children (GTK_CONTAINER (item->priv->services_buttons));

	for (iter = buttons, index = 0; iter; iter = iter->next, index++)
		if (GTK_WIDGET (button) == iter->data)
			break;

	g_list_free (buttons);
	gtk_notebook_set_current_page (GTK_NOTEBOOK (item), index + 2);
}

static void maintainr_projectbox_finalize (GObject *object)
{
}

static void maintainr_projectbox_class_init (MaintainrProjectboxClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = maintainr_projectbox_finalize;

	signals [MOVE_TOP] = g_signal_newv ("move-top",
				G_TYPE_FROM_CLASS (gobject_class), G_SIGNAL_RUN_LAST,
				NULL, NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE, 0, NULL);

	signals [CONF_CHANGE] = g_signal_newv ("conf-changed",
				G_TYPE_FROM_CLASS (gobject_class), G_SIGNAL_RUN_LAST,
				NULL, NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE, 0, NULL);

	signals [DELETE_PROJECT] = g_signal_newv ("delete-project",
				G_TYPE_FROM_CLASS (gobject_class), G_SIGNAL_RUN_LAST,
				NULL, NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE, 0, NULL);

	g_type_class_add_private (klass, sizeof (MaintainrProjectboxPrivate));
}

static GtkWidget* do_head (MaintainrProjectbox *item)
{
	GtkWidget *hbox;
	GtkWidget *button;

	hbox = gtk_hbox_new (FALSE, 0);

	item->priv->priority_icon = gtk_image_new_from_pixbuf (gdk_pixbuf_new_from_xpm_data (yellow_xpm));
	gtk_box_pack_start (GTK_BOX (hbox), item->priv->priority_icon, FALSE, FALSE, 0);

	item->priv->label = gtk_label_new ("Untitled");
	gtk_box_pack_start (GTK_BOX (hbox), item->priv->label, TRUE, TRUE, 0);

	button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_EDIT, GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text (button, "Edit the project properties");
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (show_config), item);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

	button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_GOTO_TOP, GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text (button, "Move this project on top of the priority queue");
	g_signal_connect (button, "clicked", G_CALLBACK (require_top), item);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

	return hbox;
}

static gboolean unselect_all_todos (GtkWidget *widget, GdkEventFocus *event, gpointer userdata)
{
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
	gtk_tree_selection_unselect_all (selection);
	return FALSE;
}

static gboolean edit_todo_shortcuts (GtkWidget *widget, GdkEventKey *event, MaintainrProjectbox *item)
{
	if (event->keyval == GDK_KEY_plus) {
		add_todo (item);
		return TRUE;
	}
	else if (event->keyval == GDK_KEY_minus) {
		remove_todo (item);
		return TRUE;
	}

	return FALSE;
}

static void check_if_sorted (GtkWidget *widget, GdkDragContext *drag_context, MaintainrProjectbox *item)
{
	gchar *text;
	GList *cursor;
	GList *todos;
	GList *new_todos;
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (item->priv->todos));
	todos = maintainr_projectconf_get_todos (item->priv->conf);
	new_todos = NULL;

	if (gtk_tree_model_get_iter_first (model, &iter) == TRUE) {
		do {
			gtk_tree_model_get (model, &iter, 1, &text, -1);

			for (cursor = todos; cursor; cursor = cursor->next) {
				if (strcmp (maintainr_todo_get_string (cursor->data), text) == 0) {
					todos = g_list_remove_link (todos, cursor);
					new_todos = g_list_concat (new_todos, cursor);
					break;
				}
			}

			g_free (text);

		} while (gtk_tree_model_iter_next (model, &iter) == TRUE);
	}

	maintainr_projectconf_sort_todos (item->priv->conf, new_todos);
	g_signal_emit (item, signals [CONF_CHANGE], 0, NULL);
}

static GtkWidget* do_todos (MaintainrProjectbox *item)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkListStore *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	GtkTreeSelection *selection;

	hbox = gtk_hbox_new (FALSE, 0);

	model = gtk_list_store_new (3, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_INT);
	item->priv->todos = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	gtk_box_pack_start (GTK_BOX (hbox), item->priv->todos, TRUE, TRUE, 0);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (item->priv->todos), FALSE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW (item->priv->todos), TRUE);

	renderer = gtk_cell_renderer_toggle_new ();
	gtk_cell_renderer_toggle_set_activatable (GTK_CELL_RENDERER_TOGGLE (renderer), TRUE);
	col = gtk_tree_view_column_new_with_attributes ("Check", renderer, "active", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (item->priv->todos), col);
	g_signal_connect (renderer, "toggled", G_CALLBACK (todo_check_changed), item);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "editable", TRUE, NULL);
	col = gtk_tree_view_column_new_with_attributes ("String", renderer, "text", 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (item->priv->todos), col);
	g_signal_connect (renderer, "edited", G_CALLBACK (todo_string_changed), item);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (item->priv->todos));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	g_signal_connect (item->priv->todos, "realize", G_CALLBACK (activate_focus_management), NULL);
	g_signal_connect (item->priv->todos, "focus-out-event", G_CALLBACK (unselect_all_todos), NULL);
	g_signal_connect (item->priv->todos, "key-press-event", G_CALLBACK (edit_todo_shortcuts), item);

	vbox = gtk_vbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

	button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text (button, "Add a new todo item to the list (+)");
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (add_todo), item);
	gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

	button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_REMOVE, GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text (button, "Remove the selected todo item from the list (-)");
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (remove_todo), item);
	gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

	return hbox;
}

static GtkWidget* do_buttons (MaintainrProjectbox *item)
{
	item->priv->services_buttons = gtk_hbox_new (TRUE, 0);
	return item->priv->services_buttons;
}

static GtkWidget* do_config (MaintainrProjectbox *item)
{
	GtkWidget *vbox;
	GtkWidget *table;
	GtkWidget *hbox;
	GtkWidget *button;

	vbox = gtk_vbox_new (FALSE, 10);

	table = gtk_table_new (3, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 10);
	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	item->priv->project_name = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (item->priv->project_name), "Untitled");
	gtk_table_attach_defaults (GTK_TABLE (table), gtk_label_new ("Name"), 0, 1, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (table), item->priv->project_name, 1, 2, 0, 1);

	item->priv->priority = gtk_combo_box_text_new ();
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (item->priv->priority), NULL, "High");
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (item->priv->priority), NULL, "Medium");
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (item->priv->priority), NULL, "Low");
	gtk_table_attach_defaults (GTK_TABLE (table), gtk_label_new ("Priority"), 0, 1, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE (table), item->priv->priority, 1, 2, 1, 2);

	item->priv->services_confs = gtk_notebook_new ();
	gtk_box_pack_start (GTK_BOX (vbox), item->priv->services_confs, TRUE, TRUE, 0);

	hbox = gtk_hbox_new (TRUE, 10);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 10);

	button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_DELETE, GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text (button, "Remove this project from the list");
	g_signal_connect (button, "clicked", G_CALLBACK (delete_project), item);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 10);

	button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text (button, "Save settings and back to the main screen");
	g_signal_connect (button, "clicked", G_CALLBACK (save_conf), item);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 10);

	button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_CANCEL, GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text (button, "Back to the main screen without saving");
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (show_main), item);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 10);

	return vbox;
}

static void maintainr_projectbox_init (MaintainrProjectbox *item)
{
	GtkWidget *mainbox;

	item->priv = MAINTAINR_PROJECTBOX_GET_PRIVATE (item);

	gtk_container_set_border_width (GTK_CONTAINER (item), 20);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (item), FALSE);

	mainbox = gtk_vbox_new (FALSE, 0);
	gtk_notebook_append_page (GTK_NOTEBOOK (item), mainbox, NULL);

	gtk_box_pack_start (GTK_BOX (mainbox), do_head (item), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (mainbox), do_todos (item), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (mainbox), do_buttons (item), FALSE, FALSE, 0);

	gtk_notebook_append_page (GTK_NOTEBOOK (item), do_config (item), NULL);
}

GtkWidget* maintainr_projectbox_new ()
{
	return g_object_new (MAINTAINR_PROJECTBOX_TYPE, NULL);
}

void maintainr_projectbox_set_editing_mode (MaintainrProjectbox *box)
{
	show_config (box);
}

static GtkWidget* do_service_action_panel (MaintainrProjectbox *box, MaintainrService *service)
{
	GtkWidget *frame;
	GtkWidget *panel;
	GtkWidget *buttons;
	GtkWidget *button;

	frame = gtk_vbox_new (FALSE, 0);
	panel = maintainr_service_action_panel (service);
	gtk_box_pack_start (GTK_BOX (frame), panel, TRUE, TRUE, 0);

	buttons = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (frame), buttons, FALSE, FALSE, 0);

	button = maintainr_service_action_buttons (service);
	if (button != NULL)
		gtk_box_pack_start (GTK_BOX (buttons), button, TRUE, TRUE, 0);
	else
		gtk_box_pack_start (GTK_BOX (buttons), gtk_label_new (maintainr_service_get_name (service)), TRUE, TRUE, 0);

	button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_CANCEL, GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text (button, "Back to the main screen");
	g_signal_connect_swapped (button, "clicked", G_CALLBACK (show_main), box);
	gtk_box_pack_start (GTK_BOX (buttons), button, TRUE, TRUE, 0);

	return frame;
}

void maintainr_projectbox_set_conf (MaintainrProjectbox *box, MaintainrProjectconf *conf)
{
	int pos;
	const gchar *name;
	GList *iter;
	GtkWidget *button;
	GtkTreeModel *model;
	MaintainrTodo *todo;
	MaintainrService *service;

	box->priv->conf = conf;
	name = maintainr_projectconf_get_name (conf);

	if (name != NULL) {
		gtk_label_set_text (GTK_LABEL (box->priv->label), name);
		gtk_entry_set_text (GTK_ENTRY (box->priv->project_name), name);
	}

	pos = maintainr_projectconf_get_priority (conf);
	gtk_combo_box_set_active (GTK_COMBO_BOX (box->priv->priority), pos);
	set_priority_icon (box, pos);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (box->priv->todos));

	for (pos = 0, iter = maintainr_projectconf_get_todos (conf); iter; pos++, iter = iter->next) {
		todo = iter->data;
		gtk_list_store_insert_with_values (GTK_LIST_STORE (model), NULL, G_MAXINT,
						   0, maintainr_todo_get_done (todo),
						   1, maintainr_todo_get_string (todo),
						   2, pos, -1);
	}

	for (iter = maintainr_projectconf_get_services (conf); iter; iter = iter->next) {
		service = iter->data;
		name = maintainr_service_get_name (service);

		gtk_notebook_append_page (GTK_NOTEBOOK (box->priv->services_confs), maintainr_service_config_panel (service), gtk_label_new (name));
		gtk_notebook_append_page (GTK_NOTEBOOK (box), do_service_action_panel (box, service), NULL);

		button = gtk_button_new_with_label (name);
		gtk_widget_set_no_show_all (button, TRUE);
		gtk_box_pack_start (GTK_BOX (box->priv->services_buttons), button, TRUE, TRUE, 0);
		g_signal_connect (button, "clicked", G_CALLBACK (activate_service), box);

		if (maintainr_service_get_active (service))
			gtk_widget_show (button);
		else
			gtk_widget_hide (button);

		g_signal_connect_swapped (service, "require-main-screen", G_CALLBACK (show_main), box);
	}

	if (g_signal_handler_find (box->priv->todos, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, G_CALLBACK (check_if_sorted), box) == 0)
		g_signal_connect (box->priv->todos, "drag-end", G_CALLBACK (check_if_sorted), box);

	gtk_widget_show_all (GTK_WIDGET (box));
}

MaintainrProjectconf* maintainr_projectbox_get_conf (MaintainrProjectbox *box)
{
	gboolean done;
	gchar *content;
	GList *todos;
	GtkTreeModel *model;
	GtkTreeIter iter;
	MaintainrTodo *todo;

	maintainr_projectconf_set_name (box->priv->conf, (gchar*) gtk_entry_get_text (GTK_ENTRY (box->priv->project_name)));
	maintainr_projectconf_set_priority (box->priv->conf, gtk_combo_box_get_active (GTK_COMBO_BOX (box->priv->priority)));

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (box->priv->todos));
	todos = NULL;

	if (gtk_tree_model_get_iter_first (model, &iter) == TRUE) {
		do {
			gtk_tree_model_get (model, &iter, 0, &done, 1, &content, -1);
			todo = maintainr_todo_new ();
			maintainr_todo_set_done (todo, done);
			maintainr_todo_set_string (todo, content);
			g_free (content);

			todos = g_list_prepend (todos, todo);

		} while (gtk_tree_model_iter_next (model, &iter) == TRUE);
	}

	if (todos != NULL)
		todos = g_list_reverse (todos);

	maintainr_projectconf_set_todos (box->priv->conf, todos);
	return box->priv->conf;
}
