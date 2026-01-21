#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
void *fusion(void *arg);
void print_array(Data *d);

int main(void) {

  struct timespec start, end;
  for (int i = 2; i <= (1 << 24); i *= 2) {

    int *T = random_array(i);
    Data d = {T, 0, i - 1};

    clock_gettime(CLOCK_MONOTONIC, &start);
    // Code à mesurer ici
    sort(&d);

    // print_array(&d);

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
 * Algorithme de trie fusion
 *
 *
 */
void *sort(void *arg) {
  Data *data = (Data *)arg;
  int start = data->start;
  int end = data->end;

  if (start < end) {
    int mid = start + (end - start) / 2;

    Data data_left = {data->array, start, mid};
    sort(&data_left);

    Data data_right = {data->array, mid + 1, end};
    sort(&data_right);

    fusion(data);
  }

  return NULL;
}

void *fusion(void *arg) {
  Data *data = (Data *)arg;

  int start = data->start;
  int end = data->end;
  int *array = data->array;

  int mid = start + (end - start) / 2;
  int n1 = mid - start + 1;
  int n2 = end - mid;

  int *L = (int *)malloc(n1 * sizeof(int));
  int *R = (int *)malloc(n2 * sizeof(int));

  for (int i = 0; i < n1; i++) {
    L[i] = array[start + i];
  }

  for (int j = 0; j < n2; j++) {
    R[j] = array[mid + 1 + j];
  }

  int i = 0;
  int j = 0;
  int k = start;

  while (i < n1 && j < n2) {
    if (L[i] <= R[j]) {
      array[k] = L[i];
      i++;
    } else {
      array[k] = R[j];
      j++;
    }
    k++;
  }

  while (i < n1) {
    array[k] = L[i];
    i++;
    k++;
  }

  while (j < n2) {
    array[k] = R[j];
    j++;
    k++;
  }

  free(L);
  free(R);
  return NULL;
}

void print_array(Data *data) {

  int start = data->start;
  int end = data->end;
  int *array = data->array;

  printf("start: %d, end: %d\n", start, end);

  for (int i = start; i < end; i++) {
    printf("%d ", array[i]);
  }
  printf("\n");
}
