// pkg.c
// ---------------------------
// Handle download manager

#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE

#include "pkg.h"
#include "../defines.h"
#include "../trace.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <linux/limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// configures line length
#define PKG_BUFSZ_X 4096

extern GtkWindow *w;

static pkg_list_t pkg_entries;
static GtkWidget *thefuckingbox;
static GtkWidget *header;
static GtkWidget *owndesc;
static GtkWidget *install;
static GtkWidget *removeb;
static GtkWidget *desc;
static GtkWidget *zaghahkngvyjin;
static int csel;
static char cversion[64];
char *s_sel = NULL;

static void pkg_init_list(void) {
  pkg_entries.len = 0;
  pkg_entries.entries = NULL;
}

static inline void pkg_ini_parse(FILE *fstream,
                                 void (*callback)(char *section, char *key,
                                                  char *val)) {
  char *buf = (char *)malloc(PKG_BUFSZ_X * sizeof(char));
  bool isbom = false;
  char section[PKG_BUFSZ_X - 3];
  while (fgets(buf, PKG_BUFSZ_X, fstream) != NULL) {
    buf[strcspn(buf, "\r\n")] = '\0';

    // check for BOM
    if ((unsigned char)buf[0] == 0xEF && (unsigned char)buf[1] == 0xBB &&
        (unsigned char)buf[2] == 0xBF) {
      buf += 3;
      isbom = true;
    }

    // strip trailing spaces
    int len = strlen(buf);
    while (len > 0 && isspace((int)buf[len - 1])) {
      buf[len - 1] = '\0';
      --len;
    }
    // check if comment
    if (buf[0] == ';' || buf[0] == '#') {
      trace_log(DEBUG, "ini: comment: %s\n", buf);
      continue;
    }
    if (strlen(buf) == 0) {
      continue;
    }
    if (buf[0] == '[') {
      if (buf[strlen(buf) - 1] == ']') { // copy section
        strncpy(section, buf + 1, strlen(buf) - 2);
        section[strlen(buf) - 2] = '\0';
      } else
        trace_log(WARN, "ini: malformed sec header\n");
      continue;
    }

    int offset = 0;
    char key[(PKG_BUFSZ_X - 1) / 2];
    char value[(PKG_BUFSZ_X - 1) / 2];
    sscanf(buf, " %2046[^ =] = %n", key, &offset);
    if (offset == 0) {
      trace_log(WARN, " ini: malformed kvp: %s\n", buf);
      continue;
    }
    strncpy(value, buf + offset, sizeof(value) - 1);
    trace_log(DEBUG, "ini: k \"%s\" v \"%s\"\n", key, value);
    callback(section, key, value); // call user provided parser
  }
}

static inline void pkg_list_append(char *name, int status) {
  if (!name)
    return;
  void *tmp = realloc(pkg_entries.entries,
                      (pkg_entries.len + 1) * sizeof(pkg_entries.entries[0]));
  if (tmp == NULL) {
    trace_log(ERROR, "malloc fail");
    return;
  }
  pkg_entries.entries = tmp;
  pkg_entries.entries[pkg_entries.len].pkgname = strdup(name);
  if (pkg_entries.entries[pkg_entries.len].pkgname == NULL) {
    trace_log(ERROR, "strdup fail");
    return;
  }
  pkg_entries.entries[pkg_entries.len].status = status;
  pkg_entries.len++;
}

static inline void pkg_list_free_all(void) {
  for (int i = 0; i < pkg_entries.len; ++i) {
    if (pkg_entries.entries[i].pkgname != NULL)
      free(pkg_entries.entries[i].pkgname);
  }
  free(pkg_entries.entries);
}

static inline int pkg_list_ssbn_safe(char *name, int status) {
  int found = -1;
  for (int i = 0; i < pkg_entries.len; ++i) {
    if (strcmp(pkg_entries.entries[i].pkgname, name) == 0) {
      found = i;
      break;
    }
  }
  if (found == -1)
    return 1; // not found
  pkg_entries.entries[found].status = status;
  return 0;
}

