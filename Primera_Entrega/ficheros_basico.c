#include "ficheros_basico.h"

/*-------------------------------------------------------------------
 * FUNCTION:  tamMB(unsigned int nbloques)
 * -------------------------------------------------------------------
 * 
 * Calcula el tamaño en bloques necesario para el mapa de bits.
 *
 * nbloques: Numero de bloques
 * 
 * return: Cantidad de bloques para el mapa de bits.
 * 
*/

int tamMB(unsigned int nbloques)
{
    int resul = (nbloques / 8) / BLOCKSIZE;
    if (((nbloques / 8) % BLOCKSIZE) != 0)
    {
        resul++;
    }
    return resul;
}

/*-------------------------------------------------------------------
 * FUNCTION:  tamAI(unsigned int ninodos)
 * -------------------------------------------------------------------
 * 
 * Calcula el tamaño en bloques del array de inodos.
 *
 * nbloques: Numero de bloques
 * 
 * return: Cantidad de bloques para el array de inodos.
 * 
*/

int tamAI(unsigned int ninodos)
{

    int resul = (ninodos * INODOSIZE) / BLOCKSIZE;
    if (((ninodos * INODOSIZE) % BLOCKSIZE) != 0)
    {
        resul++;
    }
    return resul;
}

/*-------------------------------------------------------------------
 * FUNCTION:  initSB(unsigned int nbloques, unsigned int ninodos)
 * -------------------------------------------------------------------
 * 
 * Inicializa los datos del superbloque.
 *
 * nbloques: Numero de bloques
 * 
 * ninodos: Numero del inodo
 * 
*/

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

/*-------------------------------------------------------------------
 * FUNCTION:  initMB()
 * -------------------------------------------------------------------
 * 
 * Inicializa los datos del mapa de bits.
 *
*/

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
/*-------------------------------------------------------------------
 * FUNCTION:  initAI()
 * -------------------------------------------------------------------
 * 
 * Inicializa los datos del mapa de inodos.
 *
 * 
*/
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

/*-------------------------------------------------------------------
 * FUNCTION:  escribir_bit(unsigned int nbloque, unsigned int bit)
 * -------------------------------------------------------------------
 * 
 * Funcion para reservar o liberar un bloque
 *
 * nbloque: bloque físico que queremos indicar si esta libre o no 
 * 
 * bit: escribe el valor indicado en el 0 (libre) o 1 (ocupado)
 * 
*/
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
/*-------------------------------------------------------------------
 * FUNCTION:  leer_bit(unsigned int nbloque)
 * -------------------------------------------------------------------
 * 
 * Lee un bit determinado del MB y devuelve su valor 
 *
 * nbloque: localizacion del bit que queremos leer dentro del MB
 * 
 * 
*/
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

/*-------------------------------------------------------------------
 * FUNCTION:  reservar_bloque()
 * -------------------------------------------------------------------
 * 
 * Encuentra el primer bloque libre, lo ocupa y devuelve su posición
 *
 * return: posición del primer bloque libre
 * 
*/
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

        memset(bufferMB, 0, BLOCKSIZE);
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

/*-------------------------------------------------------------------
 * FUNCTION:  liberar_bloque(unsigned int nbloque)
 * -------------------------------------------------------------------
 * 
 * Libera un bloque, poniendo los bits a 0 en el mapa de bits que se 
 * corresponde al bloque a eliminar.
 *
 * nbloques: Numero de bloques
 * 
 * return: Numero de bloque
 * 
*/

int liberar_bloque(unsigned int nbloque)
{
    struct superbloque SB;
    bread(posSB, &SB); //Leemos el superbloque

    escribir_bit(nbloque, 0);
    SB.cantBloquesLibres++;
    bwrite(posSB, &SB);

    return nbloque;
}

/*-------------------------------------------------------------------
 * FUNCTION:  escribir_inodo(unsigned int ninodo, struct inodo inodo)
 * -------------------------------------------------------------------
 * 
 * Escribe el contenido de una variable struct inodo en un determinado 
 * inodo del array de inodos, ninodo.
 *
 * inodo: Variable tipo struct inodo
 * 
 * ninodo: Posición del inodo en la array ded inodos
 * 
 * return EXIT_SUCCESS
 * 
*/

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

/*-------------------------------------------------------------------
 * FUNCTION:  leer_inodo(unsigned int ninodo, struct inodo *inodo)
 * -------------------------------------------------------------------
 * 
 * Lee un determinado inodo del array de inodos para volcarlo 
 * en una variable de tipo struct inodo pasada por referencia.
 *
 * inodo: Variable tipo struct inodo
 * 
 * ninodo: Posición de inodo de la array de inodos
 * 
 * return EXIT_SUCCESS
 * 
*/

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

