#include "directorios.h"

static struct UltimaEntrada UltimaEntradaEscritura;
static struct UltimaEntrada UltimaEntradaLectura;

/*-------------------------------------------------------------------
 * FUNCTION:  extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
 * -------------------------------------------------------------------
 * 
 * Dada una cadena de caracteres camino (que comience por '/'), separa su contenido en dos.
 *
 * camino: Una cadena de caracteres que empieza por /
 *
 * inicial: Porción del camino entre os dos primeros /
 *
 * final: Todo lo que queda del camino al quitar inicial
 *
 * tipo: Si se trata de un fichero o un directorio
 * 
*/
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    //Declaración de las variables
    const char SEPARADOR = '/';
    char *tmp;
    if (camino[0] != SEPARADOR)
    { //Verificamos si comienza por el caracter separador
        return -1;
    }
    //Localiza en la cadena de caracteres hasta el siguiente caracter SEPARADOR
    tmp = strchr((camino + 1), SEPARADOR);
    //En caso de haber encontrado dicho carácter quiere decir que se trata de un directorio
    //(*inicial contendrá el nombre de un directorio)
    if (tmp)
    {
        //Copiamos en inicial el nombre del directorio, hasta el siguiente caracter SEPARADOR
        strncpy(inicial, camino + 1, (strlen(camino) - strlen(tmp) - 1));
        //Asignamos el tipo directorio
        *tipo = 'd';
        //Copiamos en final la cadena de caracteres resultante de tmp
        strcpy(final, tmp);
    }
    else
    // En caso de no tener un segundo caracter SEPARADOR quiere decir que se
    // trata de un fichero
    {
        //Copiamos en inicial el nombre del fichero
        strcpy(inicial, (camino + 1));
        //Asignamos el tipo fichero
        *tipo = 'f';
        //Copiamos en final un caracter finalizador
        strcpy(final, "");
    }
    return EXIT_SUCCESS;
}

