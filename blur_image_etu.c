#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  int width;
  int height;
  int max;               // itensite max
  char type[2];          // P5,P6
  unsigned char *pixels; // pixmap
} Image;

#define K_SIZE 5
#define NB_PASS 10

// Noyau de convolution gaussien 5×5
float kernel[K_SIZE][K_SIZE] = {
    {1 / 256.0, 4 / 256.0, 6 / 256.0, 4 / 256.0, 1 / 256.0},
    {4 / 256.0, 16 / 256.0, 24 / 256.0, 16 / 256.0, 4 / 256.0},
    {6 / 256.0, 24 / 256.0, 36 / 256.0, 24 / 256.0, 6 / 256.0},
    {4 / 256.0, 16 / 256.0, 24 / 256.0, 16 / 256.0, 4 / 256.0},
    {1 / 256.0, 4 / 256.0, 6 / 256.0, 4 / 256.0, 1 / 256.0}};

/* Prototypes */
void read_pgm_image(FILE *file, Image *image);
void write_pgm_image(FILE *file, Image *image);
void gaussian_blur(Image *src, Image *trg);

int main(void) {
  FILE *f1, *f2;
  f1 = fopen("rose.pgm", "r");
  f2 = fopen("rose_blur.pgm", "w+");

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

  // 3- Appliquer NB_PASS fois le flou gaussien
  for (int pass = 0; pass < NB_PASS; pass++) {
    if (pass % 2 == 0) {
      gaussian_blur(&src, &tmp);
    } else {
      gaussian_blur(&tmp, &src);
    }
  }

  // 4- Écrire l'image résultat
  if (NB_PASS % 2 == 0) {
    write_pgm_image(f2, &src);
  } else {
    write_pgm_image(f2, &tmp);
  }

  // 5- Libérer la mémoire
  free(src.pixels);
  free(tmp.pixels);
  fclose(f1);
  fclose(f2);

  return 0;
}

/**
 * Fonction pour appliquer le flou gaussien
 *
 */
void gaussian_blur(Image *src, Image *trg) {
  int K = K_SIZE / 2;
  float sum;
  for (int iL = K; iL < src->height - K; iL++) {  // parcour des lignes
    for (int iC = K; iC < src->width - K; iC++) { // parcour des colonnes
      sum = 0.0;

      // Convolution avec le noiCau gaussien K_SIZEiLK_SIZE
      for (int j = 0; j < K_SIZE; j++) {
        for (int i = 0; i < K_SIZE; i++) {

          int pixelX = iC + i - K; // -2, -1, 0, 1, 2
          int pixelY = iL + j - K; // -2, -1, 0, 1, 2

          int index = pixelY * src->width + pixelX;

          sum += src->pixels[index] * kernel[j][i];
        }
      }

      if (sum > 255)
        sum = 255;
      if (sum < 0)
        sum = 0;

      trg->pixels[iL * trg->width + iC] = (unsigned char)sum;
    }
  }
}

/* Lit une image pgm sur le flux en paramtetre */
void read_pgm_image(FILE *file, Image *image) {
  fscanf(file, "%s\n%d %d\n%d\n", image->type, &image->width, &image->height,
         &image->max);
  fgetc(file);

  int size = image->width * image->height;

  image->pixels = malloc(size);
  fread(image->pixels, 1, size, file);
}

/* Ecrit une image sur un fic */
void write_pgm_image(FILE *file, Image *image) {
  fprintf(file, "%s\n%d %d\n%d\n", image->type, image->width, image->height,
          image->max);
  fwrite(image->pixels, 1, image->width * image->height, file);
}
