#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int main(int argc, char **argv) {
    struct hostent *ptrh;
    struct protoent *ptrp;
    struct sockaddr_in sad;
    int sd;
    int port;
    char *host;
    int n;
    char buf[1000];

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
        perror("Error: bad port number");
        exit(EXIT_FAILURE);
    }

    host = argv[1]; /* if host argument specified */

    /* Convert host name to equivalent IP address and copy to sad. */
    ptrh = gethostbyname(host);
    if (ptrh == NULL) {
        perror("Error: Invalid host");
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

    while(1) {
        bzero(buf, sizeof(buf));
        printf("Enter message to sent to the server: ");
        scanf(" %[^\n]", buf);

        n = send(sd, buf, strlen(buf), 0);
        if (n <= 0) {
            perror("Error sending message.");
            exit(EXIT_FAILURE);
        }

        // Receive echo message from the server
        bzero(buf, sizeof(buf));
        n = recv(sd, buf, sizeof(buf), 0);
        if (n <= 0) {
            perror("Error receiving data.");
            exit(EXIT_FAILURE);
        }
        printf("Received from the server: %s\n\n", buf);
    }
}

