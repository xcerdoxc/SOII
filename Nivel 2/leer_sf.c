#include "ficheros_basico.h"

int main(int argc, char **argv)
{

    //Declaramos un array que sera el contenerdo de bread
    struct superbloque superBloque;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    //Montamos el fichero
    bmount(argv[1]);

    bread(posSB, &superBloque);
    printf("DATOS DEL SUPERBLOQUE: \n");
    printf("posPrimerBloqueMB = %d \n", superBloque.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n", superBloque.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n", superBloque.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", superBloque.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n", superBloque.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", superBloque.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n", superBloque.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n", superBloque.posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n", superBloque.cantBloquesLibres);
    printf("cantInodosLibres = %d\n", superBloque.cantInodosLibres);
    printf("totBloques = %d\n", superBloque.totBloques);
    printf("totInodos = %d\n", superBloque.totInodos);
    //Mostrar también el tamaño del struct inodo y del struct del superBloque:
    printf("\nsizeof struct superbloque is: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo is: %lu\n", sizeof(struct inodo));

    
    //recorrido de la lista de inodos libres
    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES \n");
    for (int i = superBloque.posPrimerBloqueAI; i <= superBloque.posUltimoBloqueAI; i++)
    {
        bread(i, &inodos);
        
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
                printf(" %d", inodos[j].punterosDirectos[0]);
        }
    }

    //Desmontamos el fichero
    bumount();

    return EXIT_SUCCESS;
}