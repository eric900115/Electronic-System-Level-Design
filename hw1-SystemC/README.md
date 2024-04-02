# NTHU Electronic System Level Design and Synthesis HW1 - SystemC

## Gaussian blur filter with FIFO channels

### Compile and Execute
```shell
$ cd part1
$ mkdir build && cd build
$ cmake ..
$ make run
```

### Problem Formulation
- Implement a Gaussian Filter using SystemC module.
- Connect the testbench and Guassian Filter module using FIFO channels.

### Block Diagram
![image](https://github.com/eric900115/Electronic-System-Level-Design/blob/main/hw1-SystemC/img/BlockDiagram_Part1.png?raw=true)


### Implementaion
#### FIFO Channels
FIFO Channels facilitate data transfer between the testbench module and the Gaussian Filter module.
- The testbench sends data to the Filter module through the r, g, and b FIFO channels.
- After completing the convolution computation for each pixel using the Gaussian Filter, the filter module will transmit the result back to testbench through the result FIFO channel.

The code section provided below shows the declaration of FIFO channels defined within the `sc_main` function.
```C
sc_fifo<unsigned char> r;
sc_fifo<unsigned char> g;
sc_fifo<unsigned char> b;
sc_fifo<int> result;
 ```
#### GaussianFilter

The code section provided below demonstrates `do_filter` process within GuassianFilter module. This process reads a $5 \times 5$ pixel block from the image, received from the testbench through FIFO channels(r, g, b). The process then performs convolution operation with a $5 \times 5$ Gaussian Filter. After calculating the convolution operation, it outputs the result back to testbench through the result FIFO channel.

```C
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

```
#### Testbench
The provided code section below demonstrates the `do_gaussian` process within the Testbench module. This process send $5 \times 5$ pixel data blocks to the GaussianFilter module through FIFO channels for each iteration to calculate a single output pixel. After the computation, it reads the calculated result from the GuassianFilter module through the result FIFO channel.

```c
void Testbench::do_gaussian() {
  int x, y, v, u;        // for loop counter
  unsigned char R, G, B; // color of R, G, B
  int adjustX, adjustY, xBound, yBound;
  int total;

  o_rst.write(false);
  o_rst.write(true);

  for (y = 0; y != height; ++y) {
    for (x = 0; x != width; ++x) {
      adjustX = (MASK_X % 2) ? 1 : 0;
      adjustY = (MASK_Y % 2) ? 1 : 0;
      xBound = MASK_X / 2;
      yBound = MASK_Y / 2;

      for (v = -yBound; v != yBound + adjustY; ++v) {
        for (u = -xBound; u != xBound + adjustX; ++u) {
          if (x + u >= 0 && x + u < width && y + v >= 0 && y + v < height) {
            R = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 2);
            G = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 1);
            B = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 0);
          } else {
            R = 0;
            G = 0;
            B = 0;
          }
          o_r.write(R);
          o_g.write(G);
          o_b.write(B);
          wait(1); //emulate channel delay
        }
      }

      if(i_result.num_available()==0) 
        wait(i_result.data_written_event());
      
      result = i_result.read();

      *(target_bitmap + 1 * (width * y + x)) = result;
    }
  }
  sc_stop();
}
```

## Gaussian blur filter (with Data Buffers)
### Compile and Execute
```shell
$ cd part2
$ mkdir build && cd build
$ cmake ..
$ make run
```

### Problem Formulation
- Implement a Gaussian Filter using SystemC module, incorporating an additional buffer. 
- Utilize the buffer to reduce the data transfer between testbench and filter module.
- We only buffer the input pixels. The filter parameters are embedded inside the module.
- The buffer size is fixed and at most 25 pixels.
- You may choose to orgranize the buffer as row buffer (1x5, 1x16, 1x25), column buffer (5x1, 16x1, 25x1), or 2D buffer (4x4, 5x5).

### Block Diagram
![image](https://github.com/eric900115/Electronic-System-Level-Design/blob/main/hw1-SystemC/img/BlockDiagram_Part2.png?raw=true)

### Implementation

#### FIFO Channels
FIFO Channels facilitate data transfer between the testbench module and the Gaussian Filter module.
- The testbench sends data to the Filter module through the r, g, and b FIFO channels.
- After completing the convolution computation for each pixel using the Gaussian Filter, the filter module will transmit the result back to testbench through the result FIFO channel.
- An additional channel, row_start, is added to indicate whether it is the first element of a row.

The code section provided below shows the declaration of FIFO channels defined within the `sc_main` function.

```c
sc_fifo<unsigned char> r;
sc_fifo<unsigned char> g;
sc_fifo<unsigned char> b;
sc_fifo<bool> row_start;
sc_fifo<int> result;
```
#### GaussianFilter
The `do_filter` process within the GaussianFilter module is deisgned to apply Gaussian Blur to a 5x5 pixel data block extracted from an image. Here's the detailed discription to demonstrate how it works :

Initially, the process first prepares the 5x5 data pixel block which is going to be calculated. This process then performs a convolution operation on 5x5 data pixel block using a 5x5 Gaussian Filter. After the convolution calculation is completed, it send the result back to the testbench module through the result FIFO channel.

The code section provided below demonstrates `do_filter` process within GuassianFilter module. 
```c
void GaussianFilter::do_filter() {
  
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
```

#### Optimization to Reduce Data transfer

Here, I outline compare the difference in the methods to prepare $5 \times 5$ pixel data block between Part2 and Part1.

In part1, for each output pixel that needs processing, the GuassianFilter module reads a full $5 \times 5$ pixel block directly from the image. The $5 \times 5$ pixel block is received from the testbench through FIFO channels(r, g, b). 

However, in part2, an additional is $5 \times 5$ buffer is introduced to reduce data transfer. If the current processing element is the first element of a new row, the testbench sends a complete $5 \times 5$ pixel block, which is similar to part1. For subsequent pixels in the same row, the testbench sends only $5 \times 1$ pixel data block. This efficiency is possible because the remaining 5x4 pixels of the data block can be reused from buffer, which stores the data from previous cycle.

#### Testbench
The provided code section below illustrates the `do_gaussian` process within the Testbench module. This process send $5 \times 5$ pixel data blocks to the GaussianFilter module through FIFO channels for each iteration to calculate a single output pixel, if it is first element of new row. For the remaining elements in the row, the testbench send only $5 \times 1$ pixels. After the computation, it reads the calculated result from the GuassianFilter module through the result FIFO channel.
```C

void Testbench::do_guassian() {

  for (y = 0; y != height; ++y) {
    for (x = 0; x != width; ++x) {
      adjustX = (MASK_X % 2) ? 1 : 0;
      adjustY = (MASK_Y % 2) ? 1 : 0;
      xBound = MASK_X / 2;
      yBound = MASK_Y / 2;

      if(x == 0) {
        o_row_start.write(true);
        for (v = -yBound; v != yBound + adjustY; ++v) {
          for (u = -xBound; u != xBound + adjustX; ++u) {
            if (x + u >= 0 && x + u < width && y + v >= 0 && y + v < height) {
              R = *(source_bitmap +
                    bytes_per_pixel * (width * (y + v) + (x + u)) + 2);
              G = *(source_bitmap +
                    bytes_per_pixel * (width * (y + v) + (x + u)) + 1);
              B = *(source_bitmap +
                    bytes_per_pixel * (width * (y + v) + (x + u)) + 0);
            } else {
              R = 0;
              G = 0;
              B = 0;
            }
            o_r.write(R);
            o_g.write(G);
            o_b.write(B);
            wait(1); //emulate channel delay
          }
        }
      }
      else {
        o_row_start.write(false);
        u = xBound + adjustX - 1;
        for (v = -yBound; v != yBound + adjustY; ++v) {
          if (x + u >= 0 && x + u < width && y + v >= 0 && y + v < height) {
            R = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 2);
            G = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 1);
            B = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 0);
          } else {
            R = 0;
            G = 0;
            B = 0;
          }
          o_r.write(R);
          o_g.write(G);
          o_b.write(B);
          wait(1); //emulate channel delay
        }
      }

      if(i_result.num_available()==0) 
        wait(i_result.data_written_event());
      
      total = i_result.read();
      *(target_bitmap + 1 * (width * y + x)) = total;
    }
  }
  sc_stop();
}
```

## Discussion

### Compare the number of pixel transfer of between the original implementation and the one with buffers
- Part1 (without buffer) : To calculate each output pixel, it requires $5 \times 5 + 1$ data transfer. Considering there are $256 \times 256$ pixels in the case, it requires $256 \times 256 \times (5 \times 5 + 1)$ data tansfers.
- Part2 (with buffer) : To calculatethe first pixel of a row, it requires $5 \times 5 + 1$ data transfer. The remaining pixels require only $5 + 1$ data transfer. Given there are 256 rows and 256 columns, it requires $256 \times (5 \times 5 + 1) + 256 \times 255 \times (5 + 1)$ data tensfers.
- The amountof data transfer for Part1 is $\dfrac{256 \times 256 \times (5 \times 5 + 1)}{256 \times (5 \times 5 + 1) + 256 \times 255 \times (5 + 1)} = 4.28$ times greater than Part2.