int pkg_populate_list(FILE *wrldf) {
  DIR *pkgdir = opendir("/usr/share/desktop-gremlin-linux/dlpm/recipes");
  struct dirent *entry;

  if (!pkgdir) {
    trace_log(ERROR, "could not open the recipe folder");
    return 1;
  }

  while ((entry = readdir(pkgdir)) != NULL) {
    if (entry->d_type == DT_DIR) {
      struct stat buf;
      char strbuf[PATH_MAX];
      snprintf(strbuf, PATH_MAX, DEGRLI_PKG_PATH "/%s/metadata.ini",
               entry->d_name);
      if (stat(strbuf, &buf) == 0) {
        pkg_list_append(entry->d_name, DEGRLI_PKG_N_INSTALLED);
      }
      // is not a package
    }
  }

  // parse world file
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, wrldf)) != -1) {
    line[strcspn(line, "\r\n")] = '\0';
    int res = pkg_list_ssbn_safe(line, DEGRLI_PKG_INSTALLED);
    if (res != 0)
      trace_log(WARN, "Package %s in world not found", line);
  }

  free(line);

  return 0;
}

// the below used tool/dlpm.py as a reference to get this junk working

// forward decl
static _Bool pkg_is_in_wfile(char *toinstall);

// store context
typedef struct {
  gint refs;
  gboolean cancelled;
  gboolean finalized;
  gboolean error_logged;
  gboolean wait_done;
  gboolean out_done;
  gboolean err_done;
  gboolean exited;
  gboolean success;
  int exit_status;
  GtkWidget *diag;
  GtkWidget *close_button;
  GtkWidget *log_view;
  GtkTextBuffer *buffer;
  GSubprocess *proc;
  GDataInputStream *out;
  GDataInputStream *err;
  char *pkgname;
} pkg_install_ctx_t;

static void pkg_install_ctx_ref(pkg_install_ctx_t *ctx) {
  g_atomic_int_inc(&ctx->refs);
}

// dereference the context stuff
static void pkg_install_ctx_unref(pkg_install_ctx_t *ctx) {
  if (!g_atomic_int_dec_and_test(&ctx->refs))
    return;

  if (ctx->proc != NULL) {
    if (!ctx->wait_done)
      g_subprocess_force_exit(ctx->proc);
    g_object_unref(ctx->proc);
  }
  g_clear_object(&ctx->out);
  g_clear_object(&ctx->err);
  g_clear_object(&ctx->buffer);
  g_free(ctx->pkgname);
  g_free(ctx);
}

static void pkg_install_log(pkg_install_ctx_t *ctx, const char *format, ...) {
  if (ctx == NULL || ctx->cancelled || ctx->buffer == NULL)
    return;

  va_list args;
  va_start(args, format);
  char *msg = g_strdup_vprintf(format, args);
  va_end(args);

  GtkTextIter end;
  gtk_text_buffer_get_end_iter(ctx->buffer, &end);
  gtk_text_buffer_insert(ctx->buffer, &end, msg, -1);
  gtk_text_buffer_insert(ctx->buffer, &end, "\n", -1);

  if (ctx->log_view != NULL) {
    gtk_text_buffer_get_end_iter(ctx->buffer, &end);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(ctx->log_view), &end, 0.0, FALSE,
                                 0.0, 0.0);
  }
  g_free(msg);
}

// add the fucking package only to world file if it isnt
// installed; so basically the dlpm.py behavior
static int pkg_world_add(const char *pkgname) {
  const char *args[] = { "pkexec", DEGRLI_PKG_PSTUB_PATH, "add", pkgname, NULL};
  GError *error = NULL;
  GSubprocess *subprocess = NULL;
  int ret = -1;

  subprocess = g_subprocess_newv(args, G_SUBPROCESS_FLAGS_NONE, &error);
  if (error != NULL) {
    trace_log(ERROR, "Failed to launch options_pstub: %s\n", error->message);
    return -1;
  }
  g_subprocess_wait(subprocess, NULL, &error);
  
  if (error != NULL) {
    trace_log(ERROR, "wait failed: %s\n", error->message);
    g_clear_error(&error);
  } else {
    ret = g_subprocess_get_exit_status(subprocess);
  }

  g_object_unref(subprocess);
  return ret;
}

static int pkg_world_remove(const char *pkgname) {
  const char *args[] = { "pkexec", DEGRLI_PKG_PSTUB_PATH, "remove", pkgname, NULL};
  GError *error = NULL;
  GSubprocess *subprocess = NULL;
  int ret = -1;

  subprocess = g_subprocess_newv(args, G_SUBPROCESS_FLAGS_NONE, &error);
  if (error != NULL) {
    trace_log(ERROR, "Failed to launch options_pstub: %s\n", error->message);
    return -1;
  }
  g_subprocess_wait(subprocess, NULL, &error);
  
  if (error != NULL) {
    trace_log(ERROR, "wait failed: %s\n", error->message);
    g_clear_error(&error);
  } else {
    ret = g_subprocess_get_exit_status(subprocess);
  }

  g_object_unref(subprocess);
  return ret;
}

