#include "ficheros_basico.h"

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


    //Numero de inodos
    int ninodos = nbloques/4;
    int y = tamMB(nbloques);
    int x = tamAI(ninodos);

    //Inicializamos la estructura de datos
    initSB(nbloques, ninodos);
    initMB();
    initAI();

    int posDatos = 1 + y + x;

    for (int i = posDatos; i < nbloques; i++){
        bwrite(i, a); 
    }
    //Demontamos el fichero
    bumount();
    
    return EXIT_SUCCESS;
}