#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE 516 // 2 bytes opcode + 2 bytes block number + 512 bytes data

// structure for TFTP packets
struct TFTP_Packet {
    short opcode;
    union {
        char request[2 + MAX_BUFFER_SIZE]; // opcode(size = 2) + filename + mode
        struct {
            short block_num;
            char data[MAX_BUFFER_SIZE];
        } data;
    } content;
};

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    const char *port = argv[2];
    const char *filename = argv[3];

    // use of getaddrinfo to obtain server information
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server_ip, port, &hints, &res) != 0)
        error("Error getting server information");

    // creating a socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0)
        error("Error creating socket");

    printf("Socket created successfully.\n");

    // construct RRQ packet (Read Request)
    struct TFTP_Packet rrq_packet;
    rrq_packet.opcode = htons(1); // RRQ opcode
    sprintf(rrq_packet.content.request, "%s%c%s%c", filename, 0, "octet", 0);

    if (sendto(sockfd, &rrq_packet, sizeof(rrq_packet.content.request), 0, res->ai_addr, res->ai_addrlen) == -1)
        error("Error sending RRQ");

    printf("RRQ sent successfully.\n");

    // receiving the file
    FILE *file = fopen(filename, "wb");
    if (!file)
        error("Error opening file for writing");

    struct sockaddr_storage server_response_addr;
    socklen_t response_addr_len = sizeof(server_response_addr);

    // initializing with information obtained from getaddrinfo
    memcpy(&server_response_addr, res->ai_addr, res->ai_addrlen);

    while (1) {
        struct TFTP_Packet data_packet;
        printf("Waiting for reception...\n");
        ssize_t bytes_received = recvfrom(sockfd, &data_packet, sizeof(data_packet), 0, (struct sockaddr *)&server_response_addr, &response_addr_len);

        if (bytes_received == -1) {
            error("Error receiving file");
        } else if (bytes_received == 0) {
            printf("End of file detected.\n");
            break; // end of file
        } else {
            printf("Received %zd bytes.\n", bytes_received);

            // display packet content
            printf("Opcode: %d\n", ntohs(data_packet.opcode));

            if (ntohs(data_packet.opcode) == 5) {
                // display error code and error message
                short error_code;
                memcpy(&error_code, data_packet.content.data.data, sizeof(short));
                printf("Error code: %d\n", ntohs(error_code));
                printf("Error message: %s\n", data_packet.content.data.data + sizeof(short));
            } else if (ntohs(data_packet.opcode) == 3) {
                printf("Block number: %d\n", ntohs(data_packet.content.data.block_num));
                // write to the file
                size_t data_size = bytes_received - 4; // effective data size
                size_t written = fwrite(data_packet.content.data.data, 1, data_size, file);

                if (written != data_size) {
                    printf("Error writing to file.\n");
                    perror("fwrite");
                }

                // sending ACK
                struct TFTP_Packet ack_packet;
                ack_packet.opcode = htons(4); // ACK opcode
                ack_packet.content.data.block_num = data_packet.content.data.block_num;

                if (sendto(sockfd, &ack_packet, sizeof(ack_packet.content.data), 0, (struct sockaddr *)&server_response_addr, response_addr_len) == -1)
                    error("Error sending ACK");
                // if data size is less than the expected maximum size, it may indicate the end of the file
                if (data_size < MAX_BUFFER_SIZE - 4) {
                    printf("End of file detected.\n");
                    break; // end of file
                }
            }
        }
    }

    fclose(file);
    close(sockfd);
    freeaddrinfo(res);

    printf("File download completed successfully.\n");

    return 0;
}