static void pkg_mark_installed(const char *pkgname) {
  pkg_list_ssbn_safe((char *)pkgname, DEGRLI_PKG_INSTALLED);
  if (install != NULL)
    gtk_button_set_label(GTK_BUTTON(install), "Re-install/Update");
  if (removeb != NULL)
    gtk_widget_set_visible(removeb, TRUE);
}

static void pkg_mark_removed(const char *pkgname) {
  pkg_list_ssbn_safe((char *)pkgname, DEGRLI_PKG_N_INSTALLED);
  if (install != NULL)
    gtk_button_set_label(GTK_BUTTON(install), "Install");
  if (removeb != NULL)
    gtk_widget_set_visible(removeb, FALSE);
}

static void pkg_install_finish(pkg_install_ctx_t *ctx) {
  if (ctx->finalized || !ctx->wait_done || !ctx->out_done || !ctx->err_done)
    return;

  ctx->finalized = TRUE;

  if (ctx->cancelled)
    return;

  if (ctx->success) {
    if (pkg_world_add(ctx->pkgname) == 0)
      pkg_mark_installed(ctx->pkgname);
    else
      pkg_install_log(ctx, " !!! Failed to update world file");
    pkg_install_log(ctx, " [ * ] Install finish!");
  } else if (!ctx->error_logged) {
    if (ctx->exited)
      pkg_install_log(ctx, " !!! script returned non-zero return value: %d",
                      ctx->exit_status);
    else
      pkg_install_log(ctx, " !!! The install process was terminated");
  }

  if (ctx->close_button != NULL)
    gtk_widget_set_sensitive(ctx->close_button, TRUE);
  if (ctx->diag != NULL)
    gtk_window_set_deletable(GTK_WINDOW(ctx->diag), TRUE);
}

static void pkg_remove_finish(pkg_install_ctx_t *ctx) {
  if (ctx->finalized || !ctx->wait_done || !ctx->out_done || !ctx->err_done)
    return;
  ctx->finalized = TRUE;

  if (ctx->cancelled)
    return;
  if (ctx->success) {
    if (pkg_world_remove(ctx->pkgname) == 0)
      pkg_mark_removed(ctx->pkgname);
    else
      pkg_install_log(ctx, "!!! Failed to update world file");
    pkg_install_log(ctx, " [ * ] Removal finish!");
  } else if (!ctx->error_logged) {
    if (ctx->exited)
      pkg_install_log(ctx, " !!! script returned non-zero return value: %d",
                      ctx->exit_status);
    else
      pkg_install_log(ctx, " !!! The removal process was terminated");
  }

  if (ctx->close_button != NULL)
    gtk_widget_set_sensitive(ctx->close_button, TRUE);
  if (ctx->diag != NULL)
    gtk_window_set_deletable(GTK_WINDOW(ctx->diag), TRUE);
}

static void pkg_install_read_cb(GObject *source, GAsyncResult *result,
                                gpointer user_data) {
  pkg_install_ctx_t *ctx = user_data;
  GError *error = NULL;
  char *line = g_data_input_stream_read_line_finish(G_DATA_INPUT_STREAM(source),
                                                    result, NULL, &error);

  if (error != NULL) {
    pkg_install_log(ctx, " !!! Error reading install output: %s",
                    error->message);
    g_error_free(error);
  }

  if (line != NULL) {
    pkg_install_log(ctx, "%s", line);
    g_free(line);
    g_data_input_stream_read_line_async(G_DATA_INPUT_STREAM(source),
                                        G_PRIORITY_DEFAULT, NULL,
                                        pkg_install_read_cb, ctx);
    return;
  }

  if (source == G_OBJECT(ctx->out))
    ctx->out_done = TRUE;
  else if (source == G_OBJECT(ctx->err))
    ctx->err_done = TRUE;

  pkg_install_finish(ctx);
  pkg_install_ctx_unref(ctx);
}
static void pkg_remove_read_cb(GObject *source, GAsyncResult *result,
                                gpointer user_data) {
  pkg_install_ctx_t *ctx = user_data;
  GError *error = NULL;
  char *line = g_data_input_stream_read_line_finish(G_DATA_INPUT_STREAM(source),
                                                    result, NULL, &error);

  if (error != NULL) {
    pkg_install_log(ctx, " !!! Error reading remove output: %s",
                    error->message);
    g_error_free(error);
  }

  if (line != NULL) {
    pkg_install_log(ctx, "%s", line);
    g_free(line);
    g_data_input_stream_read_line_async(G_DATA_INPUT_STREAM(source),
                                        G_PRIORITY_DEFAULT, NULL,
                                        pkg_remove_read_cb, ctx);
    return;
  }

  if (source == G_OBJECT(ctx->out))
    ctx->out_done = TRUE;
  else if (source == G_OBJECT(ctx->err))
    ctx->err_done = TRUE;

  pkg_remove_finish(ctx);
  pkg_install_ctx_unref(ctx);
}

