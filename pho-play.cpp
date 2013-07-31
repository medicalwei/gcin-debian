#include "gcin.h"
#include "pho.h"

void load_tab_pho_file();

#if UNIX
#include <signal.h>
gboolean test_mode;

int pho_play(phokey_t key)
{
  if (!phonetic_speak)
    return 0;
  if (test_mode)
    return 0;
    
  static char ogg123[]="/usr/bin/ogg123";  
  if (access(ogg123, R_OK)) {
    box_warn("gcin speech needs 'ogg123' in 'vorbis-tools'");
    exit(1);
  }

  if (access(GCIN_OGG_DIR, R_OK)) {
	box_warn("You have install the speech data:'ogg.tgz'");
    exit(1);
  }
        
  static int pid;
  static time_t last_time;
  time_t t = time(NULL);
  if (!gcin_sound_play_overlap) {
    if (pid && t - last_time < 2)
      kill(pid, 9);
  }
  char *ph = phokey_to_str2(key, 1);
  
  static char tt[128];
  sprintf(tt, GCIN_OGG_DIR"/%s/%s", ph, phonetic_speak_sel);
  last_time = t;        
  
  if (pid = fork()) {
    if (pid < 0)
      dbg("cannot fork ?");
    return 1;
  }

#if 1
  close(1);
  close(2);
#endif

  execl(ogg123, "ogg123", tt, NULL);
  exit(1);
}
#else
extern "C" {
	HANDLE play_ogg_file(wchar_t *file_name);
};

void ErrorExit(LPTSTR lpszFunction);
void pho_play(phokey_t key)
{
  if (!phonetic_speak)
    return;

#if 0
  static PROCESS_INFORMATION procinfo;
  static time_t last_time;
  time_t t = time(NULL);
#if 0
  // the result is bad on windows
  if (!gcin_sound_play_overlap) {
    if (procinfo.hProcess && t - last_time < 2)
      TerminateProcess(procinfo.hProcess, 0);
  }
#endif
  char *ph = phokey_to_str2(key, 1);
  char tt[512];

  last_time = t;
  if (procinfo.hProcess) {
    CloseHandle(procinfo.hProcess);
    CloseHandle(procinfo.hThread);
  }

  sprintf(tt, "\"%s\\bin\\oggdec.exe\" -p \"%s\\ogg\\%s\\%s\"", gcin_program_files_path, gcin_program_files_path, ph, phonetic_speak_sel);
  wchar_t tt16[MAX_PATH*2];

  utf8_to_16(tt, tt16, sizeof(tt16));
#if 0
  char pro[64];
  wchar_t pro16[64];
  strcpy(pro, gcin_program_files_path);
  strcat(pro, "\\oggdec.exe");
  utf8_to_16(pro, pro16, ARRAYSIZE(tt16));
#endif
  STARTUPINFOW si;
//  PROCESS_INFORMATION pi;

  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &procinfo, sizeof(procinfo));

  if(!CreateProcessW(NULL, tt16, NULL,NULL,FALSE, CREATE_NO_WINDOW, NULL,NULL, &si, &procinfo)) {
#if _DEBUG
    ErrorExit("CreateProcessW");
    dbg("cannot exec %s\n", tt);
#endif
  }
#else
  char *ph = phokey_to_str2(key, 1);
  char tt[512];
  sprintf(tt, "%s\\ogg\\%s\\%s", gcin_program_files_path, ph, phonetic_speak_sel);
  wchar_t tt16[MAX_PATH];
  utf8_to_16(tt, tt16, ARRAYSIZE(tt16));
  play_ogg_file(tt16);
#endif
}
#endif

void char_play(char *utf8)
{
  if (!phonetic_speak || !(utf8[0]&128))
    return;

  if (!ch_pho)
    load_tab_pho_file();

  phokey_t phos[16];
  int phosN = utf8_pho_keys((char *)utf8, phos);

  if (!phosN)
    return;

#if 0
  int i;
  for(i=0; i < phosN; i++)
    pho_play(phos[i]);
#else
  pho_play(phos[0]);
#endif
}
