# NTHU Electronic System Level Design and Synthesis HW4 - RISC-V VP

- Cross-compile Gaussian Blur Filter to RISC-V VP platform.

## Gaussian Blur Filter with TLM interface

### Problem Formulation
- Port the Gaussian Blur Filter module to RISC-V VP.

### Implementaion

The Virtual Platform folder contains the vp to be put in risc-v vp. In Virtual Platform folder, GaussianFilter.h contains gaussian filter designed previously. The following code section contains the filter implementation.
```c
void do_filter(){
  { wait(CLOCK_PERIOD, SC_NS); }
  while (true) {
    val = 0;
    for (unsigned int v = 0; v < MASK_X; ++v) {
      for (unsigned int u = 0; u < MASK_Y; ++u) {
        unsigned char grey = (i_r.read() + i_g.read() + i_b.read()) / 3;
        val += grey * mask[u][v];
      }
    }
    double total = val / 273.;
    wait(431, SC_NS);
    int result = static_cast<int>(total);

    // cout << (int)result << endl;

    o_result.write(result);
  }
}
```

Software folder contains the software to be run on risc-v vp. The software will communicate with Filter Acclerator through DMA Interface.
```c
for(int i = 0; i < width; i++){
  for(int j = 0; j < height; j++){
    for(int v = -2; v <= 2; v ++){
      for(int u = -2; u <= 2; u++){
        if((v + i) >= 0  &&  (v + i ) < width && (u + j) >= 0 && (u + j) < height ){
          buffer[0] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 2);
          buffer[1] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 1);
          buffer[2] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 0);
          buffer[3] = 0;
        }else{
          buffer[0] = 0;
          buffer[1] = 0;
          buffer[2] = 0;
          buffer[3] = 0;
        }
        write_data_to_ACC(SOBELFILTER_START_ADDR, buffer, 4);
      }
    }
    read_data_from_ACC(SOBELFILTER_READ_ADDR, buffer, 4);

    memcpy(data.uc, buffer, 4);
    total = (data).sint;
    *(target_bitmap + bytes_per_pixel * (width * j + i) + 2) = total;
    *(target_bitmap + bytes_per_pixel * (width * j + i) + 1) = total;
    *(target_bitmap + bytes_per_pixel * (width * j + i) + 0) = total;
  }
}
```