static void pkg_install_wait_cb(GObject *source, GAsyncResult *result,
                                gpointer user_data) {
  pkg_install_ctx_t *ctx = user_data;
  GError *error = NULL;

  g_subprocess_wait_finish(G_SUBPROCESS(source), result, &error);
  if (error != NULL) {
    if (!ctx->cancelled)
      pkg_install_log(ctx, " !!! Waiting for install process failed: %s",
                      error->message);
    g_error_free(error);
  }

  ctx->exited = g_subprocess_get_if_exited(ctx->proc);
  if (ctx->exited)
    ctx->exit_status = g_subprocess_get_exit_status(ctx->proc);
  ctx->success = g_subprocess_get_successful(ctx->proc);
  ctx->wait_done = TRUE;

  pkg_install_finish(ctx);
  pkg_install_ctx_unref(ctx);
}
static void pkg_remove_wait_cb(GObject *source, GAsyncResult *result,
                                gpointer user_data) {
  pkg_install_ctx_t *ctx = user_data;
  GError *error = NULL;

  g_subprocess_wait_finish(G_SUBPROCESS(source), result, &error);
  if (error != NULL) {
    if (!ctx->cancelled)
      pkg_install_log(ctx, " !!! Waiting for install process failed: %s",
                      error->message);
    g_error_free(error);
  }

  ctx->exited = g_subprocess_get_if_exited(ctx->proc);
  if (ctx->exited)
    ctx->exit_status = g_subprocess_get_exit_status(ctx->proc);
  ctx->success = g_subprocess_get_successful(ctx->proc);
  ctx->wait_done = TRUE;

  pkg_remove_finish(ctx);
  pkg_install_ctx_unref(ctx);
}

