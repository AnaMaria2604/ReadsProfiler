#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>

sqlite3 *biblioteca;
sqlite3 *useri;
sqlite3 *rec;
sqlite3 *autori;
sqlite3 *genuri;
sqlite3 *subgenuri;
sqlite3 *feedback;
sqlite3 *istoric;
sqlite3_stmt *sql;

char c_gen[100];
char c_subgen[100];
char c_autor[100];
char c_titlu[100];
char c_an[100];
char c_ISBN[100];
char c_rating[100];
char username[100];
char carte[100];

#define PORT 6006

struct Continut_baza_de_date
{
    char *continut;
};

#define handle_error(msg, cod) \
    \  
                                                                                                                                                                                                                                                                            \
    {                          \
        perror(msg);           \
        exit(cod);             \
    }

int parsare(void *data, int nr_col, char **col_val, char **col_nume)
{
    struct Continut_baza_de_date *tabela = (struct Continut_baza_de_date *)data;

    for (int i = 0; i < nr_col; i++)
    {
        size_t l_veche = strlen(tabela->continut);
        size_t l_noua = l_veche + (col_val[i] ? strlen(col_val[i]) : 4) + 2;
        tabela->continut = realloc(tabela->continut, l_noua);
        if (tabela->continut == NULL)
        {
            perror("Eroare la realocare de memorie.\n");
            return SQLITE_ABORT;
        }
        if (col_val[i] != NULL)
        {
            strcat(tabela->continut, col_val[i]);
        }
        else
        {
            strcat(tabela->continut, "NULL");
        }
        strcat(tabela->continut, " ");
    }
    strcat(tabela->continut, "\n");
    return 0;
}

