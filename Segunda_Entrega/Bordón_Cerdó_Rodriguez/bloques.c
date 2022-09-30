//Bloques.c
#include "bloques.h"

//Variable
static int descriptor = 0;

/*------------------------------------------------------
 * FUNCTION:  bmount(const char *camino)
 * -----------------------------------------------------
 * 
 * Función para montar el dispositivo virtual, 
 * y dado que se trata de un fichero, esa acción consistirá en abrirlo.
 *
 * *camino: puntero donde se encuentar la ruta a crear los bloques.
 *
 * return: EXIT_FAILURE si ha fallado o descriptor si se ha ejecutado
 * correctamente.
 * 
*/

int bmount(const char *camino)
{
    umask(000);
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    //Verficamos si se ha producido un error
    if (descriptor == -1)
    {
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    return descriptor;
}

/*------------------------------------------------------
 * FUNCTION:  bumount()
 * -----------------------------------------------------
 * 
 * Desmonta el dispositivo virtual
 *
 * return: EXIT_FAILURE si ha fallado o EXIT_SUCCESS si se ha ejecutado
 * correctamente.
 * 
*/

int bumount()
{
    if (close(descriptor) < 0)
    {
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*------------------------------------------------------
 * FUNCTION:  bwrite(unsigned int nbloque, const void *buf)
 * -----------------------------------------------------
 * 
 * Escribe 1 bloque en el dispositivo virtual,
 * en el bloque físico especificado por nbloque.
 *
 * nbloque: Numero de bloques que corresponde al dispositivo virtual
 * 
 * *buf: puntero donde se realiza el volcado del contenido del buffer
 * 
 * return: EXIT_FAILURE si ha fallado o bytes si se ha ejecutado
 * correctamente.
 * 
*/

int bwrite(unsigned int nbloque, const void *buf)
{

    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) < 0)
    {
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    size_t bytes = write(descriptor, buf, BLOCKSIZE);
    if (bytes < 0)
    {
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    return bytes;
}

/*------------------------------------------------------
 * FUNCTION:  bread(unsigned int nbloque, void *buf)
 * -----------------------------------------------------
 * 
 * Lee 1 bloque del dispositivo virtual,
 * que se corresponde con el bloque físico especificado por nbloque.
 *
 * nbloque: Numero de bloques que corresponde al dispositivo virtual
 * 
 * *buf: puntero donde se realiza el volcado del contenido del buffer
 * 
 * return: EXIT_FAILURE si ha fallado o bytes si se ha ejecutado
 * correctamente.
 * 
*/

int bread(unsigned int nbloque, void *buf)
{
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) < 0)
    {
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    size_t bytes = read(descriptor, buf, BLOCKSIZE);
    if (bytes < 0)
    {
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return EXIT_FAILURE;
    }
    return bytes;
}