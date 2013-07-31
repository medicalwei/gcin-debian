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

#if UNIX
char utf8_edit[]=GCIN_SCRIPT_DIR"/utf8-edit";
#endif

static GtkWidget *check_button_root_style_use,
                 *check_button_gcin_pop_up_win,
                 *check_button_gcin_inner_frame,
#if TRAY_ENABLED
                 *check_button_gcin_status_tray,
                 *check_button_gcin_win32_icon,
                 *check_button_gcin_tray_hf_win_kbm,
#endif
                 *check_button_gcin_win_color_use,
                 *check_button_gcin_on_the_spot_key;


static GtkWidget *gcin_kbm_window = NULL, *gcin_appearance_conf_window;
static GtkClipboard *pclipboard;
static GtkWidget *opt_gcin_edit_display;
GtkWidget *main_window;
static GdkColor gcin_win_gcolor_fg, gcin_win_gcolor_bg, gcin_sel_key_gcolor;
gboolean button_order;

static gboolean close_application( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  exit(0);
}


void create_kbm_window();

static void cb_kbm()
{
  create_kbm_window();
}

static void cb_tslearn()
{
#if UNIX
  system("tslearn &");
#else
  win32exec("tslearn.exe");
#endif
  exit(0);
}


static void cb_ret(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_destroy((GtkWidget*)user_data);
}

#include <string.h>

static void create_result_win(int res, char *cmd)
{
  char tt[512];

  if (res) {
    sprintf(tt, "%s code:%d '%s'\n%s", _(_L("結果失敗")), res, strerror(res), cmd);
  }
  else
    strcpy(tt, _(_L("結果成功")));

  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_MOUSE);
  gtk_window_set_has_resize_grip(GTK_WINDOW(main_window), FALSE);

  GtkWidget *button = gtk_button_new_with_label(tt);
  gtk_container_add (GTK_CONTAINER (main_window), button);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_ret), main_window);

  gtk_widget_show_all(main_window);
}


static void cb_ts_export()
{
   GtkWidget *file_selector;
   if (button_order)
       file_selector = gtk_file_chooser_dialog_new(_(_L("請輸入要匯出的檔案名稱")),
                              GTK_WINDOW(main_window),
                              GTK_FILE_CHOOSER_ACTION_SAVE,
                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              NULL);
   else
       file_selector = gtk_file_chooser_dialog_new(_(_L("請輸入要匯出的檔案名稱")),
                              GTK_WINDOW(main_window),
                              GTK_FILE_CHOOSER_ACTION_SAVE,
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                              NULL);
   if (gtk_dialog_run (GTK_DIALOG (file_selector)) == GTK_RESPONSE_ACCEPT) {
       gchar *selected_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_selector));
       char gcin_dir[512];
       get_gcin_dir(gcin_dir);
       char cmd[512];
       char fname[256];
       char *filename=inmd[default_input_method].filename;

       if (inmd[default_input_method].method_type==method_type_TSIN)
         get_gcin_user_fname(tsin32_f, fname);
       else
       if (filename) {
         char tt[256];
         strcat(strcpy(tt, filename), ".append.gtab.tsin-db");
         if (!get_gcin_user_fname(tt, fname)) {
           strcat(strcpy(tt, filename), ".tsin-db");
           if (!get_gcin_user_fname(tt, fname))
             p_err("cannot find %s", fname);
         }
       }
#if UNIX
       snprintf(cmd, sizeof(cmd), GCIN_BIN_DIR"/tsd2a32 %s -o %s", fname, selected_filename);
       dbg("exec %s\n", cmd);
       int res = system(cmd);
       res = 0; // some problem in system(), the exit code is not reliable
       create_result_win(res, cmd);
#else
	   char para[256];
       sprintf_s(para, sizeof(para), "\"%s\" -o \"%s\"", fname, selected_filename);
	   win32exec_para("tsd2a32.exe", para);
