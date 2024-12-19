/// imageBW - A simple image processing module for BW images
///           represented using run-length encoding (RLE)
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// The AED Team <jmadeira@ua.pt, jmr@ua.pt, ...>
/// 2024

// Student authors (fill in below):
// NMec: 118745
// Name: Tiago Coelho
// NMec: 119230
// Name: Bernardo Lázaro
//
// Date: 23/11/2024
//

#include "imageBW.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instrumentation.h"

// The data structure
//
// A BW image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the pointers
// to the RLE compressed image rows.
//
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Constant value --- Use them throughout your code
// const uint8 BLACK = 1;  // Black pixel value, defined on .h
// const uint8 WHITE = 0;  // White pixel value, defined on .h
const int EOR = -1;  // Stored as the last element of a RLE row

// Internal structure for storing RLE BW images
struct image {
  uint32 width;
  uint32 height;
  int** row;  // pointer to an array of pointers referencing the compressed rows
};

// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or
// file (I/O) operations use defensive techniques.
// When one of these functions fails,
// it immediately prints an error and exits the program.
// This fail-fast approach to error handling is simpler for the programmer.

// Use the following function to check a condition
// and exit if it fails.

// Check a condition and if false, print failmsg and exit.
static void check(int condition, const char* failmsg) {
  if (!condition) {
    perror(failmsg);
    exit(errno || 255);
  }
}

/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) {  ///
  InstrCalibrate();

  InstrName[0] = "pixmem";  // InstrCount[0] will count pixel array acesses
  // Name other counters here...
  InstrName[1] = "num_ands"; // InstrCount[1] contará o números de operações &&
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
// Add more macros here...

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!

/// Auxiliary (static) functions

/// Create the header of an image data structure
/// And allocate the array of pointers to RLE rows
static Image AllocateImageHeader(uint32 width, uint32 height) {
  assert(width > 0 && height > 0);
  Image newHeader = malloc(sizeof(struct image));
  check(newHeader != NULL, "malloc");

  newHeader->width = width;
  newHeader->height = height;

  // Allocating the array of pointers to RLE rows
  newHeader->row = malloc(height * sizeof(int*));
  check(newHeader->row != NULL, "malloc");

  return newHeader;
}

/// Allocate an array to store a RLE row with n elements
static int* AllocateRLERowArray(uint32 n) {
  assert(n > 2);
  int* newArray = malloc(n * sizeof(int));
  check(newArray != NULL, "malloc");

  return newArray;
}

/// Compute the number of runs of a non-compressed (RAW) image row
static uint32 GetNumRunsInRAWRow(uint32 image_width, const uint8* RAW_row) {
  assert(image_width > 0);
  assert(RAW_row != NULL);

  // How many runs?
  uint32 num_runs = 1;
  for (uint32 i = 1; i < image_width; i++) {
    if (RAW_row[i] != RAW_row[i - 1]) {
      num_runs++;
    }
  }

  return num_runs;
}

/// Get the number of runs of a compressed RLE image row
static uint32 GetNumRunsInRLERow(const int* RLE_row) {
  assert(RLE_row != NULL);

  // Go through the RLE_row until EOR is found
  // Discard RLE_row[0], since it is a pixel color

  uint32 num_runs = 0;
  uint32 i = 1;
  while (RLE_row[i] != EOR) {
    num_runs++;
    i++;
  }

  return num_runs;
}

/// Get the number of elements of an array storing a compressed RLE image row
static uint32 GetSizeRLERowArray(const int* RLE_row) {
  assert(RLE_row != NULL);

  // Go through the array until EOR is found
  uint32 i = 0;
  while (RLE_row[i] != EOR) {
    i++;
  }

  return (i + 1);
}

/// Compress into RLE format a RAW image row
/// Allocates and returns the array storing the image row in RLE format
static int* CompressRow(uint32 image_width, const uint8* RAW_row) {
  assert(image_width > 0);
  assert(RAW_row != NULL);

  // How many runs?
  uint32 num_runs = GetNumRunsInRAWRow(image_width, RAW_row);

  // Allocate the RLE row array
  int* RLE_row = malloc((num_runs + 2) * sizeof(int));
  check(RLE_row != NULL, "malloc");

  // Go through the RAW_row
  RLE_row[0] = (int)RAW_row[0];  // Initial pixel value
  uint32 index = 1;
  int num_pixels = 1;
  for (uint32 i = 1; i < image_width; i++) {
    if (RAW_row[i] != RAW_row[i - 1]) {
      RLE_row[index++] = num_pixels;
      num_pixels = 0;
    }
    num_pixels++;
  }
  RLE_row[index++] = num_pixels;
  RLE_row[index] = EOR;  // Reached the end of the row

  return RLE_row;
}

static uint8* UncompressRow(uint32 image_width, const int* RLE_row) {
  assert(image_width > 0);
  assert(RLE_row != NULL);

  // The uncompressed row
  uint8* row = (uint8*)malloc(image_width * sizeof(uint8));
  check(row != NULL, "malloc");

  // Go through the RLE_row until EOR is found
  int pixel_value = RLE_row[0];
  uint32 i = 1;
  uint32 dest_i = 0;
  while (RLE_row[i] != EOR) {
    // For each run
    for (int aux = 0; aux < RLE_row[i]; aux++) {
      row[dest_i++] = (uint8)pixel_value;
    }
    // Next run
    i++;
    pixel_value ^= 1;
  }

  return row;
}

// Add your auxiliary functions here...

/// Image management functions

/// Create a new BW image, either BLACK or WHITE.
///   width, height : the dimensions of the new image.
///   val: the pixel color (BLACK or WHITE).
/// Requires: width and height must be non-negative, val is either BLACK or
/// WHITE.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageCreate(uint32 width, uint32 height, uint8 val) {
  assert(width > 0 && height > 0);
  assert(val == WHITE || val == BLACK);

  Image newImage = AllocateImageHeader(width, height);

  // All image pixels have the same value
  int pixel_value = (int)val;

  // Creating the image rows, each row has just 1 run of pixels
  // Each row is represented by an array of 3 elements [value,length,EOR]
  for (uint32 i = 0; i < height; i++) {
    newImage->row[i] = AllocateRLERowArray(3);
    newImage->row[i][0] = pixel_value;
    newImage->row[i][1] = (int)width;
    newImage->row[i][2] = EOR;
  }

  return newImage;
}

