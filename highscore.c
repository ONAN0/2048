#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 12345
#define MAX_BUFFER_SIZE 1024
#define FILENAME "highscore.txt"
#define PID_FILE "PID.txt"

void manage_child_pid(int pid)
{
    FILE *pid_file = fopen(PID_FILE, "a"); // Otvorí PID súbor v režime pridávania

    // Kontrola otvorenia PID súboru
    if (pid_file == NULL)
    {
        perror("Chyba pri otváraní PID súboru");
        exit(EXIT_FAILURE);
    }

    // Zapisuje PID do PID súboru
    fprintf(pid_file, "%d\n", pid);
    fflush(pid_file);

    // Kontrola zatvorenia PID súboru
    if (fclose(pid_file) != 0)
    {
        perror("Chyba pri zatváraní súboru");
        exit(EXIT_FAILURE);
    }
}

void write_highscore(char *buffer)
{
    FILE *file = fopen(FILENAME, "a"); // Otvorí súbor v režime pridávania

    // Kontrola otvorenia highscore súboru
    if (file == NULL)
    {
        perror("Chyba pri otváraní súboru");
        exit(EXIT_FAILURE);
    }

    // Zapisuje skóre do highscore súboru
    fprintf(file, "%s\n", buffer);
    fflush(file); // Zabezpečuje okamžitý zápis dát

    // Kontrola zatvorenia súboru
    if (fclose(file) != 0)
    {
        perror("Chyba pri zatváraní súboru");
        exit(EXIT_FAILURE);
    }
}

