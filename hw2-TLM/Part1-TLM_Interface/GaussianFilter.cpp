#include <cmath>
#include <iomanip>

#include "GaussianFilter.h"

GaussianFilter::GaussianFilter(sc_module_name n)
    : sc_module(n), t_skt("t_skt"), base_offset(0) {
  SC_THREAD(do_filter);

  t_skt.register_b_transport(this, &GaussianFilter::blocking_transport);
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

  unsigned char buffer[5][5];

  while (true) {

    val = 0;

    int is_row_start = i_row_start.read();
    if(is_row_start) {
      for (unsigned int v = 0; v < MASK_Y; ++v) {
        for (unsigned int u = 0; u < MASK_X; ++u) {
          unsigned char grey = round((i_r.read() * 0.299 + i_g.read() * 0.587 + i_b.read() * 0.114));
          buffer[v][u] = grey;
          val += (double)grey * mask[v][u];
        }
      }
    }
    else {
      for (unsigned int v = 0; v < MASK_Y; ++v) {
        for (unsigned int u = 0; u < MASK_X; ++u) {
          if(u != (MASK_X - 1)) {
            buffer[v][u] = buffer[v][u + 1]; // emulate shift register
            val += (double)buffer[v][u] * mask[v][u];
          }
          else {
            unsigned char grey = round((i_r.read() * 0.299 + i_g.read() * 0.587 + i_b.read() * 0.114));
            buffer[v][u] = grey;
            val += (double)grey * mask[v][u];
          }
        }
      }
    }

    int result = round(val/(double)273);
    o_result.write(result);

    //wait(10 * CLOCK_PERIOD, SC_NS); //May cause system to hang
  }
}

void GaussianFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
  sc_dt::uint64 addr = payload.get_address();
  addr = addr - base_offset;
  unsigned char *mask_ptr = payload.get_byte_enable_ptr();
  unsigned char *data_ptr = payload.get_data_ptr();
  word buffer;
  switch (payload.get_command()) {
  case tlm::TLM_READ_COMMAND:
    switch (addr) {
    case GAUSSIAN_FILTER_RESULT_ADDR:
      buffer.uint = o_result.read();
      break;
    case GAUSSIAN_FILTER_CHECK_ADDR:
      buffer.uint = o_result.num_available();
      break;
    default:
      std::cerr << "Error! GaussianFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
    data_ptr[0] = buffer.uc[0];
    data_ptr[1] = buffer.uc[1];
    data_ptr[2] = buffer.uc[2];
    data_ptr[3] = buffer.uc[3];
    delay = sc_time(5, SC_NS);
    break;

  case tlm::TLM_WRITE_COMMAND:
    switch (addr) {
    case GAUSSIAN_FILTER_R_ADDR:
      if (mask_ptr[0] == 0xff) {
        i_r.write(data_ptr[0]);
      }
      if (mask_ptr[1] == 0xff) {
        i_g.write(data_ptr[1]);
      }
      if (mask_ptr[2] == 0xff) {
        i_b.write(data_ptr[2]);
      }
      if (mask_ptr[3] == 0xff) {
        i_row_start.write(data_ptr[3]);
      }
      delay = sc_time(10, SC_NS);
      break;

    default:
      std::cerr << "Error! GaussianFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
    break;

  case tlm::TLM_IGNORE_COMMAND:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  default:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  }
  payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
}
