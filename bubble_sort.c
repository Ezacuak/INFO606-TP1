#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Trie à bulle simple.
 *
 * Verrification du temps d'execution en O(n²)
 *
 */

// la structure data sera utilisée pour le passage de parametre
// des threads
typedef struct {
  int *array;
  int start;
  int end;
} Data;

void *sort(void *arg);
int *random_array(int N);

int main(void) {

  struct timespec start, end;
  for (int i = 2; i <= (1 << 24); i *= 2) {

    int *T = random_array(i);
    Data d = {T, 0, i - 1};

    clock_gettime(CLOCK_MONOTONIC, &start);
    // Code à mesurer ici
    sort(&d);

    clock_gettime(CLOCK_MONOTONIC, &end);
    // Temps d'exécution
    double elapsed =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Affiche le temps d'éxécution sur la console
    printf("%d\t%.9f\n", i, elapsed);
  }

  return 0;
}

int *random_array(int N) {
  int *T = (int *)malloc(N * sizeof(int));
  // nb de 0 a N-1
  for (int i = 0; i < N; i++)
    T[i] = i;

  // suffle the T
  srand(time(NULL));

  // N interverti Nx
  int i1, i2, tmp;
  for (int i = 0; i < N; i++) {
    i1 = rand() % N;
    i2 = rand() % N;
    tmp = T[i1];
    T[i1] = T[i2];
    T[i2] = tmp;
  }

  return T;
}

/**
 * Algorithme de trie à bulle de complexité O(n²)
 *
 *
 */
void *sort(void *arg) {
  // recuperation des struct array, left, right
  Data *data = (Data *)arg;

  int start = data->start;
  int end = data->end;
  int *array = data->array;
  int tmp;

  for (int i = start; i < end; i++) {
    for (int j = 0; j < end -i ; j++) {
      if (array[j+1] < array[j]) {
        tmp = array[j+1];
        array[j+1] = array[j];
        array[j] = tmp;
      }
    }
  }

  return NULL;
}
