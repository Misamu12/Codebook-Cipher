
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>


// ---------------- CONFIGURATION ----------------

#define FICHIER "repertoire.txt"
#define TAILLE_MOT 100
#define TAILLE_CODE 20
#define TAILLE_MESSAGE 4096
#define TAILLE_SORTIE 8192

// ---------------- STRUCTURES ----------------

// Une entree contient un mot et son code associe.
typedef struct {
    char mot[TAILLE_MOT];
    char code[TAILLE_CODE];
} Entree;

// Le repertoire utilise un tableau dynamique.
typedef struct {
    Entree *entrees;
    size_t nombre;
    size_t capacite;
} Repertoire;

// ---------------- PROTOTYPES ----------------

void initialiserRepertoire(Repertoire *r);
void libererRepertoire(Repertoire *r);

int rechercherMot(const Repertoire *r, const char *mot);
int rechercherCode(const Repertoire *r, const char *code);

int ajouterMot(Repertoire *r, const char *mot);
int genererCode(const Repertoire *r, char *code, size_t taille);

int sauvegarderRepertoire(const Repertoire *r);
int chargerRepertoire(Repertoire *r);

void afficherRepertoire(const Repertoire *r);
void rechercherEtAfficher(const Repertoire *r);

void normaliserMot(char *mot);
int estMotValide(const char *mot);

int chiffrerMessage(Repertoire *r);
int dechiffrerMessage(const Repertoire *r);

int ajouterAvecConfirmation(Repertoire *r, const char *mot);

void menu(void);

// ---------------- GESTION DU REPERTOIRE ----------------

// Initialise le tableau dynamique.
void initialiserRepertoire(Repertoire *r) {
    r->nombre = 0;
    r->capacite = 10;

    r->entrees = malloc(r->capacite * sizeof(Entree));

    if (r->entrees == NULL) {
        printf("Erreur : allocation memoire impossible.\n");
        exit(EXIT_FAILURE);
    }
}

// Libere la memoire utilisee.
void libererRepertoire(Repertoire *r) {
    free(r->entrees);

    r->entrees = NULL;
    r->nombre = 0;
    r->capacite = 0;
}

// Convertit un mot en majuscules.
// Cette version prend en charge les lettres ASCII.
void normaliserMot(char *mot) {
    for (size_t i = 0; mot[i] != '\0'; i++) {
        mot[i] = (char)toupper((unsigned char)mot[i]);
    }
}

// Verifie qu'un mot contient uniquement des lettres ASCII.
int estMotValide(const char *mot) {
    if (mot == NULL || mot[0] == '\0') {
        return 0;
    }

    for (size_t i = 0; mot[i] != '\0'; i++) {
        if (!isalpha((unsigned char)mot[i]) ||
            (unsigned char)mot[i] > 127) {
            return 0;
        }
    }

    return 1;
}

// Recherche un mot dans le repertoire.
// Retourne son indice ou -1 si absent.
int rechercherMot(const Repertoire *r, const char *mot) {
    for (size_t i = 0; i < r->nombre; i++) {
        if (strcmp(r->entrees[i].mot, mot) == 0) {
            return (int)i;
        }
    }

    return -1;
}

// Recherche un code dans le repertoire.
int rechercherCode(const Repertoire *r, const char *code) {
    for (size_t i = 0; i < r->nombre; i++) {
        if (strcmp(r->entrees[i].code, code) == 0) {
            return (int)i;
        }
    }

    return -1;
}

// ---------------- GENERATION DES CODES ----------------

// Genere le premier code numerique disponible a partir de 1001.
int genererCode(const Repertoire *r, char *code, size_t taille) {
    unsigned long long candidat = 1001;

    while (candidat <= ULLONG_MAX) {
        snprintf(code, taille, "%llu", candidat);

        // Verifie que le code n'existe pas deja.
        if (rechercherCode(r, code) == -1) {
            return 1;
        }

        if (candidat == ULLONG_MAX) {
            break;
        }

        candidat++;
    }

    return 0;
}

// ---------------- AJOUT D'UN MOT ----------------

