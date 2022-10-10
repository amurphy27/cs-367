/* read_stdin.c - code for read_stdin function.
 * 1) DO NOT rename this file
 * 2) DO NOT add main() to this file. Create a separate file to write the driver main function (or see main_read_stdin.c)
 * */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


/**
 * Read stdin until the newline character or at most buf_len-1 characters, and save read characters in buf.
 *
 * The newline character, if any, is retained. If any characters are read, a `\0` in appended to end the string.
 *
 * @param buf Result parameter. On successful read of stdin, this is filled with read input (at most buf_len)
 * @param bul_len Length of buffer. Read at most these many characters.
 * @param more Result parameter. Contains 1 if there is more data on stdin that can be read, 0 otherwise
 * @return length of characters read from stdin
 */
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