static void pkg_install_start(pkg_install_ctx_t *ctx) {
  if (ctx->cancelled)
    return;

  char script[PATH_MAX];
  snprintf(script, sizeof(script), DEGRLI_PKG_PATH "/%s/install", ctx->pkgname);

  struct stat st;
  if (stat(script, &st) != 0) {
    pkg_install_log(ctx,
                    " !!! An error occurred running the script: %s not "
                    "found",
                    script);
    ctx->error_logged = TRUE;
    ctx->wait_done = ctx->out_done = ctx->err_done = TRUE;
    pkg_install_finish(ctx);
    return;
  }

  chmod(script, st.st_mode | S_IXUSR);

  pkg_install_log(ctx, " -> Install");
  pkg_install_log(ctx, " -> Calling script");
  pkg_install_log(ctx, " [ v ] Called script is %s", script);

  // run as root
  const char *argv[] = {"pkexec", "sh", script, NULL};
  GError *error = NULL;
  ctx->proc = g_subprocess_newv(argv,
                                G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                                    G_SUBPROCESS_FLAGS_STDERR_PIPE |
                                    G_SUBPROCESS_FLAGS_STDIN_INHERIT,
                                &error);
  if (ctx->proc == NULL) {
    pkg_install_log(ctx, " !!! An error occurred running the script: %s",
                    error != NULL ? error->message : "unknown error");
    g_clear_error(&error);
    ctx->error_logged = TRUE;
    ctx->wait_done = ctx->out_done = ctx->err_done = TRUE;
    pkg_install_finish(ctx);
    return;
  }

  ctx->out = g_data_input_stream_new(g_subprocess_get_stdout_pipe(ctx->proc));
  ctx->err = g_data_input_stream_new(g_subprocess_get_stderr_pipe(ctx->proc));
  g_data_input_stream_set_newline_type(ctx->out,
                                       G_DATA_STREAM_NEWLINE_TYPE_ANY);
  g_data_input_stream_set_newline_type(ctx->err,
                                       G_DATA_STREAM_NEWLINE_TYPE_ANY);

  pkg_install_ctx_ref(ctx);
  pkg_install_ctx_ref(ctx);
  pkg_install_ctx_ref(ctx);
  g_data_input_stream_read_line_async(ctx->out, G_PRIORITY_DEFAULT, NULL,
                                      pkg_install_read_cb, ctx);
  g_data_input_stream_read_line_async(ctx->err, G_PRIORITY_DEFAULT, NULL,
                                      pkg_install_read_cb, ctx);
  g_subprocess_wait_async(ctx->proc, NULL, pkg_install_wait_cb, ctx);
}
static void pkg_remove_start(pkg_install_ctx_t *ctx) {
  trace_log(DEBUG, "removal started\n");
  if (ctx->cancelled)
    return;

  char script[PATH_MAX];
  snprintf(script, sizeof(script), DEGRLI_PKG_PATH "/%s/remove", ctx->pkgname);

  struct stat st;
  if (stat(script, &st) != 0) {
    pkg_install_log(ctx,
                    " !!! An error occurred running the script: %s not "
                    "found",
                    script);
    ctx->error_logged = TRUE;
    ctx->wait_done = ctx->out_done = ctx->err_done = TRUE;
    pkg_remove_finish(ctx);
    return;
  }

  chmod(script, st.st_mode | S_IXUSR);

  pkg_install_log(ctx, " -> Install");
  pkg_install_log(ctx, " -> Calling script");
  pkg_install_log(ctx, " [ v ] Called script is %s", script);

  // run as root
  const char *argv[] = {"pkexec", "sh", script, NULL};
  GError *error = NULL;
  ctx->proc = g_subprocess_newv(argv,
                                G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                                    G_SUBPROCESS_FLAGS_STDERR_PIPE |
                                    G_SUBPROCESS_FLAGS_STDIN_INHERIT,
                                &error);
  if (ctx->proc == NULL) {
    pkg_install_log(ctx, " !!! An error occurred running the script: %s",
                    error != NULL ? error->message : "unknown error");
    g_clear_error(&error);
    ctx->error_logged = TRUE;
    ctx->wait_done = ctx->out_done = ctx->err_done = TRUE;
    pkg_remove_finish(ctx);
    return;
  }

  ctx->out = g_data_input_stream_new(g_subprocess_get_stdout_pipe(ctx->proc));
  ctx->err = g_data_input_stream_new(g_subprocess_get_stderr_pipe(ctx->proc));
  g_data_input_stream_set_newline_type(ctx->out,
                                       G_DATA_STREAM_NEWLINE_TYPE_ANY);
  g_data_input_stream_set_newline_type(ctx->err,
                                       G_DATA_STREAM_NEWLINE_TYPE_ANY);

  pkg_install_ctx_ref(ctx);
  pkg_install_ctx_ref(ctx);
  pkg_install_ctx_ref(ctx);
  g_data_input_stream_read_line_async(ctx->out, G_PRIORITY_DEFAULT, NULL,
                                      pkg_remove_read_cb, ctx);
  g_data_input_stream_read_line_async(ctx->err, G_PRIORITY_DEFAULT, NULL,
                                      pkg_remove_read_cb, ctx);
  g_subprocess_wait_async(ctx->proc, NULL, pkg_remove_wait_cb, ctx);
}

static void pkg_install_destroy_cb(GtkWidget *widget, gpointer user_data) {
  pkg_install_ctx_t *ctx = user_data;

  ctx->cancelled = TRUE;
  ctx->diag = NULL;
  ctx->close_button = NULL;
  ctx->log_view = NULL;
  g_clear_object(&ctx->buffer);

  if (ctx->proc != NULL && !ctx->finalized)
    g_subprocess_force_exit(ctx->proc);

  pkg_install_ctx_unref(ctx);
}

static void pkg_install_response_cb(GtkDialog *dialog, int response,
                                    gpointer user_data) {
  pkg_install_ctx_t *ctx = user_data;

  if (!ctx->finalized)
    return;

  gtk_window_destroy(GTK_WINDOW(dialog));
}