#endif
   }
   gtk_widget_destroy (file_selector);
}

static void ts_import(const gchar *selected_filename)
{
   char cmd[256];
#if UNIX
   if (inmd[default_input_method].method_type==method_type_TSIN) {
     snprintf(cmd, sizeof(cmd),
        "cd %s/.gcin && "GCIN_BIN_DIR"/tsd2a32 %s > tmpfile && cat %s >> tmpfile && "GCIN_BIN_DIR"/tsa2d32 tmpfile %s",
        getenv("HOME"), tsin32_f, selected_filename, tsin32_f);
     int res = system(cmd);
     res = 0;
     create_result_win(res, cmd);
   } else {
     char tt[512];
     sprintf(tt, GCIN_SCRIPT_DIR"/tsin-gtab-import %s '%s'", inmd[default_input_method].filename,
     selected_filename);
     system(tt);
   }
#else
   if (inmd[default_input_method].method_type==method_type_TSIN)
     win32exec_script_va("ts-import.bat", (char *)selected_filename, tsin32_f, NULL);
   else {
     win32exec_script_va("ts-gtab-import.bat", inmd[default_input_method].filename,  selected_filename, NULL);
   }
#endif
}

#if !GTK_CHECK_VERSION(2,4,0)
static void cb_file_ts_import(GtkWidget *widget, gpointer user_data)
{
   GtkWidget *file_selector = (GtkWidget *)user_data;
   const gchar *selected_filename;

   selected_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_selector));
//   g_print ("Selected filename: %s\n", selected_filename);

   ts_import(selected_filename);
}
#endif

static void cb_ts_import()
{
   /* Create the selector */

#if GTK_CHECK_VERSION(2,4,0)
   GtkWidget *file_selector;
   if (button_order)
       file_selector = gtk_file_chooser_dialog_new(_(_L("請輸入要匯入的檔案名稱")),
                              GTK_WINDOW(main_window),
                              GTK_FILE_CHOOSER_ACTION_OPEN,
                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              NULL);
   else
       file_selector = gtk_file_chooser_dialog_new(_(_L("請輸入要匯入的檔案名稱")),
                              GTK_WINDOW(main_window),
                              GTK_FILE_CHOOSER_ACTION_OPEN,
                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                              NULL);
   if (gtk_dialog_run (GTK_DIALOG (file_selector)) == GTK_RESPONSE_ACCEPT) {
       gchar *selected_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_selector));

       ts_import(selected_filename);
   }
   gtk_widget_destroy (file_selector);
#else
   GtkWidget *file_selector = gtk_file_selection_new (_("請輸入要匯入的檔案名稱"));

   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (cb_file_ts_import),
                     (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   gtk_widget_show(file_selector);
#endif
}

static void cb_ts_edit()
{
#if 0
#if UNIX
  if (inmd[default_input_method].method_type==method_type_TSIN) {
    char tt[512];
    sprintf(tt, "( cd ~/.gcin && "GCIN_BIN_DIR"/tsd2a32 %s > tmpfile && %s tmpfile && "GCIN_BIN_DIR"/tsa2d32 tmpfile %s) &",
      tsin32_f, utf8_edit, tsin32_f);
    dbg("exec %s\n", tt);
    system(tt);
  } else {
    char tt[512];
    sprintf(tt, GCIN_SCRIPT_DIR"/tsin-gtab-edit %s", inmd[default_input_method].filename);
    system(tt);
  }
#else
  if (inmd[default_input_method].method_type==method_type_TSIN)
    win32exec_script("ts-edit.bat", tsin32_f);
  else {
    win32exec_script("ts-gtab-edit.bat", inmd[default_input_method].filename);
  }
#endif
#else
#if UNIX
  system(GCIN_BIN_DIR"/ts-edit");
#else
  win32exec("ts-edit.exe");
#endif
#endif
}


