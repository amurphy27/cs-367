/* server.c - code for server program. Do not rename this file */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <regex.h>

#include "proj.h"

#define QLEN 6 //

int main(int argc, char **argv) {
    struct protoent *ptrp; /* pointer to a protocol table entry */
    struct sockaddr_in sad; /* structure to hold server's address */
    struct sockaddr_in cad; /* structure to hold client's address */
    int sd, sd2; /* socket descriptors */
    int port; /* protocol port number */
    int optval = 1; /* boolean value when we set socket option */

    if (argc != 4) {
        fprintf(stderr, "Error: Wrong number of arguments\n");
        fprintf(stderr, "usage:\n");
        fprintf(stderr, "./server server_port number_guesses secret_word\n");
        exit(EXIT_FAILURE);
    }

    char *secret_word = argv[3];
    uint8_t K = strlen(secret_word);
    if (K <= 0 || K >= 255)
    {
        fprintf(stderr, "Error: Invalid secret_word\nMust be a string of between 1 and 254\n");
        exit(EXIT_FAILURE);
    }
    uint8_t number_guesses = atoi(argv[2]);
    if (number_guesses <= 0 || number_guesses >= 26)
    {
        fprintf(stderr, "Error: Invalid number_guesses\nMust be a number between 1 and 25\n");
        exit(EXIT_FAILURE);
    }

    memset((char *) &sad, 0, sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET; /* set family to Internet */
    sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */
    

    port = atoi(argv[1]); /* convert argument to binary */
    if (port > 0) { /* test for illegal value */
        sad.sin_port = htons((u_short) port);
    } else { /* print error message and exit */
        fprintf(stderr, "Error: Bad port number %s\n", argv[1]);
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

    char board[K];  //initialize board to K dashes
    memset(&board, '-', K);

    socklen_t alen;
    while (1) {     // Main server loop to accept and handle requests 
        alen = sizeof(cad); 
        if ((sd2 = accept(sd, (struct sockaddr *) &cad, &alen)) < 0) { 
            perror("Error: Accept failed"); 
            exit(EXIT_FAILURE); 
        } 
    
        if (fork() == 0) { 
            // child process 
            close(sd); // close the main socket in the child process. The parent process listens/accept on this main socket 

            send(sd2, &K, sizeof(uint8_t), 0);  //send client K (length of secret word)

            int N = number_guesses; //initializes N for the client

            /*char buf[K + 1];
            buf[0] = N;*/

            while (strchr(board, '-') != NULL && N > 0)    //game loop
            {
                char buf[K + 1];
                buf[0] = N;
                for (int i = 1; i < K + 1; i++)
                {
                    buf[i] = board[i-1];
                }
                send(sd2, &buf, sizeof(buf), 0);
                char guess;
                recv(sd2, &guess, sizeof(int), 0);
                bool valid_guess = false;
                bool win = true;
                for (int i = 0; i < K; i++)
                {
                    if (secret_word[i] == guess && board[i] == '-')
                    {
                        board[i] = guess;
                        valid_guess = true;
                    }
                    if (board[i] == '-')
                    {
                        win = false;
                    }
                }
                if (win)
                {
                    uint8_t win = 255;
                    buf[0] = win;
                    for (int i = 1; i < K + 1; i++)
                    {
                        buf[i] = board[i-1];
                    }
                    send(sd2, &buf, sizeof(buf), 0);
                    break;
                }
                if (!valid_guess)
                {
                    N--;
                }
                if (N <= 0)
                {
                    uint8_t lose = 0;
                    send(sd2, &lose, sizeof(uint8_t), 0);
                    break;
                }
            }

            close(sd2); // close the socket once the game is over. 
            break;  // break and terminate the child process 
        } else { 
            // parent 
            close(sd2);  // Since the child process is handling the client, the parent should close this client socket 
        } 
    }
    return 0;
}

