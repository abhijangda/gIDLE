AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}
AM_CPPFLAGS = $(DEPS_CFLAGS)

bin_PROGRAMS = gIDLE shell
gIDLE_SOURCES = src/class_brower.c src/codewidget.c src/codewidget.h src/key_press_event.h src/main.c  src/menus.h src/menus_shell.h src/newprojectwin.c src/options.c src/path_browser.c src/proj_manager.c src/pyclass.c src/pyclass.h src/pymodule.c src/pymodule.h src/string_stack.c src/string_stack.h
gIDLE_LDADD = $(DEPS_LIBS) -lpthread
shell_SOURCES = src/main_shell.c
shell_LDADD = $(DEPS_LIBS) -lpthread
dist_noinst_SCRIPTS = autogen.sh
