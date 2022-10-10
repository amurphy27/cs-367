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
    int n;
    char *host; /* pointer to host name */
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
    // TODO: Implement

    fd_set read_set, ready_to_read_set;
    int max_fd;

    FD_ZERO(&read_set);
    FD_SET(sd, &read_set);
    FD_SET(0, &read_set);   //adds stdin to the read set

    max_fd = sd;

    while (1)
    {
        memcpy(&ready_to_read_set, &read_set, sizeof(read_set));

        int r = select(max_fd + 1, &ready_to_read_set, NULL, NULL, NULL);
        if (r < 0)
        {
            perror("Error with select\n");
            exit(EXIT_FAILURE);
        }
        else if (r == 0)
        {
            printf("timeout?\n");
            continue;
        }

        int cur_max_fd = max_fd;
        for (int i = 0; i <= cur_max_fd; i++)
        {
            if (FD_ISSET(i, &ready_to_read_set))
            {
                char input[256];
                char username[100];
                uint8_t inputlen = 0;
                uint32_t message_header;
                uint8_t ulen;
                bzero(&ulen, sizeof(ulen));
                bzero(input, sizeof(input));
                bzero(buf, sizeof(buf));
                bzero(username, sizeof(username));
                bzero(&message_header, sizeof(message_header));
                if (i == sd)    //socket is the server?
                {
                    //parse the control header
                    uint16_t control_header;
                    uint8_t mt;
                    uint8_t code;
                    uint8_t unc;
                    uint8_t max_ulen;
                    uint8_t pub;
                    uint8_t prv;
                    uint8_t frg;
                    uint8_t lst;
                    uint16_t length;
                    char message[256];
                    char this_user[100];
                    recv(sd, buf, sizeof(buf), 0);
                    mt = parse_mt_bit((uint8_t *) buf);
                    //printf("mt is: %d\n", mt);
                    if (mt == 0)    //control message
                    {
                        bzero(&control_header, sizeof(control_header));
                        control_header = (buf[0] << 8) | buf[1];
                        parse_control_header(&control_header, &mt, &code, &unc, &ulen);
                        //printf("control_header: %d   mt: %d   code: %d   unc: %d   ulen: %d\n", control_header, mt, code, unc, ulen);
                        int j = 2;
                        /*for (int i = 0; i < ulen; i++)
                        {
                            username[i] = buf[i+2];
                        }*/


                        for (int i = 0; i < ulen; i++)
                        {
                            if (buf[j] == 0 && (code == 2 || code == 3 || code == 6))
                            {
                                bzero(buf, sizeof(buf));
                                recv(sd, buf, sizeof(buf), 0);
                                j = 0;
                            }
                            username[i] = buf[j];
                            j++;
                        }


                        switch (code)
                        {
                            case 0: //code 0 server is full cannot accept new client
                                if (buf[1] == 0)    //if there is no ulen or unc at this point then the server is closed
                                {
                                    PRINT_MSG("Lost connection with the server\n");
                                    exit(EXIT_SUCCESS);
                                }
                                PRINT_MSG("Server is full\n");
                                exit(EXIT_SUCCESS);
                                break;
                            case 0x1:   //code 1 server is not full can accept new client
                            {
                                parse_control_header(&control_header, &mt, &code, &unc, &max_ulen);
                                while (inputlen <= 0 || inputlen > max_ulen)
                                {
                                    PRINT_wARG("Choose a username (should be less than %d): ", max_ulen+1);
                                    int more = 0;
                                    inputlen = read_stdin(input, sizeof(input), &more);
                                    fflush(stdin);
                                    input[inputlen - 1] = '\0';
                                    inputlen--;
                                }
                                bzero(&control_header, sizeof(control_header));
                                buf[0] = 32;
                                buf[1] = inputlen;
                                for (int i = 0; i < inputlen; i++)
                                {
                                    buf[i+2] = input[i];
                                    this_user[i] = input[i];
                                }
                                this_user[inputlen] = '\0';
                                n = send(sd, buf, inputlen + 2, 0);
                                if (n <= 0)
                                {
                                    perror("Error sending message.\n");
                                    exit(EXIT_FAILURE);
                                }
                                break;
                            }
                            case 0x2:  //code 2 a new user has connected
                                PRINT_USER_JOINED(username);
                                break;
                            case 0x3:  //code 3 a user has left
                                PRINT_USER_LEFT(username);
                                break;
                            case 0x4:  //code 4 user negotiation message
                                switch (unc)
                                {
                                    case 0:
                                        PRINT_MSG("Time to enter username has expired. Try again.\n");
                                        exit(EXIT_FAILURE);
                                        break;
                                    case 1:
                                        PRINT_MSG("Username is taken. Try again.\n");
                                        while (inputlen <= 0 || inputlen > max_ulen)
                                        {
                                            PRINT_wARG("Choose a username (should be less than %d): ", max_ulen+1);
                                            int more = 0;
                                            inputlen = read_stdin(input, sizeof(input), &more);
                                            fflush(stdin);
                                            input[inputlen - 1] = '\0';
                                            inputlen--;
                                        }
                                        bzero(&control_header, sizeof(control_header));
                                        buf[0] = 32;
                                        buf[1] = inputlen;
                                        for (int i = 0; i < inputlen; i++)
                                        {
                                            buf[i+2] = input[i];
                                            this_user[i] = input[i];
                                        }
                                        n = send(sd, buf, inputlen + 2, 0);
                                        if (n <= 0)
                                        {
                                            perror("Error sending message.\n");
                                            exit(EXIT_FAILURE);
                                        }
                                        break;
                                    case 2:
                                        PRINT_MSG("Username is too long. Try again.\n");
                                        while (inputlen <= 0 || inputlen > max_ulen)
                                        {
                                            PRINT_wARG("Choose a username (should be less than %d): ", max_ulen+1);
                                            int more = 0;
                                            inputlen = read_stdin(input, sizeof(input), &more);
                                            fflush(stdin);
                                            input[inputlen - 1] = '\0';
                                            inputlen--;
                                        }
                                        bzero(&control_header, sizeof(control_header));
                                        buf[0] = 32;
                                        buf[1] = inputlen;
                                        for (int i = 0; i < inputlen; i++)
                                        {
                                            buf[i+2] = input[i];
                                            this_user[i] = input[i];
                                        }
                                        n = send(sd, buf, inputlen + 2, 0);
                                        if (n <= 0)
                                        {
                                            perror("Error sending message.\n");
                                            exit(EXIT_FAILURE);
                                        }
                                        break;
                                    case 3:
                                        while (inputlen <= 0 || inputlen > max_ulen)
                                        {
                                            PRINT_wARG("Choose a username (should be less than %d): ", max_ulen+1);
                                            int more = 0;
                                            inputlen = read_stdin(input, sizeof(input), &more);
                                            fflush(stdin);
                                            input[inputlen - 1] = '\0';
                                            inputlen--;
                                        }
                                        bzero(&control_header, sizeof(control_header));
                                        buf[0] = 32;
                                        buf[1] = inputlen;
                                        for (int i = 0; i < inputlen; i++)
                                        {
                                            buf[i+2] = input[i];
                                            this_user[i] = input[i];
                                        }
                                        n = send(sd, buf, inputlen + 2, 0);
                                        if (n <= 0)
                                        {
                                            perror("Error sending message.\n");
                                            exit(EXIT_FAILURE);
                                        }
                                        break;
                                    case 4:
                                        PRINT_MSG("Username accepted\n");
                                        PRINT_MSG("> ");
                                        break;
                                    default:
                                        perror("default unc aka uh oh\n");
                                        exit(EXIT_FAILURE);
                                }
                                break;
                            case 0x5:  //code 5 message too long and discarded
                                PRINT_MSG("%% Couldn't send message. It was too long.\n");
                                break;
                            case 0x6:  //code 6 recipient is not an active user
                                PRINT_INVALID_RECIPIENT(username);
                                break;
                            case 0x7:  //code 7 badly formatted private message
                                PRINT_MSG("%% Warning: Incorrectly formatted private message. Missing recipient.\n");
                                break;
                            default:
                                perror("default code aka uh oh\n");
                                exit(EXIT_FAILURE);
                        }
                    }
                    else if (mt == 1) //chat message
                    {
                        int iBuf = 4;
                        bzero(message, sizeof(message));
                        message_header = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
                        parse_chat_header(&message_header, &mt, &pub, &prv, &frg, &lst, &ulen, &length);
                        //printf("message_header: %d   mt: %d   pub: %d   prv: %d   frg: %d   lst: %d    ulen: %d   length: %d\n", message_header, mt, pub, prv, frg, lst, ulen, length);
                        for (int i = 0; i < ulen; i++)
                        {
                            if (buf[iBuf] == 0)
                            {
                                bzero(buf, sizeof(buf));
                                recv(sd, buf, sizeof(buf), 0);
                                iBuf = 0;
                            }
                            username[i] = buf[iBuf];
                            iBuf++;
                        }
                        for (int i = 0; i < length; i++)
                        {
                            if (buf[iBuf] == 0)
                            {
                                bzero(buf, sizeof(buf));
                                recv(sd, buf, sizeof(buf), 0);
                                iBuf = 0;
                            }
                            message[i] = buf[iBuf];
                            iBuf++;
                        }

                        if (pub && !prv && !frg && !lst)    //public message
                        {
                            PRINT_PUBLIC_MSG(username, message);
                        }
                        else if (prv && !pub && !frg && !lst)   //private message
                        {
                            PRINT_PRIVATE_MSG(username, this_user, message);
                        }
                        else
                        {
                            perror("bad header\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    else    //bad mt (this should never be hit)
                    {
                        perror("bad mt\n");
                        exit(EXIT_FAILURE);
                    }
                }
                if (i == 0) //stdin is ready... get it like bonesaw
                {
                    int more = 0;
                    inputlen = read_stdin(input, sizeof(input), &more);
                    fflush(stdin);
                    input[inputlen - 1] = '\0';
                    inputlen--;

                    if (input[0] == '@')    //check to see if the message is public or private
                    {
                        //parse out the recipient
                        char *token;
                        token = strtok(input, " ");
                        token = strtok(token, "@");
                        if (!token) //checks to make sure that the recipient is sendable otherwise reading it causes seg faults
                        {
                            //PRINT_MSG("Invalid recipient\n> ");
                            break;
                        }
                        strcpy(username, token);
                        ulen = strlen(username);
                        inputlen -= ulen + 2;
                        //printf("recipient uname is: %s   ulen is: %d\n", username, ulen);
                        //printf("message is: %s    msglen is: %d\n", input+ulen+2, inputlen);

                        buf[0] = (unsigned char) 160;
                        buf[1] = 0;
                        buf[2] = ulen << 4;
                        buf[3] = inputlen;
                        for (int i = 0; i < ulen; i++)
                        {
                            buf[i+4] = username[i];
                        }
                        for (int i = 0; i < inputlen; i++)
                        {
                            //printf("char added to buf is: %c\n", input[i+ulen+2]);
                            buf[i+ulen+4] = input[i+ulen+2];
                        }

                        //printf("test is: %d\n", inputlen);
                        if (inputlen == 255)
                        {
                            //printf("found empty message\n");
                            break;
                        }

                        send(sd, buf, ulen + inputlen + 4, 0);
                    }
                    else    //public message
                    {
                        buf[0] = (unsigned char) 192;
                        buf[1] = 0;
                        buf[2] = 0;
                        buf[3] = inputlen;
                        for (int i = 0; i < inputlen; i++)
                        {
                            buf[i+4] = input[i];
                        }
                        send(sd, buf, inputlen + 4, 0);
                    }
                    PRINT_MSG("> ");
                }
            }
        }
    }
}