// Ajoute un nouveau mot avec un code genere automatiquement.
int ajouterMot(Repertoire *r, const char *mot) {

    // Verifie si le mot existe deja.
    if (rechercherMot(r, mot) != -1) {
        printf("Ce mot existe deja dans le repertoire.\n");
        return 0;
    }

    char nouveauCode[TAILLE_CODE];

    // Genere un code unique.
    if (!genererCode(r, nouveauCode, sizeof(nouveauCode))) {
        printf("Impossible de generer un nouveau code.\n");
        return 0;
    }

    // Agrandit le tableau si necessaire.
    if (r->nombre == r->capacite) {

        size_t nouvelleCapacite = r->capacite * 2;

        Entree *temp = realloc(
            r->entrees,
            nouvelleCapacite * sizeof(Entree)
        );

        if (temp == NULL) {
            printf("Erreur d'allocation memoire.\n");
            return 0;
        }

        r->entrees = temp;
        r->capacite = nouvelleCapacite;
    }

    // Ajoute la nouvelle entree.
    snprintf(
        r->entrees[r->nombre].mot,
        TAILLE_MOT,
        "%s",
        mot
    );

    snprintf(
        r->entrees[r->nombre].code,
        TAILLE_CODE,
        "%s",
        nouveauCode
    );

    r->nombre++;

    printf("\nNouveau mot ajoute avec succes !\n");
    printf("Mot  : %s\n", mot);
    printf("Code : %s\n", nouveauCode);

    return 1;
}

// Demande confirmation avant l'ajout d'un mot inconnu.
int ajouterAvecConfirmation(Repertoire *r, const char *mot) {

    char choix[20];

    printf("\nMot inconnu : %s\n", mot);
    printf("Voulez-vous l'ajouter au repertoire ? (O/N) : ");

    if (fgets(choix, sizeof(choix), stdin) == NULL) {
        return 0;
    }

    if (toupper((unsigned char)choix[0]) == 'O') {

        if (ajouterMot(r, mot)) {
            sauvegarderRepertoire(r);
            return 1;
        }
    }

    printf("Ajout refuse.\n");

    return 0;
}

// ---------------- SAUVEGARDE ----------------

// Sauvegarde les entrees dans un fichier texte.
// Format : MOT;CODE
int sauvegarderRepertoire(const Repertoire *r) {

    FILE *fichier = fopen(FICHIER, "w");

    if (fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return 0;
    }

    for (size_t i = 0; i < r->nombre; i++) {

        fprintf(
            fichier,
            "%s;%s\n",
            r->entrees[i].mot,
            r->entrees[i].code
        );
    }

    if (fclose(fichier) != 0) {
        printf("Erreur lors de la fermeture du fichier.\n");
        return 0;
    }

    return 1;
}

// ---------------- CHARGEMENT ----------------

// Charge le repertoire au lancement du programme.
int chargerRepertoire(Repertoire *r) {

    FILE *fichier = fopen(FICHIER, "r");

    // Le fichier n'existe pas encore.
    if (fichier == NULL) {
        printf("Aucun repertoire existant. Creation d'un nouveau.\n");
        return 1;
    }

    char ligne[256];

    while (fgets(ligne, sizeof(ligne), fichier) != NULL) {

        // Supprime le retour a la ligne.
        ligne[strcspn(ligne, "\r\n")] = '\0';

        char *separateur = strchr(ligne, ';');

        if (separateur == NULL) {
            continue;
        }

        *separateur = '\0';

        char *mot = ligne;
        char *code = separateur + 1;

        normaliserMot(mot);

        // Ignore les entrees invalides.
        if (!estMotValide(mot) ||
            strlen(mot) >= TAILLE_MOT ||
            strlen(code) == 0 ||
            strlen(code) >= TAILLE_CODE) {
            continue;
        }

        int codeValide = 1;

        for (size_t i = 0; code[i] != '\0'; i++) {
            if (!isdigit((unsigned char)code[i])) {
                codeValide = 0;
                break;
            }
        }

        if (!codeValide ||
            rechercherMot(r, mot) != -1 ||
            rechercherCode(r, code) != -1) {
            continue;
        }

        // Agrandit le tableau si necessaire.
        if (r->nombre == r->capacite) {

            size_t nouvelleCapacite = r->capacite * 2;

            Entree *temp = realloc(
                r->entrees,
                nouvelleCapacite * sizeof(Entree)
            );

            if (temp == NULL) {
                printf("Erreur de memoire pendant le chargement.\n");
                fclose(fichier);
                return 0;
            }

            r->entrees = temp;
            r->capacite = nouvelleCapacite;
        }

        snprintf(r->entrees[r->nombre].mot, TAILLE_MOT, "%s", mot);
        snprintf(r->entrees[r->nombre].code, TAILLE_CODE, "%s", code);

        r->nombre++;
    }

    fclose(fichier);

    printf("Repertoire charge : %zu entree(s).\n", r->nombre);

    return 1;
}