int pkg_install_gui(char *name) {
  if (name == NULL)
    return 0;

  pkg_install_ctx_t *ctx = g_new0(pkg_install_ctx_t, 1);
  ctx->refs = 1;
  ctx->pkgname = g_strdup(name);

  GtkWidget *diag = gtk_dialog_new_with_buttons(
      "Install a package", w, GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
      "_Close", GTK_RESPONSE_CLOSE, NULL);
  gtk_window_set_default_size(GTK_WINDOW(diag), 480, 360);
  gtk_window_set_deletable(GTK_WINDOW(diag), FALSE);

  ctx->diag = diag;
  ctx->close_button =
      gtk_dialog_get_widget_for_response(GTK_DIALOG(diag), GTK_RESPONSE_CLOSE);
  gtk_widget_set_sensitive(ctx->close_button, FALSE);

  GtkWidget *carea = gtk_dialog_get_content_area(GTK_DIALOG(diag));

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_set_margin_start(box, 20);
  gtk_widget_set_margin_end(box, 20);
  gtk_widget_set_margin_top(box, 20);
  gtk_widget_set_margin_bottom(box, 20);

  char *title_text = g_strdup_printf("Install log - %s", name);
  GtkWidget *title = gtk_label_new(title_text);
  g_free(title_text);
  gtk_label_set_xalign(GTK_LABEL(title), 0.0);

  GtkWidget *scrolled_window = gtk_scrolled_window_new();
  gtk_widget_set_vexpand(scrolled_window, TRUE);

  GtkWidget *log_view = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view),
                             FALSE); // Make it read-only
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(log_view), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(log_view), GTK_WRAP_NONE);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(log_view), TRUE);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), log_view);
  gtk_box_append(GTK_BOX(box), title);
  gtk_box_append(GTK_BOX(box), scrolled_window);
  gtk_box_append(GTK_BOX(carea), box);

  ctx->log_view = log_view;
  ctx->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
  g_object_ref(ctx->buffer);

  g_signal_connect(diag, "response", G_CALLBACK(pkg_install_response_cb), ctx);
  g_signal_connect(diag, "destroy", G_CALLBACK(pkg_install_destroy_cb), ctx);

  gtk_window_present(GTK_WINDOW(diag));

  pkg_install_log(ctx, " -> Installing package %s", name);
  if (pkg_is_in_wfile(name)) {
    pkg_install_log(ctx, "    -> Read world file");
    pkg_install_log(ctx, "     -> The package is currently already installed.");
    pkg_install_log(
        ctx, "     -> Re-installing anyway."); // no -f so just always force
  }

  if (!g_file_test(DEGRLI_PKG_WORLD_PATH, G_FILE_TEST_EXISTS)) {
    pkg_install_log(ctx, " !!! The world file was not found! (%s)",
                    DEGRLI_PKG_WORLD_PATH);
    pkg_install_log(
        ctx,
        " !!! Run `dlpm init` (from repo tool) before installing packages.");
    ctx->error_logged = TRUE;
    ctx->wait_done = ctx->out_done = ctx->err_done = TRUE;
    pkg_install_finish(ctx);
    return 0;
  }

  pkg_install_start(ctx);

  return 0;
}

