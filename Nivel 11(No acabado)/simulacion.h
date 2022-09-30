#include "directorios.h"

#define NUMPROCESOS 10
#define NUMESCRITURAS 10
#define REGMAX  500000
struct REGISTRO { //sizeof(struct REGISTRO): 24
   time_t fecha; //Precisión segundos
   pid_t pid; //PID del proceso que lo ha creado
   int nEscritura; //Entero con el número de escritura (de 1 a 50)
   int nRegistro; //Entero con el número del registro dentro del fichero
};
void reaper();