/// Create a new BW image, with a perfect CHESSBOARD pattern.
///   width, height : the dimensions of the new image.
///   square_edge : the lenght of the edges of the sqares making up the
///   chessboard pattern.
///   first_value: the pixel color (BLACK or WHITE) of the
///   first image pixel.
/// Requires: width and height must be non-negative, val is either BLACK or
/// WHITE.
/// Requires: for the squares, width and height must be multiples of the
/// edge lenght of the squares
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageCreateChessboard(uint32 width, uint32 height, uint32 square_edge,
                            uint8 first_value) {
  // COMPLETE THE CODE
  // ...
  check(width%square_edge == 0, "A largura da imagem tem de ser múltipla do lado do quadrado!\n");
  check(height%square_edge == 0, "O comprimento da imagem tem de ser múltipla do lado do quadrado!\n");
  check(first_value == BLACK || first_value == WHITE, "first_valua tem de ser BLACK ou WHITE!\n");

  Image newImage = AllocateImageHeader(width, height);

  uint32 bitColor = first_value == BLACK ? 0:1; // aqui estamos a fazer invertido devido à verificação que
                                                //se segue no for loop h%3==0 (que alterna logo na primeira iteração)

  // criação da primeira linha xadrez (as próximas serão iguais apenas alternando o primeiro bit (bitColor)
  int n = width / square_edge + 2;
  int *first_row = malloc(n * sizeof(int));

  *(first_row) = bitColor;
  *(first_row + n - 1) = -1;
  for (int w=1; w<n-1; w++) {
    *(first_row + w) = square_edge;
  }

  for(uint32 h=0; h<height; h++) {
    if (h%square_edge == 0) { // alternamos a cor
      bitColor = bitColor == 0 ? 1:0;
      first_row[0] = bitColor;
    }
    
    newImage->row[h] = AllocateRLERowArray(n);
    memcpy(newImage->row[h], first_row, n * sizeof(int));

  }
  free(first_row);

  return newImage;
}


