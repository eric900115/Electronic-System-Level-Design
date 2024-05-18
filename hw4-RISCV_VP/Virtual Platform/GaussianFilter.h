#ifndef GAUSSIAN_FILTER_H_
#define GAUSSIAN_FILTER_H_
#include <systemc>
#include <cmath>
#include <iomanip>
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include "filter_def.h"

struct GaussianFilter : public sc_module {
  tlm_utils::simple_target_socket<GaussianFilter> tsock;

  sc_fifo<unsigned char> i_r;
  sc_fifo<unsigned char> i_g;
  sc_fifo<unsigned char> i_b;
  sc_fifo<int> i_row_start;
  sc_fifo<int> o_result;

  SC_HAS_PROCESS(GaussianFilter);

  GaussianFilter(sc_module_name n): 
    sc_module(n), 
    tsock("t_skt"), 
    base_offset(0) 
  {
    tsock.register_b_transport(this, &GaussianFilter::blocking_transport);
    SC_THREAD(do_filter);
  }

  ~GaussianFilter() {
	}

  int val;
  unsigned int base_offset;

  void do_filter(){
    
    unsigned char buffer[5][5];

    while (true) {

      val = 0;

      bool row_start = i_row_start.read();
      if(row_start) {
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

      wait(10); //emulate module delay
    }
  }

  void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay){
    wait(delay);
    // unsigned char *mask_ptr = payload.get_byte_enable_ptr();
    // auto len = payload.get_data_length();
    tlm::tlm_command cmd = payload.get_command();
    sc_dt::uint64 addr = payload.get_address();
    unsigned char *data_ptr = payload.get_data_ptr();

    addr -= base_offset;


    // cout << (int)data_ptr[0] << endl;
    // cout << (int)data_ptr[1] << endl;
    // cout << (int)data_ptr[2] << endl;
    word buffer;

    switch (cmd) {
      case tlm::TLM_READ_COMMAND:
        // cout << "READ" << endl;
        switch (addr) {
          case GAUSSIAN_FILTER_RESULT_ADDR:
            buffer.uint = o_result.read();
            break;
          default:
            std::cerr << "READ Error! GaussianFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
          }
        data_ptr[0] = buffer.uc[0];
        data_ptr[1] = buffer.uc[1];
        data_ptr[2] = buffer.uc[2];
        data_ptr[3] = buffer.uc[3];
        break;
      case tlm::TLM_WRITE_COMMAND:
        // cout << "WRITE" << endl;
        switch (addr) {
          case GAUSSIAN_FILTER_R_ADDR:
            i_r.write(data_ptr[0]);
            i_g.write(data_ptr[1]);
            i_b.write(data_ptr[2]);
            i_row_start.write(data_ptr[3]);
            break;
          default:
            std::cerr << "WRITE Error! GaussianFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
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
};
#endif
