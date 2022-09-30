#include "ficheros_basico.h"

int main(int argc, char **argv)
{

    //Declaramos el camino o path
    char *camino = argv[1];
    //Numero de bloques
    int nbloques = atoi(argv[2]);
    //Declaramos un buffer de memoria
    unsigned char a[BLOCKSIZE];

    //Inicializamos a 0 el buffer de memoria
    memset(a, '0', BLOCKSIZE);
    //Montamos el fichero
    bmount(camino);

    //Numero de inodos
    int ninodos = nbloques / 4;
    tamMB(nbloques);
    tamAI(ninodos);

    //Inicializamos la estructura de datos
    initSB(nbloques, ninodos);
    initMB();
    initAI();

    //Creamos el directorio raiz que sera el inodo 0
    reservar_inodo('d', 7);

    //Demontamos el fichero
    bumount();

    return EXIT_SUCCESS;
}