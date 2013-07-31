#include "gcin.h"
#include "gtab.h"
#include "config.h"
#if UNIX
#include <signal.h>
#endif
#if GCIN_i18n_message
#include <libintl.h>
#endif
#include "lang.h"

extern gboolean is_chs;

static GtkWidget *check_button_root_style_use,
                 *check_button_gcin_pop_up_win,
                 *check_button_gcin_inner_frame,
#if TRAY_ENABLED
                 *check_button_gcin_status_tray,
                 *check_button_gcin_win32_icon,
                 *check_button_gcin_tray_hf_win_kbm,
                 *check_button_gcin_status_win,
#endif
                 *check_button_gcin_win_color_use,
                 *check_button_gcin_on_the_spot_key;


static GtkWidget *gcin_kbm_window = NULL, *gcin_appearance_conf_window;
static GtkClipboard *pclipboard;
static GtkWidget *opt_gcin_edit_display;
extern GtkWidget *main_window;
static GdkColor gcin_win_gcolor_fg, gcin_win_gcolor_bg, gcin_sel_key_gcolor;
static GtkWidget *opt_tray;
extern gboolean button_order;

#if USE_INDICATOR
static struct {
  char *name;
  int key;
} tray_options[]={
  {"單一圖示", GCIN_TRAY_UNIX},
  {"GTK 雙圖示", GCIN_TRAY_WIN32},
  {"Unity indicator", GCIN_TRAY_INDICATOR}
};
int tray_optionsN = sizeof(tray_options)/sizeof(tray_options[0]);
#endif

typedef struct {
  GdkColor *color;
  char **color_str;
  GtkWidget *color_selector;
  unich_t *title;
} COLORSEL;

COLORSEL colorsel[2] =
  { {&gcin_win_gcolor_fg, &gcin_win_color_fg, NULL, N_(_L("前景顏色"))},
    {&gcin_win_gcolor_bg, &gcin_win_color_bg, NULL, N_(_L("背景顏色"))}
  };

struct {
  unich_t *keystr;
  int keynum;
} edit_disp[] = {
  {N_(_L("gcin視窗")), GCIN_EDIT_DISPLAY_OVER_THE_SPOT},
  {N_(_L("應用程式編輯區")), GCIN_EDIT_DISPLAY_ON_THE_SPOT},
  {N_(_L("同時顯示")),  GCIN_EDIT_DISPLAY_BOTH},
  { NULL, 0},
};



void create_kbm_window();

static void cb_kbm()
{
  create_kbm_window();
}


#include <string.h>


static GtkWidget *spinner_gcin_font_size, *spinner_gcin_font_size_tsin_presel,
                 *spinner_gcin_font_size_symbol,*spinner_gcin_font_size_pho_near,
                 *spinner_gcin_font_size_win_kbm,
                 *spinner_gcin_font_size_win_kbm_en,
                 *spinner_gcin_font_size_tsin_pho_in, *spinner_gcin_font_size_gtab_in, *spinner_root_style_x,
                 *spinner_root_style_y, *font_sel;
static GdkColor tsin_cursor_gcolor;

static GtkWidget *label_win_color_test, *event_box_win_color_test;

static gboolean cb_appearance_conf_ok( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  int font_size = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size));
  save_gcin_conf_int(GCIN_FONT_SIZE, font_size);

#if GTK_CHECK_VERSION(2,4,0)
  char fname[128];
  strcpy(fname, gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_sel)));
  int len = strlen(fname)-1;

  while (len > 0 && isdigit(fname[len])) {
       fname[len--]=0;
  }

  while (len > 0 && fname[len]==' ') {
       fname[len--]=0;
  }

  save_gcin_conf_str(GCIN_FONT_NAME, fname);
