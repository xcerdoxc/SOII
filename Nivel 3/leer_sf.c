#include "ficheros_basico.h"

int main(int argc, char **argv)
{

       //Declaramos un array que sera el contenerdo de bread
       struct superbloque superBloque;

       struct inodo inodos[BLOCKSIZE / INODOSIZE];

       //Montamos el fichero
       bmount(argv[1]);

       bread(posSB, &superBloque);
       printf("DATOS DEL SUPERBLOQUE: \n");
       printf("posPrimerBloqueMB = %d \n", superBloque.posPrimerBloqueMB);
       printf("posUltimoBloqueMB = %d\n", superBloque.posUltimoBloqueMB);
       printf("posPrimerBloqueAI = %d\n", superBloque.posPrimerBloqueAI);
       printf("posUltimoBloqueAI = %d\n", superBloque.posUltimoBloqueAI);
       printf("posPrimerBloqueDatos = %d\n", superBloque.posPrimerBloqueDatos);
       printf("posUltimoBloqueDatos = %d\n", superBloque.posUltimoBloqueDatos);
       printf("posInodoRaiz = %d\n", superBloque.posInodoRaiz);
       printf("posPrimerInodoLibre = %d\n", superBloque.posPrimerInodoLibre);
       printf("cantBloquesLibres = %d\n", superBloque.cantBloquesLibres);
       printf("cantInodosLibres = %d\n", superBloque.cantInodosLibres);
       printf("totBloques = %d\n", superBloque.totBloques);
       printf("totInodos = %d\n", superBloque.totInodos);

       //Mostrar también el tamaño del struct inodo y del struct del superBloque:
       printf("\nsizeof struct superbloque is: %lu\n", sizeof(struct superbloque));
       printf("sizeof struct inodo is: %lu\n", sizeof(struct inodo));
       /* 
       //recorrido de la lista de inodos libres
       printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES \n");
       for (int i = superBloque.posPrimerBloqueAI; i <= superBloque.posUltimoBloqueAI; i++)
       {
              bread(i, &inodos);

              for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
              {
                     printf(" %d", inodos[j].punterosDirectos[0]);
              }
       } 
       */
       printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n\n");
       int primerBloqueReservado = reservar_bloque();
       //int primerrBloqueReservado = reservar_bloque();
       bread(posSB, &superBloque); //Volvemos a leer SB
       printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB\n", primerBloqueReservado);
       printf("SB.cantBloquesLibres = %d\n", superBloque.cantBloquesLibres);
       //printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB\n", primerrBloqueReservado);

       liberar_bloque(primerBloqueReservado);
       //liberar_bloque(primerrBloqueReservado);
       bread(posSB, &superBloque); //Volvemos a leer SB
       printf("Liberamos ese bloque y después SB.cantBloquesLibres = %d\n", superBloque.cantBloquesLibres);
       //printf("Liberamos ese bloque y después SB.cantBloquesLibres = %d",superBloque.cantBloquesLibres);

       printf("\n\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
       int bloquePosSB = posSB;
       int posbyte0 = bloquePosSB / 8;
       int posbit0 = bloquePosSB % 8;
       int nbloqueMB0 = posbyte0 / BLOCKSIZE;
       int nbloqueabs0 = superBloque.posPrimerBloqueMB + nbloqueMB0;
       printf("\n[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\nleer_bit(%d) = %d\n",
              bloquePosSB, posbyte0, posbit0, nbloqueMB0, nbloqueabs0, bloquePosSB, leer_bit(bloquePosSB));
       int bloquePosPrimMB = superBloque.posPrimerBloqueMB;
       int posbyte1 = bloquePosPrimMB / 8;
       int posbit1 = bloquePosPrimMB % 8;
       int nbloqueMB1 = posbyte1 / BLOCKSIZE;
       int nbloqueabs1 = superBloque.posPrimerBloqueMB + nbloqueMB1;
       printf("\n[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\nleer_bit(%d) = %d\n",
              bloquePosPrimMB, posbyte1, posbit1, nbloqueMB1, nbloqueabs1, bloquePosPrimMB, leer_bit(bloquePosPrimMB));
       int bloquePosUltiMB = superBloque.posUltimoBloqueMB;
       int posbyte13 = bloquePosUltiMB / 8;
       int posbit13 = bloquePosUltiMB % 8;
       int nbloqueMB13 = posbyte13 / BLOCKSIZE;
       int nbloqueabs13 = superBloque.posPrimerBloqueMB + nbloqueMB13;
       printf("\n[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\nleer_bit(%d) = %d\n",
              bloquePosUltiMB, posbyte13, posbit13, nbloqueMB13, nbloqueabs13, bloquePosUltiMB, leer_bit(bloquePosUltiMB));
       int bloquePosPrimAI = superBloque.posPrimerBloqueAI;
       int posbyte14 = bloquePosPrimAI / 8;
       int posbit14 = bloquePosPrimAI % 8;
       int nbloqueMB14 = posbyte14 / BLOCKSIZE;
       int nbloqueabs14 = superBloque.posPrimerBloqueMB + nbloqueMB14;
       printf("\n[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\nleer_bit(%d) = %d\n",
              bloquePosPrimAI, posbyte14, posbit14, nbloqueMB14, nbloqueabs14, bloquePosPrimAI, leer_bit(bloquePosPrimAI));
       int bloquePosUltiAI = superBloque.posUltimoBloqueAI;
       int posbyte3138 = bloquePosUltiAI / 8;
       int posbit3138 = bloquePosUltiAI % 8;
       int nbloqueMB3138 = posbyte3138 / BLOCKSIZE;
       int nbloqueabs3138 = superBloque.posPrimerBloqueMB + nbloqueMB3138;
       printf("\n[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\nleer_bit(%d) = %d\n",
              bloquePosUltiAI, posbyte3138, posbit3138, nbloqueMB3138, nbloqueabs3138, bloquePosUltiAI, leer_bit(bloquePosUltiAI));
       int bloquePosPrimDT = superBloque.posPrimerBloqueDatos;
       int posbyte3139 = bloquePosPrimDT / 8;
       int posbit3139 = bloquePosPrimDT % 8;
       int nbloqueMB3139 = posbyte3139 / BLOCKSIZE;
       int nbloqueabs3139 = superBloque.posPrimerBloqueMB + nbloqueMB3139;
       printf("\n[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\nleer_bit(%d) = %d\n",
              bloquePosPrimDT, posbyte3139, posbit3139, nbloqueMB3139, nbloqueabs3139, bloquePosPrimDT, leer_bit(bloquePosPrimDT));
       int bloquePosUltDT = superBloque.posUltimoBloqueDatos;
       int posbyte99999 = bloquePosUltDT / 8;
       int posbit99999 = bloquePosUltDT % 8;
       int nbloqueMB99999 = posbyte99999 / BLOCKSIZE;
       int nbloqueabs99999 = superBloque.posPrimerBloqueMB + nbloqueMB99999;
       printf("\n[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\nleer_bit(%d) = %d\n",
              bloquePosUltDT, posbyte99999, posbit99999, nbloqueMB99999, nbloqueabs99999, bloquePosUltDT, leer_bit(0));

       bread(superBloque.posPrimerBloqueAI, &inodos);
       printf("\nDATOS DEL DIRECTORIO RAIZ\n");
       printf("tipo: %c\n", inodos[0].tipo);
       printf("permisos: %d\n", inodos[0].permisos);
       struct tm *tm = localtime(&inodos[0].atime);
       printf("atime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
       tm = localtime(&inodos[0].ctime);
       printf("ctime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
       tm = localtime(&inodos[0].mtime);
       printf("mtime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
       printf("nlinks: %d\n", inodos[0].nlinks);
       printf("tamEnBytesLog: %d\n", inodos[0].tamEnBytesLog);
       printf("numBloquesOcupados: %d\n", inodos[0].numBloquesOcupados);

       //Desmontamos el fichero
       bumount();

       return EXIT_SUCCESS;
}