#ifndef GAUSSIAN_FILTER_H_
#define GAUSSIAN_FILTER_H_
#include <systemc>
using namespace sc_core;

#ifndef NATIVE_SYSTEMC
#include <cynw_p2p.h>
#endif

#include "filter_def.h"

class GaussianFilter: public sc_module
{
public:
	sc_in_clk i_clk;
	sc_in < bool > i_rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_uint<24> >::in i_rgb;
	cynw_p2p< bool >::in i_row_start;
	cynw_p2p< sc_dt::sc_uint<8> >::out o_result;
#else
	sc_fifo_in< sc_dt::sc_uint<24> > i_rgb;
	sc_fifo_in< bool > i_row_start;
	sc_fifo_out< sc_dt::sc_uint<8> > o_result;
#endif

	SC_HAS_PROCESS( GaussianFilter );
	GaussianFilter( sc_module_name n );
	~GaussianFilter();
private:
	void do_filter();
	bool is_row_start;
	sc_uint<10> buffer[5][5];
};
#endif
