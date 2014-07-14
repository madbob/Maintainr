NULL =

INCLUDES = \
	-DPREFIX=\""$(prefix)"\"				\
	-DLIBDIR=\""$(libdir)"\"				\
	-DDATADIR=\""$(datadir)"\"				\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	$(NULL)

common_cflags = \
	-I$(top_srcdir) \
	$(MAINTAINR_CFLAGS) \
	-lm \
	$(NULL)

common_ldadd = \
	$(MAINTAINR_LIBS) \
	$(NULL)

bin_PROGRAMS = maintainr

maintainr_SOURCES = \
	common.c			\
	common.h			\
	main.c				\
	maintainr-config.c		\
	maintainr-config.h		\
	maintainr-icons.h		\
	maintainr-projectbox.c		\
	maintainr-projectbox.h		\
	maintainr-projectconf.c		\
	maintainr-projectconf.h		\
	maintainr-service.c		\
	maintainr-service.h		\
	maintainr-shell.c		\
	maintainr-shell.h		\
	maintainr-todo.c		\
	maintainr-todo.h

maintainr_CFLAGS = $(common_cflags)
maintainr_LDADD = $(common_ldadd)
