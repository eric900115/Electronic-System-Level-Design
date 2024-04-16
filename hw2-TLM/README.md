# NTHU Electronic System Level Design and Synthesis HW2 - TLM

- Gaussian Blur Filter with TLM interface
- Gaussian Blur Filter with Quantum Keeper
- Gaussian Blur Filter with TLM interconnect

## Gaussian Blur Filter with TLM interface

### Compile and Execute
```shell
$ cd Part1-TLM_Interface
$ mkdir build && cd build
$ cmake ..
$ make run
```

### Problem Formulation
- Implement and wrap the Gaussian filter module (the data buffer version) with TLM interface.
- Insert delays with wait() in the target module.

### Block Diagram
![image](https://github.com/eric900115/Electronic-System-Level-Design/blob/main/hw2-TLM/img/BlockDiagram_Part1.png?raw=true)



### Implementaion
#### TLM Initiator

The initiator was declared in the constructor of `Testbench.cpp` and `Testbench.h`.

```C
class Testbench : public sc_module {
public:
  Initiator initiator;
  //...
 }
```

```C
Testbench::Testbench(sc_module_name n)
    : sc_module(n), initiator("initiator"), output_rgb_raw_data_offset(54) {
  SC_THREAD(do_gaussian);
}
```
In the `do_gaussian` function of `Testbench.cpp`, transactions are written through the initiator socket in Initiator module to send the pixels to be processed by Gaussian filter.

```C
void Testbench::do_gaussian() {

//...

    if(x == 0) {
        for (v = -yBound; v != yBound + adjustY; ++v) {
          for (u = -xBound; u != xBound + adjustX; ++u) {
            //...
            data.uc[0] = R;
            data.uc[1] = G;
            data.uc[2] = B;
            mask[0] = 0xff;
            mask[1] = 0xff;
            mask[2] = 0xff;

            if((v == -yBound) && (u == -xBound)) { // to notify this is the first element of the row
              data.uc[3] = 1;
              mask[3] = 0xff;
            }
            else {
              mask[3] = 0;
            }

            initiator.write_to_socket(GAUSSIAN_FILTER_R_ADDR, mask, data.uc, 4);
            wait(1 * CLOCK_PERIOD, SC_NS);
          }
        }
      }
      else {
        u = xBound + adjustX - 1;
        for (v = -yBound; v != yBound + adjustY; ++v) {
          //...
          data.uc[0] = R;
          data.uc[1] = G;
          data.uc[2] = B;
          mask[0] = 0xff;
          mask[1] = 0xff;
          mask[2] = 0xff;

          if(v == -yBound) { // to notify this is not the first element of current row
            data.uc[3] = 0;
            mask[3] = 0xff;
          }
          else {
            mask[3] = 0;
          }

          initiator.write_to_socket(GAUSSIAN_FILTER_R_ADDR, mask, data.uc, 4);
          wait(1 * CLOCK_PERIOD, SC_NS);
        }
      }
```
After the target module computes the Guassian filter, the initiator reads transactions to get the results.
```c
void Testbench::do_gaussian() {

//...
      initiator.read_from_socket(GAUSSIAN_FILTER_RESULT_ADDR, mask, data.uc, 4);
      total = data.sint;
      //debug
      //cout << "Now at " << sc_time_stamp() << endl; //print current sc_time

      *(target_bitmap + bytes_per_pixel * (width * y + x) + 2) = total;
      *(target_bitmap + bytes_per_pixel * (width * y + x) + 1) = total;
      *(target_bitmap + bytes_per_pixel * (width * y + x) + 0) = total;
    }
  }
  sc_stop();
}
```

#### TLM Target

In the `blocking_transport` function of target module (GaussianFilter.cpp), if the payload recived from initiator is in write mode (TLM_WRITE_COMMAND), the target function writes data recieved from initiator to the FIFO, and `GaussianFilter::do_filter()` function will calculate the result of convolution.

```c
GaussianFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
  //...
  switch (payload.get_command()) {
  //...
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

```

In the `blocking_transport` function of target module (GaussianFilter.cpp), if the payload recived from initiator is in read mode (TLM_READ_COMMAND), the target function reads data from the output of `GaussianFilter::do_filter()` function through FIFO and the initiator will read the computation results.
```c
void GaussianFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
  //...
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
```

#### Modules are connected by socket binding
Modules were connected in `main.cpp` by binding the initiator socket of Testbench to the taget sockets of GaussianFilter.
```C
Testbench tb("tb");
GaussianFilter gaussian_filter("gaussian_filter");
tb.initiator.i_skt(gaussian_filter.t_skt);
```

## Gaussian Blur Filter with Quantum Keeper
### Compile and Execute
```shell
$ cd Part2-Quantum_Keeper
$ mkdir build && cd build
$ cmake ..
$ make run
```

### Problem Formulation
- Please setup a quantum keeper to replace the wait() in the implementation.
- Does the quantum keeper version improves the simulation run time?


### Block Diagram
![image](https://github.com/eric900115/Electronic-System-Level-Design/blob/main/hw2-TLM/img/BlockDiagram_Part1.png?raw=true)

### Implementation

For this version, the wait() in the implementation will be replaced using quantum keeper.

Here, I'll domenstrate the code implemented in `Initiator` using time quantum.

The quantum keeper is setup in the constructor of Initiator module.
```c
Initiator::Initiator(sc_module_name n) : sc_module(n), i_skt("i_skt") {
  // Set the global quantum
  m_qk.set_global_quantum( sc_time(10, SC_NS) );
  m_qk.reset();
}
```
In the function in `Initiator::do_trans`, we replaced the `wait()` with the quantum keeper using `m_qk.inc(delay)` to add delay to quantum keeper and using `m_qk.sync()` to update systemC kernel counter.
```c
void Initiator::do_trans(tlm::tlm_generic_payload &trans) {
  
  //...

  i_skt->b_transport(trans, dummyDelay);

  //wait(dummyDelay);
  m_qk.inc( dummyDelay );
  if (m_qk.need_sync()) m_qk.sync();

}
```

In `testbench.cpp`, same technique is used to replace wait() with quantum keeper to reduce simulation time.

### Experiment
In this expeiment, I compare the execution time of Part1 (TLM Interface without Quatum Kepper) and Part2 (TLM Interface with Quatum Kepper). I use a 256x256 pixel image as benchmark and run Part1 and Part2 four times to observer the speedup in simulation speed using Quantum Keeper. The following table domenstrates the execution time (in microseconds) of Part1 and Part2.

| TLM Interface without Quatum Kepper| TLM Interface with Quatum Kepper | 
| -------- | -------- |
| 435163     | 454179     |
| 597666     | 393929     |
| 475301     | 387853     |
| 489807     | 307567     |

And, the following table shows the average running time (in microseconds) of Part1 and Part2.
| TLM Interface without Quatum Kepper| TLM Interface with Quatum Kepper | 
| -------- | -------- |
| 499484     | 385882     |

## Gaussian Blur Filter with TLM interconnect

### Compile and Execute
```shell
$ cd Part3-TLM_Interconnect
$ mkdir build && cd build
$ cmake ..
$ make run
```

### Problem Formulation
- Connect the quantum keeper modules of Gaussian Filter through a TLM bus.
- Insert counters in the transaction functions of TLM bus to count the number of read/write to the target module.

### Block Diagram
![image](https://github.com/eric900115/Electronic-System-Level-Design/blob/main/hw2-TLM/img/BlockDiagram_Part3.png?raw=true)

### Implementaion
Here, I'll domenstrate the code section of connection between Testbench (inititator) and GuassianFilter (target) module using bus.

```c
int main(){
  //...
  Testbench tb("tb");
  SimpleBus<1, 1> bus("bus");
  bus.set_clock_period(sc_time(CLOCK_PERIOD, SC_NS));
  GaussianFilter gaussian_filter("gaussian_filter");
  tb.initiator.i_skt(bus.t_skt[0]);
  bus.setDecode(0, GAUSSIAN_MM_BASE, GAUSSIAN_MM_BASE + GAUSSIAN_MM_SIZE - 1);
  bus.i_skt[0](gaussian_filter.t_skt);
  //...
}
```

### Experiment
In this experiment, I'll domonstrate the number of read and write operation observed in the transaction functions of TLM bus. For conducting this experiment, the following code section is added in `SimpleBus.h`.
```c
void initiatorBTransport(int SocketId, transaction_type &trans,
                           sc_core::sc_time &t) {
    //...
    // Calculate Number of transport during bus forward
    switch (trans.get_command()) {
      case tlm::TLM_READ_COMMAND:
        read_transaction_num++;
        break;
      case tlm::TLM_WRITE_COMMAND:
        write_transaction_num++;
        break;
      case tlm::TLM_IGNORE_COMMAND:
        break;
    }
    //...
}
```

The following figure shows the number of read and write operations.
![image](https://github.com/eric900115/Electronic-System-Level-Design/blob/main/hw2-TLM/img/output_part3.png?raw=true)