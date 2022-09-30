#include "ficheros.h"

/*-----------------------------------------------------------------------------------------------------------
 * FUNCTION:  mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
 * ----------------------------------------------------------------------------------------------------------
 * 
 * Escribe el contenido procedente de un buffer de memoria, buf_original, de tamaño nbytes, en un fichero/directorio 
 * (correspondiente al inodo pasado como argumento, ninodo)
 *
 * ninodo: Posición de inodo de la array de inodos
 * 
 * *buf_original: puntero que apunta al contenido del buffer de memoria que vamos a escribir
 * 
 * offset: posición de escritura inicial en bytes lógicos con respecto al ninodo
 * 
 * nbytes: número de bytes que vamos a escribir
 *
 * return: Hay que devolver la cantidad de bytes escritos realmente (Tiene que ser igual a nbytes)
 * 
*/

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    int primerBL, ultimoBl, desp1, desp2, nbfisico, numBytesEscritos = 0;

    leer_inodo(ninodo, &inodo); //Leemos el inodo
    //Verificamos primeramete si tiene permisos de escritura
    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, "NO TIENE PERMISO DE ESCRITURA \n");
        return numBytesEscritos;
    }
    else
    {
        //Verificamos de que bloque lógico a que bloque lógoco hay que escribir
        primerBL = offset / BLOCKSIZE;
        ultimoBl = (offset + nbytes - 1) / BLOCKSIZE;
        //Calculamos los desplazamientos dentro de estos bloques
        desp1 = offset % BLOCKSIZE;
        desp2 = (offset + nbytes - 1) % BLOCKSIZE;

        //Caso en que el primer y último bloque coincidan
        if (primerBL == ultimoBl)
        {
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1); //Obtenemos el número de bloque físico
            mi_signalSem();
            bread(nbfisico, buf_bloque);                           //Leemos el bloque físico del dispositivo
            memcpy(buf_bloque + desp1, buf_original, nbytes);
            numBytesEscritos = nbytes;
            bwrite(nbfisico, buf_bloque); //Escribimos el buf_bloque modifcado en el dispositivo
        }
        else //Caso en que la operación de escritura afecte a más de un bloque
        {
            //Primer bloque lógico
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1); //Obtenemos el número de bloque físico
            mi_signalSem();
            bread(nbfisico, buf_bloque);
            memcpy(buf_bloque + desp1, buf_original, (BLOCKSIZE - desp1)); //Los bytes restantes se han de copiar del buf_original al buf_bloque
            numBytesEscritos = numBytesEscritos + (BLOCKSIZE - desp1);
            bwrite(nbfisico, buf_bloque); //Escribimos el buf_bloque modificado en el dispositivo
            //Bloques lógicos intermedios
            for (int i = primerBL + 1; i < ultimoBl; i++)
            {
                mi_waitSem();
                nbfisico = traducir_bloque_inodo(ninodo, i, 1); //Obtenemos el número de bloque físico para cada bloque lógico intermedio
                mi_signalSem();
                //volcamos directamente en el dispositivo mediante un bwrite
                bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
                numBytesEscritos = numBytesEscritos + BLOCKSIZE;
            }
            //Último bloque lógico
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, ultimoBl, 1); //Obtenemos el número de bloque físico
            mi_signalSem();
            bread(nbfisico, buf_bloque);
            memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1); //Copiamos los bytes del bloque logico a buf_bloque
            numBytesEscritos = numBytesEscritos + (desp2 + 1);
            bwrite(nbfisico, buf_bloque); //Escribimos el buf_bloque modifcado en el dispositivo
        }
    }
    //Actualizamos la metainformación del inodo
    mi_waitSem();
    //Leemos el inodo actualizado
    leer_inodo(ninodo, &inodo);
    // tamEnBytesLog, solo si hemos escrito más allá del final del fichero, y por tanto el ctime
    if ((offset + nbytes) > inodo.tamEnBytesLog)
    {
        inodo.tamEnBytesLog = (offset + nbytes); //Modificamos el tamaño en bytes lógicos
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL); //Actualizar el mtime
    escribir_inodo(ninodo, inodo);
    mi_signalSem();
    //Devolvemos la cantidad de bytes escritos
    return numBytesEscritos;
}