// ---------------- AFFICHAGE ----------------

void afficherRepertoire(const Repertoire *r) {

    printf("\n========== REPERTOIRE ==========\n");

    if (r->nombre == 0) {
        printf("Le repertoire est vide.\n");
    }

    printf("%-25s | %-15s\n", "MOT", "CODE");
    printf("------------------------------------------\n");

    for (size_t i = 0; i < r->nombre; i++) {

        printf(
            "%-25s | %-15s\n",
            r->entrees[i].mot,
            r->entrees[i].code
        );
    }

    printf("------------------------------------------\n");
    printf("Nombre total : %zu mot(s)\n", r->nombre);
}

// Recherche un mot et affiche son code.
void rechercherEtAfficher(const Repertoire *r) {

    char mot[TAILLE_MOT];

    printf("Entrez le mot a rechercher : ");

    if (fgets(mot, sizeof(mot), stdin) == NULL) {
        return;
    }

    mot[strcspn(mot, "\r\n")] = '\0';

    normaliserMot(mot);

    int indice = rechercherMot(r, mot);

    if (indice != -1) {

        printf("Mot trouve : %s\n", mot);
        printf("Code associe : %s\n", r->entrees[indice].code);

    } else {

        printf("Mot absent du repertoire.\n");
    }
}

// ---------------- OUTILS DE SORTIE ----------------

// Ajoute une chaine dans le resultat sans depasser sa capacite.
int ajouterTexte(
    char *destination,
    size_t capacite,
    size_t *position,
    const char *texte
) {
    size_t longueur = strlen(texte);

    if (longueur >= capacite - *position) {
        return 0;
    }

    memcpy(destination + *position, texte, longueur);

    *position += longueur;
    destination[*position] = '\0';

    return 1;
}

// ---------------- CHIFFREMENT ----------------

// Remplace chaque mot par son code correspondant.
// Les espaces et signes de ponctuation sont conserves.
int chiffrerMessage(Repertoire *r) {

    char message[TAILLE_MESSAGE];
    char resultat[TAILLE_SORTIE] = "";

    size_t position = 0;

    printf("\nEntrez le message a chiffrer :\n");

    if (fgets(message, sizeof(message), stdin) == NULL) {
        return 0;
    }

    message[strcspn(message, "\r\n")] = '\0';

    size_t i = 0;

    while (message[i] != '\0') {

        // Si le caractere est une lettre, on extrait le mot.
        if (isalpha((unsigned char)message[i]) &&
            (unsigned char)message[i] <= 127) {

            char mot[TAILLE_MOT];
            size_t longueur = 0;

            // Extraction du mot.
            while (isalpha((unsigned char)message[i]) &&
                   (unsigned char)message[i] <= 127) {

                if (longueur >= TAILLE_MOT - 1) {
                    printf("Erreur : mot trop long.\n");
                    return 0;
                }

                mot[longueur++] = (char)toupper(
                    (unsigned char)message[i]
                );

                i++;
            }

            mot[longueur] = '\0';

            // Recherche du mot.
            int indice = rechercherMot(r, mot);

            if (indice == -1) {

                // Proposition d'ajout.
                if (!ajouterAvecConfirmation(r, mot)) {

                    printf("Chiffrement annule : mot inconnu.\n");
                    return 0;
                }

                indice = rechercherMot(r, mot);
            }

            // Remplacement par le code.
            if (!ajouterTexte(
                    resultat,
                    sizeof(resultat),
                    &position,
                    r->entrees[indice].code)) {

                printf("Message chiffre trop long.\n");
                return 0;
            }

        } else {

            // Conserve les espaces et la ponctuation.
            char caractere[2] = {message[i], '\0'};

            if (!ajouterTexte(
                    resultat,
                    sizeof(resultat),
                    &position,
                    caractere)) {

                printf("Message chiffre trop long.\n");
                return 0;
            }

            i++;
        }
    }

    // Sauvegarde apres le chiffrement.
    if (!sauvegarderRepertoire(r)) {
        printf("Attention : sauvegarde impossible.\n");
        return 0;
    }

    printf("\n========== RESULTAT ==========\n");
    printf("Message chiffre :\n%s\n", resultat);

    return 1;
}