/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail.
void ImageDestroy(Image* imgp) {
  assert(imgp != NULL);

  Image img = *imgp;

  for (uint32 i = 0; i < img->height; i++) {
    free(img->row[i]);
  }
  free(img->row);
  free(img);

  *imgp = NULL;
}

/// Printing on the console

/// Output the raw BW image
void ImageRAWPrint(const Image img) {
  assert(img != NULL);

  printf("width = %d height = %d\n", img->width, img->height);
  printf("RAW image:\n");

  // Print the pixels of each image row
  for (uint32 i = 0; i < img->height; i++) {
    // The value of the first pixel in the current row
    int pixel_value = img->row[i][0];
    for (uint32 j = 1; img->row[i][j] != EOR; j++) {
      // Print the current run of pixels
      for (int k = 0; k < img->row[i][j]; k++) {
        printf("%d", pixel_value);
      }
      // Switch (XOR) to the pixel value for the next run, if any
      pixel_value ^= 1;
    }
    // At current row end
    printf("\n");
  }
  printf("\n");
}

/// Output the compressed RLE image
void ImageRLEPrint(const Image img) {
  assert(img != NULL);

  printf("width = %d height = %d\n", img->width, img->height);
  printf("RLE encoding:\n");

  // Print the compressed rows information
  for (uint32 i = 0; i < img->height; i++) {
    uint32 j;
    for (j = 0; img->row[i][j] != EOR; j++) {
      printf("%d ", img->row[i][j]);
    }
    printf("%d\n", img->row[i][j]);
  }
  printf("\n");
}

/// PBM BW file operations

// See PBM format specification: http://netpbm.sourceforge.net/doc/pbm.html

// Auxiliary function
static void unpackBits(int nbytes, const uint8 bytes[], uint8 raw_row[]) {
  // bitmask starts at top bit
  int offset = 0;
  uint8 mask = 1 << (7 - offset);
  while (offset < 8) {  // or (mask > 0)
    for (int b = 0; b < nbytes; b++) {
      raw_row[8 * b + offset] = (bytes[b] & mask) != 0;
    }
    mask >>= 1;
    offset++;
  }
}

// Auxiliary function
static void packBits(int nbytes, uint8 bytes[], const uint8 raw_row[]) {
  // bitmask starts at top bit
  int offset = 0;
  uint8 mask = 1 << (7 - offset);
  while (offset < 8) {  // or (mask > 0)
    for (int b = 0; b < nbytes; b++) {
      if (offset == 0) bytes[b] = 0;
      bytes[b] |= raw_row[8 * b + offset] ? mask : 0;
    }
    mask >>= 1;
    offset++;
  }
}

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PBM file.
/// Only binary PBM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageLoad(const char* filename) {  ///
  int w, h;
  char c;
  FILE* f = NULL;
  Image img = NULL;

  check((f = fopen(filename, "rb")) != NULL, "Open failed");
  // Parse PBM header
  check(fscanf(f, "P%c ", &c) == 1 && c == '4', "Invalid file format");
  skipComments(f);
  check(fscanf(f, "%d ", &w) == 1 && w >= 0, "Invalid width");
  skipComments(f);
  check(fscanf(f, "%d", &h) == 1 && h >= 0, "Invalid height");
  check(fscanf(f, "%c", &c) == 1 && isspace(c), "Whitespace expected");

  // Allocate image
  img = AllocateImageHeader(w, h);

  // Read pixels
  int nbytes = (w + 8 - 1) / 8;  // number of bytes for each row
  // using VLAs...
  uint8 bytes[nbytes];
  uint8 raw_row[nbytes * 8];
  for (uint32 i = 0; i < img->height; i++) {
    check(fread(bytes, sizeof(uint8), nbytes, f) == (size_t)nbytes,
          "Reading pixels");
    unpackBits(nbytes, bytes, raw_row);
    img->row[i] = CompressRow(w, raw_row);
  }

  fclose(f);
  return img;
}