/*-----------------------------------------------------------------------------------------------------------
 * FUNCTION:  mi_read_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
 * ----------------------------------------------------------------------------------------------------------
 * 
 * Lee información de un fichero/directorio (correspondiente al nº de inodo, ninodo, pasado como argumento) 
 * y la almacena en un buffer de memoria, buf_original
 *
 * ninodo: Posición de inodo de la array de inodos
 * 
 * *buf_original: puntero que apunta al contenido del buffer de memoria que vamos a leer
 * 
 * offset: posición de escritura inicial en bytes lógicos con respecto al ninodo
 * 
 * nbytes: número de bytes que vamos a escribir
 *
 * return: Hay que devolver la cantidad de bytes leidos realmente (Tiene que ser igual a nbytes)
 * 
*/

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    int primerBL, ultimoBl, desp1, desp2, nbfisico, bytesLeidos = 0;
    leer_inodo(ninodo, &inodo); //Leemos el inodo
    //Verificamos primeramete si tiene permisos de lectura
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, "NO TIENE PERMISO DE LECTURA \n");
        return bytesLeidos;
    }
    else
    {
        if (offset >= inodo.tamEnBytesLog)
        {
            bytesLeidos = 0; // No podemos leer nada ya que el offset nos cae más allá del tamaño en bytes lógicos
            return bytesLeidos;
        }
        if ((offset + nbytes) >= inodo.tamEnBytesLog)
        {                                          //Pretende leer más allá de EOF
            nbytes = inodo.tamEnBytesLog - offset; // leemos sólo los bytes que podemos desde el offset hasta EOF
        }
        //Verificamos de que bloque lógico a que bloque lógoco hay que escribir
        primerBL = offset / BLOCKSIZE;
        ultimoBl = (offset + nbytes - 1) / BLOCKSIZE;
        //Calculamos los desplazamientos dentro de estos bloques
        desp1 = offset % BLOCKSIZE;
        desp2 = (offset + nbytes - 1) % BLOCKSIZE;

        //Caso en que el primer y último bloque coincidan
        if (primerBL == ultimoBl)
        {
            nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0); //Obtenemos el número de bloque físico
            if (nbfisico != -1)
            {
                bread(nbfisico, buf_bloque); //Leemos el bloque físico del dispositivo
                memcpy(buf_original, buf_bloque + desp1, nbytes);
            }
            bytesLeidos = nbytes;
        }
        else //Caso en que la operación de escritura afecte a más de un bloque
        {
            //Primer bloque lógico
            nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0); //Obtenemos el número de bloque físico
            if (nbfisico != -1)
            {
                bread(nbfisico, buf_bloque);
                memcpy(buf_original, buf_bloque + desp1, (BLOCKSIZE - desp1)); //Los bytes restantes se han de copiar del buf_original al buf_bloque
            }
            bytesLeidos = bytesLeidos + (BLOCKSIZE - desp1); //Contabilizamos el número de bytes leidos
            //Bloques lógicos intermedios
            for (int i = primerBL + 1; i < ultimoBl; i++)
            {
                nbfisico = traducir_bloque_inodo(ninodo, i, 0); //Obtenemos el número de bloque físico para cada bloque lógico intermedio
                if (nbfisico != -1)
                {
                    bread(nbfisico, buf_bloque);
                    //volcamos directamente en el dispositivo mediante un bwrite
                    memcpy(buf_original, buf_bloque, (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
                }
                bytesLeidos += BLOCKSIZE;
            }
            //Último bloque lógico
            nbfisico = traducir_bloque_inodo(ninodo, ultimoBl, 0); //Obtenemos el número de bloque físico
            if (nbfisico != -1)
            {
                bread(nbfisico, buf_bloque);
                memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1); //Copiamos los bytes del bloque logico a buf_bloque
            }
            bytesLeidos = bytesLeidos + (desp2 + 1); //Contabilizamos el número de bytes leidos
        }
    }
    //Actualizamos la metainformación del inodo
    mi_waitSem();
    //Leemos el inodo actualizado
    leer_inodo(ninodo, &inodo);
    inodo.atime = time(NULL); //Actualizar el mtime
    escribir_inodo(ninodo, inodo);
    mi_signalSem();
    //Devolvemos la cantidad de bytes escritos
    return bytesLeidos;
}

