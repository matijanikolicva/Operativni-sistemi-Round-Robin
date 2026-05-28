#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================
// Jednostruko spregnuta lista - definicija
// =============================================
typedef struct Proces {
    int id;
    char naziv[50];
    int vremePristizanja;
    int vremeIzvrsavanja;
    int preostaloVreme;
    int vremeZavrsavanja;
    int vremeOkretanja;
    int vremeKasnjenja;
    struct Proces* sledeci;
} Proces;

// =============================================
// Rad sa listom
// =============================================
Proces* kreirajProces(int id, const char* naziv, int pristizanje, int izvrsavanje) {
    Proces* novi = (Proces*)malloc(sizeof(Proces));
    if (novi == NULL) {
        printf("Greska: Nije moguce alocirati memoriju!\n");
        exit(1);
    }
    novi->id = id;
    strncpy(novi->naziv, naziv, 49);
    novi->naziv[49] = '\0';
    novi->vremePristizanja = pristizanje;
    novi->vremeIzvrsavanja = izvrsavanje;
    novi->preostaloVreme = izvrsavanje;
    novi->vremeZavrsavanja = 0;
    novi->vremeOkretanja = 0;
    novi->vremeKasnjenja = 0;
    novi->sledeci = NULL;
    return novi;
}

void dodajNaKraj(Proces** glava, Proces* novi) {
    if (*glava == NULL) {
        *glava = novi;
        return;
    }
    Proces* trenutni = *glava;
    while (trenutni->sledeci != NULL) {
        trenutni = trenutni->sledeci;
    }
    trenutni->sledeci = novi;
}

void prikaziListu(Proces* glava) {
    Proces* trenutni = glava;
    printf("\n%-5s %-15s %-15s %-15s %-15s %-15s %-15s\n",
           "ID", "Naziv", "Pristizanje", "Izvrsavanje", "Zavrsavanje", "Okretanje", "Kasnjenje");
    printf("%-5s %-15s %-15s %-15s %-15s %-15s %-15s\n",
           "---", "---------------", "---------------", "---------------",
           "---------------", "---------------", "---------------");
    while (trenutni != NULL) {
        printf("%-5d %-15s %-15d %-15d %-15d %-15d %-15d\n",
               trenutni->id,
               trenutni->naziv,
               trenutni->vremePristizanja,
               trenutni->vremeIzvrsavanja,
               trenutni->vremeZavrsavanja,
               trenutni->vremeOkretanja,
               trenutni->vremeKasnjenja);
        trenutni = trenutni->sledeci;
    }
}

void oslobodiListu(Proces** glava) {
    Proces* trenutni = *glava;
    while (trenutni != NULL) {
        Proces* sledeci = trenutni->sledeci;
        free(trenutni);
        trenutni = sledeci;
    }
    *glava = NULL;
}

// =============================================
// Snimanje rezultata u datoteku
// =============================================
void snimajUDatoteku(Proces* glava, int kvant, float prosecnoOkretanje, float prosecnoKasnjenje, const char* imeFajla) {
    FILE* fajl = fopen(imeFajla, "w");
    if (fajl == NULL) {
        printf("Greska: Nije moguce otvoriti datoteku za pisanje!\n");
        return;
    }

    fprintf(fajl, "==============================================\n");
    fprintf(fajl, "   REZULTATI ROUND ROBIN ALGORITMA\n");
    fprintf(fajl, "==============================================\n");
    fprintf(fajl, "Vremenski kvant: %d ms\n\n", kvant);
    fprintf(fajl, "%-5s %-15s %-12s %-12s %-12s %-12s %-12s\n",
            "ID", "Naziv", "Pristizanje", "Izvrsavanje", "Zavrsavanje", "Okretanje", "Kasnjenje");
    fprintf(fajl, "%-5s %-15s %-12s %-12s %-12s %-12s %-12s\n",
            "---", "---------------", "------------", "------------",
            "------------", "------------", "------------");

    Proces* trenutni = glava;
    while (trenutni != NULL) {
        fprintf(fajl, "%-5d %-15s %-12d %-12d %-12d %-12d %-12d\n",
                trenutni->id,
                trenutni->naziv,
                trenutni->vremePristizanja,
                trenutni->vremeIzvrsavanja,
                trenutni->vremeZavrsavanja,
                trenutni->vremeOkretanja,
                trenutni->vremeKasnjenja);
        trenutni = trenutni->sledeci;
    }

    fprintf(fajl, "\n----------------------------------------------\n");
    fprintf(fajl, "Prosecno vreme okretanja : %.2f ms\n", prosecnoOkretanje);
    fprintf(fajl, "Prosecno vreme kasnjenja : %.2f ms\n", prosecnoKasnjenje);
    fprintf(fajl, "==============================================\n");

    fclose(fajl);
    printf("\nRezultati su sacuvani u datoteku: %s\n", imeFajla);
}

