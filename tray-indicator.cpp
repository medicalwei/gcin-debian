#include "gcin.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"
#include "eggtrayicon.h"
#include <string.h>
#include <signal.h>
#include "gst.h"
#include "pho-kbm-name.h"
#include <libappindicator/app-indicator.h>
#include <dlfcn.h>
#include "mitem.h"

void cb_sim2trad(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_tog_phospeak(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void restart_gcin(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_tog_gcb(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_trad_sim_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_stat_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_about_window(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void exec_gcin_setup_(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void kbm_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_set_output_buffer_bak_to_clipboard(GtkCheckMenuItem *checkmenuitem, gpointer dat);
void cb_half_full_char(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void show_inmd_menu();
static void cb_inmd_menu(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  show_inmd_menu();
}

gboolean tsin_pho_mode();
extern int tsin_half_full, gb_output;
extern int win32_tray_disabled;
void (*f_app_indicator_set_icon_full)(AppIndicator *self, const gchar *icon_name, const gchar *icon_desc);

static AppIndicator *indicator_main, *indicator_state;

void get_icon_path(char *iconame, char fname[]);


void show_inmd_menu();

void close_all_clients();
void do_exit();

void kbm_open_close(gboolean b_show);


gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);
extern gboolean win_kbm_inited;


extern int win_kbm_on;

void load_setttings(), load_tab_pho_file();;
void update_win_kbm();
void update_win_kbm_inited(), toggle_half_full_char();
extern gboolean win_kbm_inited, stat_enabled;
void cb_fast_phonetic_kbd_switch(GtkCheckMenuItem *checkmenuitem, gpointer dat);


static GtkWidget *tray_menu=NULL, *tray_menu_state=NULL;


static MITEM mitems_main[] = {
  {"關於gcin/常見問題", GTK_STOCK_ABOUT, cb_about_window},
  {"設定/工具", GTK_STOCK_PREFERENCES, exec_gcin_setup_},
  {"gcb(剪貼區暫存)", NULL, cb_tog_gcb, &gcb_enabled},
  {"重新執行gcin", GTK_STOCK_QUIT, restart_gcin},
  {"念出發音", NULL, cb_tog_phospeak, &phonetic_speak},
  {"小鍵盤", NULL, kbm_toggle_, &win_kbm_on},
  {"選擇輸入法", GTK_STOCK_INDEX, cb_inmd_menu, NULL},
  {NULL}
};

static MITEM mitems_state[] = {
  {"切到舊的注音鍵盤", NULL, cb_fast_phonetic_kbd_switch},
  {"正→簡體", NULL, cb_trad2sim},
  {"簡→正體", NULL, cb_sim2trad},
  {"简体输出", NULL, cb_trad_sim_toggle_, &gb_output},
  {"打字速度", NULL, cb_stat_toggle_, &stat_enabled},
  {"全半形切換", NULL, cb_half_full_char, NULL},
  {"送字到剪貼區", NULL, cb_set_output_buffer_bak_to_clipboard},
  {NULL}
};



static void strip_png(char *s, char *t)
{
	strcpy(t, s);
	char *p = strchr(t, '.');
	*p=0;
}

static void create_indicator_menu(char *icon, char *icon_state)
{
  int i;  
//  dbg("create_indicator_menu %s %s %s\n", GCIN_ICON_DIR, icon, icon_state);
  
  tray_menu = create_tray_menu(mitems_main);
  tray_menu_state = create_tray_menu(mitems_state);  

  static void *handle;
  char *ld_error;

  if (!handle) {
    if (!(handle = dlopen("libappindicator.so", RTLD_LAZY)) && !(handle = dlopen("libappindicator.so.1", RTLD_LAZY))) {
      if ((ld_error = dlerror()) != NULL)
        box_warn(ld_error);
      return;
    } 
  }

  AppIndicator  *(*f_app_indicator_new_with_path) (const gchar *id, const gchar *icon_name, AppIndicatorCategory category, const gchar *icon_theme_path) = dlsym(handle, "app_indicator_new_with_path");  
  void (*f_app_indicator_set_status) (AppIndicator *self, AppIndicatorStatus  status) = dlsym(handle, "app_indicator_set_status");
  f_app_indicator_set_icon_full  = dlsym(handle, "app_indicator_set_icon_full");
  void (*f_app_indicator_set_menu) (AppIndicator *self, GtkMenu *menu) = dlsym(handle, "app_indicator_set_menu");

  indicator_main = f_app_indicator_new_with_path ("gcin-main", icon, APP_INDICATOR_CATEGORY_APPLICATION_STATUS, GCIN_ICON_DIR);
  f_app_indicator_set_status (indicator_main, APP_INDICATOR_STATUS_ACTIVE);
  f_app_indicator_set_menu (indicator_main, GTK_MENU (tray_menu));
  
  indicator_state = f_app_indicator_new_with_path ("gcin-state", icon_state, APP_INDICATOR_CATEGORY_APPLICATION_STATUS, GCIN_ICON_DIR);
  f_app_indicator_set_status (indicator_state, APP_INDICATOR_STATUS_ACTIVE);
  f_app_indicator_set_menu (indicator_state, GTK_MENU (tray_menu_state));
}


void update_item_active_unix();

void update_item_active_all_indicator()
{
}


void inmd_popup_tray();

void toggle_half_full_char();



#define GCIN_TRAY_PNG "en-gcin.png"

void disp_win_screen_status(char *in_method, char *half_status);

void set_indicator_icon(AppIndicator *ind, char *icon)
{
//	dbg("set_indicator_icon %s\n", icon);
	f_app_indicator_set_icon_full(ind, icon, "zz");
}

void load_tray_icon_indicator()
{	
//  dbg("load_tray_icon_indicator\n");
  char *tip;
  tip="";

  char *iconame;
  if (!current_CS || current_CS->im_state == GCIN_STATE_DISABLED||current_CS->im_state == GCIN_STATE_ENG_FULL) {
    iconame=GCIN_TRAY_PNG;
  } else {
    iconame=inmd[current_CS->in_method].icon;
  }

//  dbg("tsin_pho_mode() %d\n", tsin_pho_mode());

  char tt[32];
  if (current_CS && current_CS->im_state == GCIN_STATE_CHINESE && !tsin_pho_mode()) {
    if ((current_method_type()==method_type_TSIN || current_method_type()==method_type_MODULE)) {
      strcpy(tt, "en-");
      strcat(tt, iconame);
    } else {
      if (current_method_type()==method_type_GTAB)
        strcpy(tt, "en-gtab.png");
      else
        strcpy(tt, "en-tsin.png");
    }

    iconame = tt;
  }

//  dbg("iconame %s\n", iconame);
  char fname[128];
  fname[0]=0;
  if (iconame)
    get_icon_path(iconame, fname);


  char *icon_st=NULL;
  char fname_state[128];

//  dbg("%d %d\n",current_CS->im_state,current_CS->b_half_full_char);

  if (current_CS && (current_CS->im_state == GCIN_STATE_ENG_FULL ||
      current_CS->im_state != GCIN_STATE_DISABLED && current_CS->b_half_full_char ||
      current_method_type()==method_type_TSIN && tss.tsin_half_full)) {
      if (gb_output) {
        icon_st="full-simp.png";
        tip = _L("全形/簡體輸出");
      }
      else {
        icon_st="full-trad.png";
        tip = _L("全形/正體輸出");
      }
  } else {
    if (gb_output) {
      icon_st="half-simp.png";
      tip= _L("半形/簡體輸出");
    } else {
      icon_st="half-trad.png";
      tip = _L("半形/正體輸出");
    }
  }

  get_icon_path(icon_st, fname_state);
//  dbg("wwwwwwww %s\n", fname_state);

  if (gcin_status_win)
    disp_win_screen_status(fname, fname_state);


  if (!gcin_win32_icon || !gcin_status_tray)
    return;

  char icon_[32];
  char icon_state_[32];
  strip_png(iconame, icon_);
  strip_png(icon_st, icon_state_);


  if (!indicator_main) {
	create_indicator_menu(icon_, icon_state_);
  }
  
  dbg("%s %s\n", icon_, icon_state_);
  
  set_indicator_icon(indicator_main, icon_);
  set_indicator_icon(indicator_state, icon_state_);

  return;
}

void init_tray_indicator()
{
  load_tray_icon_indicator();
}

void destroy_tray_indicator()
{
}
