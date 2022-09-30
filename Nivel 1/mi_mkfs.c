#include "bloques.h"

int main(int argc, char **argv){
    //Declaramos el camino o path
    char *camino = argv[1];
    //Numero de bloques
    int nbloques = atoi(argv[2]);
    //Declaramos un buffer de memoria
    unsigned char a [BLOCKSIZE];
    //Inicializamos a 0 el buffer de memoria
    memset(a, '0', BLOCKSIZE);
    //Montamos el fichero 
    bmount(camino);
    for (int i = 0; i < nbloques; i++){
        bwrite(i, a); 
    }
    //Demontamos el fichero
    bumount();
    return EXIT_SUCCESS;
}