static void cb_ts_import_sys()
{
#if UNIX
  char tt[512];
  sprintf(tt, "cd ~/.gcin && "GCIN_BIN_DIR"/tsd2a32 %s > tmpfile && "GCIN_BIN_DIR"/tsd2a32 %s/%s >> tmpfile && "GCIN_BIN_DIR"/tsa2d32 tmpfile",
    tsin32_f, GCIN_TABLE_DIR, tsin32_f);
  dbg("exec %s\n", tt);
  system(tt);
#else
  win32exec_script("ts-import-sys.bat", tsin32_f);
#endif
}

static void cb_ts_contribute()
{
#if UNIX
  system(GCIN_BIN_DIR"/ts-contribute &");
#else
  win32exec("ts-contribute.exe");
#endif
}


static void cb_alt_shift()
{
#if UNIX
  char tt[512];
  sprintf(tt, "( cd ~/.gcin && %s phrase.table ) &", utf8_edit);
  system(tt);
#else
  char fname[512];
  get_gcin_user_fname("phrase.table", fname);
  win32exec_script("utf8-edit.bat", fname);
#endif
}


static void cb_symbol_table()
{
  char tt[512];
#if UNIX
  sprintf(tt, "( cd ~/.gcin && %s symbol-table ) &", utf8_edit);
  system(tt);
#else
  char fname[512];
  get_gcin_user_fname("symbol-table", fname);
  win32exec_script("utf8-edit.bat", fname);
#endif
}


int html_browser(char *fname);

static void cb_help()
{
#if UNIX
  html_browser(DOC_DIR"/README.html");
#else
  char fname[512];
  strcpy(fname, gcin_program_files_path);
  strcat(fname, "\\README.html");
  html_browser(fname);
#endif
}

void create_appearance_conf_window();

static void cb_appearance_conf()
{
  create_appearance_conf_window();
}

void create_gtab_conf_window();

static void cb_gtab_conf()
{
  create_gtab_conf_window();
}


static void cb_gb_output_toggle()
{
  send_gcin_message(
#if UNIX
	  GDK_DISPLAY(),
#endif
	  GB_OUTPUT_TOGGLE);
  exit(0);
}

static void cb_gb_translate_toggle()
{
#if WIN32
  win32exec("sim2trad.exe");
#else
  system(GCIN_BIN_DIR"/sim2trad &");
#endif
  exit(0);
}


static void cb_juying_learn()
{
#if WIN32
  win32exec("juyin-learn.exe");
#else
  system(GCIN_BIN_DIR"/juyin-learn &");
#endif
  exit(0);
}

