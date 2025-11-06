#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Número de filósofos/tenedores
#define N 5
#define PENSAR 1
#define COMER 0

// Semáforos (mutex) para los tenedores
sem_t tenedores[N]; 
pthread_t filosofos[N];

// Función para simular el tiempo de pensar o comer
void *ciclo_filosofo(void *num);

// -------------------------------------------------------------------
// IMPLEMENTACIÓN DE LA REGLA ASIMÉTRICA PARA EVITAR DEADLOCK
// -------------------------------------------------------------------
void tomar_tenedores(int i) {
    int tenedor_izq = i;
    int tenedor_der = (i + 1) % N; // El tenedor de la derecha es el del vecino

    // Regla asimétrica: 
    // Los filósofos 0 a 3 (i != N-1) toman Izquierda, luego Derecha.
    // El filósofo 4 (i == N-1) toma Derecha, luego Izquierda.
    if (i < N - 1) { 
        // Tomar Tenedor Izquierdo
        sem_wait(&tenedores[tenedor_izq]);
        printf("Filósofo %d (PID: %lu) tomó el tenedor IZQUIERDO (%d).\n", i, pthread_self(), tenedor_izq);
        
        // Tomar Tenedor Derecho
        sem_wait(&tenedores[tenedor_der]);
        printf("Filósofo %d (PID: %lu) tomó el tenedor DERECHO (%d) y EMPIEZA A COMER.\n", i, pthread_self(), tenedor_der);
    } else { 
        // FILÓSOFO ASIMÉTRICO (N-1): Toma Derecha, luego Izquierda
        // Tomar Tenedor Derecho
        sem_wait(&tenedores[tenedor_der]);
        printf("Filósofo %d (PID: %lu) tomó el tenedor DERECHO (%d).\n", i, pthread_self(), tenedor_der);

        // Tomar Tenedor Izquierdo
        sem_wait(&tenedores[tenedor_izq]);
        printf("Filósofo %d (PID: %lu) tomó el tenedor IZQUIERDO (%d) y EMPIEZA A COMER.\n", i, pthread_self(), tenedor_izq);
    }
}

void dejar_tenedores(int i) {
    int tenedor_izq = i;
    int tenedor_der = (i + 1) % N;

    // Liberar Tenedor Derecho y luego Izquierdo (o al revés, no importa el orden de liberación)
    sem_post(&tenedores[tenedor_der]);
    sem_post(&tenedores[tenedor_izq]);
    printf("Filósofo %d (PID: %lu) terminó de comer y SOLTÓ los tenedores (%d y %d).\n", i, pthread_self(), tenedor_izq, tenedor_der);
}

void *ciclo_filosofo(void *num) {
    int *i = (int *)num;
    int contador_comidas = 0;

    while (contador_comidas < 3) { // Comerá 3 veces para el ejemplo
        // 1. PENSAR
        printf("Filósofo %d (PID: %lu) está PENSANDO.\n", *i, pthread_self());
        sleep(rand() % 3 + 1); 

        // 2. TENER HAMBRE
        printf("Filósofo %d (PID: %lu) TIENE HAMBRE y quiere comer.\n", *i, pthread_self());
        
        // 3. INTENTAR COMER
        tomar_tenedores(*i);
        
        // 4. COMER
        printf("\tFilósofo %d (PID: %lu) ESTÁ COMIENDO...\n", *i, pthread_self());
        sleep(rand() % 3 + 1); 
        contador_comidas++;
        
        // 5. DEJAR TENEDORES Y VOLVER A PENSAR
        dejar_tenedores(*i);
    }
    printf("Filósofo %d (PID: %lu) terminó su ciclo de comidas.\n", *i, pthread_self());
    return NULL;
}

int main() {
    int i, ids[N];
    srand(time(NULL));

    // Inicializar los semáforos (tenedores) con valor 1 (disponible)
    for (i = 0; i < N; i++) {
        sem_init(&tenedores[i], 0, 1);
        ids[i] = i; // Asignar ID al filósofo
    }
    
    // Crear los hilos (filósofos)
    for (i = 0; i < N; i++) {
        pthread_create(&filosofos[i], NULL, ciclo_filosofo, &ids[i]);
    }

    // Esperar a que todos los filósofos terminen de comer
    for (i = 0; i < N; i++) {
        pthread_join(filosofos[i], NULL);
    }

    // Destruir los semáforos
    for (i = 0; i < N; i++) {
        sem_destroy(&tenedores[i]);
    }
    
    printf("\nTodos los filósofos terminaron su cena.\n");
    return 0;
}
