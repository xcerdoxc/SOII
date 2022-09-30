#include "ficheros_basico.h"
#define BLOCKSIZE 1024 // bytes

int tamMB(unsigned int nbloques)
{
    int resul = (nbloques / 8) / BLOCKSIZE;
    if (((nbloques / 8) % BLOCKSIZE) != 0)
    {
        resul++;
    }
    return resul;
}

int tamAI(unsigned int ninodos)
{

    int resul = (ninodos * INODOSIZE) / BLOCKSIZE;
    if (((ninodos * INODOSIZE) % BLOCKSIZE) != 0)
    {
        resul++;
    }
    return resul;
}

int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB;
    //Posición del primer bloque del mapa de bits
    SB.posPrimerBloqueMB = posSB + tamSB; //posSB = 0, tamSB = 1
    //Posición del último bloque del mapa de bits
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    //Posición del primer bloque del array de inodos
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    //Posición del último bloque del array de inodos
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    //Posición del primer bloque de datos
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    //Posición del último bloque de datos
    SB.posUltimoBloqueDatos = nbloques - 1;
    //Posición del inodo del directorio raíz en el array de inodos
    SB.posInodoRaiz = 0;
    //Posición del primer inodo libre en el array de inodos
    SB.posPrimerInodoLibre = 0;
    //Cantidad de bloques libres en el SF - el bloque que ocupa el SB
    SB.cantBloquesLibres = nbloques - 1;
    //Cantidad de inodos libres en el array de inodos
    SB.cantInodosLibres = ninodos;
    //Cantidad total de bloques
    SB.totBloques = nbloques;
    //Cantidad total de inodos
    SB.totInodos = ninodos;

    //Falta escribir el posSB mediante la funcion bwrite()
    bwrite(posSB, &SB);
    return EXIT_SUCCESS;
}

int initMB()
{
    //Declaramos un buffer de memoria
    unsigned char buffer[BLOCKSIZE];
    struct superbloque SB;
    bread(posSB, &SB); //Leemos el superbloque
    //Inicializamos a 0 el buffer de memoria
    memset(buffer, '0', BLOCKSIZE);
    //Inicializamos a 0 el bloque de datos
    for (int i = SB.posPrimerBloqueDatos; i <= SB.posUltimoBloqueDatos; i++)
    {
        bwrite(i, buffer);
    }
    //Marcamos como ocupado los bloques de metadatos del SB
    for (int i = posSB; i <= SB.posUltimoBloqueAI; i++)
    {
        escribir_bit(i, 1);
    }

    //actualizar la cantidad de bloques libres en el superbloque y grabarlo.
    SB.cantBloquesLibres = SB.cantBloquesLibres - (SB.posUltimoBloqueMB - SB.posPrimerBloqueMB + 1);
    bwrite(posSB, &SB);

    return EXIT_SUCCESS;
}
int initAI()
{
    struct superbloque SB;
    bread(posSB, &SB); //Leemos el superbloque
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int contInodos = SB.posPrimerInodoLibre + 1;
    bool fin = false;
    //si hemos inicializado SB.posPrimerInodoLibre = 0
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        for (int j = 0; j < BLOCKSIZE / INODOSIZE && !fin; j++)
        {
            inodos[j].tipo = 'l'; //Libre
            if (contInodos < SB.totInodos)
            {
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            }
            else
            {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                fin = true; //Salimos del fichero
            }
        }
        bwrite(i, inodos);
    }
    //actualizar la cantidad de bloques libres en el superbloque y grabarlo.
    SB.cantBloquesLibres = SB.cantBloquesLibres - (SB.posUltimoBloqueAI - SB.posPrimerBloqueAI + 1);
    bwrite(posSB, &SB);
    return EXIT_SUCCESS;
}

int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    struct superbloque SB;
    unsigned char mascara = 128; // 10000000

    bread(posSB, &SB); //Leemos el superbloque
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    //Leemos el bloque que se corresponde al byte que queremos modificar
    bread(nbloqueabs, bufferMB);

    posbyte = posbyte % BLOCKSIZE;
    mascara >>= posbit;
    if (bit == 1)
    {
        bufferMB[posbyte] |= mascara; //  operador OR para bits
    }
    else if (bit == 0)
    {
        bufferMB[posbyte] &= ~mascara; // operadores AND y NOT para bits
    }
    else
    {
        fprintf(stderr, "BIT DISTINTO DE 0/1 \n");
        return EXIT_FAILURE;
    }
    
    bwrite(nbloqueabs, bufferMB);
    return EXIT_SUCCESS;
}

