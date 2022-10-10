
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUF_LEN 10

int read_stdin(char *buf, int buf_len, int *more);

int main(int argc, char **argv) {

    FILE *fp;
    char *buf;
    int line_num = 1;

    buf = malloc(BUF_LEN);

    if (argc != 2) {
        fprintf(stderr, "Usage: ./main_read_stdin <file name>");
        exit(EXIT_FAILURE);
    }

    printf("Buffer size is %d. Enter input of any length and press enter. To exit, press Ctrl+c. Check output file <%s> to test your code.\n",
           BUF_LEN, argv[1]);

    /*
     * Read input from stdin and write to given file.
     * When writing to the file, number the input lines. Even if the input is longer than BUF_LEN, it should appear on one line in the file.
     */
    for (int i = 0; i < 5; i++) {  // loop to accept input 5 times. Can exit early with Ctrl + c.
        printf("Enter message: ");
        int more = 1;
        int n;
        fp = fopen(argv[1], "a");
        fprintf(fp, "%d. ", line_num++);
        while (more) {
            n = read_stdin(buf, BUF_LEN, &more);
            if (n > 0) {
                fprintf(fp, "%s", buf);
            }
        }
        fclose(fp);
    }

    return 0;
}
