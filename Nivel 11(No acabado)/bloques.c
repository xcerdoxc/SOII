//Bloques.c
#include "bloques.h"
#include "semaforo_mutex_posix.h"

//Variable
static int descriptor = 0;
static sem_t *mutex;
static unsigned int inside_sc = 0;

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
    if (descriptor > 0)
    {
        close(descriptor);
    }
    if (!mutex)
    { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem();
        if (mutex == SEM_FAILED)
        {
            return EXIT_FAILURE;
        }
    }
    umask(000);
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    //Verficamos si se ha producido un error
    if (descriptor == -1)
    {
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        deleteSem();
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
    descriptor = close(descriptor);
    if (descriptor < 0)
    {
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        deleteSem();
        return EXIT_FAILURE;
    }
    deleteSem();//Borramos el semaforo
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

void mi_waitSem()
{
    if (!inside_sc)
    { // inside_sc==0
        waitSem(mutex);
    }
    inside_sc++;
}

void mi_signalSem()
{
    inside_sc--;
    if (!inside_sc)
    {
        signalSem(mutex);
    }
}
