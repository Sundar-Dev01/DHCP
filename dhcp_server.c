#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000
#define POOL_SIZE 5

int main(void)
{
    int server_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_length = sizeof(client_address);
    char buffer[100];

    char *ip_pool[POOL_SIZE] = {
        "192.168.1.100",
        "192.168.1.101",
        "192.168.1.102",
        "192.168.1.103",
        "192.168.1.104"
    };

    int next_ip = 0;

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }

    printf("DHCP Server started...\n");
    printf("IP address pool initialized.\n");
    printf("Waiting for clients...\n\n");

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        recvfrom(server_socket, buffer, sizeof(buffer) - 1, 0,
                (struct sockaddr *)&client_address, &client_length);

        printf("Received: %s\n", buffer);

        if (strcmp(buffer, "DISCOVER") == 0)
        {
            if (next_ip < POOL_SIZE)
            {
                char offer[100];
                sprintf(offer, "OFFER %s", ip_pool[next_ip]);
                sendto(server_socket, offer, strlen(offer), 0,
                       (struct sockaddr *)&client_address, client_length);
                printf("Sent: %s\n\n", offer);
            }
            else
            {
                char message[] = "POOL_FULL";
                sendto(server_socket, message, strlen(message), 0,
                       (struct sockaddr *)&client_address, client_length);
                printf("No available IP addresses!\n\n");
            }
        }
        else if (strncmp(buffer, "REQUEST", 7) == 0)
        {
            char ack[100];
            char requested_ip[30];

            sscanf(buffer, "REQUEST %29s", requested_ip);
            sprintf(ack, "ACK %s", requested_ip);
            sendto(server_socket, ack, strlen(ack), 0,
                   (struct sockaddr *)&client_address, client_length);

            printf("IP %s assigned successfully.\n", requested_ip);
            printf("Sent: %s\n\n", ack);
            next_ip++;
        }
    }

    close(server_socket);
    return 0;
}