/*-------------------------------------------------------------------
 * FUNCTION: buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
 * -------------------------------------------------------------------
 * 
 * Dado un camino buscará una determinada entrada a partir de su directorio padre
 *
 * camino_parcial: camino al directorio/fichero
 *
 * p_inodo_dir: directorio padre
 * 
 * p_inodo: numero de inodo
 *
 * p_entrada: numero de entrada 
 *
 * reservar: para saber si es una consulta o creación
 *
 * permisos: permisos que asignar
 * 
*/
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    //Declaración de variables
    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    //Leemos el super bloque
    bread(posSB, &SB);

    //Verificamos si es el directorio raiz
    if (strcmp(camino_parcial, "/") == 0)
    {
        *p_inodo = SB.posInodoRaiz; //En nuestro caso el inodo valdrá 0
        *p_entrada = 0;
        return 0;
    }
    //Inicializamos antes a 0 los buffers de memoria antes de llamar a extraer_camino
    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial));
    memset(entrada.nombre, 0, TAMNOMBRE);
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -1)
    {
        return ERROR_CAMINO_INCORRECTO;
    }
    //printf("[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
    //Buscamos la entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);
    //Verificamos si el inodo tiene permisos de lectura
    if ((inodo_dir.permisos & 4) != 4)
    {
        //printf("[buscar_entrada()→ El inodo %d no tiene permisos de lectura]\n", *p_inodo_dir);
        return ERROR_PERMISO_LECTURA;
    }

    cant_entradas_inodo = (inodo_dir.tamEnBytesLog / sizeof(struct entrada)); // Calculamos la cantidad de entradas que contiene el inodo
    num_entrada_inodo = 0;                                                    //número de entrada inicial
    //Declaramos un array de entradas que caben en un bloque
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    //Inicializamos el buffer de lectura a 0
    memset(entradas, 0, sizeof(entradas));

    if (cant_entradas_inodo > 0)
    {
        //Leemos la entrada, en este caso leera todo el bloque que contiene entradas
        mi_read_f(*p_inodo_dir, entradas, 0, sizeof(entradas));
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre) != 0))
        {
            //Incrementamos el numero de entradas del inodo
            num_entrada_inodo++;
            // Verificamos si es múltiplo del número de entradas de un bloque
            if (((num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))) == 0) && (num_entrada_inodo < cant_entradas_inodo))
            {
                //Inicializamos el buffer de lectura a 0
                memset(entradas, 0, sizeof(entradas));
                //Leemos el siguiente bloque que contendra la siguiente entrada que estamos buscando
                mi_read_f(*p_inodo_dir, entradas, (sizeof(struct entrada) * num_entrada_inodo), sizeof(entradas));
            }
        }
        //Si hemos encontrado una entrada, copiamos los elementos en la variable entrada
        if (strcmp(inicial, entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre) == 0)
        {
            strcpy(entrada.nombre, entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre);
            entrada.ninodo = entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].ninodo;
        }
    }
    //En caso de que la entrada no exista y se han procesado todas las entradas
    if ((strcmp(inicial, entrada.nombre) != 0))
    {
        switch (reservar)
        {
        case 0: //Modo consulta, como no existe la entrada retornamos el error correspondiente
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            break;

        case 1: //Modo escritura. Creamos la entrada en el directorio referenciado por *p_inodo_dir
            //Si es un fichero no permitimos la escritura

            if (inodo_dir.tipo == 'f')
            {

                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            //Si es directorio comprobar si tiene permisos de escritura
            if ((inodo_dir.permisos & 2) != 2)
            {

                return ERROR_PERMISO_ESCRITURA;
            }
            else
            {
                strcpy(entrada.nombre, inicial); //Copiamos inicial en el nombre de la entrada
                if (tipo == 'd')
                {
                    if (strcmp(final, "/") == 0)
                    { //Cuelgan más directorios o ficheros
                        //Reservamos un inodo como directorio y lo asignamos a la entrada
                        entrada.ninodo = reservar_inodo(tipo, permisos);
                        //printf("[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para pruebas %s]\n", entrada.ninodo, tipo, permisos, inicial);
                    }
                    else
                    {

                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else
                { //es un fichero
                    //Reservamos un inodo como fichero y lo asignamos a la entrada
                    entrada.ninodo = reservar_inodo(tipo, permisos);
                    //printf("[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para pruebas %s]\n", entrada.ninodo, tipo, permisos, inicial);
                }
                //Escribimos la entrada en el directorio padre, para el offset escribimos justo despues de la última entrada disponible
                if (mi_write_f(*p_inodo_dir, &entrada, (sizeof(struct entrada) * num_entrada_inodo), sizeof(struct entrada)) == 0)
                { //Error de escritura
                    if (entrada.ninodo != -1)
                    { //Quiere decir que se ha reservado un idodo para la entrada
                        liberar_inodo(entrada.ninodo);
                    }
                    return EXIT_FAILURE;
                }
                printf("[buscar_entrada()→ creada entrada: %s, %d]\n", entrada.nombre, entrada.ninodo);
            }
            break;
        }
    }
    //Verificamos si hemos llegado al final del camino
    if ((strcmp(final, "") == 0) || (strcmp(final, "/") == 0))
    {

        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            //Modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        //Cortamos recursividad
        *p_inodo = entrada.ninodo;      //Asignamos al p_inodo el número de inodo del directorio o fichero creado o leido
        *p_entrada = num_entrada_inodo; //Asiganmos a p_entrada el número de su entrada
        return EXIT_SUCCESS;
    }
    else
    {
        *p_inodo_dir = entrada.ninodo;
        return (buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos));
    }
    return EXIT_SUCCESS;
}

/*-------------------------------------------------------------------
 * FUNCTION: mostrar_error_buscar_entrada(int error)
 * -------------------------------------------------------------------
 * 
 * Dado un error escribe su texto correspondiente.
 *
 * error: numero del error que le pasamos
 * 
*/
void mostrar_error_buscar_entrada(int error)
{
    // fprintf(stderr, "Error: %d\n", error);
    switch (error)
    {
    case -1:
        fprintf(stderr, "Error: Camino incorrecto.\n");
        break;
    case -2:
        fprintf(stderr, "Error: Permiso denegado de lectura.\n");
        break;
    case -3:
        fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
        break;
    case -4:
        fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
        break;
    case -5:
        fprintf(stderr, "Error: Permiso denegado de escritura.\n");
        break;
    case -6:
        fprintf(stderr, "Error: El archivo ya existe.\n");
        break;
    case -7:
        fprintf(stderr, "Error: No es un directorio.\n");
        break;
    }
}

/*-------------------------------------------------------------------
 * FUNCTION: int mi_creat(const char *camino, unsigned char permisos)
 * -------------------------------------------------------------------
 * 
 * Crea un directorio con sus permisos correspondientes
 *
 * camino: directorio que queremos crear
 *
 * permisos: permisos que le queremos asignar al directorio
 * 
*/
int mi_creat(const char *camino, unsigned char permisos)
{
    mi_waitSem();
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return error;
    }
    else
    {
        mi_signalSem();
        return 0;
    }
}

