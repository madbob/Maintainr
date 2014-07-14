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

#ifndef MAINTAINR_SHELL_H
#define MAINTAINR_SHELL_H

#include "common.h"
#include "maintainr-config.h"

#define MAINTAINR_SHELL_TYPE		(maintainr_shell_get_type ())
#define MAINTAINR_SHELL(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj),	\
					 MAINTAINR_SHELL_TYPE,			\
					 MaintainrShell))
#define MAINTAINR_SHELL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass),	\
					 MAINTAINR_SHELL_TYPE,			\
					 MaintainrShellClass))
#define IS_MAINTAINR_SHELL(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj),	\
					 MAINTAINR_SHELL_TYPE))
#define IS_MAINTAINR_SHELL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass),	\
					 MAINTAINR_SHELL_TYPE))
#define MAINTAINR_SHELL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj),	\
					 MAINTAINR_SHELL_TYPE,			\
					 MaintainrShellClass))

typedef struct _MaintainrShell		MaintainrShell;
typedef struct _MaintainrShellClass	MaintainrShellClass;
typedef struct _MaintainrShellPrivate	MaintainrShellPrivate;

struct _MaintainrShell {
	GtkBox			parent;
	MaintainrShellPrivate	*priv;
};

struct _MaintainrShellClass {
	GtkBoxClass		parent_class;
};

GType		maintainr_shell_get_type ();

GtkWidget*	maintainr_shell_new ();
void		maintainr_shell_set_config (MaintainrShell *shell, MaintainrConfig *conf);
GtkAccelGroup*	maintainr_shell_get_shortcuts (MaintainrShell *shell);

#endif
