#include "directorios.h"

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
    tmp = strchr(camino + 1, SEPARADOR);
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
        strcpy(inicial, camino + 1);
        //Asignamos el tipo fichero
        *tipo = 'f';
        //Copiamos en final un caracter finalizador
        strcpy(final, "");
    }
    return EXIT_SUCCESS;
}

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
    //int tambuffer = 1500;

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
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
        return EXIT_FAILURE;
    }
    printf("[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
    //Buscamos la entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);
    //Verificamos si el inodo tiene permisos de lectura
    if ((inodo_dir.permisos & 4) != 4)
    {
        mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
        return EXIT_FAILURE;
    }
    //printf("TambBYtel %d", inodo_dir.tamEnBytesLog);
    cant_entradas_inodo = (inodo_dir.tamEnBytesLog / sizeof(struct entrada)); // Calculamos la cantidad de entradas que contiene el inodo
    //printf("cantidad_entrada %d", cant_entradas_inodo);
    num_entrada_inodo = 0; //número de entrada inicial
    //Declaramos un array de entradas que caben en un bloque
    struct entrada entradas[BLOCKSIZE / sizeof(entrada)];
    //Inicializamos el buffer de lectura a 0
    memset(entradas, 0, sizeof(entradas));

    if (cant_entradas_inodo > 0)
    {

        //Leemos la entrada, en este caso leera todo el bloque que contiene entradas
        mi_read_f(*p_inodo_dir, entradas, 0, sizeof(entradas));
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entradas[num_entrada_inodo].nombre) != 0))
        {
            //Incrementamos el numero de entradas del inodo
            num_entrada_inodo++;
            //Inicializamos el buffer de lectura a 0
            memset(entradas, 0, sizeof(entradas));
            //Leemos el siguiente bloque que contendra la siguiente entrada que estamos buscando
            mi_read_f(*p_inodo_dir, entradas, (sizeof(entrada) * num_entrada_inodo), sizeof(entradas));
        }
        //Si hemos encontrado una entrada, copiamos los elementos en la variable entrada
        if (strcmp(inicial, entradas[num_entrada_inodo].nombre) == 0)
        {
            strcpy(entrada.nombre, entradas[num_entrada_inodo].nombre);
            entrada.ninodo = entradas[num_entrada_inodo].ninodo;
        }
    }
    //En caso de que la entrada no exista
    if (strcmp(inicial, entrada.nombre) != 0)
    {
        switch (reservar)
        {
        case 0: //Modo consulta, como no existe la entrada retornamos el error correspondiente
            mostrar_error_buscar_entrada(ERROR_NO_EXISTE_ENTRADA_CONSULTA);
            return EXIT_FAILURE;
            break;

        case 1: //Modo escritura. Creamos la entrada en el directorio referenciado por *p_inodo_dir
            //Si es un fichero no permitimos la escritura

            if (inodo_dir.tipo == 'f')
            {
                mostrar_error_buscar_entrada(ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO);
                return EXIT_FAILURE;
            }
            //Si es directorio comprobar si tiene permisos de escritura
            if ((inodo_dir.permisos & 2) != 2)
            {
                mostrar_error_buscar_entrada(ERROR_PERMISO_ESCRITURA);
                return EXIT_FAILURE;
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
                        printf("[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para pruebas %s]\n", entrada.ninodo, tipo, permisos, inicial);
                    }
                    else
                    {
                        mostrar_error_buscar_entrada(ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO);
                        return EXIT_FAILURE;
                    }
                }
                else
                { //es un fichero
                    //Reservamos un inodo como fichero y lo asignamos a la entrada
                    entrada.ninodo = reservar_inodo(tipo, permisos);
                    printf("[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para pruebas %s]\n", entrada.ninodo, tipo, permisos, inicial);
                }
                //Escribimos la entrada en el directorio padre, para el offset escribimos justo despues de la última entrada disponible
                if (mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(struct entrada)) == 0)
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
    if (strcmp(final, "/") == 0 || strcmp(final, "") == 0)
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            //Modo escritura y la entrada ya existe
            mostrar_error_buscar_entrada(ERROR_ENTRADA_YA_EXISTENTE);
            return EXIT_FAILURE;
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