#endif

  int font_size_tsin_presel = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_tsin_presel));
  save_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PRESEL, font_size_tsin_presel);

  int font_size_symbol = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_symbol));
  save_gcin_conf_int(GCIN_FONT_SIZE_SYMBOL, font_size_symbol);

  int font_size_tsin_pho_in = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_tsin_pho_in));
  save_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PHO_IN, font_size_tsin_pho_in);

  int font_size_pho_near = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_pho_near));
  save_gcin_conf_int(GCIN_FONT_SIZE_PHO_NEAR, font_size_pho_near);

  int font_size_gtab_in = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_gtab_in));
  save_gcin_conf_int(GCIN_FONT_SIZE_GTAB_IN, font_size_gtab_in);

  int font_size_win_kbm = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_win_kbm));
  save_gcin_conf_int(GCIN_FONT_SIZE_WIN_KBM, font_size_win_kbm);
  int font_size_win_kbm_en = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_win_kbm_en));
  save_gcin_conf_int(GCIN_FONT_SIZE_WIN_KBM_EN, font_size_win_kbm_en);

  int gcin_pop_up_win = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_pop_up_win));
  save_gcin_conf_int(GCIN_POP_UP_WIN, gcin_pop_up_win);

  int gcin_root_x = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_root_style_x));
  save_gcin_conf_int(GCIN_ROOT_X, gcin_root_x);

  int gcin_root_y = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_root_style_y));
  save_gcin_conf_int(GCIN_ROOT_Y, gcin_root_y);

  int style = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_root_style_use)) ?
            InputStyleRoot : InputStyleOverSpot;
  save_gcin_conf_int(GCIN_INPUT_STYLE, style);

  save_gcin_conf_int(GCIN_INNER_FRAME, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_inner_frame)));
#if TRAY_ENABLED
  save_gcin_conf_int(GCIN_STATUS_TRAY, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_status_tray)));
#if USE_INDICATOR
 save_gcin_conf_int(GCIN_WIN32_ICON,
                     tray_options[gtk_combo_box_get_active(GTK_COMBO_BOX(opt_tray))].key);
#else
  save_gcin_conf_int(GCIN_WIN32_ICON, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_win32_icon)));
#endif
#endif

  gchar *cstr = gtk_color_selection_palette_to_string(&gcin_win_gcolor_fg, 1);
  dbg("color fg %s\n", cstr);
  save_gcin_conf_str(GCIN_WIN_COLOR_FG, cstr);
  g_free(cstr);

  cstr = gtk_color_selection_palette_to_string(&gcin_win_gcolor_bg, 1);
  dbg("color bg %s\n", cstr);
  save_gcin_conf_str(GCIN_WIN_COLOR_BG, cstr);
  g_free(cstr);

  save_gcin_conf_int(GCIN_WIN_COLOR_USE, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_win_color_use)));
  save_gcin_conf_int(GCIN_ON_THE_SPOT_KEY, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_on_the_spot_key)));
  save_gcin_conf_int(GCIN_TRAY_HF_WIN_KBM, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_tray_hf_win_kbm)));
  save_gcin_conf_int(GCIN_STATUS_WIN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_status_win)));

  cstr = gtk_color_selection_palette_to_string(&gcin_sel_key_gcolor, 1);
  dbg("selkey color %s\n", cstr);
  save_gcin_conf_str(GCIN_SEL_KEY_COLOR, cstr);
  g_free(cstr);

  int idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_gcin_edit_display));
  save_gcin_conf_int(GCIN_EDIT_DISPLAY, edit_disp[idx].keynum);

  cstr = gtk_color_selection_palette_to_string(&tsin_cursor_gcolor, 1);
  dbg("color %s\n", cstr);
  save_gcin_conf_str(TSIN_CURSOR_COLOR, cstr);
  g_free(cstr);


  send_gcin_message(
#if UNIX
	  GDK_DISPLAY(),
#endif
	  CHANGE_FONT_SIZE);
  gtk_widget_destroy(gcin_appearance_conf_window); gcin_appearance_conf_window = NULL;

  return TRUE;
}

static gboolean close_appearance_conf_window( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  gtk_widget_destroy(gcin_appearance_conf_window); gcin_appearance_conf_window = NULL;
  return TRUE;
}


static void cb_savecb_gcin_win_color_fg(GtkWidget *widget, gpointer user_data)
{
  COLORSEL *sel = (COLORSEL *)user_data;
  GtkWidget *color_selector = sel->color_selector;
  GdkColor *col = sel->color;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(color_selector))), col);

  if (sel->color == &gcin_win_gcolor_fg) {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, col);
#else
    GdkRGBA rgbfg;
    gdk_rgba_parse(&rgbfg, gdk_color_to_string(col));
    gtk_widget_override_color(label_win_color_test, GTK_STATE_FLAG_NORMAL, &rgbfg);
#endif
  } else {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, col);
#else
    GdkRGBA rgbbg;
    gdk_rgba_parse(&rgbbg, gdk_color_to_string(col));
    gtk_widget_override_background_color(event_box_win_color_test, GTK_STATE_FLAG_NORMAL, &rgbbg);
#endif
  }
}

