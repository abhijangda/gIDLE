AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}
AM_CPPFLAGS = $(DEPS_CFLAGS)

bin_PROGRAMS = gIDLE
gIDLE_SOURCES = src/main.c src/main.h src/codewidget.c src/codewidget.h src/menus.c src/menus.h src/core_funcs.h src/core_funcs.c src/find_dialog.h src/find_dialog.c src/find_replace_dialog.c src/find_replace_dialog.h src/reg_exp_dialog.c src/reg_exp_dialog.h src/bookmark.h src/bookmark.c src/go_to_line_dialog.h src/go_to_line_dialog.c src/toolbar.h src/toolbar.c src/line_history_item.h src/line_history_item.c src/py_class.c src/py_func.c src/py_class.h src/py_func.h
gIDLE_CCFLAGS =   `pkg-config --cflags --libs gtk+-2.0 gtksourceview-2.0 vte`
gIDLE_LDADD = $(DEPS_LIBS) -lpthread
dist_noinst_SCRIPTS = autogen.sh
