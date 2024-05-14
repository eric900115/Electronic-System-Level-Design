# NTHU Electronic System Level Design and Synthesis HW3 - High Level Synthesis

### Problem Formulation
- Baseline Implementation
    - Write a synthesizable design of SystemC Gaussian Filter modules.
- Improve coding styles
    - Please use bit width to constrain the operators.
    - Please simplify and reduce math expressions as much as possible, e.g., removing or simplifying multiplication and division, etc.
- Optimized Implementation
    - Please use Stratus directive of loop pipelining, unrolling, etc. to improve the throughput and latency of your design.

### Compile and Execute for Baseline Version
```shell
$ cd Basic/stratus
$ make sim_V_BASIC
```

### Compile and Execute for Optimization Version
```shell
$ cd Optimization/stratus
$ make sim_V_BASIC # Part 2 : Improved coding styles
$ make sim_V_PIPELINE2 # Part 3 : Loop PipeLine II = 2
$ make sim_V_PIPELINE # Part 3 : Loop PipeLine II = 1
$ make sim_V_UNROLL # Part 3 : Loop UNROLL
```

### Result
The program will generate result `out.bmp` in the `stratus` folder.

### Block Diagram
![image](https://github.com/eric900115/Electronic-System-Level-Design/blob/main/hw3-High_Level_Synthesis/img/BlockDiagram.png?raw=true)


### Implementaion
#### Improved Coding Style (Part2)
For Part 2, I've constrained the bit width of variables and operators. Also, I've simplified operations by extracting the common divisor.

#### Optimization Version (Part3)
For the optimization version, I've tried to pipeline the inter-level loop with II = 1 and II = 2, and I also tried to unroll the loop.

### Experiment - Latency & Area
The following diagram shows the latency and area of different versions of the implementation.

|          | Latency (ns) | Area |
| -------- | -------- | -------- |
| Baseline | 83898870 | 13065    |
| Improved Coding Style | 83243510 | 11097  |
| Pipeline (II = 2) | 70766070 | 11313    |
| Pipeline (II = 1) | 44564470 | 13276    |
| Unroll   | 28272630 | 11514    |


