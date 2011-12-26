/*  Copyright (C) 2010/2011 Roberto Guido <bob4job@gmail.com>
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

#include "maintainr-shell.h"
#include "maintainr-projectbox.h"

#define MAINTAINR_SHELL_GET_PRIVATE(obj)       (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MAINTAINR_SHELL_TYPE, MaintainrShellPrivate))

struct _MaintainrShellPrivate {
	GtkWidget *projects_box;
	GtkWidget *empty_notification;
	GtkWidget *status;
	MaintainrConfig *conf;
};

G_DEFINE_TYPE (MaintainrShell, maintainr_shell, GTK_TYPE_VBOX);

static void set_empty_screen (MaintainrShell *item)
{
	item->priv->empty_notification = gtk_label_new ("");
	gtk_label_set_markup (GTK_LABEL (item->priv->empty_notification), "There are no projects registered.\nClick the + button to add a new one.");
	gtk_label_set_use_markup (GTK_LABEL (item->priv->empty_notification), TRUE);
	gtk_label_set_justify (GTK_LABEL (item->priv->empty_notification), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start (GTK_BOX (item->priv->projects_box), item->priv->empty_notification, TRUE, TRUE, 10);
	gtk_widget_show (item->priv->empty_notification);
}

static void remove_empty_screen (MaintainrShell *item) {
	if (item->priv->empty_notification != NULL) {
		gtk_container_remove (GTK_CONTAINER (item->priv->projects_box), GTK_WIDGET (item->priv->empty_notification));
		item->priv->empty_notification = NULL;
	}
}

static void set_status (MaintainrShell *shell)
{
	gchar *status;
	int days;
	time_t since;
	GList *projects;
	GtkStatusbar *bar;
	MaintainrProjectconf *top;

	bar = GTK_STATUSBAR (shell->priv->status);

	projects = maintainr_config_get_projects (shell->priv->conf);

	if (projects == NULL) {
		status = "No registered projects";
	}
	else {
		top = projects->data;

		since = maintainr_projectconf_get_top_since (top);
		if (since != 0)
			days = ceil (((double) time (NULL) - (double) since) / (double) 3600);
		else
			days = 0;

		status = g_strdup_printf ("There are %d projects - %s on top since %d days",
					  g_list_length (projects), maintainr_projectconf_get_name (top), days);
	}

	gtk_statusbar_push (bar, gtk_statusbar_get_context_id (bar, status), status);
}

static void move_to_top (MaintainrProjectbox *box, MaintainrShell *item)
{
	MaintainrProjectconf *conf;

	conf = maintainr_projectbox_get_conf (box);
	maintainr_config_force_top (item->priv->conf, conf);
	set_status (item);
	maintainr_config_save (item->priv->conf);
	gtk_box_reorder_child (GTK_BOX (item->priv->projects_box), GTK_WIDGET (box), 0);
}

static void configuration_save_required (MaintainrProjectbox *box, MaintainrShell *item)
{
	MaintainrProjectconf *conf;

	conf = maintainr_projectbox_get_conf (box);
	maintainr_config_add_project (item->priv->conf, conf);
	maintainr_config_save (item->priv->conf);
	set_status (item);
}

static void project_removed (MaintainrProjectbox *box, MaintainrShell *item)
{
	MaintainrProjectconf *conf;

	conf = maintainr_projectbox_get_conf (box);
	maintainr_config_delete_project (item->priv->conf, conf);
	gtk_container_remove (GTK_CONTAINER (item->priv->projects_box), GTK_WIDGET (box));
	maintainr_config_save (item->priv->conf);

	if (maintainr_config_get_projects_num (item->priv->conf) == 0)
		set_empty_screen (item);

	set_status (item);
}

static GtkWidget* do_project_box (MaintainrShell *item)
{
	GtkWidget *box;

	box = maintainr_projectbox_new ();
	g_signal_connect (box, "move-top", G_CALLBACK (move_to_top), item);
	g_signal_connect (box, "conf-changed", G_CALLBACK (configuration_save_required), item);
	g_signal_connect (box, "delete-project", G_CALLBACK (project_removed), item);
	return box;
}

static void add_new_project (GtkButton *button, MaintainrShell *item)
{
	GtkWidget *box;
	GtkWidget *scrolled;

	remove_empty_screen (item);

	box = do_project_box (item);
	maintainr_projectbox_set_conf (MAINTAINR_PROJECTBOX (box), maintainr_projectconf_new ());
	gtk_widget_show_all (box);

	gtk_box_pack_start (GTK_BOX (item->priv->projects_box), box, TRUE, TRUE, 10);
	gtk_box_reorder_child (GTK_BOX (item->priv->projects_box), box, 0);

	/*
		The list of projects is in a viewport, which is in the required scrolled window
	*/
	scrolled = gtk_widget_get_parent (gtk_widget_get_parent (item->priv->projects_box));
	gtk_adjustment_set_value (gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (scrolled)), 0);

	maintainr_projectbox_set_editing_mode (MAINTAINR_PROJECTBOX (box));

	set_status (item);
}