/// Save image to PBM file.
/// On success, returns unspecified integer. (No need to check!)
/// On failure, does not return, EXITS program!
int ImageSave(const Image img, const char* filename) {  ///
  assert(img != NULL);
  int w = img->width;
  int h = img->height;
  FILE* f = NULL;

  check((f = fopen(filename, "wb")) != NULL, "Open failed");
  check(fprintf(f, "P4\n%d %d\n", w, h) > 0, "Writing header failed");

  // Write pixels
  int nbytes = (w + 8 - 1) / 8;  // number of bytes for each row
  // using VLAs...
  uint8 bytes[nbytes];
  // unit8 raw_row[nbytes*8];
  for (uint32 i = 0; i < img->height; i++) {
    // UncompressRow...
    uint8* raw_row = UncompressRow(nbytes * 8, img->row[i]);
    // Fill padding pixels with WHITE
    memset(raw_row + w, WHITE, nbytes * 8 - w);
    packBits(nbytes, bytes, raw_row);
    size_t written = fwrite(bytes, sizeof(uint8), nbytes, f);
    check(written == (size_t)nbytes, "Writing pixels failed");
    free(raw_row);
  }

  // Cleanup
  fclose(f);
  return 0;
}

/// Information queries

/// Get image width
int ImageWidth(const Image img) {
  assert(img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(const Image img) {
  assert(img != NULL);
  return img->height;
}

/// Image comparison

int ImageIsEqual(const Image img1, const Image img2) {
  assert(img1 != NULL && img2 != NULL);
  
  // COMPLETE THE CODE
  check(img1->width==img2->width, "A largura das imagens não é igual");
  check(img1->height==img2->height, "A altura das imagens não é igual");

  uint32 height = img1->height;

  // 1ªOpção (Mais eficiente?
  for (uint32 h=0; h<height; h++) {
    int col=0;
    do {
      if (img1->row[h][col] != img2->row[h][col]) {
        return 0;
      }
      col++;
    } while (img1->row[h][col] != -1);
  }

  // 2ªOpção?
  // int numRuns, size;
  // for (uint32 h=0; h<img1->height; h++) {
  //   size = GetSizeRLERowArray(img1->row[h]);
  //   for (uint32 col=0; col<size; col++) {  
  //     if (img1->row[h][col] != img2->row[h][col]) {
  //       return 0;
  //     }
  //   }
  // }
  return 1; //As imagens são iguais
}

int ImageIsDifferent(const Image img1, const Image img2) {
  assert(img1 != NULL && img2 != NULL);
  return !ImageIsEqual(img1, img2);
}

/// Boolean Operations on image pixels

/// These functions apply boolean operations to images,
/// returning a new image as a result.
///
/// Operand images are left untouched and must be of the same size.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)

Image ImageNEG(const Image img) {
  assert(img != NULL);

  uint32 width = img->width;
  uint32 height = img->height;

  Image newImage = AllocateImageHeader(width, height);

  for (uint32 i = 0; i < height; i++) {
    uint32 num_elems = GetSizeRLERowArray(img->row[i]);
    newImage->row[i] = AllocateRLERowArray(num_elems);
    memcpy(newImage->row[i], img->row[i], num_elems * sizeof(int));
    newImage->row[i][0] ^= 1; // negação do primeiro elemento (com xor, 1 xor 1 = 0, 0 xor 1 = 1)
  }

  return newImage;
}

// Função AND - 1ª Implementação (Não Otimizada)
// Image ImageAND(const Image img1, const Image img2) {
//   assert(img1 != NULL && img2 != NULL);

//   // COMPLETE THE CODE
//   // You might consider using the UncompressRow and CompressRow auxiliary files
//   // Or devise a more efficient alternative
//   // ...
//   check((img1->height == img2->height) && (img1->width == img2->width), "As imagens têm tamanhos diferentes!\n");

//   Image newImage = ImageCreate(img1->width, img1->height, WHITE);

//   for (uint32 i=0; i<img1->height; i++) {
//     uint8 *row1 = UncompressRow(img1->width, img1->row[i]);
//     uint8 *row2 = UncompressRow(img2->width, img2->row[i]);
//     uint8 *newImageRow = UncompressRow(newImage->width, newImage->row[i]);

//     for (uint32 j=0; j<img1->width; j++) {
//       // Se algum dos pixels for preto
//       if (row1[j] && row2[j]) {
//         newImageRow[j] = 1;
//       }
//     }
//     newImage->row[i] = CompressRow(newImage->width, newImageRow);
//   }
//   return newImage;
// }

// Função AND - 2ª Implementação (Otimizada)
Image ImageAND(const Image img1, const Image img2) {
    assert(img1 != NULL && img2 != NULL);

    // COMPLETE THE CODE
    // You might consider using the UncompressRow and CompressRow auxiliary files
    // Or devise a more efficient alternative
    // ...

    check((img1->height == img2->height) && (img1->width == img2->width), "As imagens têm tamanhos diferentes!\n");

    Image newImage = AllocateImageHeader(img1->width, img1->height);

    for (uint32 h = 0; h < img1->height; h++) {
        int r1 = GetNumRunsInRLERow(img1->row[h]);
        int r2 = GetNumRunsInRLERow(img2->row[h]);

        int cr1 = 1; // current run (valor) [começamos em 1]
        int cr2 = 1; // current run (valor) [começamos em 1]

        int cb1 = img1->row[h][0]; // current bit (cor)
        int cb2 = img2->row[h][0]; // current bit (cor)

        int val1 = img1->row[h][cr1]; // pixels restantes da run
        int val2 = img2->row[h][cr2]; // pixels restantes da run

        int n = 1;
        
        // Como não sabemos exatamente o tamanho da row, alocamos r1+r2+2
        // para garantir que não perdemos nenhum valor
        // no fim da row alocaremos a memória estritamente necessária
        // através do contador/index n
        newImage->row[h] = calloc((r1 + r2 + 2), sizeof(int));
        newImage->row[h][0] = cb1 && cb2;

        while (cr1 <= r1 && cr2 <= r2) {
            int cb = cb1 && cb2;  // color bit (da comparação atual)
            
            if (val1 > val2) {
                newImage->row[h][n] += val2;
                val1 -= val2;
                val2 = img2->row[h][++cr2];
                cb2 ^= 1;
            } else if (val1 < val2) {
                newImage->row[h][n] += val1;
                val2 -= val1;
                val1 = img1->row[h][++cr1];
                cb1 ^= 1;
            } else {
                newImage->row[h][n] += val1;
                val1 = img1->row[h][++cr1];
                val2 = img2->row[h][++cr2];
                cb1 ^= 1;
                cb2 ^= 1;
            }
            
            if (cb != (cb1 && cb2)) { // não trocamos de run se a próxima cor não for diferente
                n++;
            }
        }
        
        newImage->row[h][n+1] = EOR;
        newImage->row[h] = realloc(newImage->row[h], (n + 2) * sizeof(int));
    }
    return newImage;
}

Image ImageOR(const Image img1, const Image img2) {
  assert(img1 != NULL && img2 != NULL);

  // COMPLETE THE CODE
  // You might consider using the UncompressRow and CompressRow auxiliary files
  // Or devise a more efficient alternative
  // ...
    check((img1->height == img2->height) && (img1->width == img2->width), "As imagens têm tamanhos diferentes!\n");

    Image newImage = AllocateImageHeader(img1->width, img1->height);

    for (uint32 h = 0; h < img1->height; h++) {
        int r1 = GetNumRunsInRLERow(img1->row[h]);
        int r2 = GetNumRunsInRLERow(img2->row[h]);

        int cr1 = 1; // current run (valor) [começamos em 1]
        int cr2 = 1; // current run (valor) [começamos em 1]

        int cb1 = img1->row[h][0]; // current bit (cor)
        int cb2 = img2->row[h][0]; // current bit (cor)

        int val1 = img1->row[h][cr1]; // pixels restantes da run
        int val2 = img2->row[h][cr2]; // pixels restantes da run

        int n = 1;
        
        // Como não sabemos exatamente o tamanho da row, alocamos r1+r2+2
        // para garantir que não perdemos nenhum valor
        // no fim da row realocaremos a memória estritamente necessária
        // através do contador/index n
        newImage->row[h] = calloc((r1 + r2 + 2), sizeof(int));
        newImage->row[h][0] = cb1 || cb2;

        while (cr1 <= r1 && cr2 <= r2) {
            int cb = cb1 || cb2;  // color bit (da comparação atual)

            if (val1 > val2) {
                newImage->row[h][n] += val2;
                val1 -= val2;
                val2 = img2->row[h][++cr2];
                cb2 ^= 1;
            } else if (val1 < val2) {
                newImage->row[h][n] += val1;
                val2 -= val1;
                val1 = img1->row[h][++cr1];
                cb1 ^= 1;
            } else {
                newImage->row[h][n] += val1;
                val1 = img1->row[h][++cr1];
                val2 = img2->row[h][++cr2];
                cb1 ^= 1;
                cb2 ^= 1;
            }

            if (cb != (cb1 || cb2)) { // não trocamos de run se a próxima cor não for diferente
                n++;
                newImage->row[h][n] = 0;
            }
        }

        newImage->row[h][n+1] = EOR;
        newImage->row[h] = realloc(newImage->row[h], (n + 2) * sizeof(int));
    }

    return newImage;
}

Image ImageXOR(Image img1, Image img2) {
  assert(img1 != NULL && img2 != NULL);

  // COMPLETE THE CODE
  // You might consider using the UncompressRow and CompressRow auxiliary files
  // Or devise a more efficient alternative
  // ...
    check((img1->height == img2->height) && (img1->width == img2->width), "As imagens têm tamanhos diferentes!\n");

    Image newImage = AllocateImageHeader(img1->width, img1->height);

    for (uint32 h = 0; h < img1->height; h++) {
        int r1 = GetNumRunsInRLERow(img1->row[h]);
        int r2 = GetNumRunsInRLERow(img2->row[h]);

        int cr1 = 1; // current run (valor) [começamos em 1]
        int cr2 = 1; // current run (valor) [começamos em 1]

        int cb1 = img1->row[h][0]; // current bit (cor)
        int cb2 = img2->row[h][0]; // current bit (cor)

        int val1 = img1->row[h][cr1]; // pixels restantes da run
        int val2 = img2->row[h][cr2]; // pixels restantes da run

        int n = 1;
        
        // Como não sabemos exatamente o tamanho da row, alocamos r1+r2+2
        // para garantir que não perdemos nenhum valor
        // no fim da row alocaremos a memória estritamente necessária
        // através do contador/index n
        newImage->row[h] = calloc((r1 + r2 + 2), sizeof(int));
        newImage->row[h][0] = cb1 != cb2;

        while (cr1 <= r1 && cr2 <= r2) {
            int cb = cb1 != cb2;  // color bit (da comparação atual)

            if (val1 > val2) {
                newImage->row[h][n] += val2;
                val1 -= val2;
                val2 = img2->row[h][++cr2];
                cb2 ^= 1;
            } else if (val1 < val2) {
                newImage->row[h][n] += val1;
                val2 -= val1;
                val1 = img1->row[h][++cr1];
                cb1 ^= 1;
            } else {
                newImage->row[h][n] += val1;
                val1 = img1->row[h][++cr1];
                val2 = img2->row[h][++cr2];
                cb1 ^= 1;
                cb2 ^= 1;
            }

            if (cb != (cb1 != cb2)) { // não trocamos de run se a próxima cor não for diferente
                n++;
                newImage->row[h][n] = 0;
            }
        }

        newImage->row[h][n+1] = EOR;
        newImage->row[h] = realloc(newImage->row[h], (n + 2) * sizeof(int));
    }

    return newImage;
}

/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)

/// Mirror an image = flip top-bottom.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageHorizontalMirror(const Image img) {
  assert(img != NULL);

  uint32 width = img->width;
  uint32 height = img->height;

  Image newImage = AllocateImageHeader(width, height);

  for (uint32  h=0; h<height; h++) {
    int row_size = GetSizeRLERowArray(img->row[height-h-1]);
    newImage->row[h] = AllocateRLERowArray(row_size);
    memcpy(newImage->row[h], img->row[height-h-1], row_size * sizeof(int));
  }

  return newImage;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)