char leer_bit(unsigned int nbloque)
{

    struct superbloque SB;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128; // 10000000
    //Leemos el superBloque
    bread(posSB, &SB);

    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    bread(nbloqueabs, bufferMB);
    //Calculamos el byte que contiene el bit que queremos leer
    posbyte = posbyte % BLOCKSIZE;

    mascara >>= posbit;           // desplazamiento de bits a la derecha
    mascara &= bufferMB[posbyte]; // operador AND para bits

    return (mascara >>= (7 - posbit)); // desplazamiento de bits a la derecha
}

int reservar_bloque()
{

    struct superbloque SB;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    bool encontBloque = false;
    bool encontByte = false;
    int nbloque = 0;
    unsigned char mascara = 128; // 10000000
    int posBloqueMB;
    int posbyte;
    int posbit = 0;
    bread(posSB, &SB);                 //Leemos el superbloque
    memset(bufferAux, 255, BLOCKSIZE); // llenamos el buffer auxiliar con 1s

    if (SB.cantBloquesLibres)
    {
        //Localizamos la posicion del primer bloque que contenga un 0
        for (int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB && !encontBloque; i++)
        {
            bread(i, bufferMB);
            if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0)
            {
                posBloqueMB = i;
                encontBloque = true;
            }
        }
        for (int i = 0; i < BLOCKSIZE && encontBloque && !encontByte; i++)
        { // esperando a explicacion de adelaida
            if (bufferMB[i] < 255)
            {
                posbyte = i;
                encontByte = true;
            }
        }
        while (bufferMB[posbyte] && mascara)
        {                            // operador AND para bits
            bufferMB[posbyte] <<= 1; // desplazamiento de bits a la izquierda
            posbit++;
        }
        //printf("bloque %d, byte %d, bit %d", posBloqueMB, posbyte, posbit);
        nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
        escribir_bit(nbloque, 1);
        SB.cantBloquesLibres--;

        memset(bufferMB,0,BLOCKSIZE); 
        bwrite(nbloque, bufferMB);

        bwrite(posSB, &SB);
    }
    else
    {
        fprintf(stderr, "ERROR NO HAY BLOQUES LIBRES \n");
        return EXIT_FAILURE;
    }

    return nbloque;
}

int liberar_bloque(unsigned int nbloque)
{
    struct superbloque SB;
    bread(posSB, &SB); //Leemos el superbloque

    escribir_bit(nbloque, 0);
    SB.cantBloquesLibres++;
    bwrite(posSB, &SB);

    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo inodo)
{
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    bread(posSB, &SB); //Leemos el superbloque
    //LEER CON EN EL NUMERO DE BLOQUE DEL NINODO QUE NOS HA PASADO POR PARAMETRO
    int nbloque = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));
    bread(nbloque, inodos);
    //escribimos el inodo en el lugar correspondiente del array
    inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = inodo;

    bwrite(nbloque, inodos);
    return EXIT_SUCCESS;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    bread(posSB, &SB); //Leemos el superbloque

    int numinodo = ninodo / (BLOCKSIZE / INODOSIZE); //miramos que bloque de inodos es
    int posicion = numinodo + SB.posPrimerBloqueAI;  //miramos que bloque es
    bread(posicion, inodos);
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];
    return 0;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos)
{

    struct superbloque SB;
    struct inodo inodo[BLOCKSIZE / INODOSIZE];
    int posInodoReservado;
    bread(posSB, &SB); //Leemos el superbloque
    if (SB.cantInodosLibres)
    {
        posInodoReservado = SB.posPrimerInodoLibre;
        leer_inodo(posInodoReservado, inodo);
        SB.posPrimerInodoLibre = inodo->punterosDirectos[0];

        //INICIALIZAMOS LOS CAMPOS DEL INODO AL QUE APUNTABA INICIALMENTE EL SUPERBLOQUE
        inodo->tipo = tipo;
        inodo->permisos = permisos;
        inodo->nlinks = 1;
        inodo->tamEnBytesLog = 0;
        inodo->atime = time(NULL);
        inodo->ctime = time(NULL);
        inodo->mtime = time(NULL);
        inodo->numBloquesOcupados = 0;
        for (int i = 0; i < 12; i++)
        {
            inodo->punterosDirectos[i] = 0;
        }
        for (int i = 0; i < 3; i++)
        {
            inodo->punterosIndirectos[i] = 0;
        }

        //ESCRIBIMOS EL INODO INICIALIZADO
        escribir_inodo(posInodoReservado, *inodo);
        //ACTUALIZAMOS LA CANTIDAD DE INODOS LIBRES Y REESCRIBIMOS EL SUPERBLOQUE
        SB.cantInodosLibres--;
        bwrite(posSB, &SB);
    }
    else
    {
        fprintf(stderr, "ERROR NO HAY INODOS LIBRES \n");
        return EXIT_FAILURE;
    }
    return posInodoReservado;
}