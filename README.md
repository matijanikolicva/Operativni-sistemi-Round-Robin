# Operativni-sistemi-Round-Robin
# Seminarski rad - Round Robin algoritam

Ovaj repozitorijum sadrži seminarski rad iz predmeta Operativni sistemi.

## Tema

Algoritmi dodele procesora procesima - Round Robin algoritam.

## Sadržaj repozitorijuma

- `Seminarski_Round_Robin.docx` - dokumentacija seminarskog rada
- `round_robin.c` - programski kod u C jeziku
- `procesi.txt` - primer ulazne datoteke sa procesima
- `rezultati_rr.txt` - primer izlazne datoteke sa rezultatima simulacije

## Opis programa

Program simulira Round Robin algoritam raspoređivanja procesa. Procesi se učitavaju iz datoteke ili se unose ručno, zatim se smeštaju u jednostruko spregnutu listu. Korisnik unosi vremenski kvant, a program kružno raspoređuje procese, računa vreme završavanja, vreme okretanja i vreme kašnjenja.

## Pokretanje programa

Program je pisan u programskom jeziku C.

Primer kompajliranja:

```bash
gcc round_robin.c -o round_robin
