
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "proj.h"

int read_stdin(char *buf, int buf_len, int *more) {
    int tot_chars = 0;

	if (fgets(buf ,buf_len ,stdin))
	{
		tot_chars = strlen(buf);
	}
	*more = 1;
	if (strchr(buf, '\n'))
	{
		*more = 0;
	}
	return tot_chars;
}
