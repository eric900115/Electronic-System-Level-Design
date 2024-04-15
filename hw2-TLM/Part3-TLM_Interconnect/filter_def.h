#ifndef FILTER_DEF_H_
#define FILTER_DEF_H_

// gaussian mask parameters
const int MASK_N = 1;
const int MASK_X = 5;
const int MASK_Y = 5;

// gaussian Filter inner transport addresses
// Used between blocking_transport() & do_filter()
const int GAUSSIAN_FILTER_R_ADDR = 0x00000000;
const int GAUSSIAN_FILTER_RESULT_ADDR = 0x00000004;
const int GAUSSIAN_FILTER_CHECK_ADDR = 0x00000008;

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
