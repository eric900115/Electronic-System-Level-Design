#include <iostream>
#include <string>
using namespace std;

// Wall Clock Time Measurement
#include <sys/time.h>

#include "GaussianFilter.h"
#include "Testbench.h"

// TIMEVAL STRUCT IS Defined ctime
// use start_time and end_time variables to capture
// start of simulation and end of simulation
struct timeval start_time, end_time;

// int main(int argc, char *argv[])
int sc_main(int argc, char **argv) {
  if ((argc < 3) || (argc > 4)) {
    cout << "No arguments for the executable : " << argv[0] << endl;
    cout << "Usage : >" << argv[0] << " in_image_file_name out_image_file_name"
         << endl;
    return 0;
  }

  Testbench tb("tb");
  GaussianFilter gaussian_filter("gaussian_filter");
  tb.initiator.i_skt(gaussian_filter.t_skt);

  tb.read_bmp(argv[1]);

  gettimeofday(&start_time, NULL);
  sc_start();
  gettimeofday(&end_time, NULL);

  unsigned long execution_time =  1000000 * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_usec - start_time.tv_usec;

  std::cout << "Total Execution Time for the Program : " << execution_time << " microseconds" << std::endl;

  std::cout << "Simulated time == " << sc_core::sc_time_stamp() << std::endl;
  tb.write_bmp(argv[2]);

  return 0;
}
