#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

#include <signal.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <pty.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "sti/sti.h"
#define HAVE_STI

#include "str_utils.h"

#include "parser.h"


#define new(t, v) t v = calloc(1, sizeof(*v));


