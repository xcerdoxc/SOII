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
        //Grabar bufer a 0s pendiente
        
        memset(bufferMB,0,BLOCKSIZE); 
        bwrite(nbloque, bufferMB);// pruebas

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

    int nbloque = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE)); //miramos que bloque de inodos es
    bread(nbloque, inodos);
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];
    return 0;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos)
{

    struct superbloque SB;
    struct inodo inodo;
    int posInodoReservado;
    bread(posSB, &SB); //Leemos el superbloque
    if (SB.cantInodosLibres)
    {
        posInodoReservado = SB.posPrimerInodoLibre;
        leer_inodo(posInodoReservado, &inodo);
        SB.posPrimerInodoLibre = inodo.punterosDirectos[0];

        //INICIALIZAMOS LOS CAMPOS DEL INODO AL QUE APUNTABA INICIALMENTE EL SUPERBLOQUE
        inodo.tipo = tipo;
        inodo.permisos = permisos;
        inodo.nlinks = 1;
        inodo.tamEnBytesLog = 0;
        inodo.atime = time(NULL);
        inodo.ctime = time(NULL);
        inodo.mtime = time(NULL);
        inodo.numBloquesOcupados = 0;
        for (int i = 0; i < 12; i++)
        {
            inodo.punterosDirectos[i] = 0;
        }
        for (int i = 0; i < 3; i++)
        {
            inodo.punterosIndirectos[i] = 0;
        }

        //ESCRIBIMOS EL INODO INICIALIZADO
        escribir_inodo(posInodoReservado, inodo);
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
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{
    if (nblogico < DIRECTOS)
    {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    else
    {
        *ptr = 0;
        fprintf(stderr, "BLOQUE LÓGICO FUERA DEL RANGO\n");
        return EXIT_FAILURE;
    }
}
int obtener_indice(unsigned int nblogico, unsigned int nivel_punteros)
{
    if (nblogico < DIRECTOS)
    {
        return nblogico;
    }
    else if (nblogico < INDIRECTOS0)
    {
        return nblogico - DIRECTOS;
    }
    else if (nblogico < INDIRECTOS1)
    {
        if (nivel_punteros == 2)
        {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }
    else if (nblogico < INDIRECTOS2)
    {
        if (nivel_punteros == 3)
        {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        else if (nivel_punteros == 2)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    return EXIT_FAILURE;
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar)
{
    struct inodo inodo;
    unsigned int ptr; 
    int ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];

    //Inicializamos las variables
    leer_inodo(ninodo, &inodo);
    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); //0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL;//el nivel_punteros mas alto es el que cuelga del inodo
    while (nivel_punteros > 0)
    { //Iteramos para cada nivel de indirectos
        if (ptr == 0)
        {
            if (reservar == 0)
            {
                fprintf(stderr, "ERROR LECTURA BLOQUE INEXISTENTE\n");
                return -1;
            }
            else
            { //reservar bloques punteros y crear enlaces desde el inodo hasta los datos
                salvar_inodo = 1;
                ptr = reservar_bloque(); //de punteros
                inodo.numBloquesOcupados++;//Actualizamos el inodo
                inodo.ctime = time(NULL);
                if (nivel_punteros == nRangoBL)
                {
                    //el bloque cuelga directamente del inodo
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr; 
                    fprintf(stdout,"[traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] = %d (reservado BF %d para punteros_nivel%d)]\n", (nRangoBL-1), ptr, ptr, nivel_punteros);
                }
                else
                {                         //el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr; 
                    fprintf(stdout,"[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para punteros_nivel%d)]\n", nivel_punteros+1,indice, ptr, ptr, nivel_punteros);                    
                    bwrite(ptr_ant, buffer);
                }
            }
        }
        bread(ptr, buffer);
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        //guardamos el puntero
        //fprintf(stdout,"[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para BL %d)]\n", nivel_punteros, indice, ptr, ptr, nblogico);
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        
        nivel_punteros--;
        
    } 
    //Ya estamos a nivel de datos
    if (ptr == 0)
    { //no existe bloque de datos
        if (reservar == 0)
        {
            fprintf(stderr, "ERROR LECTURA BLOQUE INEXISTENTE\n");
            return -1;
        }
        else
        {
            salvar_inodo = 1;
            ptr = reservar_bloque(); //reservamos un bloque fisico de datos
            inodo.numBloquesOcupados++;//Actualizamos el inodo
            inodo.ctime = time(NULL);
            if (nRangoBL==0)//Si el rango del bloque lógico esta en los directos se guarda en el inodo directamente
            {
                inodo.punterosDirectos[nblogico] = ptr; 
                fprintf(stdout,"[traducir_bloque_inodo()→ inodo.punterosDirectos[%d] = %d (reservado BF %d BL %d)]\n", nblogico, ptr, ptr, nblogico);
            }
            else//Sino si  se encuantra en un indirecto se guarda en el buffer de bloque de punteros adecuado 
            {
                buffer[indice] = ptr; 
                //fprintf(stdout,"[traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] = %d (reservado BF %d para punteros_nivel%d)]\n", (nRangoBL-1), ptr, ptr, nivel_punteros);
                fprintf(stdout,"[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para BL %d)]\n", nivel_punteros+1, indice, ptr, ptr, nblogico);
                bwrite(ptr_ant, buffer);
            }
        }
    }
    if (salvar_inodo == 1)//Verificamos si se ha de guardar un inodo
    {
        escribir_inodo(ninodo, inodo);
    }
    return ptr; //Devolvemos el bloque físico correspondiente
}