// ---------------- DECHIFFREMENT ----------------

// Remplace chaque suite de chiffres par le mot correspondant.
int dechiffrerMessage(const Repertoire *r) {

    char message[TAILLE_MESSAGE];
    char resultat[TAILLE_SORTIE] = "";

    size_t position = 0;

    printf("\nEntrez le message chiffre :\n");

    if (fgets(message, sizeof(message), stdin) == NULL) {
        return 0;
    }

    message[strcspn(message, "\r\n")] = '\0';

    size_t i = 0;

    while (message[i] != '\0') {

        // Detection d'un code numerique.
        if (isdigit((unsigned char)message[i])) {

            char code[TAILLE_CODE];
            size_t longueur = 0;

            while (isdigit((unsigned char)message[i])) {

                if (longueur >= TAILLE_CODE - 1) {
                    printf("Erreur : code trop long.\n");
                    return 0;
                }

                code[longueur++] = message[i];
                i++;
            }

            code[longueur] = '\0';

            // Recherche du code.
            int indice = rechercherCode(r, code);

            if (indice == -1) {

                printf(
                    "Erreur : code %s introuvable dans le repertoire.\n",
                    code
                );

                printf("Dechiffrement impossible.\n");

                return 0;
            }

            // Remplacement du code par le mot.
            if (!ajouterTexte(
                    resultat,
                    sizeof(resultat),
                    &position,
                    r->entrees[indice].mot)) {

                printf("Message dechiffre trop long.\n");
                return 0;
            }

        } else {

            // Conserve les separateurs.
            char caractere[2] = {message[i], '\0'};

            if (!ajouterTexte(
                    resultat,
                    sizeof(resultat),
                    &position,
                    caractere)) {

                printf("Message dechiffre trop long.\n");
                return 0;
            }

            i++;
        }
    }

    printf("\n========== RESULTAT ==========\n");
    printf("Message original :\n%s\n", resultat);

    return 1;
}

// ---------------- MENU PRINCIPAL ----------------

void menu(void) {

    printf("\n");
    printf("========================================\n");
    printf("       CHIFFREMENT PAR REPERTOIRE       \n");
    printf("========================================\n");

    printf("1. Chiffrer un message\n");
    printf("2. Dechiffrer un message\n");
    printf("3. Afficher le repertoire\n");
    printf("4. Rechercher un mot\n");
    printf("5. Quitter\n");

    printf("----------------------------------------\n");
    printf("Votre choix : ");
}

// ---------------- PROGRAMME PRINCIPAL ----------------

int main(void) {

    Repertoire repertoire;

    // Initialisation du repertoire.
    initialiserRepertoire(&repertoire);

    // Chargement des donnees existantes.
    if (!chargerRepertoire(&repertoire)) {
        libererRepertoire(&repertoire);
        return EXIT_FAILURE;
    }

    int choix;

    do {

        menu();

        char saisie[32];

        if (fgets(saisie, sizeof(saisie), stdin) == NULL) {
            break;
        }

        char *fin;
        long valeur = strtol(saisie, &fin, 10);

        // Verification de la saisie du menu.
        while (*fin == ' ' || *fin == '\t' ||
               *fin == '\r' || *fin == '\n') {
            fin++;
        }

        if (fin == saisie || *fin != '\0' ||
            valeur < 1 || valeur > 5) {

            printf("Choix invalide. Reessayez.\n");
            continue;
        }

        choix = (int)valeur;

        switch (choix) {

            case 1:
                chiffrerMessage(&repertoire);
                break;

            case 2:
                dechiffrerMessage(&repertoire);
                break;

            case 3:
                afficherRepertoire(&repertoire);
                break;

            case 4:
                rechercherEtAfficher(&repertoire);
                break;

            case 5:
                printf("Fermeture du programme...\n");
                break;
        }

    } while (choix != 5);

    // Sauvegarde finale.
    sauvegarderRepertoire(&repertoire);

    // Liberation de la memoire.
    libererRepertoire(&repertoire);

    printf("Au revoir !\n");

    return EXIT_SUCCESS;
}