int pkg_remove_gui(char *name) {
  if (name == NULL)
    return 0;

  pkg_install_ctx_t *ctx = g_new0(pkg_install_ctx_t, 1);
  ctx->refs = 1;
  ctx->pkgname = g_strdup(name);

  GtkWidget *diag = gtk_dialog_new_with_buttons(
      "Remove a package", w, GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
      "_Close", GTK_RESPONSE_CLOSE, NULL);
  gtk_window_set_default_size(GTK_WINDOW(diag), 480, 360);
  gtk_window_set_deletable(GTK_WINDOW(diag), FALSE);

  ctx->diag = diag;
  ctx->close_button =
      gtk_dialog_get_widget_for_response(GTK_DIALOG(diag), GTK_RESPONSE_CLOSE);
  gtk_widget_set_sensitive(ctx->close_button, FALSE);

  GtkWidget *carea = gtk_dialog_get_content_area(GTK_DIALOG(diag));

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_set_margin_start(box, 20);
  gtk_widget_set_margin_end(box, 20);
  gtk_widget_set_margin_top(box, 20);
  gtk_widget_set_margin_bottom(box, 20);

  char *title_text = g_strdup_printf("Remove log - %s", name);
  GtkWidget *title = gtk_label_new(title_text);
  g_free(title_text);
  gtk_label_set_xalign(GTK_LABEL(title), 0.0);

  GtkWidget *scrolled_window = gtk_scrolled_window_new();
  gtk_widget_set_vexpand(scrolled_window, TRUE);

  GtkWidget *log_view = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view),
                             FALSE); // Make it read-only
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(log_view), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(log_view), GTK_WRAP_NONE);
  gtk_text_view_set_monospace(GTK_TEXT_VIEW(log_view), TRUE);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), log_view);
  gtk_box_append(GTK_BOX(box), title);
  gtk_box_append(GTK_BOX(box), scrolled_window);
  gtk_box_append(GTK_BOX(carea), box);

  ctx->log_view = log_view;
  ctx->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
  g_object_ref(ctx->buffer);

  g_signal_connect(diag, "response", G_CALLBACK(pkg_install_response_cb), ctx);
  g_signal_connect(diag, "destroy", G_CALLBACK(pkg_install_destroy_cb), ctx);

  gtk_window_present(GTK_WINDOW(diag));

  pkg_install_log(ctx, " -> Removing package %s", name);
  if (pkg_is_in_wfile(name)) {
    pkg_install_log(ctx, "    -> Read world file");
    pkg_install_log(ctx, "     -> The package is currently already installed.");
    pkg_install_log(
        ctx, "     -> Removing..."); // no -f so just always force
  

  if (!g_file_test(DEGRLI_PKG_WORLD_PATH, G_FILE_TEST_EXISTS)) {
    pkg_install_log(ctx, " !!! The world file was not found! (%s)",
                    DEGRLI_PKG_WORLD_PATH);
    pkg_install_log(
        ctx,
        " !!! Run `dlpm init` (from repo tool) before installing packages.");
    ctx->error_logged = TRUE;
    ctx->wait_done = ctx->out_done = ctx->err_done = TRUE;
    pkg_remove_finish(ctx);
    return 0;
  }

  pkg_remove_start(ctx);
  } else {
    pkg_install_log(ctx, "    -> Nothing to do.");
    gtk_widget_set_sensitive(ctx->close_button, TRUE);
    gtk_window_set_deletable(GTK_WINDOW(ctx->diag), TRUE);
  }

  return 0;
}

void pkg_populate_gui(GListStore *self) {
  for (int i = 0; i < pkg_entries.len; ++i) {
    g_list_store_append(self,
                        gtk_string_object_new(pkg_entries.entries[i].pkgname));
  }
}

static void pkg_install_hook(GtkButton *button, gpointer user_data) {
  pkg_install_gui(s_sel);
}

static void pkg_remove_hook(GtkButton *button, gpointer user_data) {
  pkg_remove_gui(s_sel);
}

static inline void remove_outer_quotes(char *str) {
  size_t len = strlen(str);

  // Check if the string has at least 2 characters and matching outer quotes
  if (len >= 2 && ((str[0] == '"' && str[len - 1] == '"') ||
                   (str[0] == '\'' && str[len - 1] == '\''))) {

    // Move all characters (including '\0') 1 position to the left
    memmove(str, str + 1, len - 1);
    str[len - 2] = '\0'; // Truncate the trailing quote
  }
}

#define MATCH(a, b) (strcmp(a, b) == 0)
static void pkg_ini_callback(char *section, char *key, char *value) {
  if MATCH (section, "Info") {
    if MATCH (key, "Name") {
      remove_outer_quotes(value);
      gtk_label_set_text(GTK_LABEL(header), value);
    }
    if MATCH (key, "Description") {
      remove_outer_quotes(value);
      gtk_label_set_text(GTK_LABEL(desc), value);
    }
    if MATCH (key, "Owner") {
      remove_outer_quotes(value);
      gtk_label_set_text(GTK_LABEL(owndesc), value);
    }
    if MATCH (key, "Version") {
      remove_outer_quotes(value);
      strcpy(cversion, value); // save version
      char buf[256];
      snprintf(buf, sizeof(buf), "Package upstream (if any) version: %s",
               value);
      gtk_label_set_text(GTK_LABEL(zaghahkngvyjin), buf);
    }
  }
}
#undef MATCH