static gboolean cb_gcin_win_color_fg( GtkWidget *widget,
                                   gpointer   data)
{
  COLORSEL *sel = (COLORSEL *)data;
  GtkWidget *color_selector = gtk_color_selection_dialog_new (_(sel->title));

  gdk_color_parse(*sel->color_str, sel->color);

  gtk_color_selection_set_current_color(
          GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(color_selector))),
          sel->color);


  sel->color_selector = color_selector;

  gtk_widget_show((GtkWidget*)color_selector);
#if 1
  if (gtk_dialog_run(GTK_DIALOG(color_selector)) == GTK_RESPONSE_OK)
    cb_savecb_gcin_win_color_fg((GtkWidget *)color_selector, (gpointer) sel);
  gtk_widget_destroy(color_selector);
#endif
  return TRUE;
}

void disp_fg_bg_color()
{
  dbg("disp_fg_bg_color\n");
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(check_button_gcin_win_color_use))) {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, &gcin_win_gcolor_fg);
    dbg("gcin_win_gcolor_bg %d\n", gcin_win_gcolor_bg);
    gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, &gcin_win_gcolor_bg);
#else
    GdkRGBA rgbfg, rgbbg;
    gdk_rgba_parse(&rgbfg, gdk_color_to_string(&gcin_win_gcolor_fg));
    gdk_rgba_parse(&rgbbg, gdk_color_to_string(&gcin_win_gcolor_bg));
    gtk_widget_override_color(label_win_color_test, GTK_STATE_FLAG_NORMAL, &rgbfg);
    gtk_widget_override_background_color(event_box_win_color_test, GTK_STATE_FLAG_NORMAL, &rgbbg);
#endif
  } else {
#if !GTK_CHECK_VERSION(2,91,6)
    gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, NULL);
    gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, NULL);
#else
    gtk_widget_override_color(label_win_color_test, GTK_STATE_FLAG_NORMAL, NULL);
    gtk_widget_override_background_color(event_box_win_color_test, GTK_STATE_FLAG_NORMAL, NULL);
#endif
  }

  char *key_color = gtk_color_selection_palette_to_string(&gcin_sel_key_gcolor, 1);
  unich_t tt[512];
#if UNIX
#if PANGO_VERSION_CHECK(1,22,0)
  sprintf
(tt, _(_L("<span foreground=\"%s\" font=\"%d\">7</span><span font=\"%d\">測試</span>")), key_color,
gcin_font_size_tsin_presel, gcin_font_size_tsin_presel);
#else
  sprintf
(tt, _(_L("<span foreground=\"%s\" font_desc=\"%d\">7</span><span font_desc=\"%d\">測試</span>")), key_color,
gcin_font_size_tsin_presel, gcin_font_size_tsin_presel);
#endif
#else
  swprintf
(tt, _L("<span foreground=\"%S\" font=\"%d\">7</span><span font=\"%d\">測試</span>"), key_color, gcin_font_size_tsin_presel, gcin_font_size_tsin_presel);
#endif

  gtk_label_set_markup(GTK_LABEL(label_win_color_test), _(tt));
}

static void cb_save_gcin_sel_key_color(GtkWidget *widget, gpointer user_data)
{
  GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)user_data;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(color_selector)), &gcin_sel_key_gcolor);
  gcin_sel_key_color = gtk_color_selection_palette_to_string(&gcin_sel_key_gcolor, 1);

  g_snprintf(eng_color_full_str, 128, "<span foreground=\"%s\">%s</span>", gcin_sel_key_color, _(eng_full_str));
  g_snprintf(cht_color_full_str, 128, "<span foreground=\"%s\">%s</span>", gcin_sel_key_color, _(cht_full_str));

  disp_fg_bg_color();
}


static gboolean cb_gcin_sel_key_color( GtkWidget *widget, gpointer data)
{
   GtkWidget *color_selector = gtk_color_selection_dialog_new (_(_L("選擇鍵的顏色")));

   gtk_color_selection_set_current_color(
           GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(color_selector))),
           &gcin_sel_key_gcolor);


   gtk_widget_show(color_selector);
   if (gtk_dialog_run(GTK_DIALOG(color_selector)) == GTK_RESPONSE_OK)
     cb_save_gcin_sel_key_color(color_selector, (gpointer) color_selector);
   gtk_widget_destroy(color_selector);

   return TRUE;
}

void cb_button_gcin_on_the_spot_key(GtkToggleButton *togglebutton, gpointer user_data)
{
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(togglebutton)))
    return;
  int i;
  for (i=0; edit_disp[i].keystr; i++)
   if (edit_disp[i].keynum == GCIN_EDIT_DISPLAY_ON_THE_SPOT)
     gtk_combo_box_set_active (GTK_COMBO_BOX (opt_gcin_edit_display), i);
}

