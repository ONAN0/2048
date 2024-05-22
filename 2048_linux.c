#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <ncurses.h>
#include <math.h>

#define SIZE 4
#define PORT 12345
#define MAX_BUFFER_SIZE 1024

int pole[SIZE][SIZE];

void nahodny_dielok()
{
   int prazne_pole[SIZE * SIZE][2];
   int miesto_prazneho_pola = 0;

   // Najdenie 0 pola
   for (int i = 0; i < SIZE; i++)
   {
      for (int j = 0; j < SIZE; j++)
      {
         // Uklada suradnice kazdeho policka s hodnotou 0
         if (pole[i][j] == 0)
         {
            prazne_pole[miesto_prazneho_pola][0] = i;
            prazne_pole[miesto_prazneho_pola][1] = j;
            miesto_prazneho_pola++;
         }
      }
   }

   // Dosadza nahodne cislo na nahodne miesto
   if (miesto_prazneho_pola > 0)
   {
      // Vybera nahodne miesto
      int index = rand() % miesto_prazneho_pola;

      // Nahodne vybera hodnotu medzi 2 a 4
      int value = (rand() % 2 + 1) * 2;

      // Dosadza hodnotu do daneho pola
      int x = prazne_pole[index][0];
      int y = prazne_pole[index][1];
      pole[x][y] = value;
   }
}

void zakladne_pole()
{
   // Vytvori ciste 0-ove pole
   for (int i = 0; i < SIZE; i++)
   {
      for (int j = 0; j < SIZE; j++)
      {
         pole[i][j] = 0;
      }
   }

   // Vlozi dve nahodne cisla do hracieho pola
   nahodny_dielok();
   nahodny_dielok();
}

void vypis_pola()
{
   for (int i = 0; i <= (SIZE * 2); i++)
   {
      if (i % 2 == 0)
      {
         for (int j = 0; j <= (SIZE * 2); j++)
         {
            if (j % 2 == 0)
            {
               printw("+");
            }
            else
            {
               printw("-------");
            }
         }
      }
      else
      {
         for (int j = 0; j <= (SIZE * 2); j++)
         {
            if (j % 2 == 0)
            {
               printw("|");
            }
            else
            {
               int hodnota = pole[i / 2][j / 2];
               init_pair(1, 0, 0); // Definuje farebny par (cislo paru, farba textu, farba pozadia)
               init_pair(2, 0, 226);
               init_pair(3, 0, 220);
               init_pair(4, 0, 214);
               init_pair(5, 0, 208);
               init_pair(6, 0, 202);
               init_pair(7, 0, 196);
               init_pair(8, 0, 197);
               init_pair(9, 0, 198);
               init_pair(10, 0, 199);
               init_pair(11, 0, 200);
               init_pair(12, 0, 201);

               if (hodnota != 0)
               {
                  hodnota = log2(hodnota);
               }

               attron(COLOR_PAIR(hodnota + 1));
               printw(" %5d ", pole[i / 2][j / 2]);
               attroff(COLOR_PAIR(hodnota + 1));
            }
         }
      }

      printw("\n");
   }
}

int koniec_hry()
{
   int i, j;

   // Kontrola moznych tahov
   for (i = 0; i < SIZE; i++)
   {
      for (j = 0; j < SIZE; j++)
      {
         // Kontrola prázdneho miesta
         if (pole[i][j] == 0)
         {
            return 0;
         }

         // Kontrola dvoch rovnakych cisel pod sebou
         if (i > 0 && pole[i][j] == pole[i - 1][j])
         {
            return 0;
         }

         // Kontrola dvoch rovnakych cisel vedla seba
         if (j > 0 && pole[i][j] == pole[i][j - 1])
         {
            return 0;
         }
      }
   }

   return 1;
}

void hore()
{
   for (int j = 0; j < SIZE; j++)
   {
      for (int i = 1; i < SIZE; i++)
      {
         int k = i;
         while (k > 0 && pole[k][j] != 0)
         {
            if (pole[k - 1][j] == 0)
            {
               pole[k - 1][j] = pole[k][j];
               pole[k][j] = 0;
               k--;
            }
            else if (pole[k - 1][j] == pole[k][j])
            {
               pole[k - 1][j] *= 2;
               pole[k][j] = 0;
               break;
            }
            else
            {
               break;
            }
         }
      }
   }
}

void dole()
{
   for (int j = 0; j < SIZE; j++)
   {
      for (int i = SIZE - 2; i >= 0; i--)
      {
         int k = i;
         while (k < SIZE - 1 && pole[k][j] != 0)
         {
            if (pole[k + 1][j] == 0)
            {
               pole[k + 1][j] = pole[k][j];
               pole[k][j] = 0;
               k++;
            }
            else if (pole[k + 1][j] == pole[k][j])
            {
               pole[k + 1][j] *= 2;
               pole[k][j] = 0;
               break;
            }
            else
            {
               break;
            }
         }
      }
   }
}

void vlavo()
{
   for (int i = 0; i < SIZE; i++)
   {
      for (int j = 1; j < SIZE; j++)
      {
         int k = j;
         while (k > 0 && pole[i][k] != 0)
         {
            if (pole[i][k - 1] == 0)
            {
               pole[i][k - 1] = pole[i][k];
               pole[i][k] = 0;
               k--;
            }
            else if (pole[i][k - 1] == pole[i][k])
            {
               pole[i][k - 1] *= 2;
               pole[i][k] = 0;
               break;
            }
            else
            {
               break;
            }
         }
      }
   }
}

