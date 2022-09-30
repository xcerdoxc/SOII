 #include "ficheros.h"

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
        return EXIT_FAILURE;
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
            nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1); //Obtenemos el número de bloque físico
            bread(nbfisico, buf_bloque);                           //Leemos el bloque físico del dispositivo
            memcpy(buf_bloque + desp1, buf_original, nbytes);
            numBytesEscritos = nbytes;
            bwrite(nbfisico, buf_bloque); //Escribimos el buf_bloque modifcado en el dispositivo
        }
        else //Caso en que la operación de escritura afecte a más de un bloque
        {
            //Primer bloque lógico
            nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1); //Obtenemos el número de bloque físico
            bread(nbfisico, buf_bloque);
            memcpy(buf_bloque + desp1, buf_original, (BLOCKSIZE - desp1)); //Los bytes restantes se han de copiar del buf_original al buf_bloque
            numBytesEscritos = numBytesEscritos + (BLOCKSIZE - desp1);
            bwrite(nbfisico, buf_bloque); //Escribimos el buf_bloque modificado en el dispositivo
            //Bloques lógicos intermedios
            for (int i = primerBL + 1; i < ultimoBl; i++)
            {
                nbfisico = traducir_bloque_inodo(ninodo, i, 1); //Obtenemos el número de bloque físico para cada bloque lógico intermedio
                //volcamos directamente en el dispositivo mediante un bwrite
                bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
                numBytesEscritos = numBytesEscritos + BLOCKSIZE;
            }
            //Último bloque lógico
            nbfisico = traducir_bloque_inodo(ninodo, ultimoBl, 1); //Obtenemos el número de bloque físico
            bread(nbfisico, buf_bloque);
            memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1); //Copiamos los bytes del bloque logico a buf_bloque
            numBytesEscritos = numBytesEscritos + (desp2 + 1);
            bwrite(nbfisico, buf_bloque); //Escribimos el buf_bloque modifcado en el dispositivo
        }
    }
    //Actualizamos la metainformación del inodo
    //Leemos el inodo actualizado
    leer_inodo(ninodo, &inodo);
    // tamEnBytesLog, solo si hemos escrito más allá del final del fichero, y por tanto el ctime
    if ((offset + nbytes) > inodo.tamEnBytesLog){
        inodo.tamEnBytesLog = (offset + nbytes);//Modificamos el tamaño en bytes lógicos
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL); //Actualizar el mtime
    escribir_inodo(ninodo, inodo);
    //Devolvemos la cantidad de bytes escritos
    return numBytesEscritos;
}
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    int primerBL, ultimoBl, desp1, desp2, nbfisico, bytesLeidos = 0;
    leer_inodo(ninodo, &inodo); //Leemos el inodo
    //Verificamos primeramete si tiene permisos de escritura
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, "NO TIENE PERMISO DE LECTURA \n");
        return EXIT_FAILURE;
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
            if (nbfisico != -1){
                bread(nbfisico, buf_bloque);                         //Leemos el bloque físico del dispositivo
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
                memcpy(buf_original,  buf_bloque + desp1, (BLOCKSIZE - desp1)); //Los bytes restantes se han de copiar del buf_original al buf_bloque

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
    //Leemos el inodo actualizado
    leer_inodo(ninodo, &inodo);
    inodo.atime = time(NULL); //Actualizar el mtime
    escribir_inodo(ninodo, inodo);
    //Devolvemos la cantidad de bytes escritos
    return bytesLeidos;
}
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);//Leemos el inodo
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
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    struct inodo inodo;
    leer_inodo(ninodo, &inodo); //Leemos el inodo
    inodo.permisos = permisos;
    escribir_inodo(ninodo, inodo); //Salvamos el inodo
    return EXIT_SUCCESS;
}