void combo_selected(GtkWidget *widget, gpointer window)
{
  int idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_gcin_edit_display));
  if (edit_disp[idx].keynum !=  GCIN_EDIT_DISPLAY_ON_THE_SPOT) {
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check_button_gcin_on_the_spot_key), FALSE);
  }
}

static GtkWidget *create_gcin_edit_display()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);
  GtkWidget *label = gtk_label_new(_(_L("編輯區顯示")));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_gcin_edit_display = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
  GtkWidget *menu = gtk_menu_new ();
#endif
  gtk_box_pack_start (GTK_BOX (hbox), opt_gcin_edit_display, FALSE, FALSE, 0);

  int i, current_idx=0;

  for(i=0; edit_disp[i].keystr; i++) {
#if !GTK_CHECK_VERSION(2,4,0)
    GtkWidget *item = gtk_menu_item_new_with_label (_(edit_disp[i].keystr));
#endif

    if (edit_disp[i].keynum == gcin_edit_display)
      current_idx = i;

#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_gcin_edit_display), _(edit_disp[i].keystr));
#else
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
#endif
  }

#if !GTK_CHECK_VERSION(2,4,0)
  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_gcin_edit_display), menu);
#endif
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_gcin_edit_display), current_idx);

  label = gtk_label_new(_(_L("按鍵顯示於\n應用程式")));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  check_button_gcin_on_the_spot_key = gtk_check_button_new ();
  g_signal_connect (G_OBJECT (check_button_gcin_on_the_spot_key), "toggled",
                    G_CALLBACK (cb_button_gcin_on_the_spot_key), NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_on_the_spot_key),
       gcin_on_the_spot_key);
  gtk_box_pack_start (GTK_BOX (hbox), check_button_gcin_on_the_spot_key, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(opt_gcin_edit_display), "changed",
        G_CALLBACK(combo_selected), (gpointer) NULL);

  return hbox;
}



static gboolean cb_gcin_win_color_use(GtkToggleButton *togglebutton, gpointer user_data)
{
  dbg("cb_gcin_win_color_use\n");
  disp_fg_bg_color();
  return TRUE;
}

static GtkWidget *da_cursor;
static void cb_save_tsin_cursor_color(GtkWidget *widget, gpointer user_data)
{
  GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)user_data;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(color_selector)), &tsin_cursor_gcolor);

#if !GTK_CHECK_VERSION(2,91,6)
  gtk_widget_modify_bg(da_cursor, GTK_STATE_NORMAL, &tsin_cursor_gcolor);
#else
  GdkRGBA rgbbg;
  gdk_rgba_parse(&rgbbg, gdk_color_to_string(&tsin_cursor_gcolor));
  gtk_widget_override_background_color(da_cursor, GTK_STATE_FLAG_NORMAL, &rgbbg);
#endif
}

static gboolean cb_tsin_cursor_color( GtkWidget *widget, gpointer   data )
{
   GtkWidget *color_selector = gtk_color_selection_dialog_new (_(_L("編輯區游標的顏色")));

   gtk_color_selection_set_current_color(
           GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(color_selector))),
           &tsin_cursor_gcolor);

   gtk_widget_show((GtkWidget*)color_selector);
#if 1
   if (gtk_dialog_run(GTK_DIALOG(color_selector)) == GTK_RESPONSE_OK)
     cb_save_tsin_cursor_color((GtkWidget *)color_selector, (gpointer) color_selector);
   gtk_widget_destroy((GtkWidget *)color_selector);
#endif
   return TRUE;
}

int html_browser(char *s);

static gboolean cb_appearance_help( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  html_browser("http://hyperrate.com/topic-files-dir/48/26548-L0IuDjFh0n/appearance_setting_help.html");
  return TRUE;
}

#if USE_INDICATOR
static int get_currnet_tray_option_idx()
{
  int i;
  for(i=0; i < tray_optionsN; i++)
    if (tray_options[i].key == gcin_win32_icon)
      return i;

  p_err("tsin-space-opt->%d is not valid", gcin_win32_icon);
  return -1;
}

GtkWidget *create_status_icon_opts()
{
  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  opt_tray = gtk_combo_box_new_text ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_tray, FALSE, FALSE, 0);

  int i;
  int current_idx = get_currnet_tray_option_idx();

  for(i=0; i < tray_optionsN; i++) {
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_tray), tray_options[i].name);
  }

  dbg("current_idx:%d\n", current_idx);

  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_tray), current_idx);

  return hbox;
}
#endif