void vpravo()
{
   for (int i = 0; i < SIZE; i++)
   {
      for (int j = SIZE - 2; j >= 0; j--)
      {
         int k = j;
         while (k < SIZE - 1 && pole[i][k] != 0)
         {
            if (pole[i][k + 1] == 0)
            {
               pole[i][k + 1] = pole[i][k];
               pole[i][k] = 0;
               k++;
            }
            else if (pole[i][k + 1] == pole[i][k])
            {
               pole[i][k + 1] *= 2;
               pole[i][k] = 0;
               break;
            }
            else
            {
               break;
            }
         }
      }
   }
}

int main()
{
   int sock = 0;
   struct sockaddr_in serv_addr;

   // Vytvorenie socketu
   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("Chyba pri vytváraní socketu");
      exit(EXIT_FAILURE);
   }

   // Nastavenie parametrov pre adresu servera
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(PORT);

   // Konverzia IP adresy z textovej podoby na binárnu formu
   if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
   {
      perror("Neplatná adresa/Adresa nie je podporovaná");
      exit(EXIT_FAILURE);
   }

   // Pripojenie k serveru
   if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
   {
      perror("Pripojenie zlyhalo");
      exit(EXIT_FAILURE);
   }

   // Vytvorenie časovača
   timer_t timerid;
   struct sigevent sev;
   struct itimerspec its;

   // Nastavenie callback funkcie pre časovač
   sev.sigev_notify = SIGEV_NONE;
   sev.sigev_notify_function = NULL;
   sev.sigev_notify_attributes = NULL;
   sev.sigev_value.sival_ptr = &timerid;

   // Vytvorenie časovača
   if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
   {
      perror("Chyba pri vytváraní časovača");
      exit(EXIT_FAILURE);
   }

   // Nastavenie časovača tak, aby vypršal po 4 sekundách
   its.it_value.tv_sec = 4;
   its.it_value.tv_nsec = 0;
   its.it_interval.tv_sec = 0; // Jednorazový časovač
   its.it_interval.tv_nsec = 0;

   // Nastavenie časovača
   if (timer_settime(timerid, 0, &its, NULL) == -1)
   {
      perror("Chyba pri nastavovaní časovača");
      exit(EXIT_FAILURE);
   }

   // Čakanie na vypršanie časovača s výpisom zostávajúceho času
   while (timer_gettime(timerid, &its) == 0 && (its.it_value.tv_sec > 0))
   {
      printf("\033[H\033[J");
      printf("Hra sa začne o %d \n", (int)its.it_value.tv_sec);
      sleep(1);
   }

   // Zrušenie časovača
   if (timer_delete(timerid) == -1)
   {
      perror("Chyba pri zrušení časovača");
      exit(EXIT_FAILURE);
   }

   // Inicializácia ncurses
   initscr();
   curs_set(0);   // Nastavenie viditeľnosti kurzora
   start_color(); // Zapnutie podpory farieb

   srand(time(NULL));
   zakladne_pole();
   char move;

   // Hlavná hra - nekonečný ciklus
   while (1)
   {
      clear();

      printw("Vitajte v 2048!\n");
      printw("Stlacte 'W' na pohyb hore, 'S' na pohyb dole, 'A' na pohyb vlavo, 'D' na pohyb vpravo a 'Q' pre ukoncenie.\n");

      vypis_pola();

      // Kontrola konca hry
      if (koniec_hry())
      {
         printw("Hra skoncila! Mozete stlacit 'Q' na ukoncenie.\n");
      }

      refresh(); // Obnovenie obrazovky

      move = getch();

      if (move == 'Q' || move == 'q')
      {
         break;
      }

      int pole_pred[SIZE][SIZE];

      // Zálohovanie aktuálneho stavu poľa
      for (int i = 0; i < SIZE; i++)
      {
         for (int j = 0; j < SIZE; j++)
         {
            pole_pred[i][j] = pole[i][j];
         }
      }

      // Spracovanie pohybu hráča
      switch (move)
      {
      case 'W':
      case 'w':
         hore();
         break;
      case 'S':
      case 's':
         dole();
         break;
      case 'A':
      case 'a':
         vlavo();
         break;
      case 'D':
      case 'd':
         vpravo();
         break;
      }

      // Kontrola zmeny poľa a pridanie náhodnej dlaždice
      int zmena = 0;
      for (int i = 0; i < SIZE; i++)
      {
         for (int j = 0; j < SIZE; j++)
         {
            if (pole[i][j] != pole_pred[i][j])
            {
               zmena = 1;
               break;
            }
         }
      }

      if (zmena)
      {
         nahodny_dielok();
      }

      sleep(0.5);
   }

   // Výpočet celkového skóre
   int sum = 0;
   for (int i = 0; i < SIZE; i++)
   {
      for (int j = 0; j < SIZE; j++)
      {
         sum += pole[i][j];
      }
   }

   // Ukončenie ncurses
   endwin();

   printf("Dakujem vam za hranie mojej verzie 2048!\n");
   printf("Vase skore je %d\n", sum);

   // Preposlanie skóre na server
   char message[MAX_BUFFER_SIZE];
   sprintf(message, "%d", sum);
   send(sock, message, strlen(message), 0);

   sleep(2);

   // Poslanie správy pre ukončenie spojenia
   char bye[] = "exit";
   send(sock, bye, strlen(bye), 0);

   // Čakanie na odpoveď od servera
   char response[MAX_BUFFER_SIZE] = {0};
   recv(sock, response, MAX_BUFFER_SIZE, 0);
   printf("Odpoveď od servera: %s\n", response);

   // Uzavretie spojenia so serverom
   close(sock);

   return 0;
}