/*-------------------------------------------------------------------
 * FUNCTION: int mi_dir(const char *camino, char *buffer)
 * -------------------------------------------------------------------
 * 
 * Muestra las entradas de directorio 
 *
 * camino: directorio que queremos mirar
 *
 * buffer: buffer donde se guardara el contenido del directorio
 * 
 * return: numero de entradas encontradas
 * 
*/
int mi_dir(const char *camino, char *buffer, char tipo)
{

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error, contador_entradas = 0, cant_entradas_inodo = 0;
    //Declaramos un inodo
    struct inodo inodo;
    //Declaramos un buffer de n entradas
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    //Declaramos un struct entrada
    struct entrada entrada;
    struct tm *tm; //ver info: struct tm
    char tmp[50];
    char nombre1[100];

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        return error;
    }
    //Leemos el inodo
    leer_inodo(p_inodo, &inodo);
    //Verificamos si el inodo tiene permisos de lectura
    if ((inodo.permisos & 4) != 4)
    {
        mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
        return -1;
    }
    //Comprobamos que se trata de un directorio
    if (tipo != 'd')
    {
        //Se trata de un fichero
        mi_read_f(p_inodo_dir, &entrada, sizeof(struct entrada) * p_entrada, sizeof(struct entrada));
        //Leemos la informacion de los inodos
        leer_inodo(entrada.ninodo, &inodo);
        // Escribir entrada en el buffer con el formato adecuado.
        char bufferAux[10];

        sprintf(bufferAux, "%c\t", inodo.tipo);
        strcat(buffer, RED);
        strcat(buffer, bufferAux); //Concatenamos en el buffer de memoria separado con un tabulador

        //Incorporamos informacion acerca de los permisos
        strcat(buffer, YELLOW);
        if (inodo.permisos & 4)
            strcat(buffer, "r");
        else
            strcat(buffer, "-");
        if (inodo.permisos & 2)
            strcat(buffer, "w");
        else
            strcat(buffer, "-");
        if (inodo.permisos & 1)
            strcat(buffer, "x\t\t");
        else
            strcat(buffer, "-\t\t");

        //Incorporamos la informacion acerca del time
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, GREEN);
        strcat(buffer, tmp);

        sprintf(bufferAux, "%d\t\t", inodo.tamEnBytesLog);
        strcat(buffer, CYAN);
        strcat(buffer, bufferAux); //Concatenamos en el buffer de memoria separado con un tabulador

        sprintf(nombre1, "%s", entrada.nombre);
        strcat(buffer, BLUE);
        strcat(buffer, nombre1); //imprimimos el nombre del directorio

        //Separamos los archivos por un caracter separador
        strcat(buffer, RESET);
        strcat(buffer, "\n");

        //Incrementamos el contador
        contador_entradas++;
    }
    else
    { //Se trata de un directorio

        //Calculamos la cantidad de entradas de inodos
        cant_entradas_inodo = (inodo.tamEnBytesLog / sizeof(struct entrada));
        //Inicializamos 0 el buffer de n entradas
        memset(entradas, 0, sizeof(entradas));
        if (cant_entradas_inodo > 0)
        {
            //Leemos de forma secuencial el contenido del inodo de tipo directorio
            mi_read_f(p_inodo, entradas, 0, sizeof(entradas));

            contador_entradas = 0;
            while (contador_entradas < cant_entradas_inodo) //Iteramos para cada entrada del bloque
            {
                //Leemos la informacion de los inodos
                //struct inodo inodo;
                leer_inodo(entradas[contador_entradas % (BLOCKSIZE / sizeof(struct entrada))].ninodo, &inodo);

                // Escribir entrada en el buffer con el formato adecuado.
                char bufferAux[10];
                sprintf(bufferAux, "%c\t", inodo.tipo);
                strcat(buffer, RED);
                strcat(buffer, bufferAux); //Concatenamos en el buffer de memoria separado con un tabulador

                //Incorporamos informacion acerca de los permisos
                strcat(buffer, YELLOW);
                if (inodo.permisos & 4)
                    strcat(buffer, "r");
                else
                    strcat(buffer, "-");
                if (inodo.permisos & 2)
                    strcat(buffer, "w");
                else
                    strcat(buffer, "-");
                if (inodo.permisos & 1)
                    strcat(buffer, "x\t\t");
                else
                    strcat(buffer, "-\t\t");

                //Incorporamos la informacion acerca del time
                tm = localtime(&inodo.mtime);
                sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                strcat(buffer, GREEN);
                strcat(buffer, tmp);

                sprintf(bufferAux, "%d\t\t", inodo.tamEnBytesLog);
                strcat(buffer, CYAN);
                strcat(buffer, bufferAux); //Concatenamos en el buffer de memoria separado con un tabulador

                sprintf(nombre1, "%s", entradas[contador_entradas % (BLOCKSIZE / sizeof(struct entrada))].nombre);
                strcat(buffer, BLUE);
                strcat(buffer, nombre1); //imprimimos el nombre del directorio

                //Separamos los archivos por un caracter separador
                strcat(buffer, RESET);
                strcat(buffer, "\n");

                //Incrementamos el contador
                contador_entradas++;

                // Verificamos si es múltiplo del número de entradas de un bloque
                if (contador_entradas % (BLOCKSIZE / sizeof(struct entrada)) == 0)
                {
                    //Leemos el siguiente bloque con entradas
                    //Inicializamos 0 el buffer de n entradas
                    memset(entradas, 0, sizeof(entradas));
                    //Leemos de forma secuencial el contenido del inodo de tipo directorio
                    mi_read_f(p_inodo, entradas, (sizeof(struct entrada) * contador_entradas), sizeof(entradas));
                }
            }
        }
    }

    //Retornamos el número de entradas
    return contador_entradas;
}

