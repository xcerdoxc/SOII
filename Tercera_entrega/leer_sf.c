#include "directorios.h"

void mostrar_buscar_entrada(char *camino, char reservar)
{
       unsigned int p_inodo_dir = 0;
       unsigned int p_inodo = 0;
       unsigned int p_entrada = 0;
       int error;
       printf("\ncamino: %s, reservar: %d\n", camino, reservar);
       if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0)
       {
              mostrar_error_buscar_entrada(error);
       }
       printf("**********************************************************************\n");
       return;
}

int main(int argc, char **argv)
{
       //Declaramos los structs correspondientes
       struct superbloque superBloque;
       //struct inodo inodos;

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

       /*  //Mostrar también el tamaño del struct inodo y del struct del superBloque:
    printf("\nsizeof struct superbloque is: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo is: %lu\n", sizeof(struct inodo));

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

   leer_inodo(0, &inodos);
    printf("\nDATOS DEL DIRECTORIO RAIZ\n");
    printf("tipo: %c\n", inodos.tipo);
    printf("permisos: %d\n", inodos.permisos);
    struct tm *tm = localtime(&inodos.atime);
    printf("atime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    tm = localtime(&inodos.ctime);
    printf("ctime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    tm = localtime(&inodos.mtime);
    printf("mtime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    printf("nlinks: %d\n", inodos.nlinks);
    printf("tamEnBytesLog: %d\n", inodos.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", inodos.numBloquesOcupados);

       //Reservamos un inodo previamente
       reservar_inodo('f', 6);

       printf("\nINODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n");

       traducir_bloque_inodo(1, 8, 1);
       traducir_bloque_inodo(1, 204, 1);
       traducir_bloque_inodo(1, 30004, 1);
       traducir_bloque_inodo(1, 400004, 1);
       traducir_bloque_inodo(1, 468750, 1);
             
       leer_inodo(1, &inodos);
       printf("\nDATOS DEL DIRECTORIO RAIZ\n");
       printf("tipo: %c\n", inodos.tipo);
       printf("permisos: %d\n", inodos.permisos);
       struct tm *tm = localtime(&inodos.atime);
       printf("atime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
       tm = localtime(&inodos.ctime);
       printf("ctime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
       tm = localtime(&inodos.mtime);
       printf("mtime: %d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
       printf("nlinks: %d\n", inodos.nlinks);
       printf("tamEnBytesLog: %d\n", inodos.tamEnBytesLog);
       printf("numBloquesOcupados: %d\n", inodos.numBloquesOcupados); 
       
       bread(posSB, &superBloque);
       printf("\nposPrimerInodoLibre = %d\n", superBloque.posPrimerInodoLibre);
       //Desmontamos el fichero
        */
       //Mostrar creación directorios y errore
       /*
       mostrar_buscar_entrada("pruebas/", 1);           //ERROR_CAMINO_INCORRECTO
       mostrar_buscar_entrada("/pruebas/", 0);          //ERROR_NO_EXISTE_ENTRADA_CONSULTA
       mostrar_buscar_entrada("/pruebas/docs/", 1);     //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
       mostrar_buscar_entrada("/pruebas/", 1);          // creamos /pruebas/
       mostrar_buscar_entrada("/pruebas/docs/", 1);     //creamos /pruebas/docs/
       mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
       mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);
       //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
       mostrar_buscar_entrada("/pruebas/", 1);          //ERROR_ENTRADA_YA_EXISTENTE
       mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
       mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
       mostrar_buscar_entrada("/pruebas/casos/", 1);    //creamos /pruebas/casos/
       mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
       */

       bumount(argv[1]);

       return EXIT_SUCCESS;
}
