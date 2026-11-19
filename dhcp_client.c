#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000


int main(void)
{
    int client_socket;
    struct sockaddr_in server_address;
    socklen_t server_length = sizeof(server_address);
    char buffer[100];
    char offered_ip[30];

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Step 1: DHCP Discover */
    printf("Sending DHCP DISCOVER...\n");
    strcpy(buffer, "DISCOVER");
    sendto(client_socket, buffer, strlen(buffer), 0,
           (struct sockaddr *)&server_address, sizeof(server_address));

    /* Step 2: Receive DHCP Offer */
    memset(buffer, 0, sizeof(buffer));
    recvfrom(client_socket, buffer, sizeof(buffer) - 1, 0,
            (struct sockaddr *)&server_address, &server_length);
    printf("Received: %s\n", buffer);

    if (strncmp(buffer, "OFFER", 5) != 0)
    {
        printf("No IP offer received. Exiting.\n");
        close(client_socket);
        return 1;
    }

    sscanf(buffer, "OFFER %29s", offered_ip);

    /* Step 3: DHCP Request */
    printf("\nSending DHCP REQUEST for %s...\n", offered_ip);
    sprintf(buffer, "REQUEST %s", offered_ip);
    sendto(client_socket, buffer, strlen(buffer), 0,
           (struct sockaddr *)&server_address, sizeof(server_address));

    /* Step 4: Receive DHCP ACK */
    memset(buffer, 0, sizeof(buffer));
    recvfrom(client_socket, buffer, sizeof(buffer) - 1, 0,
            (struct sockaddr *)&server_address, &server_length);
    printf("Received: %s\n", buffer);

    if (strncmp(buffer, "ACK", 3) == 0)
    {
        printf("\n==================================\n");
        printf(" DHCP IP ADDRESS ALLOCATION SUCCESS\n");
        printf("==================================\n");
        printf("Assigned IP : %s\n", offered_ip);
        printf("Subnet Mask : 255.255.255.0\n");
        printf("Gateway    : 192.168.1.1\n");
        printf("Lease Time : 60 seconds (Demo)\n");
    }
    else
    {
        printf("IP allocation failed.\n");
    }

    close(client_socket);
    return 0;
}