#if 0
int gcin_pid;
static void cb_gcin_exit()
{
#if UNIX
  kill(gcin_pid, 9);
#else
  TerminateProcess(
#endif
}
#endif

void create_gtablist_window();
static void cb_default_input_method()
{
  create_gtablist_window();
}

void create_about_window();
void set_window_gcin_icon(GtkWidget *window);

static gboolean timeout_minimize_main_window(gpointer data)
{
  gtk_window_resize(GTK_WINDOW(main_window), 32, 32);
  return FALSE;
}

static void
expander_callback (GObject    *object,
                   GParamSpec *param_spec,
                   gpointer    user_data)
{
  GtkExpander *expander;
  expander = GTK_EXPANDER (object);

  if (gtk_expander_get_expanded (expander)) {
  } else {
    // cannot do this here
    // gtk_window_resize(GTK_WINDOW(main_window), 32, 32);
    g_timeout_add(200, timeout_minimize_main_window, NULL);
  }
}

#include "pho.h"
#include "tsin.h"
#include "gst.h"
#include "im-client/gcin-im-client-attr.h"
#include "win1.h"
#include "gcin-module.h"
#include "gcin-module-cb.h"
GCIN_module_callback_functions *init_GCIN_module_callback_functions(char *sofile);

static void create_main_win()
{
  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

  gtk_window_set_has_resize_grip(GTK_WINDOW(main_window), FALSE);

  g_signal_connect (G_OBJECT (main_window), "delete_event",
                     G_CALLBACK (close_application),
                     NULL);

  set_window_gcin_icon(main_window);

  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (main_window), vbox);

  GtkWidget *button_kbm = gtk_button_new_with_label(_(_L("gcin 注音/詞音/拼音 設定")));
  gtk_box_pack_start (GTK_BOX (vbox), button_kbm, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_kbm), "clicked",
                    G_CALLBACK (cb_kbm), NULL);

  GtkWidget *button_appearance_conf = gtk_button_new_with_label(_(_L("外觀設定")));
  gtk_box_pack_start (GTK_BOX (vbox), button_appearance_conf, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_appearance_conf), "clicked",
                    G_CALLBACK (cb_appearance_conf), NULL);

  GtkWidget *button_gtab_conf = gtk_button_new_with_label(_(_L("倉頡/行列/嘸蝦米/大易設定")));
  gtk_box_pack_start (GTK_BOX (vbox), button_gtab_conf, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_gtab_conf), "clicked",
                    G_CALLBACK (cb_gtab_conf), NULL);

  int i;
  for (i=0; i < inmdN; i++) {
    INMD *pinmd = &inmd[i];
    if (pinmd->method_type != method_type_MODULE || pinmd->disabled)
      continue;

    GCIN_module_callback_functions *f = init_GCIN_module_callback_functions(pinmd->filename);
    if (!f)
      continue;

    if (!f->module_setup_window_create) {
      free(f);
      continue;
    }

    char tt[128];
    strcpy(tt, pinmd->cname);
    strcat(tt, _(_L("設定")));
    GtkWidget *button_chewing_input_method = gtk_button_new_with_label(tt);
    gtk_box_pack_start (GTK_BOX (vbox), button_chewing_input_method, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (button_chewing_input_method), "clicked",
                    G_CALLBACK (f->module_setup_window_create), NULL);
  }

  GtkWidget *button_default_input_method = gtk_button_new_with_label(_(_L("內定輸入法 & 開啟/關閉")));
  gtk_box_pack_start (GTK_BOX (vbox), button_default_input_method, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_default_input_method), "clicked",
                    G_CALLBACK (cb_default_input_method), NULL);


  GtkWidget *button_alt_shift = gtk_button_new_with_label(_(_L("alt-shift 片語編輯")));
  gtk_box_pack_start (GTK_BOX (vbox), button_alt_shift, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_alt_shift), "clicked",
                    G_CALLBACK (cb_alt_shift), NULL);

  GtkWidget *button_symbol_table = gtk_button_new_with_label(_(_L("符號表編輯")));
  gtk_box_pack_start (GTK_BOX (vbox), button_symbol_table, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_symbol_table), "clicked",
                    G_CALLBACK (cb_symbol_table), NULL);

#if TRAY_ENABLED
  if (!gcin_status_tray)
  {
#endif
    GtkWidget *button_gb_output_toggle = gtk_button_new_with_label(_(_L("簡體字輸出切換")));
    gtk_box_pack_start (GTK_BOX (vbox), button_gb_output_toggle, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (button_gb_output_toggle), "clicked",
                      G_CALLBACK (cb_gb_output_toggle), NULL);
#if TRAY_ENABLED
  }