void operatii(int client, int *OK, int *OK2, int *OK3)
{
    char msg[100];
    bzero(msg, 100);

    printf("[server]Asteptam mesajul...\n");
    fflush(stdout);

    if (read(client, msg, 100) <= 0)
    {
        perror("[server]Eroare la read() de la client.\n");
        close(client);
        return;
    }
    printf("[server]Mesajul a fost receptionat...%s\n", msg);

    if (strstr(msg, "Login: "))
    {
        char msgrasp[100];
        bzero(msgrasp, 100);

        strncpy(username, msg + strlen("Login: "), strlen(msg) - strlen("Login: "));

        if (sqlite3_open("useri_conectati.db", &useri) != SQLITE_OK)
        {
            perror("Nu se poate deschide aceasta baza de date.\n");
        }

        char *eroare = 0;
        char intrare[1000];
        sprintf(intrare, "INSERT INTO useri (Username) VALUES ('%s');", username);
        sqlite3_exec(useri, intrare, 0, 0, &eroare);

        sqlite3_close(useri);

        char mesaj[100];
        strcpy(mesaj, "A fost conectat user-ul cu numele: ");
        strcat(mesaj, username);
        strcat(mesaj, ".");
        strcpy(msgrasp, mesaj);

        if (write(client, msgrasp, 100) <= 0)
        {
            perror("[server]Eroare la write() catre client.\n");
            return;
        }
        else
        {
            printf("[server]Mesajul a fost trasmis cu succes.\n");
            *OK = 1;
        }
    }
    else if ((strstr(msg, "Cautare dupa subgen: ")))
    {
        if (*OK == 1 && *OK2 == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "S-a gasit comanda de cautare dupa subgen.");

            strncpy(c_subgen, msg + strlen("Cautare dupa subgen: "), strlen(msg) - strlen("Cautare dupa subgen: "));

            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Cautare dupa gen: ")))
    {
        if (*OK == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "S-a gasit comanda de cautare dupa gen.");

            strncpy(c_gen, msg + strlen("Cautare dupa gen: "), strlen(msg) - strlen("Cautare dupa gen: "));

            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
            {
                printf("[server]Mesajul a fost trasmis cu succes.\n");
                *OK2 = 1;
            }
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Afisare")))
    {
        if (*OK == 1)
        {
            char msgrasp[1000];
            bzero(msgrasp, 1000);

            char intrare[1000];
            char gen_instr[1000], subgen_instr[1000], autor_instr[1000], titlu_instr[1000], an_instr[1000], ISBN_instr[1000], rating_instr[1000];
            int nr = 0;
            int ok_gen = 0;
            int ok_subgen = 0;
            int ok_autor = 0;
            int ok_titlu = 0;
            int ok_an = 0;
            int ok_ISBN = 0;
            int ok_rating = 0;
            strcpy(intrare, "SELECT * FROM biblioteca WHERE ");
            if (c_gen[0] != '\0')
            {
                nr++;
                ok_gen = 1;
                sprintf(gen_instr, "Gen = '%s' ", c_gen);
            }
            if (c_subgen[0] != '\0')
            {
                nr++;
                ok_subgen = 1;
                sprintf(subgen_instr, "Subgen = '%s' ", c_subgen);
            }
            if (c_autor[0] != '\0')
            {
                nr++;
                ok_autor = 1;
                sprintf(autor_instr, "Autor = '%s' ", c_autor);
            }
            if (c_titlu[0] != '\0')
            {
                nr++;
                ok_titlu = 1;
                sprintf(titlu_instr, "Titlu = '%s' ", c_titlu);
            }
            if (c_an[0] != '\0')
            {
                nr++;
                ok_an = 1;
                sprintf(an_instr, "An = '%s' ", c_an);
            }
            if (c_ISBN[0] != '\0')
            {
                nr++;
                ok_ISBN = 1;
                sprintf(ISBN_instr, "ISBN = '%s' ", c_ISBN);
            }
            if (c_rating[0] != '\0')
            {
                nr++;
                ok_rating = 1;
                sprintf(rating_instr, "Rating = '%s' ", c_rating);
            }
            if (nr == 0)
            {
                char intrare1[1000];
                if (sqlite3_open("biblioteca.db", &biblioteca) != SQLITE_OK)
                {
                    perror("Nu se poate deschide aceasta baza de date.\n");
                }
                strcpy(intrare1, "SELECT * FROM biblioteca;");
            }
            else if (nr == 1)
            {
                if (ok_gen == 1)
                {
                    char *eroare = 0;
                    if (sqlite3_open("biblioteca.db", &biblioteca) != SQLITE_OK)
                    {
                        perror("Nu se poate deschide aceasta baza de date.\n");
                    }
                    strcat(intrare, gen_instr);
                    strcat(intrare, ";");
                    printf("%s\n", intrare);
                    sqlite3_exec(biblioteca, intrare, 0, 0, &eroare);
                    sqlite3_close(biblioteca);
                }
                else if (ok_subgen == 1)
                {
                    char *eroare = 0;
                    if (sqlite3_open("biblioteca.db", &biblioteca) != SQLITE_OK)
                    {
                        perror("Nu se poate deschide aceasta baza de date.\n");
                    }
                    strcat(intrare, subgen_instr);
                    strcat(intrare, ";");
                    sqlite3_exec(biblioteca, intrare, 0, 0, &eroare);
                    sqlite3_close(biblioteca);
                }
                else if (ok_autor == 1)
                {
                    char *eroare = 0;
                    if (sqlite3_open("biblioteca.db", &biblioteca) != SQLITE_OK)
                    {
                        perror("Nu se poate deschide aceasta baza de date.\n");
                    }
                    strcat(intrare, autor_instr);
                    strcat(intrare, ";");
                    sqlite3_exec(biblioteca, intrare, 0, 0, &eroare);
                    sqlite3_close(biblioteca);
                }
                else if (ok_titlu == 1)
                {
                    char *eroare = 0;
                    if (sqlite3_open("biblioteca.db", &biblioteca) != SQLITE_OK)
                    {
                        perror("Nu se poate deschide aceasta baza de date.\n");
                    }
                    strcat(intrare, titlu_instr);
                    strcat(intrare, ";");
                    sqlite3_exec(biblioteca, intrare, 0, 0, &eroare);
                    sqlite3_close(biblioteca);
                }
                else if (ok_an == 1)
                {
                    char *eroare = 0;
                    if (sqlite3_open("biblioteca.db", &biblioteca) != SQLITE_OK)
                    {
                        perror("Nu se poate deschide aceasta baza de date.\n");
                    }
                    strcat(intrare, an_instr);
                    strcat(intrare, ";");
                    sqlite3_exec(biblioteca, intrare, 0, 0, &eroare);
                    sqlite3_close(biblioteca);
                }
                else if (ok_rating == 1)
                {
                    char *eroare = 0;
                    if (sqlite3_open("biblioteca.db", &biblioteca) != SQLITE_OK)
                    {
                        perror("Nu se poate deschide aceasta baza de date.\n");
                    }
                    strcat(intrare, rating_instr);
                    strcat(intrare, ";");
                    sqlite3_exec(biblioteca, intrare, 0, 0, &eroare);
                    sqlite3_close(biblioteca);
                }
                else if (ok_ISBN == 1)
                {
                    char *eroare = 0;
                    if (sqlite3_open("biblioteca.db", &biblioteca) != SQLITE_OK)
                    {
                        perror("Nu se poate deschide aceasta baza de date.\n");
                    }
                    strcat(intrare, ISBN_instr);
                    strcat(intrare, ";");
                    sqlite3_exec(biblioteca, intrare, 0, 0, &eroare);
                    sqlite3_close(biblioteca);
                }
            }
            else if (nr >= 2)
            {
                int nr2 = 0;
                char *eroare = 0;
                if (sqlite3_open("biblioteca.db", &biblioteca) != SQLITE_OK)
                {
                    perror("Nu se poate deschide aceasta baza de date.\n");
                }

                if (ok_gen == 1)
                {
                    nr2++;
                    if (nr2 == 1)
                    {
                        strcat(intrare, gen_instr);
                    }
                    else
                    {
                        strcat(intrare, "and ");
                        strcat(intrare, gen_instr);
                    }
                }
                if (ok_subgen == 1)
                {
                    nr2++;
                    if (nr2 == 1)
                    {
                        strcat(intrare, subgen_instr);
                    }
                    else
                    {
                        strcat(intrare, "and ");
                        strcat(intrare, subgen_instr);
                    }
                }
                if (ok_autor == 1)
                {
                    nr2++;
                    if (nr2 == 1)
                    {
                        strcat(intrare, autor_instr);
                    }
                    else
                    {
                        strcat(intrare, "and ");
                        strcat(intrare, autor_instr);
                    }
                }
                if (ok_titlu == 1)
                {
                    nr2++;
                    if (nr2 == 1)
                    {
                        strcat(intrare, titlu_instr);
                    }
                    else
                    {
                        strcat(intrare, "and ");
                        strcat(intrare, titlu_instr);
                    }
                }
                if (ok_an == 1)
                {
                    nr2++;
                    if (nr2 == 1)
                    {
                        strcat(intrare, an_instr);
                    }
                    else
                    {
                        strcat(intrare, "and ");
                        strcat(intrare, an_instr);
                    }
                }
                if (ok_rating == 1)
                {
                    nr2++;
                    if (nr2 == 1)
                    {
                        strcat(intrare, rating_instr);
                    }
                    else
                    {
                        strcat(intrare, "and ");
                        strcat(intrare, rating_instr);
                    }
                }
                if (ok_ISBN == 1)
                {
                    nr2++;
                    if (nr2 == 1)
                    {
                        strcat(intrare, ISBN_instr);
                    }
                    else
                    {
                        strcat(intrare, "and ");
                        strcat(intrare, ISBN_instr);
                    }
                }

                strcat(intrare, ";");
                printf("%s\n", intrare);
                sqlite3_exec(biblioteca, intrare, 0, 0, &eroare);
                sqlite3_close(biblioteca);
            }

            if (sqlite3_open("biblioteca.db", &biblioteca) != SQLITE_OK)
            {
                perror("Nu se poate deschide aceasta baza de date.\n");
            }
            struct Continut_baza_de_date select;
            select.continut = calloc(1, sizeof(char));

            char *eroare = 0;
            char *intrare2 = malloc(sizeof(char) * (strlen(intrare) + 1));

            strcpy(intrare2, intrare);

            int rez = sqlite3_exec(biblioteca, intrare2, parsare, &select, &eroare);
            if (rez != SQLITE_OK)
            {
                fprintf(stderr, "Eroare: %s\n", eroare);
                sqlite3_free(eroare);
            }
            strcpy(msgrasp, select.continut);
            free(select.continut);

            char intrare10[1000];
            char *eroare10 = 0;

            if (sqlite3_open("istoric.db", &istoric) != SQLITE_OK)
            {
                perror("Nu se poate deschide aceasta baza de date.\n");
            }

            sprintf(intrare10, "INSERT INTO istoric (Username, Descarcare, Titlu, Autor, Gen, Subgen) VALUES ('%s',0,'%s', '%s', '%s', '%s');", username, c_titlu, c_autor, c_gen, c_subgen);
            printf("intrare: %s\n", intrare10);

            sqlite3_exec(istoric, intrare10, 0, 0, &eroare10);

            sqlite3_close(istoric);

            if (write(client, msgrasp, 1000) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
            {
                printf("[server]Mesajul a fost trasmis cu succes.\n");
                *OK2 = 1;
            }
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Cautare dupa autor: ")))
    {
        if (*OK == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "S-a gasit comanda de cautare dupa autor.");

            strncpy(c_autor, msg + strlen("Cautare dupa autor: "), strlen(msg) - strlen("Cautare dupa autor: "));

            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Cautare dupa titlu: ")))
    {
        if (*OK == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "S-a gasit comanda de cautare dupa titlu.");

            strncpy(c_titlu, msg + strlen("Cautare dupa titlu: "), strlen(msg) - strlen("Cautare dupa titlu: "));
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Cautare dupa an: ")))
    {
        if (*OK == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "S-a gasit comanda de cautare dupa an.");

            strncpy(c_an, msg + strlen("Cautare dupa an: "), strlen(msg) - strlen("Cautare dupa an: "));

            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Cautare dupa ISBN: ")))
    {
        if (*OK == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "S-a gasit comanda de cautare dupa ISBN.");

            strncpy(c_ISBN, msg + strlen("Cautare dupa ISBN: "), strlen(msg) - strlen("Cautare dupa ISBN: "));

            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Cautare dupa rating: ")))
    {
        if (*OK == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "S-a gasit comanda de cautare dupa rating.");

            strncpy(c_rating, msg + strlen("Cautare dupa rating: "), strlen(msg) - strlen("Cautare dupa rating: "));

            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Istoric")))
    {
        if (*OK == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            if (sqlite3_open("istoric.db", &istoric) != SQLITE_OK)
            {
                perror("Nu se poate deschide aceasta baza de date.\n");
            }
            char intrare[1000];
            sprintf(intrare, "SELECT * FROM istoric WHERE Username='%s';", username);
            char *intrare2 = malloc(sizeof(char) * (strlen(intrare) + 1));
            strcpy(intrare2, intrare);
            struct Continut_baza_de_date istoric_user;
            istoric_user.continut = calloc(1, sizeof(char));
            char *eroare = 0;
            sqlite3_exec(istoric, intrare2, parsare, &istoric_user, &eroare);
            strcpy(msgrasp, istoric_user.continut);
            free(istoric_user.continut);

            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Recomandari")))
    {
        if (*OK == 1)
        {
            char continut_istoric[100];
            bzero(continut_istoric, 100);

            if (sqlite3_open("istoric.db", &istoric) != SQLITE_OK)
            {
                perror("Nu se poate deschide aceasta baza de date.\n");
            }
            char intrare[1000];
            sprintf(intrare, "SELECT * FROM istoric WHERE Username='%s';", username);
            char *intrare2 = malloc(sizeof(char) * (strlen(intrare) + 1));
            strcpy(intrare2, intrare);
            struct Continut_baza_de_date istoric_user;
            istoric_user.continut = calloc(1, sizeof(char));
            char *eroare = 0;
            sqlite3_exec(istoric, intrare2, parsare, &istoric_user, &eroare);
            strcpy(continut_istoric, istoric_user.continut);
            free(istoric_user.continut);

            char msgrasp[100];
            bzero(msgrasp, 100);

            int nr = 0;

            if (!strstr(continut_istoric, "1984"))
            {
                nr++;
            }
            if (!strstr(continut_istoric, "SecretulMenajerei"))
            {
                nr++;
            }
            if (!strstr(continut_istoric, "OFataDinBucati"))
            {
                nr++;
            }
            if (!strstr(continut_istoric, "OViataMarunta"))
            {
                nr++;
            }

            if (nr != 0)
            {
                char meeesaj[100];
                strcpy(meeesaj, " ");
                if (!strstr(continut_istoric, "1984"))
                {
                    strcat(meeesaj, "|1984|");
                }
                if (!strstr(continut_istoric, "SecretulMenajerei"))
                {
                    strcat(meeesaj, "|SecretulMenajerei|");
                }
                if (!strstr(continut_istoric, "OFataDinBucati"))
                {
                    strcat(meeesaj, "|OFataDinBucati|");
                }
                if (!strstr(continut_istoric, "OViataMarunta"))
                {
                    strcat(meeesaj, "|OViataMarunta|");
                }
                strcpy(msgrasp, meeesaj);
            }
            else
                strcpy(msgrasp, "Nu avem recomandari pentru dumneavoastra la acest moment.");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Carti ce vor aparea curand")))
    {
        if (*OK == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            if (sqlite3_open("recomandari.db", &rec) != SQLITE_OK)
            {
                perror("Nu se poate deschide aceasta baza de date.\n");
            }

            char *intrare11 = "SELECT * FROM recomandari;";
            struct Continut_baza_de_date recomandari;
            recomandari.continut = calloc(1, 3 * sizeof(char));

            char *eroare11 = 0;
            sqlite3_exec(rec, intrare11, parsare, &recomandari, &eroare11);
            strcat(msgrasp, recomandari.continut);
            free(recomandari.continut);

            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Descarcare: ")))
    {
        if (*OK == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            strncpy(carte, msg + strlen("Descarcare: "), strlen(msg) - strlen("Descarcare: "));
            strcpy(msgrasp, carte);

            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
            {

                char intrare10[1000];
                char *eroare10 = 0;
                if (sqlite3_open("istoric.db", &istoric) != SQLITE_OK)
                {
                    perror("Nu se poate deschide aceasta baza de date.\n");
                }
                sprintf(intrare10, "INSERT INTO istoric (Username, Descarcare, Titlu, Autor, Gen, Subgen) VALUES ('%s',1,'%s', '', '', '');", username, c_titlu);
                printf("intrare: %s\n", intrare10);
                sqlite3_exec(istoric, intrare10, 0, 0, &eroare10);

                sqlite3_close(istoric);

                printf("[server]Mesajul a fost trasmis cu succes.\n");
                *OK3 = 1;
            }
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Tabela")))
    {
        if (*OK == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "S-a gasit comanda de descarcare a unei carti.");

            if (sqlite3_open("feedback.db", &feedback) != SQLITE_OK)
            {
                perror("Nu se poate deschide aceasta baza de date.\n");
            }

            char *intrare = "SELECT * FROM rating;";
            struct Continut_baza_de_date rating;
            rating.continut = calloc(1, sizeof(char));

            char *eroare = 0;
            sqlite3_exec(feedback, intrare, parsare, &rating, &eroare);
            strcpy(msgrasp, rating.continut);
            free(rating.continut);

            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
            {
                printf("[server]Mesajul a fost trasmis cu succes.\n");
                *OK3 = 1;
            }
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Rating")))
    {
        if (*OK == 1 && *OK3 == 1)
        {
            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Introduceti feedback ul:");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");

            if (read(client, msg, 100) <= 0)
            {
                perror("[server]Eroare la read() de la client.\n");
                return;
            }

            if (sqlite3_open("feedback.db", &feedback) != SQLITE_OK)
            {
                perror("Nu se poate deschide aceasta baza de date.\n");
            }

            char intrare[1000];
            char *eroare = 0;
            sprintf(intrare, "INSERT INTO rating (Username, Carte, Nota) VALUES ('%s','%s','%s');", username, carte, msg);

            sqlite3_exec(feedback, intrare, 0, 0, &eroare);
            sqlite3_close(feedback);

            bzero(msgrasp, 100);
            strcpy(msgrasp, "Multumim de feedback.");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
        else
        {
            printf("Nu se poate utiliza aceasta operatie.\n");

            char msgrasp[100];
            bzero(msgrasp, 100);

            strcpy(msgrasp, "Nu se poate utiliza aceasta operatie");
            if (write(client, msgrasp, 100) <= 0)
            {
                perror("[server]Eroare la write() catre client.\n");
                return;
            }
            else
                printf("[server]Mesajul a fost trasmis cu succes.\n");
        }
    }
    else if ((strstr(msg, "Quit")))
    {
        char msgrasp[100];
        bzero(msgrasp, 100);

        strcpy(msgrasp, "S-a gasit comanda de quit.");

        if (sqlite3_open("useri_conectati.db", &useri) != SQLITE_OK)
        {
            perror("Nu se poate deschide aceasta baza de date.\n");
        }

        char *eroare = 0;
        char intrare[1000];
        sprintf(intrare, "DELETE FROM useri WHERE Username = '%s';", username);
        sqlite3_exec(useri, intrare, 0, 0, &eroare);

        sqlite3_close(useri);

        if (write(client, msgrasp, 100) <= 0)
        {
            perror("[server]Eroare la write() catre client.\n");
            return;
        }
        else
        {
            printf("[server]Mesajul a fost trasmis cu succes.\n");
            exit(0);
        }
    }
    else
    {
        char msgrasp[100];
        bzero(msgrasp, 100);

        strcpy(msgrasp, "Comanda nu este una corecta");

        if (write(client, msgrasp, 100) <= 0)
        {
            perror("[server]Eroare la write() catre client.\n");
            return;
        }
    }
}

int main()
{
    if (sqlite3_open("biblioteca.db", &biblioteca) != SQLITE_OK)
    {
        perror("Nu se poate deschide aceasta baza de date.\n");
    }

    if (sqlite3_open("autori.db", &autori) != SQLITE_OK)
    {
        perror("Nu se poate deschide aceasta baza de date.\n");
    }

    if (sqlite3_open("istoric.db", &istoric) != SQLITE_OK)
    {
        perror("Nu se poate deschide aceasta baza de date.\n");
    }

    if (sqlite3_open("feedback.db", &feedback) != SQLITE_OK)
    {
        perror("Nu se poate deschide aceasta baza de date.\n");
    }

    if (sqlite3_open("useri_conectati.db", &useri) != SQLITE_OK)
    {
        perror("Nu se poate deschide aceasta baza de date.\n");
    }

    if (sqlite3_open("recomandari.db", &rec) != SQLITE_OK)
    {
        perror("Nu se poate deschide aceasta baza de date.\n");
    }

    if (sqlite3_open("genuri.db", &genuri) != SQLITE_OK)
    {
        perror("Nu se poate deschide aceasta baza de date.\n");
    }

    if (sqlite3_open("subgenuri.db", &subgenuri) != SQLITE_OK)
    {
        perror("Nu se poate deschide aceasta baza de date.\n");
    }

    char *tabela_rec = "CREATE TABLE IF NOT EXISTS recomandari("
                       "NR INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "Titlu TEXT NOT NULL,"
                       "Autor TEXT NOT NULL "
                       ");";

    char *tabela_autori = "CREATE TABLE IF NOT EXISTS autori("
                          "NR INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "Autor TEXT NOT NULL, "
                          "Gen1 TEXT NOT NULL, "
                          "Gen2 TEXT NOT NULL"
                          ");";

    char *tabela_genuri = "CREATE TABLE IF NOT EXISTS genuri("
                          "NR INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "Gen TEXT NOT NULL"
                          ");";

    char *tabela_subgenuri = "CREATE TABLE IF NOT EXISTS subgenuri("
                             "NR INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "Subgen TEXT NOT NULL"
                             ");";

    char *tabela_carti = "CREATE TABLE IF NOT EXISTS biblioteca("
                         "NR INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "Titlu TEXT NOT NULL,"
                         "Autor TEXT NOT NULL, "
                         "Gen TEXT NOT NULL, "
                         "Subgen TEXT NOT NULL, "
                         "Rating INTEGER NOT NULL, "
                         "ISBN INTEGER NOT NULL, "
                         "An INTEGER NOT NULL"
                         ");";

    char *tabela_useri = "CREATE TABLE IF NOT EXISTS useri("
                         "NR INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "Username TEXT NOT NULL"
                         ");";

    char *tabela_rating = "CREATE TABLE IF NOT EXISTS rating("
                          "Username TEXT NOT NULL,"
                          "Carte TEXT NOT NULL,"
                          "Nota TEXT NOT NULL"
                          ");";

    char *tabela_istoric = "CREATE TABLE IF NOT EXISTS istoric("
                           "Username TEXT NOT NULL,"
                           "Descarcare INTEGER NOT NULL,"
                           "Titlu TEXT,"
                           "Autor TEXT, "
                           "Gen TEXT, "
                           "Subgen TEXT "
                           ");";

    char *eroare = 0;
    char *eroare1 = 0;
    char *eroare2 = 0;
    char *eroare3 = 0;
    char *eroare4 = 0;
    char *eroare5 = 0;
    char *eroare6 = 0;
    char *eroare7 = 0;
    char *eroare8 = 0;
    char *eroare9 = 0;
    char *eroare10 = 0;
    char *eroare11 = 0;
    char *eroare12 = 0;
    char *eroare13 = 0;
    char *eroare14 = 0;
    char *eroare15 = 0;
    char *eroare16 = 0;
    char *eroare17 = 0;
    char *eroare18 = 0;
    char *eroare19 = 0;
    char *eroare20 = 0;
    char *eroare21 = 0;
    char *eroare22 = 0;
    char *eroare23 = 0;
    char *eroare24 = 0;

    char intrare1[1000], intrare2[1000], intrare3[1000], intrare4[1000];

    sqlite3_exec(istoric, tabela_istoric, 0, 0, &eroare7);
    sqlite3_exec(autori, tabela_autori, 0, 0, &eroare20);
    sqlite3_exec(useri, tabela_useri, 0, 0, &eroare);
    sqlite3_exec(genuri, tabela_genuri, 0, 0, &eroare12);
    sqlite3_exec(subgenuri, tabela_subgenuri, 0, 0, &eroare13);
    sqlite3_exec(rec, tabela_rec, 0, 0, &eroare8);
    sqlite3_exec(feedback, tabela_rating, 0, 0, &eroare6);
    sqlite3_exec(biblioteca, tabela_carti, 0, 0, &eroare1);

    strcpy(intrare1, "INSERT INTO biblioteca (Titlu, Autor, Gen, Subgen, Rating, ISBN, An) VALUES ('1984', 'GeorgeOrwell', 'Stiintifico-fantastic', 'Fictiune', 4, 9789734694365, 1949);");
    strcpy(intrare2, "INSERT INTO biblioteca (Titlu, Autor, Gen, Subgen, Rating, ISBN, An) VALUES ('SecretulMenajerei', 'FreidaMcFadden', 'Thriller', 'Horror', 4, 9786303051307, 2023);");
    strcpy(intrare3, "INSERT INTO biblioteca (Titlu, Autor, Gen, Subgen, Rating, ISBN, An) VALUES ('OFataDinBucati', 'KathleenGlasgow', 'Fictiune', 'Romance', 4, 9786069425183, 2017);");
    strcpy(intrare4, "INSERT INTO biblioteca (Titlu, Autor, Gen, Subgen, Rating, ISBN, An) VALUES ('OViataMarunta', 'HanyaYanagihara', 'Fictiune', 'Fictiune', 4, 9781447294832, 2016);");

    sqlite3_exec(biblioteca, intrare1, 0, 0, &eroare2);
    sqlite3_exec(biblioteca, intrare2, 0, 0, &eroare3);
    sqlite3_exec(biblioteca, intrare3, 0, 0, &eroare4);
    sqlite3_exec(biblioteca, intrare4, 0, 0, &eroare5);

    char intr1[1000], intr2[1000], intr3[1000];

    strcpy(intr1, "INSERT INTO recomandari (Titlu, Autor) VALUES ('PacientaTacuta', 'AlexMichaelides');");
    strcpy(intr2, "INSERT INTO recomandari (Titlu, Autor) VALUES ('Mesagerul', 'MarkusZusak')");
    strcpy(intr3, "INSERT INTO recomandari (Titlu, Autor) VALUES ('Verity', 'ColleenHoover');");

    sqlite3_exec(rec, intr1, 0, 0, &eroare9);
    sqlite3_exec(rec, intr2, 0, 0, &eroare10);
    sqlite3_exec(rec, intr3, 0, 0, &eroare11);

    char intr11[1000], intr12[1000], intr13[1000];

    strcpy(intr11, "INSERT INTO genuri (Gen) VALUES ('Stiintifico-fantastic');");
    strcpy(intr12, "INSERT INTO genuri (Gen) VALUES ('Thriller')");
    strcpy(intr13, "INSERT INTO genuri (Gen) VALUES ('Fictiune');");

    sqlite3_exec(genuri, intr11, 0, 0, &eroare14);
    sqlite3_exec(genuri, intr12, 0, 0, &eroare15);
    sqlite3_exec(genuri, intr13, 0, 0, &eroare16);

    char intr111[1000], intr112[1000], intr113[1000];

    strcpy(intr111, "INSERT INTO subgenuri (Subgen) VALUES ('Horror');");
    strcpy(intr112, "INSERT INTO subgenuri (Subgen) VALUES ('Romance')");
    strcpy(intr113, "INSERT INTO subgenuri (Subgen) VALUES ('Fictiune');");

    sqlite3_exec(subgenuri, intr111, 0, 0, &eroare17);
    sqlite3_exec(subgenuri, intr112, 0, 0, &eroare18);
    sqlite3_exec(subgenuri, intr113, 0, 0, &eroare19);

    char intr1111[1000], intr1112[1000], intr1113[1000], intr1114[1000];

    strcpy(intr1111, "INSERT INTO autori (Autor,Gen1,Gen2) VALUES ('GeorgeOrwell', 'Stiintifico-fantastic', 'Fictiune');");
    strcpy(intr1112, "INSERT INTO autori (Autor,Gen1,Gen2) VALUES ('FreidaMcFadden', 'Thriller', 'Horror')");
    strcpy(intr1113, "INSERT INTO autori (Autor,Gen1,Gen2) VALUES ('HanyaYanagihara', 'Fictiune', 'Fictiune');");
    strcpy(intr1114, "INSERT INTO autori (Autor,Gen1,Gen2) VALUES ('KathleenGlasgow', 'Fictiune', 'Romance');");

    sqlite3_exec(autori, intr1111, 0, 0, &eroare21);
    sqlite3_exec(autori, intr1112, 0, 0, &eroare22);
    sqlite3_exec(autori, intr1113, 0, 0, &eroare23);
    sqlite3_exec(autori, intr1114, 0, 0, &eroare24);

    sqlite3_close(biblioteca);
    sqlite3_close(genuri);
    sqlite3_close(autori);
    sqlite3_close(subgenuri);
    sqlite3_close(feedback);
    sqlite3_close(useri);
    sqlite3_close(istoric);

    struct sockaddr_in server;
    struct sockaddr_in from;
    int sd;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        handle_error("[server]Eroare la socket().\n", 11);
    }

    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        handle_error("[server]Eroare la bind().\n", 12);
    }

    if (listen(sd, 1) == -1)
    {
        handle_error("[server]Eroare la listen().\n", 13);
    }

    int *login_val = mmap(NULL,
                          sizeof(int),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS,
                          -1,
                          0);

    if (login_val == MAP_FAILED)
    {
        handle_error("Eroare la mapare!", 19);
    }

    *login_val = 0;

    int *gen_val = mmap(NULL,
                        sizeof(int),
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS,
                        -1,
                        0);

    if (gen_val == MAP_FAILED)
    {
        handle_error("Eroare la mapare!", 19);
    }

    *gen_val = 0;

    int *descarcare_val = mmap(NULL,
                               sizeof(int),
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS,
                               -1,
                               0);

    if (descarcare_val == MAP_FAILED)
    {
        handle_error("Eroare la mapare!", 19);
    }

    *descarcare_val = 0;

    while (1)
    {
        int client;
        int length = sizeof(from);

        client = accept(sd, (struct sockaddr *)&from, &length);
        if (client < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        int pid;
        if ((pid = fork()) == -1)
        {
            close(client);
            continue;
        }
        else if (pid > 0)
        {
            close(client);
            while (waitpid(-1, NULL, WNOHANG))
                ;
            continue;
        }
        else if (pid == 0)
        {
            close(sd);
            while (1)
            {
                operatii(client, login_val, gen_val, descarcare_val);
            }
            exit(0);
        }
        close(client);
        exit(0);
    }
    if (munmap(gen_val, sizeof(int)) == -1)
    {
        handle_error("Eroare la inchiderea maparii!", 39);
    }
    if (munmap(descarcare_val, sizeof(int)) == -1)
    {
        handle_error("Eroare la inchiderea maparii!", 39);
    }

    if (munmap(login_val, sizeof(int)) == -1)
    {
        handle_error("Eroare la inchiderea maparii!", 40);
    }
}