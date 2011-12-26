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

#include "common.h"
#include "maintainr-config.h"
#include "maintainr-shell.h"

int main (int argc, char **argv) {
	GtkWidget *window;
	GtkWidget *shell;
	MaintainrConfig *config;

	gtk_init (&argc, &argv);
	g_set_application_name ("Maintainr");

	g_log_set_always_fatal (G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL);

	maintainr_config_handle_backup ();
	config = maintainr_config_read_configuration ();
	if (config == NULL)
		exit (1);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 300, 500);
	g_signal_connect (window, "delete-event", G_CALLBACK (gtk_main_quit), NULL);

	shell = maintainr_shell_new ();
	gtk_container_add (GTK_CONTAINER (window), shell);
	maintainr_shell_set_config (MAINTAINR_SHELL (shell), config);
	gtk_window_add_accel_group (GTK_WINDOW (window), maintainr_shell_get_shortcuts (MAINTAINR_SHELL (shell)));

	gtk_widget_show_all (window);
	gtk_main ();

	exit (0);
}
