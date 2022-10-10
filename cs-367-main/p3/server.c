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
#include <time.h>

#include "proj.h"
#include "trie.h"

#define QLEN 6
#define ALPHABET_LEN 26
#define MAX_LINE_LENGTH 30

int main(int argc, char **argv) {
    struct protoent *ptrp; /* pointer to a protocol table entry */
    struct sockaddr_in sad; /* structure to hold server's address */
    struct sockaddr_in cad; /* structure to hold client's address */
    int sd, sd2, sd3; /* socket descriptors */
    int port; /* protocol port number */
    int optval = 1; /* boolean value when we set socket option */

    if (argc != 5) {
        fprintf(stderr, "Error: Wrong number of arguments\n");
        fprintf(stderr, "usage:\n");
        fprintf(stderr, "./server port board_size timeout dictionary\n");
        exit(EXIT_FAILURE);
    }

    uint8_t board_size = atoi(argv[2]);
    uint8_t seconds_per_round = atoi(argv[3]);
    FILE *fp = fopen(argv[4], "r");

    if (!fp)    //check that filepath is valid
    {
        fprintf(stderr, "Error: Invalid Filepath\n");
        exit(EXIT_FAILURE);
    }

    //Build dictionary from the text file given for this round
    trie_node* trie_root = trie_create();
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, fp))
    {
        line[strlen(line)-1] = '\0';
        trie_insert(trie_root, line, strlen(line));
    }
    fclose(fp);

    /*---------- PRINT STATEMENT FOR TESTING ----------*/
    //printf("board_size is: %d\nseconds_per_round is: %d\n", board_size, seconds_per_round);

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

    // TODO: Add server logic to accept and handle clients.
    socklen_t alen;
    while (1) {     // Main server loop to accept and handle requests
        alen = sizeof(cad);
        if ((sd2 = accept(sd, (struct sockaddr *) &cad, &alen)) < 0) {
            perror("Error: Accept failed");
            exit(EXIT_FAILURE);
        }

        uint8_t player1Score = 0;
        uint8_t player2Score = 0;
        char init_buf[3];

        init_buf[0] = 1;
        init_buf[1] = board_size;
        init_buf[2] = seconds_per_round;

        send(sd2, &init_buf, sizeof(uint8_t)*3, 0);

        //pick up the second player
        if ((sd3 = accept(sd, (struct sockaddr *) &cad, &alen)) < 0)
        {
            perror("Error: Accept failed");
            exit(EXIT_FAILURE);
        }

        init_buf[0] = 2;
        send(sd3, &init_buf, sizeof(uint8_t)*3, 0);

        if (fork() == 0) {
            // child process
            close(sd); // close the main socket in the child process. The parent process listens/accept on this main socket

            // TODO: Add server game logic here.
            char buf[board_size+3];
            uint8_t round = 0;

            srand(time(NULL));

            //REMEMBER TO INSERT SOME SORT OF CHECK FOR CLIENT CONNECTIVITY
            while (1)    //Game loop
            {
                //printf("beginning of a game loop\n");
                sleep(1);   //pause to let previous sends be recieved before sending more sends

                //initialize board
                char board[board_size+1];
                for (int i = 0; i < board_size - 1; i++)    //generate all but 1 letter for board
                {
                    board[i] = (rand()%(122-97)) + 97;  //generates ascii dec between 97 and 122 (a-z)
                }
                //generate last letter of board, if no vowels in board already make last letter a vowel
                if (!strchr(board, 'a') && !strchr(board, 'e') && !strchr(board, 'i') && !strchr(board, 'o') && !strchr(board, 'u'))    //no vowels found, insert vowel
                {
                    //printf("no vowels found\n");
                    char vowels[5] = {'a', 'e', 'i', 'o', 'u'};
                    board[board_size-1] = vowels[rand() % 6];
                }
                else    //vowel found, insert random letter
                {
                    board[board_size-1] = (rand()%(122-97)) + 97;
                }
                board[board_size] = '\0';
                //printf("board is: %s\n", board);
                round++;

                buf[0] = player1Score;
                buf[1] = player2Score;
                buf[2] = round;
                for (int i = 3; i < board_size + 3; i++)    //insert board into the buffer
                {
                    buf[i] = board[i-3];
                }
                send(sd2, &buf, sizeof(buf), 0);
                send(sd3, &buf, sizeof(buf), 0);

                int turn;

                //setup turn y/n at beginning of a new round
                if (round % 2 == 0) //player 2 starts round
                {
                    turn = 0;
                }
                else
                {
                    turn = 1;
                }

                while (1)    //round loop
                {
                    //printf("turn# is %d\n", turn);
                    if (turn % 2 == 0)  //player 2 turn
                    {

                        sleep(1);   //breaks up the double send so client has time to process the first on
                        buf[0] = 'N';
                        send(sd2, &buf, sizeof(char), 0);
                        buf[0] = 'Y';
                        send(sd3, &buf, sizeof(char), 0);

                        //printf("player 2 turn\n");
                        recv(sd3, &buf, board_size + 1, 0);
                        int wordLen = buf[0];
                        char word[wordLen+1];
                        for (int i = 0; i < wordLen; i++)
                        {
                            word[i] = buf[i+1];
                        }
                        word[wordLen] = '\0';
                        //printf("word is %s\n", word);
                        char tempBoard[board_size];
                        strcpy(tempBoard, board);
                        bool validLetters = true;
                        for (int i = 0; i < wordLen; i++)   //check to make sure word is made of board letters
                        {
                            char* check = strchr(tempBoard, word[i]);
                            if (check)
                            {
                                *check = '?';
                            }
                            else    //invalid word
                            {
                                //printf("invalid letters\n");
                                validLetters = false;
                                break;
                            }
                        }
                        if (trie_search(trie_root, word, wordLen) == 1 && validLetters) //check if word is valid and not used already
                        {
                            //printf("valid word\n");
                            trie_delete(trie_root, word, wordLen);  //removes word from trie tree so repeats of this word are invalid

                            buf[0] = 1;
                            send(sd3, &buf, sizeof(uint8_t), 0);
                            buf[0] = wordLen;
                            for (int i = 0; i < wordLen; i++)
                            {
                                buf[i+1] = word[i];
                            }
                            send(sd2, &buf, wordLen + 1, 0);
                            turn++;
                        }
                        else    //invalid word
                        {
                            //printf("invalid trie\n");

                            buf[0] = (uint8_t) 0;
                            send(sd2, &buf, sizeof(uint8_t), 0);
                            send(sd3, &buf, sizeof(uint8_t), 0);
                            player1Score++;
                            break;
                        }
                    }
                    else    //player 1 turn
                    {

                        sleep(1);   //breaks up the double send so client has time to process the first on
                        buf[0] = 'Y';
                        send(sd2, &buf, sizeof(char), 0);
                        buf[0] = 'N';
                        send(sd3, &buf, sizeof(char), 0);

                        //printf("player 1 turn\n");
                        recv(sd2, &buf, board_size + 1, 0);
                        int wordLen = buf[0];
                        char word[wordLen];
                        for (int i = 0; i < wordLen; i++)
                        {
                            word[i] = buf[i+1];
                        }
                        //printf("word is %s\n", word);
                        char tempBoard[board_size];
                        strcpy(tempBoard, board);
                        bool validLetters = true;
                        for (int i = 0; i < wordLen; i++)   //check to make sure word is made of board letters
                        {
                            char* check = strchr(tempBoard, word[i]);
                            if (check)
                            {
                                *check = '?';
                            }
                            else    //invalid word
                            {
                                //printf("invalid letters\n");
                                validLetters = false;
                                break;
                            }
                        }
                        if (trie_search(trie_root, word, wordLen) == 1 && validLetters) //check if word is valid and not used already
                        {
                            //printf("valid word\n");
                            trie_delete(trie_root, word, wordLen);  //removes word from trie tree so repeats of this word are invalid

                            buf[0] = 1;
                            send(sd2, &buf, sizeof(uint8_t), 0);
                            buf[0] = wordLen;
                            for (int i = 0; i < wordLen; i++)
                            {
                                buf[i+1] = word[i];
                            }
                            send(sd3, &buf, wordLen + 1, 0);
                            turn++;
                        }
                        else    //invalid word
                        {
                            //printf("invalid trie\n");

                            buf[0] = (uint8_t) 0;
                            send(sd2, &buf, sizeof(uint8_t), 0);
                            send(sd3, &buf, sizeof(uint8_t), 0);
                            player2Score++;
                            break;
                        }
                    }
                }
            }

            close(sd2); // close the socket once the game is over.
	        close(sd3);
            break;  // break and terminate the child process
        } else {
            // parent
            close(sd2);  // Since the child process is handling the client, the parent should close this client socket
        }
    }

    return 0;
}