// =============================================
// Citanje procesa iz datoteke
// =============================================
Proces* ucitajIzDatoteke(const char* imeFajla, int* brojProcesa) {
    FILE* fajl = fopen(imeFajla, "r");
    if (fajl == NULL) {
        printf("Greska: Datoteka '%s' nije pronadjena!\n", imeFajla);
        return NULL;
    }

    Proces* glava = NULL;
    *brojProcesa = 0;
    int id, pristizanje, izvrsavanje;
    char naziv[50];

    // Format linije: id naziv pristizanje izvrsavanje
    while (fscanf(fajl, "%d %s %d %d", &id, naziv, &pristizanje, &izvrsavanje) == 4) {
        // Uslovno grananje - validacija podataka
        if (izvrsavanje <= 0) {
            printf("Upozorenje: Proces %d ima neispravno vreme izvrsavanja, preskacemo.\n", id);
            continue;
        }
        if (pristizanje < 0) {
            printf("Upozorenje: Proces %d ima negativno vreme pristizanja, postavljamo na 0.\n", id);
            pristizanje = 0;
        }
        Proces* novi = kreirajProces(id, naziv, pristizanje, izvrsavanje);
        dodajNaKraj(&glava, novi);
        (*brojProcesa)++;
    }

    fclose(fajl);
    printf("Uspesno ucitano %d procesa iz datoteke '%s'.\n", *brojProcesa, imeFajla);
    return glava;
}