static _Bool pkg_is_in_wfile(char *toinstall) {
  FILE *f = fopen(DEGRLI_PKG_WORLD_PATH, "r");
  if (!f) {
    trace_log(WARN, "Could not open world file: %s\n" DEGRLI_PKG_WORLD_PATH);
    return false;
  }
  char *line = NULL;
  size_t len = 0;
  while (getline(&line, &len, f) != -1) {
    line[strcspn(line, "\r\n")] = '\0';

    if (strcmp(line, toinstall) == 0) {
      free(line);
      fclose(f);
      return true;
    }
  }
  free(line);
  fclose(f);
  return false;
}

void pkg_update_box(GListStore *dat, int sel) {
  csel = sel;
  gpointer item = g_list_model_get_object(G_LIST_MODEL(dat), sel);
  char fpathbuf[PATH_MAX];
  if (item) {
    GtkStringObject *so = GTK_STRING_OBJECT(item);
    s_sel = gtk_string_object_get_string(so);
    snprintf(fpathbuf, sizeof(fpathbuf),
             "/usr/share/desktop-gremlin-linux/dlpm/recipes/%s/metadata.ini",
             s_sel);
    trace_log(DEBUG, "sel=%d s_sel=%s path=%s\n", sel, s_sel, fpathbuf);
    FILE *metadata = fopen(fpathbuf, "r");
    if (!metadata)
      return;
    pkg_ini_parse(metadata, pkg_ini_callback);
    if (pkg_is_in_wfile((char *)s_sel)) {
      gtk_widget_set_visible(removeb, true);
      gtk_button_set_label(GTK_BUTTON(install), "Re-install/Update");
    } else {
      gtk_widget_set_visible(removeb, false);
      gtk_button_set_label(GTK_BUTTON(install), "Install");
    }
    g_object_unref(item);
    fclose(metadata);
  }
}

void pkg_setup_box(GtkWidget *self) {
  thefuckingbox = self;
  header = gtk_label_new("PACKAGE NAME");
  gtk_widget_set_halign(header, GTK_ALIGN_START);
  gtk_widget_add_css_class(header, "title-1");
  owndesc = gtk_label_new("OWNERNAME");
  gtk_widget_set_halign(owndesc, GTK_ALIGN_START);
  gtk_widget_add_css_class(owndesc, "dim-label"); // give it that soft feeling
  install = gtk_button_new_with_label("Install");
  gtk_widget_set_halign(install, GTK_ALIGN_START);
  gtk_widget_set_hexpand(install, FALSE);
  removeb = gtk_button_new_with_label("Remove");
  gtk_widget_set_halign(removeb, GTK_ALIGN_START);
  gtk_widget_set_hexpand(removeb, false);
  gtk_widget_add_css_class(removeb, "destructive-action");
  gtk_widget_set_visible(removeb, false);
  g_signal_connect(install, "clicked", G_CALLBACK(pkg_install_hook), NULL);
  gtk_widget_set_margin_start(install, 20);
  g_signal_connect(removeb, "clicked", G_CALLBACK(pkg_remove_hook), NULL);
  gtk_widget_set_margin_start(removeb, 20);
  GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  desc = gtk_label_new("DESCRIPTION TEXT BLABLABLA I LOVE UMAUMSUME");
  gtk_widget_set_halign(desc, GTK_ALIGN_START);
  gtk_label_set_wrap(GTK_LABEL(desc), TRUE);
  zaghahkngvyjin = gtk_label_new("v4.2.0");
  gtk_widget_add_css_class(zaghahkngvyjin, "dim-label");
  gtk_box_append(GTK_BOX(thefuckingbox), header);
  gtk_box_append(GTK_BOX(thefuckingbox), owndesc);
  gtk_box_append(GTK_BOX(thefuckingbox), install);
  gtk_box_append(GTK_BOX(thefuckingbox), removeb);
  gtk_box_append(GTK_BOX(thefuckingbox), separator);
  gtk_box_append(GTK_BOX(thefuckingbox), desc);
  gtk_box_append(GTK_BOX(thefuckingbox), zaghahkngvyjin);
}

int pkg_init(void) {
  pkg_init_list();
  FILE *wrldf = fopen(DEGRLI_PKG_WORLD_PATH, "r");
  if (pkg_populate_list(wrldf) != 0) {
    trace_log(ERROR, "pkg_populate_list fail");
    return 1;
  }
  fclose(wrldf);
  return 0;
}