Image ImageVerticalMirror(const Image img) {
  assert(img != NULL);

  uint32 width = img->width;
  uint32 height = img->height;

  Image newImage = AllocateImageHeader(width, height);

  // COMPLETE THE CODE
  int size;
  for (uint32 h=0; h<height; h++) {

    size = GetSizeRLERowArray(img->row[h]);
    int cb = img->row[h][0];  // color bit
    cb = size%2==0 ? 1-cb:cb; // alteração de color bit caso size seja par (imagem espelhada começa com a cor oposta)
    
    newImage->row[h] = AllocateRLERowArray(size);
    newImage->row[h][0] = cb;
    for (int i=1; i<size-1; i++) {
      newImage->row[h][i] = img->row[h][size-1-i];
    }
    newImage->row[h][size-1] = EOR;
  }
  
  return newImage;
}
  
/// Replicate img2 at the bottom of imag1, creating a larger image
/// Requires: the width of the two images must be the same.
/// Returns the new larger image.
/// Ensures: The original images are not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageReplicateAtBottom(const Image img1, const Image img2) {
  assert(img1 != NULL && img2 != NULL);
  assert(img1->width == img2->width);

  uint32 new_width = img1->width;
  uint32 new_height = img1->height + img2->height;

  Image newImage = AllocateImageHeader(new_width, new_height);

  uint32 h1;
  int size;
  for(h1=0; h1<img1->height; h1++) {
    size = GetSizeRLERowArray(img1->row[h1]);
    newImage->row[h1] = AllocateRLERowArray(size);
    memcpy(newImage->row[h1], img1->row[h1], size * sizeof(int));
  }
  for (uint32 h2=0; h2<img2->height; h2++) {
    size = GetSizeRLERowArray(img2->row[h2]);
    newImage->row[h1+h2] = AllocateRLERowArray(size);
    memcpy(newImage->row[h1+h2], img2->row[h2], size * sizeof(int));
  }

  return newImage;
}