/*-------------------------------------------------------------------
 * FUNCTION:  reservar_inodo(unsigned char tipo, unsigned char permisos)
 * -------------------------------------------------------------------
 * 
 * Encuentra el prmer inodo libre, lo reserva, devuelve su número y 
 * actualiza la lista enlazada de inodos libres.
 *
 * tipo: character que representa el tipo de inodo
 * 
 * permisos: character que representa los permisos del inodo
 * 
 * return: posición del inodo reservado
 * 
*/
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
/*-----------------------------------------------------------------------------------------------
 * FUNCTION:  obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
 * ----------------------------------------------------------------------------------------------
 * 
 * Obtiene el rango  de punteros en el que se sitúa el bloque lógico que buscamos
 *
 * nblogico: Bloque lógico
 * 
 * 
*/
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

/*-------------------------------------------------------------------
 * FUNCTION:  obtener_indice(unsigned int nblogico, unsigned int nivel_punteros)
 * -------------------------------------------------------------------
 * 
 * Ozbtiene los índices de los bloques de punteros.
 * 
 * nblogico: Bloque lógico
 * 
 * nivel_punteros: Indica el nivel en el cual se encuntra el bloque lógico
 * 
*/

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

/*-------------------------------------------------------------------
 * FUNCTION:  traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar)
 * -------------------------------------------------------------------
 * 
 * Esta función se encarga de obtener el nº de bloque físico 
 * correspondiente a un bloque lógico determinado del inodo indicado. 
 *
 * ninodos: Posición de inodo de la array de inodos
 * 
 * nblogico: Bloque lógico
 * 
 * reservar: Sirva tanto para consultar (reservar = 0) como tambien para consultar y 
 * reservar un bloque libre sin ningún bloque físico apuntado (reservar = 1)
 * 
 * return el bloque físico correspondiente
*/

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
    nivel_punteros = nRangoBL;                           //el nivel_punteros mas alto es el que cuelga del inodo
    while (nivel_punteros > 0)
    { //Iteramos para cada nivel de indirectos
        if (ptr == 0)
        {
            if (reservar == 0)
            {
                return -1;
            }
            else
            { //reservar bloques punteros y crear enlaces desde el inodo hasta los datos
                salvar_inodo = 1;
                ptr = reservar_bloque();    //de punteros
                inodo.numBloquesOcupados++; //Actualizamos el inodo
                inodo.ctime = time(NULL);
                if (nivel_punteros == nRangoBL)
                {
                    //el bloque cuelga directamente del inodo
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr;
                    fprintf(stdout, "[traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] = %d (reservado BF %d para punteros_nivel%d)]\n", (nRangoBL - 1), ptr, ptr, nivel_punteros);
                }
                else
                { //el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr;
                    fprintf(stdout, "[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para punteros_nivel%d)]\n", nivel_punteros + 1, indice, ptr, ptr, nivel_punteros);
                    bwrite(ptr_ant, buffer);
                }
            }
        }
        bread(ptr, buffer);
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        //guardamos el puntero
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        nivel_punteros--;
    }
    //Ya estamos a nivel de datos
    if (ptr == 0)
    { //no existe bloque de datos
        if (reservar == 0)
        {
            return -1;
        }
        else
        {
            salvar_inodo = 1;
            ptr = reservar_bloque();    //reservamos un bloque fisico de datos
            inodo.numBloquesOcupados++; //Actualizamos el inodo
            inodo.ctime = time(NULL);
            if (nivel_punteros == nRangoBL) //Si el rango del bloque lógico esta en los directos se guarda en el inodo directamente
            {
                inodo.punterosDirectos[nblogico] = ptr;
                fprintf(stdout, "[traducir_bloque_inodo()→ inodo.punterosDirectos[%d] = %d (reservado BF %d BL %d)]\n", nblogico, ptr, ptr, nblogico);
            }
            else //Sino si  se encuantra en un indirecto se guarda en el buffer de bloque de punteros adecuado
            {
                buffer[indice] = ptr;
                fprintf(stdout, "[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para BL %d)]\n", nivel_punteros + 1, indice, ptr, ptr, nblogico);
                bwrite(ptr_ant, buffer);
            }
        }
    }
    if (salvar_inodo == 1) //Verificamos si se ha de guardar un inodo
    {
        escribir_inodo(ninodo, inodo);
    }
    return ptr; //Devolvemos el bloque físico correspondiente
}
/*-------------------------------------------------------------------
 * FUNCTION:  liberar_inodo(unsigned int ninodo)
 * -------------------------------------------------------------------
 * 
 * Libera un inodo
 *
 * ninodo: Posición de inodo de la array de inodos
 * 
 * 
*/
int liberar_inodo(unsigned int ninodo){
    //Declaración de las variables
    struct inodo inodo;
    struct superbloque SB;
    int liberados = 0;
    //leemos el inodo
    leer_inodo (ninodo, &inodo);

    liberados = liberar_bloques_inodo(0, &inodo);
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - liberados;//Esta resta tiene que dar 0
    //Actualizamos el inodo para asi poder enlazarlo con los inodos libres
    inodo.tipo='l';
    inodo.tamEnBytesLog=0;
    //Leemos el superbloque
    bread(posSB, &SB);

    //inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    SB.cantBloquesLibres++;

    //Escribimos el inodo
    escribir_inodo(ninodo, inodo);

    //Escribimos el superBloque en el dispositivo
    bwrite(posSB,&SB);

    return ninodo; 
} 
/*-------------------------------------------------------------------
 * FUNCTION:  liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
 * -------------------------------------------------------------------
 * 
 * Libera todos los inodos a partir del bloque logico pasado por parametro
 * 
 * primerBL: bloque logico a partir que tenemos que liberar inodos
 * 
 * 
*/
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    //Declaración de las variables
    unsigned int nivel_punteros, indice, ptr, nBL, ultimoBL;
    int nRangoBL, liberados;
    unsigned int bloques_punteros[3][NPUNTEROS];
    unsigned char bufAux_punteros[BLOCKSIZE];
    int ptr_nivel[3];//punteros a bloques de punteros de cada nivel
    int indices[3];//Inidice de cada nivel
    liberados = 0;//numero de bloques liberados
    if (inodo->tamEnBytesLog == 0)  
    {
        return 0;
    }
    // el fichero está vacío obtenemos el último bloque lógico del inodo
    if ((inodo->tamEnBytesLog) % BLOCKSIZE == 0)
    {
        ultimoBL = inodo->tamEnBytesLog / (BLOCKSIZE - 1);
    }
    else
    {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }
    fprintf(stdout, "[liberar_bloques_inodo()→ primerBL: %d, últimoBL: %d ]\n", primerBL, ultimoBL);
    memset(bufAux_punteros, 0, BLOCKSIZE);
    ptr = 0;
    //Iteramos para todos los bloques logicos 
    for (nBL = primerBL; nBL <= ultimoBL; nBL++)
    {
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);//Obtenemos el nivel en el cual se encuentra el bloque logico
        if(nRangoBL < 0){
            fprintf(stderr, "ERROR");
            return 0;
        }
        nivel_punteros = nRangoBL;
        while (ptr > 0 && nivel_punteros > 0)
        {
            indice = obtener_indice(nBL, nivel_punteros);
            if(indice == 0 || nBL == primerBL){
                bread(ptr, bloques_punteros[nivel_punteros-1]);
            }
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }
        
        if (ptr > 0) // Si existe bloque de datos
        {
            liberar_bloque(ptr);
            liberados++;
            fprintf(stdout, "[liberar_bloques_inodo()→ liberado BF: %d de datos para BL %d]\n", ptr, nBL);
            if (nRangoBL == 0)
            { //Es un puntero directo
                inodo->punterosDirectos[nBL] = 0;
            }
            else
            {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL)
                {
                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];
                   
                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0)
                    {
                        //No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        liberar_bloque(ptr);
                        liberados++;
                        
                        //Incluir mejora para saltar los bloques que no sea necesario explorar!!!
                        if (nivel_punteros == nRangoBL)
                        {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        fprintf(stdout, "[liberar_bloques_inodo()→ liberado BF: %d de punteros_nivel%d correspondiente al BL %d]\n", ptr, nivel_punteros, nBL);
                        nivel_punteros++;
                        
                    }
                    else
                    { //Escribimos en el dispositivo el bloque de punteros modificado
                        bwrite(ptr, bloques_punteros[nivel_punteros - 1]);
                        //Hemos de salir del bucle ya que no seran necesario liberar los bloques
                        //de niveles superiores de los que cuelgan
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
    }
    fprintf(stdout, "[liberar_bloques_inodo()→ total bloques liberados: %d]\n", liberados);
    return liberados;
}