/*-------------------------------------------------------------------
 * FUNCTION: mi_chmod(const char *camino, unsigned char permisos)
 * -------------------------------------------------------------------
 * 
 * Cambia los permisos de un directorio o fichero
 *
 * camino: directorio al que queremos cambiar los permisos
 *
 * permisos: permisos que le queremos poner al directorio o fichero
 * 
*/
int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    //Verificamos si la entrada existe
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        return error;
    }
    //Llamamos a la funcion que se encarga de modificar los permisos
    mi_chmod_f(p_inodo, permisos);

    return EXIT_SUCCESS;
}

/*-------------------------------------------------------------------
 * FUNCTION: mi_stat(const char *camino, struct STAT *p_stat)
 * -------------------------------------------------------------------
 * 
 * Muestra la información del inodo de un fichero
 *
 * camino: directorio que queremos saber la información
 *
 * p_stat: struct stat del inodo asociado al fichero que queremos ver
 * 
*/
int mi_stat(const char *camino, struct STAT *p_stat)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    //Verificamos si la entrada existe
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        return error;
    }
    //Llamamos a la funcion que nos visualiza la informacion del inodo
    mi_stat_f(p_inodo, p_stat);
    //Imprimimos el número del inodo
    printf("Nº de inodo: %d\n", p_inodo);

    return EXIT_SUCCESS;
}

