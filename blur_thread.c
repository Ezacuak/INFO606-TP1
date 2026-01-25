#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
  int width;
  int height;
  int max;               // intensité max
  char type[2];          // P5, P6
  unsigned char *pixels; // pixmap
} Image;

#define K_SIZE 5
#define NB_PASS 10
#define NUM_THREADS 4 // Nombre de threads

// Noyau de convolution gaussien 5×5
float kernel[K_SIZE][K_SIZE] = {
    {1 / 256.0, 4 / 256.0, 6 / 256.0, 4 / 256.0, 1 / 256.0},
    {4 / 256.0, 16 / 256.0, 24 / 256.0, 16 / 256.0, 4 / 256.0},
    {6 / 256.0, 24 / 256.0, 36 / 256.0, 24 / 256.0, 6 / 256.0},
    {4 / 256.0, 16 / 256.0, 24 / 256.0, 16 / 256.0, 4 / 256.0},
    {1 / 256.0, 4 / 256.0, 6 / 256.0, 4 / 256.0, 1 / 256.0}};

// Structure pour passer les données aux threads
typedef struct {
  Image *src;
  Image *trg;
  int start_line;
  int end_line;
  int thread_id;
} BlurData;

/* Prototypes */
void read_pgm_image(FILE *file, Image *image);
void write_pgm_image(FILE *file, Image *image);
void *gaussian_blur_thread(void *arg);
void gaussian_blur_parallel(Image *src, Image *trg);

int main(void) {
  FILE *f1, *f2;
  f1 = fopen("rose.pgm", "r");
  f2 = fopen("rose_blur_threaded.pgm", "w+");

  if (f1 == NULL || f2 == NULL) {
    printf("Erreur ouverture fichier\n");
    return 1;
  }

  Image src, tmp;

  // 1- Lire l'image source
  read_pgm_image(f1, &src);

  // 2- Initialiser l'image temporaire
  tmp = src;
  tmp.pixels = malloc(src.width * src.height);


  for (int pass = 0; pass < NB_PASS; pass++) {
    if (pass % 2 == 0) {
      gaussian_blur_parallel(&src, &tmp);
    } else {
      gaussian_blur_parallel(&tmp, &src);
    }
  }

  // 4- Écrire l'image résultat
  if (NB_PASS % 2 == 0) {
    write_pgm_image(f2, &src);
  } else {
    write_pgm_image(f2, &tmp);
  }

  printf("Image sauvegardée: rose_blur_threaded.pgm\n");

  // 5- Libérer la mémoire
  free(src.pixels);
  free(tmp.pixels);
  fclose(f1);
  fclose(f2);

  return 0;
}

/**
 * Fonction exécutée par chaque thread
 * Applique le flou gaussien sur un ensemble de lignes
 */
void *gaussian_blur_thread(void *arg) {
  BlurData *data = (BlurData *)arg;
  int K = K_SIZE / 2;

  // Parcourir les lignes assignées à ce thread
  for (int iL = data->start_line; iL < data->end_line; iL++) {
    for (int iC = K; iC < data->src->width - K; iC++) {
      float sum = 0.0;

      // Convolution avec le noyau gaussien 5×5
      for (int j = 0; j < K_SIZE; j++) {
        for (int i = 0; i < K_SIZE; i++) {
          // Position du pixel source
          int pixelX = iC + i - K;
          int pixelY = iL + j - K;

          // Index dans le tableau 1D
          int index = pixelY * data->src->width + pixelX;

          // Accumuler la somme pondérée
          sum += data->src->pixels[index] * kernel[j][i];
        }
      }

      // Écrire le pixel dans l'image cible
      // S'assurer que la valeur reste dans [0, 255]
      if (sum > 255)
        sum = 255;
      if (sum < 0)
        sum = 0;

      data->trg->pixels[iL * data->trg->width + iC] = (unsigned char)sum;
    }
  }

  return NULL;
}

/**
 * Version parallèle du flou gaussien
 * Divise l'image en bandes horizontales et les traite en parallèle
 */
void gaussian_blur_parallel(Image *src, Image *trg) {
  pthread_t threads[NUM_THREADS];
  BlurData thread_data[NUM_THREADS];
  int K = K_SIZE / 2;

  // Calculer le nombre de lignes à traiter (en excluant les bords)
  int total_lines = src->height - 2 * K;
  int lines_per_thread = total_lines / NUM_THREADS;

  // Créer les threads
  for (int t = 0; t < NUM_THREADS; t++) {
    thread_data[t].src = src;
    thread_data[t].trg = trg;
    thread_data[t].thread_id = t;

    // Calculer les lignes de début et de fin pour ce thread
    thread_data[t].start_line = K + t * lines_per_thread;

    if (t == NUM_THREADS - 1) {
      // Le dernier thread prend les lignes restantes
      thread_data[t].end_line = src->height - K;
    } else {
      thread_data[t].end_line = K + (t + 1) * lines_per_thread;
    }

    pthread_create(&threads[t], NULL, gaussian_blur_thread, &thread_data[t]);
  }

  // Attendre que tous les threads se terminent
  for (int t = 0; t < NUM_THREADS; t++) {
    pthread_join(threads[t], NULL);
  }
}

/* Lit une image PGM sur le flux en paramètre */
void read_pgm_image(FILE *file, Image *image) {
  fscanf(file, "%s\n%d %d\n%d\n", image->type, &image->width, &image->height,
         &image->max);
  fgetc(file); // Consommer le '\n' restant

  int size = image->width * image->height;
  image->pixels = malloc(size);
  fread(image->pixels, 1, size, file);
}

/* Écrit une image PGM sur un fichier */
void write_pgm_image(FILE *file, Image *image) {
  fprintf(file, "%s\n%d %d\n%d\n", image->type, image->width, image->height,
          image->max);
  fwrite(image->pixels, 1, image->width * image->height, file);
}
