#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis: ./mi_ls <disco> </ruta_directorio>\n");
        return EXIT_FAILURE;
    }
    //Montamos el disco
    bmount(argv[1]);
    //Declaramos un buffer donde guardaremos los datos que visualizaremos
    char buffer[TAMBUFFER];
    //Inicializacmos el buffer a 0
    memset(buffer, 0, TAMBUFFER);

    int total = mi_dir(argv[2], buffer); //Llamamos al método

    if (total > 0) //Verificamos si hay entradas disponibles para visualizar
    {
        printf("Total: %d \n", total);
        printf("Tipo\tModo\t\tmTime\t\t\tTamaño\t\tNombre\n");
        printf("--------------------------------------------------------------------------------\n");

        printf("%s\n", buffer);
    }
    //Desmontamos el disco
    bumount();
    return EXIT_SUCCESS;
}