/*-------------------------------------------------------------------
 * FUNCTION: mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
 * -------------------------------------------------------------------
 * 
 * Escribir contenido en un fichero
 *
 * camino: el directorio a escribir
 *
 * buf: texto a escribir
 * 
 * offset: lugar donde se escribira
 * 
 * nbytes: longitud del buffer
 * 
 * return: numero de bytes escritos
 * 
*/
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error, escritos;
    //Comprobamos si la escritura es sobre un mismo inodo
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
    {
        //printf("[mi_write() → Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\n");
        //Si se cumple asignamos el inodo donde se ha escrito anteriormente
        p_inodo = UltimaEntradaEscritura.p_inodo;
    }
    else
    {
        //Verificamos si la entrada existe y obtenemos el valor de p_inodo
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)
        {
            mostrar_error_buscar_entrada(error);
            return error;
        }
        //Actualizamos el campo del p_inodo y el camino UltimaEntradaEscrita
        //printf("[mi_write() → Actualizamos la caché de escritura]\n");
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }
    //Llamamos a la funcion mi_write_f
    escritos = mi_write_f(p_inodo, buf, offset, nbytes);
    //Retornamos los bytes escritos
    return escritos;
}

/*-------------------------------------------------------------------
 * FUNCTION: mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
 * -------------------------------------------------------------------
 * 
 * Lee el contenido de un fichero
 *
 * camino: el fichero a leer
 *
 * buf: donde se guarda la lectura
 * 
 * offset: offset actual que leer
 * 
 * nbytes: tamaño del buffer
 * 
 * return: numero de bytes leidos
 * 
*/
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error, leidos;
    //Comprobamos si la lectura es sobre un mismo inodo
    if (strcmp(UltimaEntradaLectura.camino, camino) == 0)
    {
        //printf("[mi_read() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n");
        //Si se cumple asignamos el inodo donde se ha escrito anteriormente
        p_inodo = UltimaEntradaLectura.p_inodo;
    }
    else
    {
        //Verificamos si la entrada existe y obtenemos el valor de p_inodo
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)
        {
            mostrar_error_buscar_entrada(error);
            return error;
        }
        //Actualizamos el campo del p_inodo y el camino UltimaEntradaLectura
        //printf("[mi_read() → Actualizamos la caché de lectura]\n");
        strcpy(UltimaEntradaLectura.camino, camino);
        UltimaEntradaLectura.p_inodo = p_inodo;
    }
    //Llamamos a la funcion mi_read_f
    leidos = mi_read_f(p_inodo, buf, offset, nbytes);
    //Retornamos los bytes escritos
    return leidos;
}

/*-------------------------------------------------------------------
 * FUNCTION: mi_link(const char *camino1, const char *camino2)
 * -------------------------------------------------------------------
 * 
 * Crea un enlaze entre dos directorios
 *
 * camino1: camino que si existe
 *
 * camino2: camino que no existe que hay que crear con enlaze
 * 
*/
int mi_link(const char *camino1, const char *camino2)
{
    mi_waitSem();
    //Declaramos un inodo1
    struct inodo inodo1;
    struct entrada entrada2;
    unsigned int p_inodo_dir1 = 0;
    unsigned int p_inodo1 = 0;
    unsigned int p_entrada1 = 0;
    int error1;
    if ((error1 = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 0)) < 0)
    {
        mostrar_error_buscar_entrada(error1);
        mi_signalSem();
        return error1;
    }
    //Leemos el inodo
    leer_inodo(p_inodo1, &inodo1);
    //Verificamos si el inodo1 tiene permisos de lectura
    if ((inodo1.permisos & 4) != 4)
    {
        mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
        mi_signalSem();
        return ERROR_PERMISO_LECTURA;
    }
    //Declaramos un inodo2
    unsigned int p_inodo_dir2 = 0;
    unsigned int p_inodo2 = 0;
    unsigned int p_entrada2 = 0;
    int error2;

    if ((error2 = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6)) < 0)
    {
        mostrar_error_buscar_entrada(error2);
        mi_signalSem();
        return error2;
    }
    //Inicializamos el campo de entrada2 nombre
    memset(entrada2.nombre, 0, TAMNOMBRE);
    //Leemos la entrada creada de camino2
    mi_read_f(p_inodo_dir2, &entrada2, (p_entrada2 * sizeof(struct entrada)), sizeof(struct entrada));
    // Asociamos a esta entrada el mismo inodo que el asociado a la entrada de camino1,
    entrada2.ninodo = p_inodo1;
    //Escribimos la entrada modificada en p_inodo_dir2
    mi_write_f(p_inodo_dir2, &entrada2, (p_entrada2 * sizeof(struct entrada)), sizeof(struct entrada));

    //Liberamos el inodo que se ha asociado a la entrada creada, p_inodo2
    liberar_inodo(p_inodo2);

    //Incrementamos la cantidad de enlaces de p_inodo1, actualizamos el ctime
    inodo1.nlinks++;
    inodo1.ctime = time(NULL);

    //Guardamos el inodo
    escribir_inodo(p_inodo1, inodo1);
    mi_signalSem();
    return EXIT_SUCCESS;
}