// =============================================
// Round Robin algoritam
// =============================================
void roundRobin(Proces* glava, int brojProcesa, int kvant) {
    if (glava == NULL || brojProcesa == 0) {
        printf("Greska: Nema procesa za rasporedivanje!\n");
        return;
    }

    // Kopiramo preostalo vreme (resetujemo za simulaciju)
    Proces* tmp = glava;
    while (tmp != NULL) {
        tmp->preostaloVreme = tmp->vremeIzvrsavanja;
        tmp = tmp->sledeci;
    }

    int* preostalo = (int*)malloc(brojProcesa * sizeof(int));
    int* indeksi = (int*)malloc(brojProcesa * sizeof(int));
    if (preostalo == NULL || indeksi == NULL) {
        printf("Greska pri alokaciji memorije!\n");
        return;
    }

    // Punjenje pomocnih nizova iz liste
    Proces* trenutni = glava;
    for (int i = 0; i < brojProcesa; i++) {
        preostalo[i] = trenutni->vremeIzvrsavanja;
        indeksi[i] = i;
        trenutni = trenutni->sledeci;
    }

    printf("\n==============================================\n");
    printf("   ROUND ROBIN SIMULACIJA (kvant = %d ms)\n", kvant);
    printf("==============================================\n");
    printf("%-10s %-15s %-10s %-10s\n", "Vreme", "Proces", "Pocetak", "Kraj");
    printf("%-10s %-15s %-10s %-10s\n", "----------", "---------------", "----------", "----------");

    int vremeKurent = 0;
    int zavrseni = 0;

    // Ciklus - Round Robin petlja
    while (zavrseni < brojProcesa) {
        int nekiIzvrsio = 0;

        // Prolazimo kroz sve procese kružno
        Proces* proc = glava;
        for (int i = 0; i < brojProcesa; i++) {
            // Uslovno grananje - proveravamo da li proces ima preostalo vreme i da li je stigao
            if (preostalo[i] > 0 && proc->vremePristizanja <= vremeKurent) {
                nekiIzvrsio = 1;
                int pocetak = vremeKurent;

                // Uslovno grananje - da li proces može završiti u ovom kvantu
                if (preostalo[i] <= kvant) {
                    vremeKurent += preostalo[i];
                    preostalo[i] = 0;
                    zavrseni++;
                    proc->vremeZavrsavanja = vremeKurent;
                    proc->vremeOkretanja = vremeKurent - proc->vremePristizanja;
                    proc->vremeKasnjenja = proc->vremeOkretanja - proc->vremeIzvrsavanja;
                    printf("%-10d %-15s %-10d %-10d  [ZAVRSEN]\n",
                           pocetak, proc->naziv, pocetak, vremeKurent);
                } else {
                    vremeKurent += kvant;
                    preostalo[i] -= kvant;
                    printf("%-10d %-15s %-10d %-10d\n",
                           pocetak, proc->naziv, pocetak, vremeKurent);
                }
            }
            proc = proc->sledeci;
        }

        // Ako nijedan proces nije izvrsio (cekamo pristizanje)
        if (!nekiIzvrsio) {
            vremeKurent++;
        }
    }

    // Racunanje proseka
    float ukupnoOkretanje = 0, ukupnoKasnjenje = 0;
    Proces* p = glava;
    while (p != NULL) {
        ukupnoOkretanje += p->vremeOkretanja;
        ukupnoKasnjenje += p->vremeKasnjenja;
        p = p->sledeci;
    }

    float prosecnoOkretanje = ukupnoOkretanje / brojProcesa;
    float prosecnoKasnjenje = ukupnoKasnjenje / brojProcesa;

    printf("\n==============================================\n");
    printf("Prosecno vreme okretanja : %.2f ms\n", prosecnoOkretanje);
    printf("Prosecno vreme kasnjenja : %.2f ms\n", prosecnoKasnjenje);
    printf("==============================================\n");

    // Prikaz rezultata iz liste
    prikaziListu(glava);

    // Snimanje u datoteku
    snimajUDatoteku(glava, kvant, prosecnoOkretanje, prosecnoKasnjenje, "rezultati_rr.txt");

    free(preostalo);
    free(indeksi);
}

// =============================================
// Kreiranje podrazumevane ulazne datoteke
// =============================================
void kreirajPodrazumevanuDatoteku(const char* imeFajla) {
    FILE* fajl = fopen(imeFajla, "w");
    if (fajl == NULL) {
        printf("Greska: Nije moguce kreirati datoteku!\n");
        return;
    }
    // Format: id naziv pristizanje izvrsavanje
    fprintf(fajl, "1 P1-Preglednik 0 8\n");
    fprintf(fajl, "2 P2-Kompajler  2 5\n");
    fprintf(fajl, "3 P3-Stampa     4 3\n");
    fprintf(fajl, "4 P4-Antivirus  6 7\n");
    fprintf(fajl, "5 P5-Azuriranje 8 4\n");
    fclose(fajl);
    printf("Kreirana podrazumevana datoteka: %s\n", imeFajla);
}

// =============================================
// Meni
// =============================================
void prikaziMeni() {
    printf("\n==============================================\n");
    printf("   ROUND ROBIN ALGORITAM DODELE PROCESORA\n");
    printf("==============================================\n");
    printf("  1. Unos procesa rucno\n");
    printf("  2. Ucitavanje procesa iz datoteke\n");
    printf("  3. Kreiranje primerne datoteke sa procesima\n");
    printf("  4. Izlaz\n");
    printf("==============================================\n");
    printf("Izaberite opciju: ");
}