/// Replicate img2 to the right of imag1, creating a larger image
/// Requires: the height of the two images must be the same.
/// Returns the new larger image.
/// Ensures: The original images are not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageReplicateAtRight(const Image img1, const Image img2) {
  assert(img1 != NULL && img2 != NULL);
  assert(img1->height == img2->height);

  uint32 new_width = img1->width + img2->width;
  uint32 new_height = img1->height;

  Image newImage = AllocateImageHeader(new_width, new_height);

  // COMPLETE THE CODE
  // ...
  for (uint32 h=0; h<new_height; h++) {
    int s1 = GetSizeRLERowArray(img1->row[h]);
    int s2 = GetSizeRLERowArray(img2->row[h]);

    int isEven = s1%2==0 ? 1:0;
    int sameColor = img1->row[h][0] == img2->row[h][0] ? 1:0;  // img1 e img2 começam na mesma cor

    int n = s1 + s2 - (sameColor ? (isEven ? 2 : 3) : (isEven ? 3 : 2));

    newImage->row[h] = AllocateRLERowArray(n);

    int w1;
    for (w1=0; w1<s1-1; w1++) { // copia elementos de img1->row até '-1' (exclusive)
      newImage->row[h][w1] = img1->row[h][w1];
    }

    int w2 = 0;
    if ((isEven && !sameColor) || (!isEven && sameColor)) {
      newImage->row[h][w1-1] += img2->row[h][1];
      w1--;
      w2++;
    }

    while(w2<s2-1) {
      newImage->row[h][w1+w2] = img2->row[h][w2+1];
      w2++;
    }
  }
  return newImage;
}