void create_appearance_conf_window()
{
  if (gcin_appearance_conf_window) {
    gtk_window_present(GTK_WINDOW(gcin_appearance_conf_window));
    return;
  }

  load_setttings();

  gcin_appearance_conf_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(gcin_appearance_conf_window), GTK_WIN_POS_MOUSE);


  gtk_window_set_has_resize_grip(GTK_WINDOW(gcin_appearance_conf_window), FALSE);

  g_signal_connect (G_OBJECT (gcin_appearance_conf_window), "delete_event",
                    G_CALLBACK (close_appearance_conf_window),
                    NULL);

  gtk_window_set_title (GTK_WINDOW (gcin_appearance_conf_window), _(_L("輸入視窗外觀設定")));
  gtk_container_set_border_width (GTK_CONTAINER (gcin_appearance_conf_window), 3);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gcin_appearance_conf_window), vbox_top);

  GtkWidget *hboxLR = gtk_hbox_new (FALSE, 20);
  gtk_box_pack_start (GTK_BOX (vbox_top), hboxLR, FALSE, FALSE, 0);

  GtkWidget *vbox_L = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hboxLR), vbox_L, FALSE, FALSE, 0);
  GtkWidget *vbox_R = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hboxLR), vbox_R, FALSE, FALSE, 0);


  GtkWidget *hbox_gcin_font_size = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_L), hbox_gcin_font_size, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size = gtk_label_new(_(_L("字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size), label_gcin_font_size, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size = gtk_spin_button_new (adj_gcin_font_size, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size), spinner_gcin_font_size, FALSE, FALSE, 0);


  GtkWidget *hbox_gcin_font_size_symbol = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_L), hbox_gcin_font_size_symbol, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_symbol = gtk_label_new(_(_L("符號選擇視窗字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_symbol), label_gcin_font_size_symbol, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_symbol =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_symbol, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_symbol = gtk_spin_button_new (adj_gcin_font_size_symbol, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_symbol), spinner_gcin_font_size_symbol, FALSE, FALSE, 0);


  GtkWidget *hbox_gcin_font_size_tsin_presel = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_L), hbox_gcin_font_size_tsin_presel, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_tsin_presel = gtk_label_new(_(_L("詞音&gtab預選詞視窗字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_presel), label_gcin_font_size_tsin_presel, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_tsin_presel =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_tsin_presel, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_tsin_presel = gtk_spin_button_new (adj_gcin_font_size_tsin_presel, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_presel), spinner_gcin_font_size_tsin_presel, FALSE, FALSE, 0);


  GtkWidget *hbox_gcin_font_size_tsin_pho_in = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_L), hbox_gcin_font_size_tsin_pho_in, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_tsin_pho_in = gtk_label_new(_(_L("詞音注音輸入區字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_pho_in), label_gcin_font_size_tsin_pho_in, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_tsin_pho_in =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_tsin_pho_in, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_tsin_pho_in = gtk_spin_button_new (adj_gcin_font_size_tsin_pho_in, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_pho_in), spinner_gcin_font_size_tsin_pho_in, FALSE, FALSE, 0);


  GtkWidget *hbox_gcin_font_size_pho_near = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_L), hbox_gcin_font_size_pho_near, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_pho_near = gtk_label_new(_(_L("詞音近似音顯示字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_pho_near), label_gcin_font_size_pho_near, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_pho_near =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_pho_near, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_pho_near = gtk_spin_button_new (adj_gcin_font_size_pho_near, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_pho_near), spinner_gcin_font_size_pho_near, FALSE, FALSE, 0);


  GtkWidget *hbox_gcin_font_size_gtab_in = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_L), hbox_gcin_font_size_gtab_in, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_gtab_in = gtk_label_new(_(_L("gtab(倉頡…)輸入區字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_gtab_in), label_gcin_font_size_gtab_in, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_gtab_in =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_gtab_in, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_gtab_in = gtk_spin_button_new (adj_gcin_font_size_gtab_in, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_gtab_in), spinner_gcin_font_size_gtab_in, FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_font_size_win_kbm = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_L), hbox_gcin_font_size_win_kbm, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_win_kbm = gtk_label_new(_(_L("小鍵盤字型大小")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_win_kbm), label_gcin_font_size_win_kbm, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_win_kbm =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_win_kbm, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_win_kbm = gtk_spin_button_new (adj_gcin_font_size_win_kbm, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_win_kbm), spinner_gcin_font_size_win_kbm, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_win_kbm_en = gtk_label_new(_(_L("英數")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_win_kbm), label_gcin_font_size_win_kbm_en, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_win_kbm_en =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_win_kbm_en, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_win_kbm_en = gtk_spin_button_new (adj_gcin_font_size_win_kbm_en, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_win_kbm), spinner_gcin_font_size_win_kbm_en, FALSE, FALSE, 0);


#if GTK_CHECK_VERSION(2,4,0)
  char tt[128];
  sprintf(tt, "%s %d", gcin_font_name, gcin_font_size);
  font_sel = gtk_font_button_new_with_font (tt);
  gtk_box_pack_start (GTK_BOX (vbox_L), font_sel, FALSE, FALSE, 0);
#endif

  GtkWidget *hbox_gcin_pop_up_win = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_R), hbox_gcin_pop_up_win, FALSE, FALSE, 0);
  GtkWidget *label_gcin_pop_up_win = gtk_label_new(_(_L("彈出式輸入視窗")));
  gtk_box_pack_start (GTK_BOX(hbox_gcin_pop_up_win), label_gcin_pop_up_win, FALSE, FALSE, 0);
  check_button_gcin_pop_up_win = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_pop_up_win),
       gcin_pop_up_win);
  gtk_box_pack_start (GTK_BOX(hbox_gcin_pop_up_win), check_button_gcin_pop_up_win, FALSE, FALSE, 0);

  GtkWidget *frame_root_style = gtk_frame_new(_(_L("固定 gcin 視窗位置")));
  gtk_box_pack_start (GTK_BOX (vbox_R), frame_root_style, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_root_style), 3);
  GtkWidget *vbox_root_style = gtk_vbox_new (FALSE, 10);
  gtk_container_add (GTK_CONTAINER (frame_root_style), vbox_root_style);

  GtkWidget *hbox_root_style = gtk_hbox_new (FALSE, 10);

//  GtkWidget *hbox_root_style_use = gtk_hbox_new (FALSE, 10);
//  gtk_box_pack_start (GTK_BOX(vbox_root_style), hbox_root_style_use, FALSE, FALSE, 0);
  GtkWidget *label_root_style_use = gtk_label_new(_(_L("使用")));
  gtk_box_pack_start (GTK_BOX(hbox_root_style), label_root_style_use, FALSE, FALSE, 0);
  check_button_root_style_use = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_root_style_use),
       gcin_input_style == InputStyleRoot);
  gtk_box_pack_start (GTK_BOX(hbox_root_style), check_button_root_style_use, FALSE, FALSE, 0);


  gtk_box_pack_start (GTK_BOX(vbox_root_style), hbox_root_style, FALSE, FALSE, 0);

  GtkAdjustment *adj_root_style_x =
   (GtkAdjustment *) gtk_adjustment_new (gcin_root_x, 0.0, 5120.0, 1.0, 1.0, 0.0);
  spinner_root_style_x = gtk_spin_button_new (adj_root_style_x, 0, 0);
  gtk_widget_set_hexpand (spinner_root_style_x, TRUE);
  gtk_container_add (GTK_CONTAINER (hbox_root_style), spinner_root_style_x);

  GtkAdjustment *adj_root_style_y =
   (GtkAdjustment *) gtk_adjustment_new (gcin_root_y, 0.0, 2880.0, 1.0, 1.0, 0.0);
  spinner_root_style_y = gtk_spin_button_new (adj_root_style_y, 0, 0);
  gtk_widget_set_hexpand (spinner_root_style_y, TRUE);
  gtk_container_add (GTK_CONTAINER (hbox_root_style), spinner_root_style_y);


  gtk_box_pack_start (GTK_BOX(vbox_R), create_gcin_edit_display(), FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_inner_frame = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_L), hbox_gcin_inner_frame, FALSE, FALSE, 0);
  GtkWidget *label_gcin_inner_frame = gtk_label_new(_(_L("顯示邊框線")));
  gtk_box_pack_start (GTK_BOX(hbox_gcin_inner_frame), label_gcin_inner_frame, FALSE, FALSE, 0);
  check_button_gcin_inner_frame = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_inner_frame),
       gcin_inner_frame);
  gtk_box_pack_start (GTK_BOX(hbox_gcin_inner_frame), check_button_gcin_inner_frame, FALSE, FALSE, 0);

#if TRAY_ENABLED
  GtkWidget *hbox_gcin_status_tray = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX(vbox_R), hbox_gcin_status_tray, FALSE, FALSE, 0);

  GtkWidget *hbox_label_gcin_status_tray = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX(hbox_gcin_status_tray), hbox_label_gcin_status_tray, FALSE, FALSE, 0);
  GtkWidget *label_gcin_status_tray = gtk_label_new(_(_L("面板狀態(tray)")));
  gtk_box_pack_start (GTK_BOX(hbox_label_gcin_status_tray), label_gcin_status_tray, FALSE, FALSE, 0);
  check_button_gcin_status_tray = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX(hbox_label_gcin_status_tray), check_button_gcin_status_tray, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_status_tray), gcin_status_tray);


  GtkWidget *hbox_label_gcin_status_tray_windows_style = gtk_hbox_new(FALSE, 0);