static void skip_next_project (GtkButton *button, MaintainrShell *item)
{
	int index;
	GList *projects;
	GList *boxes;
	GList *piter;
	GList *biter;

	maintainr_config_step_projects (item->priv->conf);

	projects = maintainr_config_get_projects (item->priv->conf);
	boxes = gtk_container_get_children (GTK_CONTAINER (item->priv->projects_box));

	for (piter = projects, index = 0; piter; piter = piter->next, index++) {
		for (biter = boxes; biter; biter = biter->next) {
			if (piter->data == maintainr_projectbox_get_conf (MAINTAINR_PROJECTBOX (biter->data))) {
				gtk_box_reorder_child (GTK_BOX (item->priv->projects_box), biter->data, index);
				break;
			}
		}
	}

	maintainr_config_save (item->priv->conf);
	set_status (item);
}

static void maintainr_shell_finalize (GObject *object)
{
	MaintainrShell *shell;

	shell = MAINTAINR_SHELL (object);
	g_object_unref (shell->priv->conf);
}

static void maintainr_shell_class_init (MaintainrShellClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = maintainr_shell_finalize;

	g_type_class_add_private (klass, sizeof (MaintainrShellPrivate));
}

static void maintainr_shell_init (MaintainrShell *item)
{
	GtkWidget *buttons;
	GtkWidget *scroll;
	GtkWidget *button;

	item->priv = MAINTAINR_SHELL_GET_PRIVATE (item);
	gtk_box_set_spacing (GTK_BOX (item), 5);

	gtk_box_set_homogeneous (GTK_BOX (item), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (item), 0);

	buttons = gtk_hbox_new (TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (buttons), 10);
	gtk_box_pack_start (GTK_BOX (item), buttons, FALSE, FALSE, 10);

	button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text (button, "Create a new project");
	gtk_box_pack_start (GTK_BOX (buttons), button, TRUE, TRUE, 10);
	g_signal_connect (button, "clicked", G_CALLBACK (add_new_project), item);

	button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_INDEX, GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text (button, "Push on top the next project by priority");
	gtk_box_pack_start (GTK_BOX (buttons), button, TRUE, TRUE, 10);
	g_signal_connect (button, "clicked", G_CALLBACK (skip_next_project), item);

	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (item), scroll, TRUE, TRUE, 0);

	item->priv->projects_box = gtk_vbox_new (FALSE, 0);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll), item->priv->projects_box);

	item->priv->status = gtk_statusbar_new ();
	gtk_box_pack_start (GTK_BOX (item), item->priv->status, FALSE, FALSE, 0);
}

GtkWidget* maintainr_shell_new ()
{
	return g_object_new (MAINTAINR_SHELL_TYPE, NULL);
}

void maintainr_shell_set_config (MaintainrShell *shell, MaintainrConfig *conf)
{
	GList *projects;
	GList *iter;
	GtkWidget *box;

	shell->priv->conf = conf;
	projects = maintainr_config_get_projects (conf);

	if (projects == NULL || g_list_length (projects) == 0) {
		set_empty_screen (shell);
	}
	else {
		remove_empty_screen (shell);

		for (iter = projects; iter; iter = iter->next) {
			box = do_project_box (shell);
			maintainr_projectbox_set_conf (MAINTAINR_PROJECTBOX (box), iter->data);
			gtk_box_pack_start (GTK_BOX (shell->priv->projects_box), box, TRUE, TRUE, 10);
		}
	}

	set_status (shell);
}

static gboolean scroll_in_projects (GtkAccelGroup *accel_group, GObject *acceleratable,
				    guint keyval, GdkModifierType modifier, MaintainrShell *shell)
{
	int i;
	GList *boxes;
	GList *iter;

	boxes = gtk_container_get_children (GTK_CONTAINER (shell->priv->projects_box));

	for (i = 0, iter = boxes; iter; i++, iter = iter->next) {
		printf ("%d = %s %s\n", i, gtk_widget_is_focus (iter->data) ? "true" : "false", gtk_widget_has_focus (iter->data) ? "true" : "false");
	}

	printf ("\n");
	g_list_free (boxes);
	return TRUE;
}

GtkAccelGroup* maintainr_shell_get_shortcuts (MaintainrShell *shell)
{
	GtkAccelGroup *group;

	group = gtk_accel_group_new ();
	gtk_accel_group_connect (group, GDK_KEY_Up, GDK_CONTROL_MASK, 0, g_cclosure_new (G_CALLBACK (scroll_in_projects), shell, NULL));
	gtk_accel_group_connect (group, GDK_KEY_Down, GDK_CONTROL_MASK, 0, g_cclosure_new (G_CALLBACK (scroll_in_projects), shell, NULL));
	return group;
}
