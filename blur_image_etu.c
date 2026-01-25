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

// noyau de convolution
float kernel[K_SIZE][K_SIZE] = {
    {1 / 256.0, 4 / 256.0, 6 / 256.0, 4 / 256.0, 1 / 256.0},
    {4 / 256.0, 16 / 256.0, 24 / 256.0, 16 / 256.0, 4 / 256.0},
    {6 / 256.0, 24 / 256.0, 36 / 256.0, 24 / 256.0, 6 / 256.0},
    {4 / 256.0, 16 / 256.0, 24 / 256.0, 16 / 256.0, 4 / 256.0},
    {1 / 256.0, 4 / 256.0, 6 / 256.0, 4 / 256.0, 1 / 256.0}};

/* Lit un fichier sur un flux d'entrée */
void read_pgm_image(FILE *file, Image *image);

/* Ecrit un fichier sur un flux */
void write_pgm_image(FILE *file, Image *image);

/* Applique un flou gaussien sur l'image */
void gaussian_blur(Image *src, Image *trg);

int main(void) {
  FILE *f1, *f2;
  f1 = fopen("rose.pgm", "r");
  f2 = fopen("rose_blur.pgm", "w+");

  Image src, tmp;

  // 1- init la source
  read_pgm_image(f1, &src);

  // 2- init les images temporaires
  tmp = src;
  tmp.pixels = malloc(src.width * src.height);

  // 3- blur de src -> tmp
  for (int p = 0; p < NB_PASS; p++) {
    gaussian_blur(&src, &tmp);
  }

  // 4- ecrit l'image resultat apres NB_PASS passage
  write_pgm_image(f2, &tmp);

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

          int X = iC + i - K;
          int Y = iL + j - K;

          int idx = Y * src->width + X;
          sum += src->pixels[idx] * kernel[j][i];
        }
      }

      // Ecrire les pixels dans trg
      trg->pixels[trg->width * iL + iC] = src->pixels[trg->width * iL + iC];
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
