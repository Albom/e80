#include "kolibri.h"

///=============================

#define MB_OK 0
#define MB_OKCANCEL 1
#define MB_ABORTRETRYIGNORE 2 
#define MB_YESNOCANCEL 3
#define MB_YESNO 4
#define MB_RETRYCANCEL 5

#define IDOK 1
#define IDCANCEL 2
#define IDABORT 3
#define IDRETRY 4
#define IDIGNORE 5
#define IDYES 6
#define IDNO 7

///=============================

char MessageBox(char *text, char *caption, int type);
