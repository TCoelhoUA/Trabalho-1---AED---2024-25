// imageBWTest - A program that performs some image processing.
//
// This program is an example use of the imageBW module,
// a programming project for the course AED, DETI / UA.PT
//
// You may freely use and modify this code, NO WARRANTY, blah blah,
// as long as you give proper credit to the original and subsequent authors.
//
// The AED Team <jmadeira@ua.pt, jmr@ua.pt, ...>
// 2024

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imageBW.h"
#include "instrumentation.h"

int main(int argc, char* argv[]) {
  if (argc != 1) {
    fprintf(stderr, "Usage: %s  # no arguments required (for now)\n", argv[0]);
    exit(1);
  }

  // To initalize operation counters
  ImageInit();

  // Creating and displaying some images
  printf("white_image\n");
  Image white_image = ImageCreate(20, 25, WHITE);
  ImageRAWPrint(white_image);

  printf("black_image\n");
  Image black_image = ImageCreate(20, 25, BLACK);
  ImageRAWPrint(black_image);

  // Testes width
  printf("chess1\n");
  Image chess1 = ImageCreateChessboard(20, 25, 5, 1);
  ImageRAWPrint(chess1);

  printf("chess2\n");
  Image chess2 = ImageCreateChessboard(80, 25, 5, 1);
  ImageRAWPrint(chess2);

  printf("chess3\n");
  Image chess3 = ImageCreateChessboard(120, 25, 5, 1);
  ImageRAWPrint(chess3);
  //-------------------

  // Testes height
  printf("chess4\n");
  Image chess4 = ImageCreateChessboard(25, 20, 5, 1);
  ImageRAWPrint(chess4);

  printf("chess5\n");
  Image chess5 = ImageCreateChessboard(25, 80, 5, 1);
  ImageRAWPrint(chess5);

  printf("chess6\n");
  Image chess6 = ImageCreateChessboard(25, 120, 5, 1);
  ImageRAWPrint(chess6);
  //-------------------

  // Testes square_edge
  printf("chess7\n");
  Image chess7 = ImageCreateChessboard(100, 100, 2, 1);
  ImageRAWPrint(chess7);

  printf("chess8\n");
  Image chess8 = ImageCreateChessboard(100, 100, 5, 1);
  ImageRAWPrint(chess8);

  printf("chess9\n");
  Image chess9 = ImageCreateChessboard(100, 100, 10, 1);
  ImageRAWPrint(chess9);
  // -------------------

  // EQUAL (BLACK + BLACK)
  printf("black_image EQUAL black_image\n");
  printf("isEqual? %d <- (Should be 1)\n\n", ImageIsEqual(black_image, black_image));

  // NEG -> imageNEG
  printf("NEG white_image\n");
  Image imageNEG = ImageNEG(white_image);
  ImageRAWPrint(imageNEG);

  // AND (BLACK + CHESS) -> imageAND_BLACK
  printf("black_image AND chess1\n");
  Image imageAND_BLACK = ImageAND(black_image, chess1);
  ImageRAWPrint(imageAND_BLACK);

  // AND (WHITE + CHESS) -> imageAND_WHITE
  printf("white_image AND chess1\n");
  Image imageAND_WHITE = ImageAND(white_image, chess1);
  ImageRAWPrint(imageAND_WHITE);

  // OR (BLACK + CHESS) -> imageOR_BLACK
  printf("black_image OR chess1\n");
  Image imageOR_BLACK = ImageOR(black_image, chess1);
  ImageRAWPrint(imageOR_BLACK);

  // OR (WHITE + CHESS) -> imageOR_WHITE
  printf("white_image OR chess1\n");
  Image imageOR_WHITE = ImageOR(white_image, chess1);
  ImageRAWPrint(imageOR_WHITE);

  // XOR (BLACK + CHESS) -> imageXOR_BLACK
  printf("black_image XOR chess1\n");
  Image imageXOR_BLACK = ImageXOR(black_image, chess1);
  ImageRAWPrint(imageXOR_BLACK);

  // XOR (WHITE + CHESS) -> imageXOR_WHITE
  printf("white_image XOR chess1\n");
  Image imageXOR_WHITE = ImageXOR(white_image, chess1);
  ImageRAWPrint(imageXOR_WHITE);

  // REPB (WHITE + CHESS) -> imageREPB_WHITE
  printf("white_image REPB chess1\n");
  Image imageREPB_WHITE = ImageReplicateAtBottom(white_image, chess1);
  ImageRAWPrint(imageREPB_WHITE);

  // HMIRROR -> imageHMIRROR
  printf("HMIRROR imageREPB_WHITE\n");
  Image imageHMIRROR = ImageHorizontalMirror(imageREPB_WHITE);
  ImageRAWPrint(imageHMIRROR);

  // REPR (imageREPB_WHITE + imageHMIRROR) -> imageREPR
  printf("imageREPB_WHITE REPR imageHMIRROR\n");
  Image imageREPR = ImageReplicateAtRight(imageREPB_WHITE, imageHMIRROR);
  ImageRAWPrint(imageREPR);

  // VMIRROR -> imageVMIRROR
  printf("VMIRROR imageREPR\n");
  Image imageVMIRROR = ImageVerticalMirror(imageREPR);
  ImageRAWPrint(imageVMIRROR);

  // Testes para relatório - ImageAND() //
  Image white = ImageCreate(100, 100, WHITE);
  Image black = ImageCreate(100, 100, BLACK);

  printf("white AND chess7\n");
  Image tiny_white = ImageAND(white, chess7);
  printf("white AND chess8\n");
  Image mid_white = ImageAND(white, chess8);
  printf("white AND chess9\n");
  Image large_white = ImageAND(white, chess9);

  printf("black AND chess7\n");
  Image tiny_black = ImageAND(black, chess7);
  printf("black AND chess8\n");
  Image mid_black = ImageAND(black, chess8);
  printf("black AND chess9\n");
  Image large_black = ImageAND(black, chess9);
  // ---------------------------------- //

  // Images destruction
  ImageDestroy(&white_image);
  ImageDestroy(&black_image);

  ImageDestroy(&chess1);
  ImageDestroy(&chess2);
  ImageDestroy(&chess3);
  ImageDestroy(&chess4);
  ImageDestroy(&chess5);
  ImageDestroy(&chess6);
  ImageDestroy(&chess7);
  ImageDestroy(&chess8);
  ImageDestroy(&chess9);

  ImageDestroy(&imageNEG);

  ImageDestroy(&imageAND_BLACK);
  ImageDestroy(&imageAND_WHITE);

  ImageDestroy(&imageOR_BLACK);
  ImageDestroy(&imageOR_WHITE);

  ImageDestroy(&imageXOR_BLACK);
  ImageDestroy(&imageXOR_WHITE);

  ImageDestroy(&imageREPB_WHITE);
  ImageDestroy(&imageHMIRROR);
  ImageDestroy(&imageREPR);
  ImageDestroy(&imageVMIRROR);

  ImageDestroy(&white);
  ImageDestroy(&black);

  ImageDestroy(&tiny_white);
  ImageDestroy(&mid_white);
  ImageDestroy(&large_white);

  ImageDestroy(&tiny_black);
  ImageDestroy(&mid_black);
  ImageDestroy(&large_black);
  // -------------------------

  return 0;
}