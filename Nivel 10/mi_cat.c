#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
        return EXIT_FAILURE;
    }
    if (bmount(argv[1]) < 0)
    {
        return EXIT_FAILURE;
    }
    //Declaramos las variable
    int offset, leidos, tambuffer = 1500;
    int contador = 0;
    char buffer_texto[tambuffer];
    char *camino = argv[2];
    //Comprobamos si se trata de un fichero
    if (argv[2][strlen(argv[2]) - 1] != '/')
    {
        offset = 0; //Inicializamos el offset
        //Inicializamos a el buffer de texto
        memset(buffer_texto, 0, tambuffer);
        leidos = mi_read(camino, buffer_texto, offset, tambuffer);
        contador = leidos;
        while (leidos > 0)
        {
            //Mostramos los resultados por la pantalla
            printf("%s", buffer_texto);
            offset = offset + tambuffer; //incrementamos el offset en funcion del tamaño del buffer
            memset(buffer_texto, 0, tambuffer);
            leidos = mi_read(camino, buffer_texto, offset, tambuffer);
            contador = contador + leidos;
        }
        printf("\n");
        //Mostramos el número de bytes leídos y del tamaño en bytes lógicos del inodo
        printf("Total_leido: %d\n", contador);
    }
    else
    {
        fprintf(stderr, "Error: la ruta se corresponde a un directorio\n");
        exit(-1);
    }

    //Desmontamos el fichero
    bumount();
    return EXIT_SUCCESS;
}