#if UNIX
#if USE_INDICATOR
  gtk_box_pack_start (GTK_BOX(hbox_gcin_status_tray), create_status_icon_opts(), FALSE, FALSE, 20);
#else
  gtk_box_pack_start (GTK_BOX(hbox_gcin_status_tray), hbox_label_gcin_status_tray_windows_style, FALSE, FALSE, 20);
  GtkWidget *label_gcin_status_tray_windows_style = gtk_label_new(_(_L("雙圖示")));
  gtk_box_pack_start (GTK_BOX(hbox_label_gcin_status_tray_windows_style), label_gcin_status_tray_windows_style, FALSE, FALSE, 0);
  check_button_gcin_win32_icon = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX(hbox_label_gcin_status_tray_windows_style), check_button_gcin_win32_icon, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_win32_icon), gcin_win32_icon);
#endif
#else
  gtk_box_pack_start (GTK_BOX(hbox_gcin_status_tray), hbox_label_gcin_status_tray_windows_style, FALSE, FALSE, 10);
#endif

  GtkWidget *hbox_label_gcin_tray_hf_win_kbm = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX(hbox_gcin_status_tray), hbox_label_gcin_tray_hf_win_kbm, FALSE, FALSE, 0);
  GtkWidget *label_gcin_tray_hf_win_kbm = gtk_label_new(_(_L("全半形左鍵\n切換小鍵盤")));
  gtk_box_pack_start (GTK_BOX(hbox_label_gcin_tray_hf_win_kbm), label_gcin_tray_hf_win_kbm, FALSE, FALSE, 0);
  check_button_gcin_tray_hf_win_kbm = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX(hbox_label_gcin_tray_hf_win_kbm), check_button_gcin_tray_hf_win_kbm, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_tray_hf_win_kbm),
       gcin_tray_hf_win_kbm);

  GtkWidget *hbox_gcin_status_win = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX(vbox_R), hbox_gcin_status_win, FALSE, FALSE, 0);
  GtkWidget *label_gcin_status_win = gtk_label_new(_(_L("狀態跳出顯示")));
  gtk_box_pack_start (GTK_BOX(hbox_gcin_status_win), label_gcin_status_win, FALSE, FALSE, 0);
  check_button_gcin_status_win = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX(hbox_gcin_status_win), check_button_gcin_status_win, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_status_win), gcin_status_win);