#endif

  GtkWidget *button_gb_translate_toggle = gtk_button_new_with_label(_(_L("剪貼區 簡體字->正體字")));
  gtk_box_pack_start (GTK_BOX (vbox), button_gb_translate_toggle, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_gb_translate_toggle), "clicked",
                    G_CALLBACK (cb_gb_translate_toggle), NULL);

  GtkWidget *button_juying_learn_toggle = gtk_button_new_with_label(_(_L("剪貼區 注音查詢")));
  gtk_box_pack_start (GTK_BOX (vbox), button_juying_learn_toggle, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_juying_learn_toggle), "clicked",
                    G_CALLBACK (cb_juying_learn), NULL);

  GtkWidget *expander_ts = gtk_expander_new (_(_L("詞庫選項")));
  gtk_box_pack_start (GTK_BOX (vbox), expander_ts, FALSE, FALSE, 0);
  g_signal_connect (expander_ts, "notify::expanded",
                  G_CALLBACK (expander_callback), NULL);

  GtkWidget *vbox_ts = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (expander_ts), vbox_ts);


  GtkWidget *button_ts_export = gtk_button_new_with_label(_(_L("詞庫匯出")));
  gtk_widget_set_hexpand (button_ts_export, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_export, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_ts_export), "clicked",
                    G_CALLBACK (cb_ts_export), NULL);

  GtkWidget *button_ts_import = gtk_button_new_with_label(_(_L("詞庫匯入")));
  gtk_widget_set_hexpand (button_ts_import, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_import, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_ts_import), "clicked",
                    G_CALLBACK (cb_ts_import), NULL);

  GtkWidget *button_ts_edit = gtk_button_new_with_label(_(_L("詞庫編輯")));
  gtk_widget_set_hexpand (button_ts_edit, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_edit, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_edit), "clicked",
                    G_CALLBACK (cb_ts_edit), NULL);

  if (inmd[default_input_method].method_type == method_type_TSIN) {
  GtkWidget *button_tslearn = gtk_button_new_with_label(_(_L("讓詞音從文章學習詞")));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_tslearn, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_tslearn), "clicked",
                    G_CALLBACK (cb_tslearn), NULL);

  GtkWidget *button_ts_import_sys = gtk_button_new_with_label(_(_L("匯入系統的詞庫")));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_import_sys, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_import_sys), "clicked",
                    G_CALLBACK (cb_ts_import_sys), NULL);

  GtkWidget *button_ts_contribute = gtk_button_new_with_label(_(_L("貢獻選擇的詞庫")));
  gtk_box_pack_start (GTK_BOX (vbox_ts), button_ts_contribute, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_contribute), "clicked",
                    G_CALLBACK (cb_ts_contribute), NULL);
  }


  GtkWidget *button_about = gtk_button_new_with_label(_(_L("關於 gcin")));
  gtk_box_pack_start (GTK_BOX (vbox), button_about, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_about), "clicked",
                    G_CALLBACK (create_about_window),  NULL);


  GtkWidget *button_help = gtk_button_new_from_stock (GTK_STOCK_HELP);
  gtk_box_pack_start (GTK_BOX (vbox), button_help, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_help), "clicked",
                    G_CALLBACK (cb_help), NULL);


  GtkWidget *button_quit = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  gtk_box_pack_start (GTK_BOX (vbox), button_quit, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_quit), "clicked",
                    G_CALLBACK (close_application), NULL);

  gtk_widget_show_all(main_window);
}


void init_TableDir(), exec_setup_scripts();
#if WIN32
void init_gcin_program_files();
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif


int main(int argc, char **argv)
{
//  char *messages=getenv("LC_MESSAGES");
#if 0
  char *ctype=getenv("LC_CTYPE");
  if (!(ctype && strstr(ctype, "zh_CN")))
    putenv("LANGUAGE=zh_TW.UTF-8");
#endif

  set_is_chs();


#if UNIX
  setenv("GCIN_BIN_DIR", GCIN_BIN_DIR, TRUE);
  setenv("UTF8_EDIT", utf8_edit, TRUE);
#endif

  exec_setup_scripts();

  init_TableDir();

  load_setttings();

  load_gtab_list(FALSE);

  gtk_init(&argc, &argv);

#if GCIN_i18n_message
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif

  g_object_get(gtk_settings_get_default(), "gtk-alternative-button-order", &button_order, NULL);

  create_main_win();

  // once you invoke gcin-setup, the left-right buton tips is disabled
  save_gcin_conf_int(LEFT_RIGHT_BUTTON_TIPS, 0);

  pclipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);

  gtk_main();

  return 0;
}
