#include "directorios.h"
int main(int argc, char const *argv[])
{
    //Declaración de las variables
    int offset, leidos, contador, tambuffer = 1500;
    char *buffer;
    char ruta_destino[100], buffer_texto[tambuffer];
    struct STAT stat;
        unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    //Comprobamos el número de parametros introducidos
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: ./mi_cp_f <disco> </origen/nombre> </destino/> \n");
        return EXIT_FAILURE;
    }
    //Comprobamos si la ruta origen se trata de un fichero
    if (argv[2][strlen(argv[2]) - 1] == '/')
    {
        fprintf(stderr, "Error: No se puede copiar un directorio\n");
        return EXIT_FAILURE;
    }
    //Comprobamos que la ruta destino se trata de un directorio
    if (argv[3][strlen(argv[3]) - 1] != '/')
    {
        fprintf(stderr, "Error: La ruta destino tiene que ser un directorio\n");
        return EXIT_FAILURE;
    }
    //Montamos el dispositivo
    if (bmount(argv[1]) < 0)
    {
        return EXIT_FAILURE;
    }
    //obtenemos informacion de los permisos del fichero que vamos a copiar a traves del mi_stat
    mi_stat(argv[2], &stat);
    //Obtenemos el nombre del fichero que se encuentra en la ruta original
    buffer = strrchr(argv[2], '/') + 1;
    //Copiamos la ruta destino que recibimos por parametro en un buffer ruta_destino
    strcpy(ruta_destino, argv[3]);
    //Concatenamos la ruta destino con el nombre del fichero que acabamos de extraer
    strcat(ruta_destino, buffer);
    //Creamos el ficheo en la ruta destino con todos los permisos temporalmente
    mi_creat(ruta_destino, 7); 
    //Ahora copiamos el contenido al nuevo fichero creado
    //Inicializamos a 0 el offset y el número de bytes leidos
    offset = 0;
    //Inicializamos a 0 el contenido del buffer de texto
    memset(buffer_texto, 0, tambuffer);
            //Verificamos si la entrada existe y obtenemos el valor de p_inodo
        if ((error = buscar_entrada(argv[2], &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)
        {
            mostrar_error_buscar_entrada(error);
            return error;
        }
    //Leemos el primer bloque
    leidos = mi_read_f(p_inodo, buffer_texto, offset, tambuffer);
    contador = leidos;
    //Bucle que se encarga de iterar por todo el contenido del fichero original
    while (leidos > 0)
    {
        //Guardamos el contenido del fichero en el fichero destino
        mi_write(ruta_destino, buffer_texto, offset, leidos);
        //Incrementamos el buffer
        offset += tambuffer;
        //Volvemos a inicializar el buffer para asi poder volver a leer posteriormente
        memset(buffer_texto, 0, tambuffer);
        leidos = mi_read_f(p_inodo, buffer_texto, offset, tambuffer);
        contador += leidos;
    }
    printf("CONTADOR: %d\n", contador);
    //Cambiamos los permisos de la ruta destino
    mi_chmod(ruta_destino, stat.permisos); 
    //Desmontamos el dispositivo
    bumount();
    return EXIT_SUCCESS;
}
