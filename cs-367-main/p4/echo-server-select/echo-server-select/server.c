#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define QLEN 2 /* size of request queue */
int visits = 0; /* counts client connections */


int main(int argc, char **argv) {
    struct protoent *ptrp;
    struct sockaddr_in sad;
    struct sockaddr_in cad;
    int sd, sd2;
    int port;
    int optval = 1;
    char buf[1000];

    if (argc != 2) {
        fprintf(stderr, "Error: Wrong number of arguments\n");
        fprintf(stderr, "usage:\n");
        fprintf(stderr, "./server server_port\n");
        exit(EXIT_FAILURE);
    }

    memset((char *) &sad, 0, sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET; /* set family to Internet */
    sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */


    port = atoi(argv[1]); /* convert argument to binary */
    if (port > 0) { /* test for illegal value */
        sad.sin_port = htons((u_short) port);
    } else { /* print error message and exit */
        perror("Error: bad port number");
        exit(EXIT_FAILURE);
    }

    /* Map TCP transport protocol name to protocol number */
    if (((long int) (ptrp = getprotobyname("tcp"))) == 0) {
        fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
        exit(EXIT_FAILURE);
    }

    /* Create a socket */
    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }


    /* Allow reuse of port - avoid "Bind failed" issues */
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("Error setting socket option");
        exit(EXIT_FAILURE);
    }

    /* Bind a local address to the socket */
    if (bind(sd, (struct sockaddr *) &sad, sizeof(sad)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }


    /* Specify size of request queue */
    if (listen(sd, QLEN) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    fd_set read_set, ready_to_read_set;
    int max_fd;

    FD_ZERO(&read_set);
    FD_SET(sd, &read_set);
    max_fd = sd;

    while (1) {

        memcpy(&ready_to_read_set, &read_set, sizeof(read_set));

        int r = select(max_fd + 1, &ready_to_read_set, NULL, NULL, NULL);
        if (r < 0) {
            perror("Error with select");
            exit(EXIT_FAILURE);
        } else if (r == 0) {
            fprintf(stderr, "Select returned 0 even though timeout arg is NULL");
            continue;
        }

        /*
         * One or more descriptors (in the range 0 to max_fd) that select is monitoring are ready
         * Check which one and act accordingly.
         */
        int curr_max_fd = max_fd;
        for (int i = 0; i <= curr_max_fd; i++) {
            if (FD_ISSET(i, &ready_to_read_set)) {
                if (i == sd) {
                    /*
                     * The server's main socket is ready. Some client is trying to connect.
                     * Accept the client and add the new socket to read_set so select can monitor
                     * this new socket.
                     */
                    socklen_t alen = sizeof(cad);
                    if ((sd2 = accept(sd, (struct sockaddr *) &cad, &alen)) < 0) {
                        fprintf(stderr, "Error: Accept failed\n");
                        exit(EXIT_FAILURE);
                    }

                    FD_SET(sd2, &read_set);
                    max_fd = sd2 > max_fd ? sd2 : max_fd; // Don't forget to update max_fd. Not updating this is a common error.

                } else {
                    /*
                     * Anything else that select is monitoring is a client's network socket.
                     * If a socket is ready, the server reads the client's message and echoes it back.
                     */

                    bzero(buf, sizeof(buf));
                    int n = recv(i, buf, sizeof(buf), 0);
                    if (n <= 0) {
                        if (n < 0) {
                            perror("Error receiving data from client");
                        } else {
                            fprintf(stderr, "Client disconnected\n");
                        }

                        /*
                         * Disconnect client
                         * 1. close its socket
                         * 2. clear that socket from read_set as we don't need select to monitor it anymore
                         */
                        close(i);
                        FD_CLR(i, &read_set); // clear this
                        // No need to change max_fd
                    }
                    send(i, buf, n, 0);
                }
            }
        }
    }
}