void xor_text_files(const char *file1, const char *file2, const char *outputFile)
{
    FILE *input1 = fopen(file1, "r");
    FILE *input2 = fopen(file2, "r");
    FILE *output = fopen(outputFile, "w");

    if (input1 == NULL || input2 == NULL || output == NULL)
    {
        perror("Chyba pri otváraní súborov");
        return;
    }

    int byte1, byte2;
    while ((byte1 = fgetc(input1)) != EOF && (byte2 = fgetc(input2)) != EOF)
    {
        fputc(byte1 ^ byte2, output);
    }

    // Ak je jeden súbor dlhší ako druhý, pokračuj v XOR s 0
    while ((byte1 = fgetc(input1)) != EOF)
    {
        fputc(byte1 ^ 0, output);
    }

    while ((byte2 = fgetc(input2)) != EOF)
    {
        fputc(0 ^ byte2, output);
    }

    if (fclose(input1) != 0)
    {
        perror("Chyba pri zatváraní súboru");
        exit(EXIT_FAILURE);
    }

    if (fclose(input2) != 0)
    {
        perror("Chyba pri zatváraní súboru");
        exit(EXIT_FAILURE);
    }

    if (fclose(output) != 0)
    {
        perror("Chyba pri zatváraní súboru");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    //* Síťová komunikácia ---------------------------------------------------------------------------

    int sock = 0;
    struct sockaddr_in serv_addr;

    // Vytvorenie socketu
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Chyba pri vytváraní socketu");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Konvertuje IPv4 a IPv6 adresy z textového do binárneho tvaru
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        perror("Neplatná adresa/Adresa nie je podporovaná");
        exit(EXIT_FAILURE);
    }

    // Pripojenie na server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Pripojenie zlyhalo");
        close(sock);
        exit(EXIT_FAILURE);
    }

    //* -----------------------------------------------------------------------------------------------
    //* Záznam PID ------------------------------------------------------------------------------------

    // Otvorí PID súbor v režime zápisu
    FILE *pid_file = fopen(PID_FILE, "w");

    if (pid_file == NULL)
    {
        perror("Chyba pri otváraní PID súboru");
        exit(EXIT_FAILURE);
    }

    // Zapíše PID rodičovského procesu do PID súboru
    fprintf(pid_file, "%d\n", getpid());
    fflush(pid_file);

    //* -----------------------------------------------------------------------------------------------
    //* Vytvorenie potrubia (pipe) -------------------------------------------------------------------

    int pipe_fd[2];
    int child1_child2[2];

    if (pipe(pipe_fd) == -1)
    {
        perror("Vytvorenie potrubia zlyhalo");
        exit(EXIT_FAILURE);
    }

    if (pipe(child1_child2) == -1)
    {
        perror("Vytvorenie potrubia zlyhalo");
        exit(EXIT_FAILURE);
    }

    //* -----------------------------------------------------------------------------------------------
    //* Vytvorenie procesov ---------------------------------------------------------------------------

    pid_t child_pid1, child_pid2;

    child_pid1 = fork();

    if (child_pid1 == -1)
    {
        perror("Fork pre Child 1 zlyhal");
        exit(EXIT_FAILURE);
    }

    //* Child 1 ---------------------------------------------------------------------------------------

    if (child_pid1 == 0)
    {
        close(sock);
        close(pipe_fd[1]);
        close(child1_child2[0]);

        manage_child_pid(getpid());

        while (1)
        {
            char buffer_od_rodica[MAX_BUFFER_SIZE];
            ssize_t bytes_read = read(pipe_fd[0], buffer_od_rodica, MAX_BUFFER_SIZE);

            if (bytes_read == -1)
            {
                perror("Čítanie z potrubia v Child 1 zlyhalo");
                exit(EXIT_FAILURE);
            }

            if (strcmp(buffer_od_rodica, "exit") == 0)
            {
                break;
            }

            // Vytlačí prijatý buffer od rodiča
            printf("Child 1 prijal: %.*s\n", (int)bytes_read, buffer_od_rodica);

            write_highscore(buffer_od_rodica);
        }

        int go_1 = 1;

        if ((write(child1_child2[1], &go_1, sizeof(int))) == -1)
        {
            perror("Zápis do potrubia v Child 1 zlyhal");
            exit(EXIT_FAILURE);
        }

        // Zatvára čítaciu časť potrubia zo strany Child 1
        close(pipe_fd[0]);
        close(child1_child2[1]);

        exit(EXIT_SUCCESS);
    }

    //* -----------------------------------------------------------------------------------------------

    else
    {
        // Toto vykonáva Rodič

        pid_t child_pid2 = fork();

        if (child_pid2 == -1)
        {
            perror("Fork pre Child 2 zlyhal");
            exit(EXIT_FAILURE);
        }

        //* Child 2 -----------------------------------------------------------------------------------

        if (child_pid2 == 0)
        {
            close(sock);
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            close(child1_child2[1]);

            manage_child_pid(getpid());

            int go = 0;

            do
            {
                read(child1_child2[0], &go, sizeof(int));
            } while (go == 0);

            char *file1 = "highscore.txt";
            char *file2 = "xor_file.txt";
            char *outputFile = "output.txt";

            xor_text_files(file1, file2, outputFile);

            close(child1_child2[0]);
            exit(EXIT_SUCCESS);
        }

        //* -------------------------------------------------------------------------------------------

        else
        {
            // Toto vykonáva Rodič

            close(pipe_fd[0]);
            close(child1_child2[0]);
            close(child1_child2[1]);

            while (1)
            {
                // Generovanie dát na odoslanie
                char from_server[MAX_BUFFER_SIZE];

                memset(from_server, 0, MAX_BUFFER_SIZE);

                if (recv(sock, from_server, MAX_BUFFER_SIZE, 0) <= 0)
                {
                    perror("Prijatie zo servera zlyhalo");
                    close(sock);
                    exit(EXIT_FAILURE);
                }

                printf("Správa od Servera: %s\n", from_server);

                // Zapíše dáta do potrubia
                ssize_t bytes_written = write(pipe_fd[1], from_server, MAX_BUFFER_SIZE);

                if (strcmp(from_server, "exit") == 0)
                {
                    break;
                }

                if (bytes_written == -1)
                {
                    perror("Zápis do potrubia zlyhal");
                    exit(EXIT_FAILURE);
                }

                // Odpovie serveru
                char response[] = "Tvoje skóre bolo uložené.";
                send(sock, response, strlen(response), 0);
            }

            // Zatvára zapisovaciu časť potrubia zo strany Rodiča
            close(pipe_fd[1]);

            // Čaká na deti
            waitpid(child_pid1, NULL, 0);
            waitpid(child_pid2, NULL, 0);
        }
    }

    //* -----------------------------------------------------------------------------------------------

    // Kontrola zatvorenia PID súboru
    if (fclose(pid_file) != 0)
    {
        perror("Chyba pri zatváraní súboru");
        exit(EXIT_FAILURE);
    }

    return 0;
}
