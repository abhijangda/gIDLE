AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}
AM_CPPFLAGS = $(DEPS_CFLAGS)

bin_PROGRAMS = gIDLE
gIDLE_SOURCES = src/main.c src/main.h src/codewidget.c src/codewidget.h src/menus.c src/menus.h src/core_funcs.h src/core_funcs.c src/find_dialog.h src/find_dialog.c src/find_replace_dialog.c src/find_replace_dialog.h src/reg_exp_dialog.c src/reg_exp_dialog.h src/bookmark.h src/bookmark.c src/go_to_line_dialog.h src/go_to_line_dialog.c src/toolbar.h src/toolbar.c src/line_history_item.h src/line_history_item.c src/py_class.c src/py_func.c src/py_class.h src/py_func.h src/code_folding_widget.h src/code_folding_widget.c src/pty_fork.c src/pty_fork.h src/python_shell.c src/python_shell.h src/run_script_dialog.h src/run_script_dialog.c src/py_variable.c src/py_variable.h src/code_assists.h src/code_assists.c src/py_module.h src/py_module.c src/codewidget_parser.c src/line_iterator.h src/line_iterator.c src/py_static_var.c src/py_static_var.h src/class_browser.c src/class_browser.h src/project_class.c src/project_class.h src/new_proj_dlg.h src/new_proj_dlg.c
gIDLE_LDADD = $(DEPS_LIBS) -lpthread
dist_noinst_SCRIPTS = autogen.sh
