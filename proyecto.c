#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

// Definiciones Globales
#define NUM_TAREAS 100000000       // N grande: Sumar hasta 100 millones
#define NUM_HILOS 4                // Número de hilos a usar (T)

// Estructura para pasar parámetros a cada hilo
typedef struct {
    long long inicio;
    long long fin;
    double tiempo_ejecucion; // Tiempo que tarda este hilo
} ThreadData;

// Mutex para proteger la variable de tiempo total compartido (opcional, pero útil para fines de reporte)
pthread_mutex_t mutex_tiempo = PTHREAD_MUTEX_INITIALIZER;

// Función que cada hilo ejecutará
void *procesar_rango(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long long suma_parcial = 0;
    struct timespec start, end;
    
    // Medición del tiempo de ejecución del hilo
    clock_gettime(CLOCK_MONOTONIC, &start); // Recomendación para medición precisa [cite: 36]

    // Simulación de la tarea intensiva: Sumar los números en su rango asignado
    for (long long i = data->inicio; i <= data->fin; i++) {
        suma_parcial += i; 
        // Se puede añadir una operación compleja aquí para simular más carga:
        // double temp = sqrt(i) * log(i); 
    }

    // Fin de la medición
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Cálculo del tiempo de ejecución del hilo (en segundos)
    data->tiempo_ejecucion = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("Hilo %lu: Rango [%lld - %lld] terminado. Suma parcial: %lld. Tiempo: %.4f s\n", 
           pthread_self(), data->inicio, data->fin, suma_parcial, data->tiempo_ejecucion);

    pthread_exit(NULL);
}

int main() {
    pthread_t hilos[NUM_HILOS];
    ThreadData data[NUM_HILOS];
    long long elementos_por_hilo = NUM_TAREAS / NUM_HILOS;
    struct timespec inicio_total, fin_total;
    double tiempo_total_paralelo;
    
    // 1. Medición del Tiempo Total (Inicio)
    clock_gettime(CLOCK_MONOTONIC, &inicio_total);

    // 2. Asignación de Tareas y Creación de Hilos
    long long inicio_rango = 1;
    for (int i = 0; i < NUM_HILOS; i++) {
        data[i].inicio = inicio_rango;
        data[i].fin = inicio_rango + elementos_por_hilo - 1;
        
        // El último hilo toma el restante si NUM_TAREAS no es divisible
        if (i == NUM_HILOS - 1) {
            data[i].fin = NUM_TAREAS;
        }

        // Crear el hilo y asignarle la función y los datos
        if (pthread_create(&hilos[i], NULL, procesar_rango, &data[i]) != 0) {
            perror("Error al crear el hilo");
            return 1;
        }
        inicio_rango = data[i].fin + 1;
    }

    // 3. Esperar a que todos los hilos terminen (Sincronización)
    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }
    
    // 4. Medición del Tiempo Total (Fin)
    clock_gettime(CLOCK_MONOTONIC, &fin_total);

    // Cálculo del tiempo total paralelo
    tiempo_total_paralelo = (fin_total.tv_sec - inicio_total.tv_sec) + (fin_total.tv_nsec - inicio_total.tv_nsec) / 1000000000.0;

    printf("\n------------------------------------------------\n");
    printf("Tiempo Total de Ejecución (PARALELO con %d hilos): %.4f segundos\n", NUM_HILOS, tiempo_total_paralelo);
    printf("------------------------------------------------\n");

    // Opcional: Implementar y medir el tiempo secuencial aquí para la comparación 

    pthread_mutex_destroy(&mutex_tiempo);
    return 0;
}
