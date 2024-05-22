#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define PORT 12345
#define MAX_BUFFER_SIZE 1024

void sigint_handler(int signum)
{
    printf("\nSignal %d zachyteny a ignorovany.\n", signum);
}

// Funkcia pre obsluhu komunikácie s klientomc
void *handle_client(void *arg)
{
    int *client_sockets = (int *)arg;
    int aktualny_client = client_sockets[0];
    int druhy_client = client_sockets[1];
    char buffer[MAX_BUFFER_SIZE];

    while (1)
    {
        // Ignoruje vypnutie
        signal(SIGINT, sigint_handler);

        // Vyčistenie bufferu pred prijatím údajov
        memset(buffer, 0, sizeof(buffer));

        // Prijatie údajov od aktuálneho klienta
        if (recv(aktualny_client, buffer, MAX_BUFFER_SIZE, 0) <= 0)
        {
            perror("Prijatie udajov od klienta zlyhalo");
            break;
        }

        // Výpis správy od aktuálneho klienta
        printf("Sprava od klienta: %s\n", buffer);

        // Odoslanie správy druhému klientovi
        send(druhy_client, buffer, strlen(buffer), 0);
    }

    // Zatvorenie socketov pre klientov
    close(aktualny_client);
    close(druhy_client);

    // Ukončenie vlákna
    pthread_exit(NULL);
}

int main()
{
    int server_fd, novy_socket1, novy_socket2;
    struct sockaddr_in adresa;
    int addrlen = sizeof(adresa);
    pthread_t thread1, thread2;
    int client_sockets1[2], client_sockets2[2];

    // Vytvorenie socketu
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Vytvorenie socketu zlyhalo");
        exit(EXIT_FAILURE);
    }

    adresa.sin_family = AF_INET;
    adresa.sin_addr.s_addr = INADDR_ANY;
    adresa.sin_port = htons(PORT);

    // Pripojenie socketu k adrese
    if (bind(server_fd, (struct sockaddr *)&adresa, sizeof(adresa)) < 0)
    {
        perror("Bind zlyhal");
        exit(EXIT_FAILURE);
    }

    // Počúvanie pre prichádzajúce spojenia
    if (listen(server_fd, 3) < 0)
    {
        perror("Pocuvanie zlyhalo");
        exit(EXIT_FAILURE);
    }

    printf("Server pocuva na porte %d...\n", PORT);

    // Prijatie spojenia od klienta 1
    if ((novy_socket1 = accept(server_fd, (struct sockaddr *)&adresa, (socklen_t *)&addrlen)) < 0)
    {
        perror("Prijatie zlyhalo");
        exit(EXIT_FAILURE);
    }

    printf("Pripojenie prijate od %s:%d\n", inet_ntoa(adresa.sin_addr), ntohs(adresa.sin_port));

    // Prijatie spojenia od klienta 2
    if ((novy_socket2 = accept(server_fd, (struct sockaddr *)&adresa, (socklen_t *)&addrlen)) < 0)
    {
        perror("Prijatie zlyhalo");
        exit(EXIT_FAILURE);
    }

    printf("Pripojenie prijate od %s:%d\n", inet_ntoa(adresa.sin_addr), ntohs(adresa.sin_port));

    // Nastavenie polí socketov pre klientov
    client_sockets1[0] = novy_socket1;
    client_sockets1[1] = novy_socket2;

    client_sockets2[0] = novy_socket2;
    client_sockets2[1] = novy_socket1;

    // Vytvorenie vlákien na obsluhu komunikácie s klientmi
    if (pthread_create(&thread1, NULL, handle_client, (void *)client_sockets1) != 0)
    {
        perror("Vytvorenie thread-u zlyhalo");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread2, NULL, handle_client, (void *)client_sockets2) != 0)
    {
        perror("Vytvorenie thread-u zlyhalo");
        exit(EXIT_FAILURE);
    }

    // Čakanie na ukončenie vlákien
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Zatvorenie serverového socketu
    close(server_fd);

    return 0;
}