/*-------------------------------------------------------------------
 * FUNCTION: mi_unlink(const char *camino)
 * -------------------------------------------------------------------
 * 
 * Elimina ficheros o directorios tanto con enlazes como sin ellos
 *
 * camino: fichero o directorio a eliminar
 *
 * return: numero del inodo liberado
 * 
*/
int mi_unlink(const char *camino)
{
    mi_waitSem();
    //Declaramos un inodo
    struct inodo inodo;
    struct inodo inodo_dir;
    struct entrada entrada;
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return error;
    }
    //Leemos el inodo
    leer_inodo(p_inodo, &inodo);
    if ((inodo.tipo == 'd') && (inodo.tamEnBytesLog > 0))
    {
        fprintf(stderr, "Error: El directorio %s no está vacío\n", camino);
        mi_signalSem();
        return -1;
    }
    //leemos el inodo asociado al directorio que contiene la entrada que queremos eliminar
    leer_inodo(p_inodo_dir, &inodo_dir);

    //Calculamos el número de entradas
    int num_entradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    //Verificamos si la entrada a eliminar es la última
    if (p_entrada == (num_entradas - 1))
    {
        //Truncamos el inodo a su tamaño menos el tamaño de una entrada
        mi_truncar_f(p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof(struct entrada));
    }
    else
    { //No es la última entrada
        //Leemos la última entrada
        mi_read_f(p_inodo_dir, &entrada, (num_entradas - 1) * sizeof(struct entrada), sizeof(struct entrada));
        //Escribimos en la posicion de la entrada que queremos eliminar
        mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada));
        //Truncamos el inodo
        mi_truncar_f(p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof(struct entrada));
    }
    //Leemos el inodo asociado a la entrada eliminada para decrementar el nº de enlaces.
    inodo.nlinks--;
    if (inodo.nlinks == 0) //Verificamos si quedan enlaces relacionados a este inodo
    {
        liberar_inodo(p_inodo);
    }
    else
    {
        //Actualizamos el ctime y lo guardamos
        inodo.ctime = time(NULL);
        escribir_inodo(p_inodo, inodo);
    }
    mi_signalSem();
    return EXIT_SUCCESS;
}
int mi_unlinkR (const char *camino)
{
    //Declaracion de variables
    int tam_entrada = BLOCKSIZE / sizeof(struct entrada);
    struct entrada entradas[tam_entrada];
    struct entrada entrada;
    struct inodo inodo;
    int num_entradas, error, leidos, offset;
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        return error;
    }
    //Inicializamos el offset a 0
    offset = 0;
    //Inicializamos el struct entradas
    memset(entradas, 0, tam_entrada);
    //Lee el contenido del inodo
    leidos = mi_read_f(p_inodo, entradas, offset, tam_entrada);
    //Bucle que se encraga de recorrer todas las entradas del directorio
    while (leidos > 0)
    {
        //Calculamos el número de entradas leidas
        num_entradas = leidos / sizeof(struct entrada);

        //Recorremos todas las entradas leidas
        while (num_entradas)
        {
            //Leemos el inodo de la entrada.
            leer_inodo(entradas[num_entradas].ninodo, &inodo);
            //Verificamos si se trata de un directorio
            if (inodo.tipo == 'd')
            {
                //Como se trata de un directorio volvemos a llamar recursivamente a la funcion
                mi_unlinkR(entradas[num_entradas].nombre);
            }
            else
            {
                //Sino si se trata de un fichero, borramos el fichero del dispositivo
                error = mi_unlink(entradas[num_entradas].nombre);
            }
            //Decrementamos el número de entradas
            num_entradas--;
        }
        //Actualiza el offset de lectura e indice
        offset += tam_entrada;
        //Volvemos a inicializar el buffer de entradas
        memset(entradas, 0, tam_entrada);
        //Leemos las siguientes entradas.
        leidos = mi_read_f(p_inodo, entradas, offset, tam_entrada);
    }
    //Por último eliminamos el directorio por completo el directorio que recibimos por parametro
    //mi_unlink(camino);
    struct inodo inodo_dir;
    leer_inodo(p_inodo_dir, &inodo_dir);
    //Calculamos el número de entradas
    int num_entrada = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    //Verificamos si la entrada a eliminar es la última
    if (p_entrada == (num_entrada - 1))
    {
        //Truncamos el inodo a su tamaño menos el tamaño de una entrada
        mi_truncar_f(p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof(struct entrada));
    }
    else
    { //No es la última entrada
        //Leemos la última entrada
        mi_read_f(p_inodo_dir, &entrada, (num_entrada - 1) * sizeof(struct entrada), sizeof(struct entrada));
        //Escribimos en la posicion de la entrada que queremos eliminar
        mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada));
        //Truncamos el inodo
        mi_truncar_f(p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof(struct entrada));
    }
    //inodo.nlinks--;
   // if (inodo.nlinks == 0) //Verificamos si quedan enlaces relacionados a este inodo
  //  {
        liberar_inodo(p_inodo);
   // }
    // else
    // {
    //     //Actualizamos el ctime y lo guardamos
    //     inodo.ctime = time(NULL);
    //     escribir_inodo(p_inodo, inodo);
    // } 

    return EXIT_SUCCESS;
}
int mi_rn(const char *camino, const char *nombre)
{

    //Declaramos un inodo
    struct inodo inodo;
    struct entrada entrada;
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;

    //comprobamos que el directorio o fichero existe
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        return error;
    }
    leer_inodo(p_inodo, &inodo);                                                                  //leemos el inodo para saber su type
    mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)); //leemos su estrada para modificar el nombre
    int c = 0;
    char *caminofinal="/";
    char *token;
    char camino2[1000];
    char **args;
    
    memset(camino2, 0, 1000);
    sprintf(camino2, "%s", camino);
    printf("%s \n",camino2);
    //camino2 = strrchr(camino, '/')+1; //alomejor lo que esta mal es entrada.nombre de buscar entrada y esto deberia de funcar
    token = strtok(camino2, "/");
    args[c] = token;
        while (args[c] != NULL)
        {
            printf("Token: %s\n", args[c]);
            token = strtok(NULL, "/");

            c++;
            args[c] = token;
        }
    for (int i = 0; i < c - 1; i++)
    {
        strcat(caminofinal, args[i]);
        strcat(caminofinal, "/");
    }
    printf("%s\n", caminofinal);

    strcat(caminofinal, nombre);
    printf("%s\n", caminofinal);
    //comprobamos que el nuevo nombre no exista
    unsigned int p_inodo_dir2 = 0;
    unsigned int p_inodo2 = 0;
    unsigned int p_entrada2 = 0;
    if ((error = buscar_entrada(caminofinal, &p_inodo_dir2, &p_inodo2, &p_entrada2, 0, 0)) < 0)
    {
        
        if (inodo.tipo == 'd')
        {
            char nombredir[60];
            sprintf(nombredir, "%s", nombre);
            strcat(nombredir, "/");
            sprintf(entrada.nombre, "%s", nombredir);
        }
        else if (inodo.tipo == 'f')
        {
            sprintf(entrada.nombre, "%s", nombre);
        }
        printf("escritura");
        //guardamos la entrada
        mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada));
    }
    else
    {
        printf("NOMBRE SELECCIONADO YA EXISTENTE");
    }
    return EXIT_SUCCESS;
}

// int mi_cp_f(const char *caminoFichero, const char *caminoDestino)
// {

// }