/*-------------------------------------------------------------------
 * FUNCTION:  int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
 * ------------------------------------------------------------------
 * 
 * Devuelve la metainformación de un fichero/directorio (correspondiente al nº de inodo pasado como argumento):
 *
 * ninodo: Posición de inodo de la array de inodos
 * 
 * *p_stat: tipo estructurado struct STAT
 * 
*/

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;
    leer_inodo(ninodo, &inodo); //Leemos el inodo
    //Asignamos los campos al struct STAT con los valores del inodo
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return EXIT_SUCCESS;
}

/*-------------------------------------------------------------------
 * FUNCTION:  int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
 * ------------------------------------------------------------------
 * 
 * Cambia los permisos de un fichero/directorio 
 * (correspondiente al nº de inodo pasado como argumento, ninodo) con el valor que indique el argumento permisos.
 *
 * ninodo: Posición de inodo de la array de inodos
 * 
 * permisos: Caracter permiso (0-7)
 * 
*/

int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    struct inodo inodo;
    mi_waitSem();
    leer_inodo(ninodo, &inodo); //Leemos el inodo
    inodo.permisos = permisos;
    escribir_inodo(ninodo, inodo); //Salvamos el inodo
    mi_signalSem();
    return EXIT_SUCCESS;
}

/*-------------------------------------------------------------------
 * FUNCTION:  mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
 * -------------------------------------------------------------------
 * 
 * Trunca un fichero/directorio (correspondiente al nº de inodo, ninodo, pasado como argumento) 
 * a los bytes indicados como nbytes, liberando los bloques necesarios.
 *
 * ninodo: Posición de inodo de la array de inodos
 * 
 * nbytes: Numero debytes que tenemos que eliminar
 * 
 * return: La cantidad de bloques liberados.
 * 
*/

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    struct inodo inodo;
    int primerBL;
    leer_inodo(ninodo, &inodo);

    if ((inodo.permisos & 2) == 2)
    {
        if (inodo.tamEnBytesLog <= nbytes)
        {
            fprintf(stderr, "NO SE PUEDE TRUNCAR MÁS ALLÁ DEL TAMAÑO LOGICO DEL FICHERO \n");
            return -1;
        }
        //evitamos que se lea mas alla del tamaño en bytes logico del fichero
        if (nbytes % BLOCKSIZE == 0)
        { //buscamos primer bloque logico a liberar
            primerBL = nbytes / BLOCKSIZE;
        }
        else
        {
            primerBL = nbytes / BLOCKSIZE + 1;
        }
        mi_waitSem();
        int liberados = liberar_bloques_inodo(primerBL, &inodo);

        //actualizamos el inodo
        inodo.mtime = time(NULL);
        inodo.ctime = time(NULL);
        inodo.tamEnBytesLog = nbytes;
        inodo.numBloquesOcupados = inodo.numBloquesOcupados - liberados;

        //Escribimos el inodo
        escribir_inodo(ninodo, inodo);
        mi_signalSem();
        //fprintf(stdout, "liberados: %d \n", liberados);
        return liberados;
    }
    else
    {
        fprintf(stderr, "NO TIENE PERMISO DE ESCRITURA \n");
        return -1;
    }
}