#endif

  GtkWidget *frame_win_color = gtk_frame_new(_(_L("顏色選擇")));
  gtk_box_pack_start (GTK_BOX (vbox_L), frame_win_color, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_win_color), 1);
  GtkWidget *vbox_win_color = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame_win_color), vbox_win_color);

  GtkWidget *hbox_win_color_use = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_win_color), hbox_win_color_use, FALSE, FALSE, 0);
  GtkWidget *label_win_color_use = gtk_label_new(_(_L("取代主題顏色")));
  gtk_box_pack_start (GTK_BOX(hbox_win_color_use), label_win_color_use, FALSE, FALSE, 0);
  check_button_gcin_win_color_use = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_win_color_use),
       gcin_win_color_use);

  g_signal_connect (G_OBJECT (check_button_gcin_win_color_use), "clicked",
                    G_CALLBACK (cb_gcin_win_color_use), NULL);

  gtk_box_pack_start (GTK_BOX(hbox_win_color_use), check_button_gcin_win_color_use, FALSE, FALSE, 0);
  event_box_win_color_test = gtk_event_box_new();
// this will make the color test failed
//  gtk_event_box_set_visible_window (GTK_EVENT_BOX(event_box_win_color_test), FALSE);
  gtk_box_pack_start (GTK_BOX(vbox_win_color), event_box_win_color_test, FALSE, FALSE, 0);
  label_win_color_test = gtk_label_new(NULL);
  gtk_container_add (GTK_CONTAINER(event_box_win_color_test), label_win_color_test);
  GtkWidget *hbox_win_color_fbg = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vbox_win_color), hbox_win_color_fbg, FALSE, FALSE, 0);
  GtkWidget *button_fg = gtk_button_new_with_label(_(_L("前景顏色")));
  gtk_widget_set_hexpand (button_fg, TRUE);
  gtk_widget_set_halign (button_fg, GTK_ALIGN_FILL);
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_fg, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_fg), "clicked",
                    G_CALLBACK (cb_gcin_win_color_fg), &colorsel[0]);
  gdk_color_parse(gcin_win_color_fg, &gcin_win_gcolor_fg);
