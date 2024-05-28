#ifndef FILTER_DEF_H_
#define FILTER_DEF_H_

#define CLOCK_PERIOD 10


#define MAX_IMAGE_BUFFER_LENTH 1024
#define THRESHOLD 90

// gaussian mask parameters
const int MASK_X = 5;
const int MASK_Y = 5;
const int DMA_TRANS = 64;

// gaussian Filter inner transport addresses
// Used between blocking_transport() & do_filter()
const int GAUSSIAN_FILTER_R_ADDR = 0x00000000;
const int GAUSSIAN_FILTER_RESULT_ADDR = 0x00000004;

const int GAUSSIAN_FILTER_RS_R_ADDR   = 0x00000000;
const int GAUSSIAN_FILTER_RS_W_WIDTH  = 0x00000004;
const int GAUSSIAN_FILTER_RS_W_HEIGHT = 0x00000008;
const int GAUSSIAN_FILTER_RS_W_DATA   = 0x0000000C;
const int GAUSSIAN_FILTER_RS_RESULT_ADDR = 0x00800000;


union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

// Gaussian mask
const double mask[MASK_X][MASK_Y] = {
  {1, 4, 7, 4, 1},
  {4, 16, 26, 16, 4},
  {7, 26, 41, 26, 7},
  {4, 16, 26, 16, 4},
  {1, 4, 7, 4, 1}
};
#endif