// =============================================
// Rucni unos procesa
// =============================================
Proces* rucniUnos(int* brojProcesa) {
    printf("\nUnesite broj procesa: ");
    scanf("%d", brojProcesa);

    // Uslovno grananje - validacija broja procesa
    if (*brojProcesa <= 0 || *brojProcesa > 20) {
        printf("Neispravno: broj procesa mora biti izmedju 1 i 20.\n");
        *brojProcesa = 0;
        return NULL;
    }

    Proces* glava = NULL;
    for (int i = 0; i < *brojProcesa; i++) {
        char naziv[50];
        int pristizanje, izvrsavanje;
        printf("\n--- Proces %d ---\n", i + 1);
        printf("Naziv procesa: ");
        scanf("%s", naziv);
        printf("Vreme pristizanja (ms): ");
        scanf("%d", &pristizanje);
        printf("Vreme izvrsavanja (ms): ");
        scanf("%d", &izvrsavanje);

        // Uslovno grananje - validacija
        if (izvrsavanje <= 0) {
            printf("Upozorenje: Vreme izvrsavanja mora biti pozitivno. Postavljamo na 1.\n");
            izvrsavanje = 1;
        }
        if (pristizanje < 0) {
            printf("Upozorenje: Vreme pristizanja ne moze biti negativno. Postavljamo na 0.\n");
            pristizanje = 0;
        }

        Proces* novi = kreirajProces(i + 1, naziv, pristizanje, izvrsavanje);
        dodajNaKraj(&glava, novi);
    }
    return glava;
}

// =============================================
// Glavni program
// =============================================
int main() {
    Proces* glava = NULL;
    int brojProcesa = 0;
    int kvant = 0;
    int opcija;
    char imeFajla[100];

    printf("\n============================================================\n");
    printf("  Simulacija Round Robin algoritma dodele procesora\n");
    printf("  Predmet: Operativni sistemi\n");
    printf("  Tehnicki fakultet 'Mihajlo Pupin', Zrenjanin\n");
    printf("============================================================\n");

    // Ciklus - glavni meni
    do {
        prikaziMeni();
        scanf("%d", &opcija);

        // Uslovno grananje - obrada opcija menija
        if (opcija == 1) {
            // Rucni unos
            if (glava != NULL) {
                oslobodiListu(&glava);
                brojProcesa = 0;
            }
            glava = rucniUnos(&brojProcesa);
            if (glava == NULL) continue;

            printf("\nUnesite vremenski kvant (ms): ");
            scanf("%d", &kvant);

            // Uslovno grananje - validacija kvanta
            if (kvant <= 0) {
                printf("Upozorenje: Kvant mora biti pozitivan. Postavljamo na 2.\n");
                kvant = 2;
            }
            roundRobin(glava, brojProcesa, kvant);

        } else if (opcija == 2) {
            // Ucitavanje iz datoteke
            printf("Unesite naziv datoteke: ");
            scanf("%s", imeFajla);

            if (glava != NULL) {
                oslobodiListu(&glava);
                brojProcesa = 0;
            }
            glava = ucitajIzDatoteke(imeFajla, &brojProcesa);
            if (glava == NULL) continue;

            printf("Unesite vremenski kvant (ms): ");
            scanf("%d", &kvant);

            if (kvant <= 0) {
                printf("Upozorenje: Kvant mora biti pozitivan. Postavljamo na 2.\n");
                kvant = 2;
            }
            roundRobin(glava, brojProcesa, kvant);

        } else if (opcija == 3) {
            // Kreiranje primerne datoteke
            printf("Unesite naziv datoteke (npr. procesi.txt): ");
            scanf("%s", imeFajla);
            kreirajPodrazumevanuDatoteku(imeFajla);

        } else if (opcija == 4) {
            printf("\nHvala na koristenju programa. Dovidjenja!\n\n");
        } else {
            printf("Nepoznata opcija! Pokusajte ponovo.\n");
        }

    } while (opcija != 4);

    // Oslobadjanje memorije (citanje iz liste + ciscenje)
    if (glava != NULL) {
        oslobodiListu(&glava);
    }

    return 0;
}
