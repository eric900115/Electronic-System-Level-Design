#include <cmath>

#include "GaussianFilter.h"

GaussianFilter::GaussianFilter(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_filter);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);
}

// gaussian mask
const double mask[MASK_X][MASK_Y] = {
  {1, 4, 7, 4, 1},
  {4, 16, 26, 16, 4},
  {7, 26, 41, 26, 7},
  {4, 16, 26, 16, 4},
  {1, 4, 7, 4, 1} 
};

void GaussianFilter::do_filter() {
  
  while (true) {

    val = 0;

    for (unsigned int v = 0; v < MASK_Y; ++v) {
      for (unsigned int u = 0; u < MASK_X; ++u) {
        unsigned char grey = round((i_r.read() * 0.299 + i_g.read() * 0.587 + i_b.read() * 0.114));
        val += (double)grey * mask[v][u];
      }
    }

    int result = round(val/(double)273);
    o_result.write(result);

    wait(10); //emulate module delay
  }

}
