#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define FILENAME "log_incremento.txt"

int main()
{
    int pid;
    int contador = 0; // Variable a incrementar
    FILE *file;
    int i;

    // Paso 1: Inicialización y Encabezado del archivo por el padre
    file = fopen(FILENAME, "w"); // Abre/Crea el archivo
    if (file == NULL) {
        printf("Error al abrir el archivo!\n");
        return 1;
    }
    fprintf(file, "Registro de valores de la variable:\n");
    fclose(file); // Cierra para que el hijo pueda acceder

    pid = fork();

    switch(pid)
    {
        case -1:
            // Error
            printf("fatal: No se ha podido crear el proceso hijo\n");
            break;

        case 0:
            // Caso 0: Proceso hijo
            for(i = 1; i <= 10; i++) 
            {
                // Espera para dar tiempo al padre a que incremente.
                sleep(1); 

                // El hijo abre en modo "append" (a) para añadir la línea
                file = fopen(FILENAME, "a");
                if (file != NULL) {
                    // El hijo registra el valor esperado (i * 10)
                    fprintf(file, "Proceso hijo PID:%d, Valor registrado: %d\n", getpid(), i * 10);
                    fclose(file);
                }
            }
            printf("Proceso hijo PID:%d terminado. Valores registrados en %s\n", getpid(), FILENAME);
            break;

        default:
            // default: Proceso padre
            printf("Proceso padre PID:%d esperando y modificando la variable...\n", getpid());

            for(i = 1; i <= 10; i++)
            {
                // El padre incrementa la variable 'contador' en pasos de 10
                contador += 10; 
                printf("Proceso padre: Contador incrementado a %d\n", contador);

                // Espera para dar tiempo al hijo a que registre el valor.
                sleep(1); 
            }

            // Espera a que el proceso hijo termine
            wait(NULL); 

            printf("Proceso padre PID:%d terminado.\n", getpid());
            break;
    }

	    return 0;
}
