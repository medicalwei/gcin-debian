#include "gcin.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"
#include "eggtrayicon.h"
#include "gst.h"
#include "pho-kbm-name.h"

GtkWidget *win_screen_status, *icon_hf, *icon_inmd;
static int timeout_handle;

static char *cur_file_hf, *cur_file_inmd;

static gboolean clear_timeout()
{
  if (!timeout_handle)
    return FALSE;
  g_source_remove(timeout_handle);
  timeout_handle = 0;
  return TRUE;
}

void close_win_status()
{
  if (!clear_timeout())
    return;
  gtk_widget_hide(win_screen_status);
}

static gboolean timeout_hide(gpointer data)
{
  close_win_status();
  return FALSE;
}

gboolean win_is_visible();
static int old_x, old_y;

void disp_win_screen_status(char *in_method, char *half_status)
{
  dbg("disp_win_screen_status\n");
  if (
//  tss.c_len || ggg.gbufN ||
  cur_file_hf && !strcmp(cur_file_hf, half_status) &&
  cur_file_inmd && !strcmp(cur_file_inmd, in_method) 
#if 0  
  && old_x==current_in_win_x && old_y==current_in_win_y
#endif  
  )
    return;
    
  old_x = current_in_win_x;
  old_y = current_in_win_y;

  clear_timeout();
  free(cur_file_hf); cur_file_hf = strdup(half_status);
  free(cur_file_inmd); cur_file_inmd = strdup(in_method);

  if (!win_screen_status) {
    win_screen_status = create_no_focus_win();
    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_add(GTK_CONTAINER(win_screen_status), hbox);
    icon_inmd = gtk_image_new_from_file(in_method);
    gtk_box_pack_start (GTK_BOX (hbox), icon_inmd, FALSE, FALSE, 0);
#if 1
    icon_hf = gtk_image_new_from_file(half_status);
    gtk_box_pack_start (GTK_BOX (hbox), icon_hf, FALSE, FALSE, 0);
#endif    
  } else {
#if 1
    gtk_image_set_from_file(GTK_IMAGE(icon_hf), half_status);
#endif
    gtk_image_set_from_file(GTK_IMAGE(icon_inmd), in_method);
  }

  gtk_widget_show_all(win_screen_status);
  gtk_window_present(GTK_WINDOW(win_screen_status));

  timeout_handle = g_timeout_add(1000, timeout_hide, NULL);
  int w,h;
  get_win_size(win_screen_status, &w, &h);

  int x = current_in_win_x;
  int y = current_in_win_y + (win_is_visible()?win_yl:0);


  if (x + w > dpy_xl)
    x = dpy_xl - w;

  if (y + h > dpy_yl)
    y = win_y - h;

  gtk_window_move(GTK_WINDOW(win_screen_status), x, y);
}
