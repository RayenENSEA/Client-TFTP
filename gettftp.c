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

    // Construction du paquet RRQ (Read Request)
    struct TFTP_Packet rrq_packet;
    rrq_packet.opcode = htons(1); // RRQ opcode
    sprintf(rrq_packet.content.request, "%s%c%s%c", filename, 0, "octet", 0);

    if (sendto(sockfd, &rrq_packet, sizeof(rrq_packet.content.request), 0, res->ai_addr, res->ai_addrlen) == -1)
    	error("Erreur lors de l'envoi du RRQ");

    printf("RRQ envoyé avec succès.\n");

    // Réception du fichier
    FILE *file = fopen(filename, "wb");
    if (!file)
        error("Erreur lors de l'ouverture du fichier en écriture");

    struct sockaddr_storage server_response_addr;
    socklen_t response_addr_len = sizeof(server_response_addr);

    // Initialise avec les informations obtenues de getaddrinfo
    memcpy(&server_response_addr, res->ai_addr, res->ai_addrlen);

    while (1) {
        struct TFTP_Packet data_packet;
        printf("Attente de réception...\n");
        ssize_t bytes_received = recvfrom(sockfd, &data_packet, sizeof(data_packet), 0, (struct sockaddr *)&server_response_addr, &response_addr_len);

        if (bytes_received == -1) {
            error("Erreur lors de la réception du fichier");
        } else if (bytes_received == 0) {
            printf("Fin du fichier détectée.\n");
            break;  // Fin du fichier
        } else {
            printf("Réception de %zd octets.\n", bytes_received);

            // Affichage du contenu du paquet
            printf("Opcode: %d\n", ntohs(data_packet.opcode));

            if (ntohs(data_packet.opcode) == 5) {
                // Affichage du code d'erreur et du message d'erreur
                short error_code;
                memcpy(&error_code, data_packet.content.data.data, sizeof(short));
                printf("Code d'erreur: %d\n", ntohs(error_code));
                printf("Message d'erreur: %s\n", data_packet.content.data.data + sizeof(short));
            } else if (ntohs(data_packet.opcode) == 3) {
                printf("Block number: %d\n", ntohs(data_packet.content.data.block_num));
                // Écriture dans le fichier
                size_t data_size = bytes_received - 4; // Taille effective des données
                size_t written = fwrite(data_packet.content.data.data, 1, data_size, file);

                if (written != data_size) {
                    printf("Erreur lors de l'écriture dans le fichier.\n");
                    perror("fwrite");
                }

                // Envoi de l'ACK
                struct TFTP_Packet ack_packet;
                ack_packet.opcode = htons(4); // ACK opcode
                ack_packet.content.data.block_num = data_packet.content.data.block_num;

                if (sendto(sockfd, &ack_packet, sizeof(ack_packet.content.data), 0, (struct sockaddr *)&server_response_addr, response_addr_len) == -1)
                    error("Erreur lors de l'envoi de l'ACK");
                // Si la taille des données est inférieure à la taille maximale attendue, cela peut indiquer la fin du fichier
                if (data_size < MAX_BUFFER_SIZE - 4) {
                    printf("Fin du fichier détectée.\n");
                    break;  // Fin du fichier
                }
            }
        }
    }

    fclose(file);
    close(sockfd);
    freeaddrinfo(res);

    printf("Téléchargement du fichier terminé avec succès.\n");

    return 0;
}

