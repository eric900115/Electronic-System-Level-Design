#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "GaussianFilter.h"

GaussianFilter::GaussianFilter( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	HLS_FLATTEN_ARRAY(buffer);
#endif
	SC_THREAD( do_filter );
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);
        
#ifndef NATIVE_SYSTEMC
	i_rgb.clk_rst(i_clk, i_rst);
  o_result.clk_rst(i_clk, i_rst);
	i_row_start.clk_rst(i_clk, i_rst);
#endif
}

GaussianFilter::~GaussianFilter() {}

// gaussian mask
/*const int mask[MASK_N][MASK_X][MASK_Y] = {{{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}},

                                    {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}}};*/

const sc_uint<6> mask[MASK_X][MASK_Y] = {
  {1, 4, 7, 4, 1},
  {4, 16, 26, 16, 4},
  {7, 26, 41, 26, 7},
  {4, 16, 26, 16, 4},
  {1, 4, 7, 4, 1} 
};


void GaussianFilter::do_filter() {
	
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_rgb.reset();
		o_result.reset();
		i_row_start.reset();
#endif
		wait();
	}

	while (true) {

		sc_uint<19> val = 0;

#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("input");
			is_row_start = i_row_start.get();
			wait();
		}
#else
		is_row_start = i_is_row_start.read();
#endif

		if(is_row_start) {
			for (unsigned int u = 0; u<MASK_Y; ++u) {
				for (unsigned int v = 0; v<MASK_X; ++v) {
#ifndef NATIVE_SYSTEMC
					#ifdef UNROLL
						HLS_UNROLL_LOOP(ON, "unroll");
					#endif
					#ifdef PIPELINE
						HLS_PIPELINE_LOOP(SOFT_STALL, II, "Loop" ); // PIPELINE == II
					#endif
#endif
					sc_dt::sc_uint<24> rgb;
#ifndef NATIVE_SYSTEMC
					{
						HLS_DEFINE_PROTOCOL("input");
						rgb = i_rgb.get();
						wait();
					}
#else
					rgb = i_rgb.read();
#endif
					{
						HLS_CONSTRAIN_LATENCY(0, 1, "ADD");
						sc_uint<10> grey = (rgb.range(7,0) + rgb.range(15,8) + rgb.range(23, 16));
						buffer[v][u] = grey;
					}
				}
			}
		}
		else {
			for (unsigned int u = 0; u<MASK_Y; ++u) {
				for (unsigned int v = 0; v<MASK_X; ++v) {
#ifndef NATIVE_SYSTEMC
					#ifdef UNROLL
						HLS_UNROLL_LOOP(ON, "unroll");
					#endif
					#ifdef PIPELINE
						HLS_PIPELINE_LOOP(SOFT_STALL, II, "Loop" ); // PIPELINE == II
					#endif
#endif
					if(u == (MASK_X - 1)) {
						sc_dt::sc_uint<24> rgb;
						
#ifndef NATIVE_SYSTEMC
						{
							HLS_DEFINE_PROTOCOL("input");
							rgb = i_rgb.get();
							wait();
						}
#else
						rgb = i_rgb.read();
#endif
						{
							HLS_CONSTRAIN_LATENCY(0, 1, "ADD");
							sc_uint<10> grey = (rgb.range(7,0) + rgb.range(15,8) + rgb.range(23, 16));
							buffer[v][u] = grey;
						}
					}
					else {
						buffer[v][u] = buffer[v][u + 1];
					}
				}
			}
		}

		for (unsigned int u = 0; u<MASK_Y; ++u) {
			for (unsigned int v = 0; v<MASK_X; ++v) {
#ifndef NATIVE_SYSTEMC
					#ifdef UNROLL
						HLS_UNROLL_LOOP(ON, "unroll");
					#endif
					#ifdef PIPELINE
						HLS_PIPELINE_LOOP(SOFT_STALL, II, "Loop" ); // PIPELINE == II
					#endif
#endif
				{
					HLS_CONSTRAIN_LATENCY(0, 1, "MAC");
					val += (buffer[v][u] * mask[v][u]);
				}
			}
		}

		sc_uint<8> out;
		{
			HLS_CONSTRAIN_LATENCY(0, 1, "DIV");
			out = (sc_uint<8>)(val / 819);
		}
		
#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("output");
			o_result.put(out);
			wait();
		}
#else
		o_result.write(out);
#endif
	}
}
