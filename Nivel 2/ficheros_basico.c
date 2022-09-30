#include "ficheros_basico.h"
#define BLOCKSIZE 1024 // bytes

int tamMB(unsigned int nbloques){
    int resul = (nbloques/8)/BLOCKSIZE;
    if(((nbloques/8)%BLOCKSIZE)!=0){
        resul++;
    }
    return resul;
}

int tamAI(unsigned int ninodos){
    
    int resul= (ninodos*INODOSIZE)/BLOCKSIZE;
    if(((ninodos*INODOSIZE)%BLOCKSIZE)!=0){
        resul++;
    }
    return resul;
}

int initSB(unsigned int nbloques, unsigned int ninodos){
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
    SB.posUltimoBloqueDatos = nbloques-1;
    //Posición del inodo del directorio raíz en el array de inodos
    SB.posInodoRaiz = 0;
    //Posición del primer inodo libre en el array de inodos
    SB.posPrimerInodoLibre = 0;
    //Cantidad de bloques libres en el SF
    SB.cantBloquesLibres = nbloques;
    //Cantidad de inodos libres en el array de inodos
    SB.cantInodosLibres = ninodos;
    //Cantidad total de bloques
    SB.totBloques = nbloques;
    //Cantidad total de inodos
    SB.totInodos= ninodos;
    
    //Falta escribir el posSB mediante la funcion bwrite()
    bwrite(posSB, &SB);
    return EXIT_SUCCESS;    
}

int initMB(){
    //Declaramos un buffer de memoria
    unsigned char buffer [BLOCKSIZE];
    struct superbloque SB;
    bread(posSB, &SB);//Leemos el superbloque
    //Inicializamos a 0 el buffer de memoria
    memset(buffer, '0', BLOCKSIZE);

    for (int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++)
    {
        bwrite(i, buffer);
    }

    return EXIT_SUCCESS;
}
int initAI(){
    struct superbloque SB;
    bread(posSB, &SB);//Leemos el superbloque
    struct inodo inodos [BLOCKSIZE/INODOSIZE];
    int contInodos = SB.posPrimerInodoLibre + 1;
    bool fin = false;
    //si hemos inicializado SB.posPrimerInodoLibre = 0
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        for (int  j = 0; j < BLOCKSIZE/INODOSIZE && !fin ; j++)
        {
            inodos[j].tipo = 'l';//Libre
            if(contInodos < SB.totInodos){
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            }else{
                inodos[j].punterosDirectos[0] = UINT_MAX;
                    fin = true;//Salimos del fichero
            }
        }
        bwrite(i,inodos);
    }
    return EXIT_SUCCESS;
}