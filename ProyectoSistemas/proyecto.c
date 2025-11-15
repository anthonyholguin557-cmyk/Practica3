#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define NUM_HILOS 4

// Lista de archivos a procesar (uno por hilo)
char *archivos[NUM_HILOS] = {
    "archivo1.txt",
    "archivo2.txt",
    "archivo3.txt",
    "archivo4.txt"
};

// Estructura por hilo
typedef struct {
    int id;
    char *nombre_archivo;
    long lineas_leidas;
    double tiempo_ejecucion;
} ThreadData;

pthread_mutex_t mutex_resultados = PTHREAD_MUTEX_INITIALIZER;

long lineas_totales_paralelo = 0;
double tiempo_total_hilos = 0;

// -----------------------------------------------------------
// FUNCIÓN DE HILO: procesar un archivo contando líneas
// -----------------------------------------------------------
void *procesar_archivo(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    FILE *f = fopen(data->nombre_archivo, "r");
    struct timespec t1, t2;

    if (!f) {
        printf("ERROR: No se pudo abrir %s\n", data->nombre_archivo);
        pthread_exit(NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);

    char buffer[1024];
    long contador = 0;

    while (fgets(buffer, sizeof(buffer), f)) {
        contador++;
        // Simulación de carga extra (opcional)
        // for (int i=0; i < 5000; i++) {}
    }

    fclose(f);

    clock_gettime(CLOCK_MONOTONIC, &t2);

    data->lineas_leidas = contador;
    data->tiempo_ejecucion = 
        (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1e9;

    // Zona crítica
    pthread_mutex_lock(&mutex_resultados);
    lineas_totales_paralelo += contador;
    tiempo_total_hilos += data->tiempo_ejecucion;
    pthread_mutex_unlock(&mutex_resultados);

    printf("Hilo %d procesó %s -> %ld líneas en %.4f s\n",
           data->id, data->nombre_archivo, contador, data->tiempo_ejecucion);

    pthread_exit(NULL);
}

// -----------------------------------------------------------
// PROCESAMIENTO SECUENCIAL DE TODOS LOS ARCHIVOS
// -----------------------------------------------------------
double procesamiento_secuencial() {
    long total_lineas = 0;
    struct timespec t1, t2;

    clock_gettime(CLOCK_MONOTONIC, &t1);

    for (int i = 0; i < NUM_HILOS; i++) {

        FILE *f = fopen(archivos[i], "r");
        if (!f) {
            printf("ERROR: No se pudo abrir %s\n", archivos[i]);
            continue;
        }

        char buffer[1024];
        long contador = 0;

        while (fgets(buffer, sizeof(buffer), f))
            contador++;

        fclose(f);

        printf("[Secuencial] Archivo %s -> %ld líneas\n", archivos[i], contador);

        total_lineas += contador;
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);

    printf("\nTotal líneas SECUENCIAL: %ld\n", total_lineas);

    return (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1e9;
}

// -----------------------------------------------------------
// MAIN
// -----------------------------------------------------------
int main() {
    pthread_t hilos[NUM_HILOS];
    ThreadData data[NUM_HILOS];

    struct timespec inicio, fin;

    // ------------------------------------------
    // PROCESAMIENTO PARALELO
    // ------------------------------------------
    printf("\n===== PROCESAMIENTO PARALELO =====\n");

    lineas_totales_paralelo = 0;
    tiempo_total_hilos = 0;

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int i = 0; i < NUM_HILOS; i++) {
        data[i].id = i;
        data[i].nombre_archivo = archivos[i];
        data[i].lineas_leidas = 0;

        pthread_create(&hilos[i], NULL, procesar_archivo, &data[i]);
    }

    // Esperar a que terminen
    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &fin);

    double tiempo_paralelo =
        (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("\nTotal líneas PARALELO: %ld\n", lineas_totales_paralelo);
    printf("Tiempo total paralelo: %.4f s\n", tiempo_paralelo);
    printf("Suma de tiempos individuales: %.4f s\n", tiempo_total_hilos);

    // ------------------------------------------
    // PROCESAMIENTO SECUENCIAL
    // ------------------------------------------
    printf("\n===== PROCESAMIENTO SECUENCIAL =====\n");

    double tiempo_secuencial = procesamiento_secuencial();
    printf("Tiempo total secuencial: %.4f s\n", tiempo_secuencial);

    // ------------------------------------------
    // COMPARACIÓN FINAL
    // ------------------------------------------
    printf("\n========== COMPARACIÓN ==========\n");
    printf("Paralelo:    %.4f s\n", tiempo_paralelo);
    printf("Secuencial:  %.4f s\n", tiempo_secuencial);
    printf("Speedup:     %.4f x\n", tiempo_secuencial / tiempo_paralelo);
    printf("==================================\n");

    pthread_mutex_destroy(&mutex_resultados);
    return 0;
}

