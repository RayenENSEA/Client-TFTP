#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE 516

void receiveFile(int sockfd, const char *file_name) {
    printf("Receiving file...\n");

    FILE *file = fopen(file_name, "wb");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    uint16_t expectedBlock = 1;  // The expected block number to receive

    while (1) {
        char buffer[MAX_BUFFER_SIZE];
        ssize_t bytesRead = recv(sockfd, buffer, sizeof(buffer), 0);
	printf("%ld",bytesRead);

        if (bytesRead == -1) {
            perror("Error receiving data");
            fclose(file);
            exit(EXIT_FAILURE);
        } else if (bytesRead == 0) {
            // End of file
            printf("End of file.\n");
            break;
        } else if (bytesRead < 4) {
            fprintf(stderr, "Received unexpected packet with size %zd\n", bytesRead);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        uint16_t opcode = (buffer[0] << 8) | buffer[1];
        uint16_t receivedBlock = (buffer[2] << 8) | buffer[3];

        printf("Received packet: opcode=%d, block=%d, data size=%zd\n", opcode, receivedBlock, bytesRead - 4);

        if (opcode == 3) {
            // Rest of the code remains the same...
        } else if (opcode == 5) {
            // Rest of the code remains the same...
        } else {
            fprintf(stderr, "Received unexpected opcode: %d\n", opcode);
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);

    printf("File reception completed.\n");
}



void sendReadRequest(int sockfd, const char *file_name) {
    printf("Sending read request...\n");

    // Build the read request (RRQ) packet
    char requestPacket[MAX_BUFFER_SIZE];
    memset(requestPacket, 0, MAX_BUFFER_SIZE);
    requestPacket[0] = 0; // Opcode for RRQ (read request)

    // Add file name and mode to the request packet
    strncpy(requestPacket + 2, file_name, strlen(file_name));
    strcpy(requestPacket + 2 + strlen(file_name), "octet"); // Mode "octet"

    // Add TFTP Option Extension (RFC2347)
    uint16_t *optionCode = (uint16_t *)(requestPacket + 2 + strlen(file_name) + 1);
    *optionCode = htons(0x0001); // Option code for "tsize" option
    uint16_t *optionSize = (uint16_t *)(requestPacket + 2 + strlen(file_name) + 1 + 2);
    *optionSize = htons(0); // Set size to 0 for initial request

    // Send the read request to the server
    if (send(sockfd, requestPacket, 2 + strlen(file_name) + 1 + 2 + 1, 0) == -1) {
        perror("Error sending read request");
        exit(EXIT_FAILURE);
    }

    printf("Read request sent.\n");
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_address> <port> <file_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_address = argv[1];
    const char *port = argv[2];
    const char *file_name = argv[3];

    struct addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Permet IPv4 ou IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket (UDP) */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    int status = getaddrinfo(server_address, port, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockfd == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sockfd, result->ai_addr, result->ai_addrlen) == -1) {
        perror("Error connecting to server");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connection to the server successful.\n");

    // Envoyer la demande de lecture (read request)
    sendReadRequest(sockfd, file_name);

    // Recevoir le fichier
    receiveFile(sockfd, file_name);

    // Fermer la socket
    close(sockfd);

    freeaddrinfo(result); /* Libérer la mémoire allouée par getaddrinfo */

    return 0;
}

