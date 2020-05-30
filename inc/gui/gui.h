#ifndef GUI_
#define GUI_

#include <cairo.h>
#include <gtk/gtk.h>
#include <stdbool.h>

enum PRICE_ACTION_COLUMNS
{
  PAC_SEC,
  PAC_BID,
  PAC_ASK,
  PAC_LST,
  PAC_CHG,
  PAC_NUM
};

void gui_start (int argc, char **argv);

#endif
