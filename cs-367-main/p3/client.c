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

    // TODO: Add client logic
    char init_buf[3];
    recv(sd, &init_buf, sizeof(uint8_t)*3, 0);
    uint8_t playerNum = init_buf[0];
    uint8_t board_size = init_buf[1];
    uint8_t seconds_per_round = init_buf[2];
    char board[board_size+1];
    char buf[board_size+3];
    if (playerNum == 1)
    {
        PRINT_wARG("You are Player 1... the game will begin when Player 2 joins...\nBoard size: %d\nSeconds per turn: %d\n\n", board_size, seconds_per_round);
    }
    else
    {
        PRINT_wARG("You are Player 2...\nBoard size: %d\nSeconds per turn: %d\n\n", board_size, seconds_per_round);
    }

    while (1)   //game loop
    {
        //printf("beginning of game loop\n");
        recv(sd, &buf, sizeof(buf), 0);
        uint8_t player1Score = buf[0];
        uint8_t player2Score = buf[1];
        uint8_t round = buf[2];
        for (int i = 0; i < board_size; i++)
        {
            board[i] = buf[i+3];
        }
        board[board_size] = '\0';

        if (player1Score >= 3)  //player 1 win
        {
            if (playerNum == 1) //this player is player 1
            {
                PRINT_MSG("You Won!\n");
            }
            else    //other player won
            {
                PRINT_MSG("You Lost!\n");
            }
            break;
        }
        else if (player2Score >= 3) //player 2 win
        {
            if (playerNum == 2) //this player is player 2
            {
                PRINT_MSG("You Won!\n");
            }
            else    //other player won
            {
                PRINT_MSG("You Lost!\n");
            }
            break;
        }
        else    //no winner game goes on
        {
            //Print Round Header
            PRINT_wARG("Round %d...\nScore is %d-%d\nBoard:", round, player1Score, player2Score);
            for (int i = 0; i < board_size; i++)
            {
                PRINT_wARG(" %c", board[i]);
            }
            PRINT_MSG("\n");

            //recv(sd, &buf, sizeof(char), 0);
            while (1)//round loop
            {

                recv(sd, &buf, sizeof(char), 0);

                //printf("buf is %s\n", buf);
                if (buf[0] == 'Y')  //this players turn
                {
                    PRINT_MSG("Your turn, enter word: ");

                    char tmp_buf[10];
                    char input[board_size];
                    int more = 0;
                    uint8_t input_length = read_stdin(input, 10, &more);
                    fflush(stdin);
                    while (more == 1)
                    {
                        read_stdin(tmp_buf, 10, &more);
                    }

                    input[input_length - 1] = '\0';
                    input_length--;

                    buf[0] = input_length;
                    for (int i = 0; i < input_length; i++)
                    {
                        buf[i+1] = input[i];
                    }

                    send(sd, &buf, input_length + 1, 0);

                    recv(sd, &buf, sizeof(uint8_t), 0);
                    if (buf[0] == 1)    //word was valid
                    {
                        PRINT_MSG("Valid word!\n");
                    }
                    else
                    {
                        PRINT_MSG("Invalid word!\n\n");
                        break;
                    }
                }
                else
                {
                    PRINT_MSG("Please wait for opponent to enter word...\n");

                    recv(sd, &buf, board_size + 1, 0);
                    if (buf[0] == 0)    //opponent made invalid word
                    {
                        PRINT_MSG("Opponent lost the round!\n\n");
                        break;
                    }
                    else
                    {
                        char opWord[buf[0]+1];
                        for (int i = 0; i < buf[0]; i++)
                        {
                            opWord[i] = buf[i+1];
                        }
                        opWord[(int) buf[0]] = '\0';
                        PRINT_wARG("Opponent entered \"%s\"\n", opWord);
                    }
                }
            }
        }
    }

    close(sd);

    exit(EXIT_SUCCESS);
}