//  gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, &gcin_win_gcolor_fg);
  gdk_color_parse(gcin_win_color_bg, &gcin_win_gcolor_bg);
//  gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, &gcin_win_gcolor_bg);

  GtkWidget *button_bg = gtk_button_new_with_label(_(_L("背景顏色")));
  gtk_widget_set_hexpand (button_bg, TRUE);
  gtk_widget_set_halign (button_bg, GTK_ALIGN_FILL);
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_bg, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_bg), "clicked",
                    G_CALLBACK (cb_gcin_win_color_fg), &colorsel[1]);

  GtkWidget *button_gcin_sel_key_color = gtk_button_new_with_label(_(_L("選擇鍵顏色")));
  gtk_widget_set_hexpand (button_gcin_sel_key_color, TRUE);
  gtk_widget_set_halign (button_gcin_sel_key_color, GTK_ALIGN_FILL);
  g_signal_connect (G_OBJECT (button_gcin_sel_key_color), "clicked",
                    G_CALLBACK (cb_gcin_sel_key_color), G_OBJECT (gcin_kbm_window));
  gdk_color_parse(gcin_sel_key_color, &gcin_sel_key_gcolor);
  gtk_container_add (GTK_CONTAINER (hbox_win_color_fbg), button_gcin_sel_key_color);

  disp_fg_bg_color();


  GtkWidget *frame_tsin_cursor_color = gtk_frame_new(_(_L("游標的顏色")));
  gtk_box_pack_start (GTK_BOX (vbox_L), frame_tsin_cursor_color, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_cursor_color), 1);
  GtkWidget *button_tsin_cursor_color = gtk_button_new();
  g_signal_connect (G_OBJECT (button_tsin_cursor_color), "clicked",
                    G_CALLBACK (cb_tsin_cursor_color), G_OBJECT (gcin_kbm_window));
  da_cursor =  gtk_drawing_area_new();
  gtk_container_add (GTK_CONTAINER (button_tsin_cursor_color), da_cursor);
  gdk_color_parse(tsin_cursor_color, &tsin_cursor_gcolor);
#if !GTK_CHECK_VERSION(2,91,6)
  gtk_widget_modify_bg(da_cursor, GTK_STATE_NORMAL, &tsin_cursor_gcolor);
#else
  GdkRGBA rgbbg;
  gdk_rgba_parse(&rgbbg, gdk_color_to_string(&tsin_cursor_gcolor));
  gtk_widget_override_background_color(da_cursor, GTK_STATE_FLAG_NORMAL, &rgbbg);
#endif
  gtk_widget_set_size_request(da_cursor, 16, 2);
  gtk_container_add (GTK_CONTAINER (frame_tsin_cursor_color), button_tsin_cursor_color);


  GtkWidget *hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
//  gtk_grid_set_column_homogeneous(GTK_GRID(hbox_cancel_ok), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_cancel_ok, FALSE, FALSE, 0);

  GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  if (button_order)
    gtk_box_pack_end (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);
  else
    gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT (button_cancel), "clicked",
                            G_CALLBACK (close_appearance_conf_window),
                            G_OBJECT (gcin_appearance_conf_window));

  GtkWidget *button_close = gtk_button_new_from_stock (GTK_STOCK_OK);

  if (button_order)
    gtk_box_pack_end (GTK_BOX (hbox_cancel_ok), button_close, TRUE, TRUE, 0);
  else
    gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_close, TRUE, TRUE, 0);

  g_signal_connect_swapped (G_OBJECT (button_close), "clicked",
                            G_CALLBACK (cb_appearance_conf_ok),
                            G_OBJECT (gcin_kbm_window));

  GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button_close);

  GtkWidget *button_help = gtk_button_new_from_stock (GTK_STOCK_HELP);
  gtk_box_pack_end (GTK_BOX (hbox_cancel_ok), button_help, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (button_help), "clicked",
                            G_CALLBACK (cb_appearance_help),
                            G_OBJECT (gcin_appearance_conf_window));

  gtk_widget_show_all (gcin_appearance_conf_window);

  return;
}
