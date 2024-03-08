#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#define handle_error(msg, cod) \
    \  
                                                                                                                                                                                                                                                                              \
    {                          \
        perror(msg);           \
        exit(cod);             \
    }

int PORT;

char carte[1000];

void Descarcare_carte_dorita(const char *fisier_desc, const char *carte)
{
    CURL *sesiune_curenta;
    sesiune_curenta = curl_easy_init();
    if (sesiune_curenta)
    {
        FILE *file = fopen(fisier_desc, "wb");
        if (!file)
        {
            printf("Nu se poate deschide.\n");
            exit(0);
        }
        curl_easy_setopt(sesiune_curenta, CURLOPT_URL, carte);
        curl_easy_setopt(sesiune_curenta, CURLOPT_WRITEDATA, file);
        curl_easy_perform(sesiune_curenta);
        curl_easy_cleanup(sesiune_curenta);
        fclose(file);
    }
    curl_global_cleanup();
}

int main(int argc, char *argv[])
{

    int sd;
    struct sockaddr_in server;
    char msg[100];

    if (argc != 3)
    {
        printf("Comanda nu este scrisa bine: ./numedocument adresa port");
        return 0;
    }

    PORT = atoi(argv[2]);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        handle_error("Eroare la socket", 5);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(PORT);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        handle_error("Eroare la connect", 6);
    }

    bzero(msg, 100);

    printf("[client]Pentru a va conecta utilizati: ~Login: username~\n");

    fflush(stdout);
    char comenzi[100];
    bzero(msg, 100);

    while (!feof(stdin), fgets(comenzi, sizeof(comenzi), stdin) != NULL)
    {
        if (comenzi[strlen(comenzi) - 1] == '\n')
            comenzi[strlen(comenzi) - 1] = '\0';

        if (write(sd, comenzi, 100) <= 0)
        {
            handle_error("Eroare la write", 7);
        }
        if (strstr(comenzi, "Login: "))
        {
            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 8);
            }
            else
            {
                printf("[client]%s\n", msg);
                if (!strstr(msg, "Comanda nu este una corecta"))
                {
                    printf("\n[client]Optiuni generale:\n");
                    printf("[client]Cautare dupa subgen:\n");
                    printf("[client]Cautare dupa gen:\n");
                    printf("[client]Cautare dupa autor:\n");
                    printf("[client]Cautare dupa titlu:\n");
                    printf("[client]Cautare dupa an:\n");
                    printf("[client]Cautare dupa ISBN:\n");
                    printf("[client]Cautare dupa rating:\n");
                    printf("[client]Afisare\n");
                    printf("[client]Istoric\n");
                    printf("[client]Recomandari\n");
                    printf("[client]Carti ce vor aparea curand\n");
                    printf("[client]Descarcare:\n");
                    printf("[client]Rating\n");
                    printf("[client]Tabela\n");
                    printf("[client]Quit\n\n");
                }
            }
        }
        else if ((strstr(comenzi, "Cautare dupa subgen: ")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 23);
            }
            else
                printf("[client]%s\n", msg);
        }
        else if ((strstr(comenzi, "Cautare dupa gen: ")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 1);
            }
            else
                printf("[client]%s\n", msg);
        }
        else if ((strstr(comenzi, "Cautare dupa autor: ")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 2);
            }
            else
                printf("[client]%s\n", msg);
        }
        else if ((strstr(comenzi, "Afisare")))
        {
            char msg[1000];
            bzero(msg, 1000);

            if (read(sd, msg, 1000) < 0)
            {
                handle_error("Eroare la read", 2);
            }
            else
                printf("[client]%s\n", msg);
        }
        else if ((strstr(comenzi, "Cautare dupa titlu: ")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 3);
            }
            else
                printf("[client]%s\n", msg);
        }
        else if ((strstr(comenzi, "Cautare dupa an: ")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 4);
            }
            else
                printf("[client]%s\n", msg);
        }
        else if ((strstr(comenzi, "Cautare dupa ISBN: ")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 5);
            }
            else
                printf("[client]%s\n", msg);
        }
        else if ((strstr(comenzi, "Cautare dupa rating: ")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 6);
            }
            else
                printf("[client]%s\n", msg);
        }
        else if ((strstr(comenzi, "Istoric")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 7);
            }
            else
                printf("%s\n", msg);
        }
        else if ((strstr(comenzi, "Recomandari")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 8);
            }
            else
                printf("[client]%s\n", msg);
        }
        else if ((strstr(comenzi, "Carti ce vor aparea curand")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 8);
            }
            else
                printf("%s\n", msg);
        }
        else if ((strstr(comenzi, "Descarcare: ")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 14);
            }
            else
            {
                strcpy(carte, msg);

                if (strcmp(carte, "1984") == 0)
                {
                    Descarcare_carte_dorita("1984.pdf", "https://rauterberg.employee.id.tue.nl/lecturenotes/DDM110%20CAS/Orwell-1949%201984.pdf");
                }
                else if (strcmp(carte, "SecretulMenajerei") == 0)
                {
                    Descarcare_carte_dorita("SecretulMenajerei.pdf", "https://101books.club/pdf/2208/Freida-McFadden-Secretul-menajerei.pdf");
                }
                else if (strcmp(carte, "OFataDinBucati") == 0)
                {
                    Descarcare_carte_dorita("OFataDinBucati.pdf", "https://101books.club/pdf/1435/o-fata-din-bucati-kathleen-glasgow.pdf");
                }
                else if (strcmp(carte, "OViataMarunta") == 0)
                {
                    Descarcare_carte_dorita("OViataMarunta.pdf", "https://101books.club/pdf/1740/o-viata-marunta-hanya-yanagihara.pdf");
                }

                strcpy(msg, "Cartea s-a descarcat.\n");
                printf("[client]%s\n", msg);
            }
        }
        else if ((strstr(comenzi, "Tabela")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 14);
            }
            else
            {
                printf("[client]\n%s\n", msg);
            }
        }
        else if ((strstr(comenzi, "Rating")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 15);
            }
            else
            {
                printf("[client]%s\n", msg);
            }

            fgets(comenzi, sizeof(comenzi), stdin);

            if (write(sd, comenzi, 100) <= 0)
            {
                handle_error("Eroare la write", 7);
            }

            bzero(msg, 100);
            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 15);
            }
            else
            {
                printf("[client]%s\n", msg);
            }
        }
        else if ((strstr(comenzi, "Quit")))
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 9);
            }
            else
            {
                exit(0);
                close(sd);
            }
        }
        else
        {
            char msg[100];
            bzero(msg, 100);

            if (read(sd, msg, 100) < 0)
            {
                handle_error("Eroare la read", 10);
            }
            else
                printf("[client]%s\n", msg);
        }
    }
    close(sd);
}