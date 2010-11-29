#include "gcin.h"

int main()
{
  gdk_init(NULL, NULL);
  send_gcin_message(GDK_DISPLAY(), GCIN_EXIT);

  return 0;
}
