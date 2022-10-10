/* client.c - code for client program. Do not rename this file */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#include "proj.h"

int main( int argc, char **argv) {
    struct hostent *ptrh; /* pointer to a host table entry */
    struct protoent *ptrp; /* pointer to a protocol table entry */
    struct sockaddr_in sad; /* structure to hold an IP address */
    int sd; /* socket descriptor */
    int port; /* protocol port number */
    char *host; /* pointer to host name */
    //int n; /* number of characters read */

    memset((char *) &sad, 0, sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET; /* set family to Internet */


    if (argc != 3) {
        fprintf(stderr, "Error: Wrong number of arguments\n");
        fprintf(stderr, "usage:\n");
        fprintf(stderr, "./client server_address server_port\n");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[2]); /* convert to binary */
    if (port > 0) /* test for legal value */
        sad.sin_port = htons((u_short) port);
    else {
        fprintf(stderr, "Error: bad port number %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    host = argv[1]; /* if host argument specified */

    /* Convert host name to equivalent IP address and copy to sad. */
    ptrh = gethostbyname(host);
    if (ptrh == NULL) {
        fprintf(stderr, "Error: Invalid host: %s\n", host);
        exit(EXIT_FAILURE);
    }

    memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

    /* Map TCP transport protocol name to protocol number. */
    if (((long int) (ptrp = getprotobyname("tcp"))) == 0) {
        perror("Error: Cannot map \"tcp\" to protocol number");
        exit(EXIT_FAILURE);
    }

    /* Create a socket. */
    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* Connect the socket to the specified server. */
    if (connect(sd, (struct sockaddr *) &sad, sizeof(sad)) < 0) {
        perror("Connect failed");
        exit(EXIT_FAILURE);
    }

    /* Repeatedly read data from socket and write to user's screen. */
    uint8_t K;
    recv(sd, &K, sizeof(uint8_t), 0);
    char board[K+1];
    while (1)    //game loop
    {
        char buf[K+1];
        recv(sd, &buf, K+1, 0);
        if (buf[0] == 0)
        {
            PRINT_wARG("Board: %s\n", board);
            PRINT_MSG("You lost\n");
            break;
        }
        for (int i = 0; i < K; i++)
        {
            board[i] = buf[i+1];
        }
        board[K] = '\0';
        if (buf[0] == -1)
        {
            PRINT_wARG("Board: %s\n", board);
            PRINT_MSG("You won\n");
            break;
        }
        PRINT_wARG("Board: %s (%d guesses left)\n", board, buf[0]);
        PRINT_MSG("Enter Guess: ");

        char tmp_buf[10];
        char input[10];
        int more = 0;
        read_stdin(input, 10, &more);
        while (more == 1)
        {
            read_stdin(tmp_buf, 10, &more);
        }
        fflush(stdin);
        send(sd, input, 1, 0);
    }

    close(sd);

    exit(EXIT_SUCCESS);
}

