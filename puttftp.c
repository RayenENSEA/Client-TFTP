#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE 516 // 2 bytes opcode + 2 bytes block number + 512 bytes data

// Structure pour les paquets TFTP
struct TFTP_Packet {
    short opcode;
    union {
        char request[2 + MAX_BUFFER_SIZE]; // opcode(2) + filename + mode
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

    // Utilisation de getaddrinfo pour obtenir les informations sur le serveur
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server_ip, port, &hints, &res) != 0)
        error("Erreur lors de la récupération des informations sur le serveur");

    // Création du socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0)
        error("Erreur lors de la création du socket");

    printf("Socket créé avec succès.\n");

    // Construction du paquet WRQ (Write Request)
    struct TFTP_Packet wrq_packet;
    wrq_packet.opcode = htons(2); // WRQ opcode
    int length = sprintf(wrq_packet.content.request, "%s%c%s%c", filename, 0, "octet", 0);

    if (sendto(sockfd, &wrq_packet, length +2, 0, res->ai_addr, res->ai_addrlen) == -1)
        error("Erreur lors de l'envoi du WRQ");

    printf("WRQ envoyé avec succès.\n");

    // Ouverture du fichier à envoyer
    FILE *file = fopen(filename, "rb");
    if (!file)
        error("Erreur lors de l'ouverture du fichier en lecture");

    struct sockaddr_storage server_response_addr;
    socklen_t response_addr_len = sizeof(server_response_addr);

    // Initialise avec les informations obtenues de getaddrinfo
    memcpy(&server_response_addr, res->ai_addr, res->ai_addrlen);

    short block_num = 0;
    while (1) {
        struct TFTP_Packet data_packet;
        data_packet.opcode = htons(3); // DATA opcode
        data_packet.content.data.block_num = htons(block_num);

        // Lecture des données du fichier
        size_t bytes_read = fread(data_packet.content.data.data, 1, MAX_BUFFER_SIZE - 4, file);

        if (bytes_read == 0) {
            printf("Fin du fichier détectée.\n");
            break;  // Fin du fichier
        }

        // Envoi du paquet DATA
        if (sendto(sockfd, &data_packet, sizeof(short) + bytes_read + 2, 0, (struct sockaddr *)&server_response_addr, response_addr_len) == -1)
            error("Erreur lors de l'envoi du paquet DATA");

        printf("Envoi du paquet DATA (block %d) avec succès.\n", block_num);

        // Attente de l'ACK
        struct TFTP_Packet ack_packet;
        ssize_t ack_size = recvfrom(sockfd, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr *)&server_response_addr, &response_addr_len);
        //ack_packet.content.data.block_num = htons(ack_packet.content.data.block_num) - htons(1);

        printf("Opcode de l'ACK: %d\n", ntohs(ack_packet.opcode));
        if (ack_size == -1)
            error("Erreur lors de la réception de l'ACK");

        if (ntohs(ack_packet.opcode) != 4 || ntohs(ack_packet.content.data.block_num) != block_num)
            error("ACK incorrect");

        printf("ACK reçu avec succès (Block %d).\n", block_num);

        // Incrémentation du numéro de bloc
        block_num++;

    }

    fclose(file);
    close(sockfd);
    freeaddrinfo(res);

    printf("Envoi du fichier terminé avec succès.\n");

    return 0;
}

