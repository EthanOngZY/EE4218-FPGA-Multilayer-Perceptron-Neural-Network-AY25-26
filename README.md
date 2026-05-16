# EE4218-FPGA-Multilayer-Perceptron-Neural-Network-AY25-26
Final project for EE4218 Sem 2 AY25/26, done by me and my teammates. Implemented on the Kria KV260 SOM Vision Starter Kit, with Vitis/Vivado versions 2025.1 and 2025.2.

In this project we implemented a simple MLP through software, Hardware Description Language (HDL), and High Level Synthesis (HLS) to work on an FPGA. This MLP takes in a 64x7 input matrix, an 8x2 layer 1 weights matrix, and a 3x1 output layer weights matrix and returns a 64x1 output matrix. In the original template data, the bias component of the weights matrices were the first element. For our implmentation, we moved that bias term to the last element. The activation function (in this case, sigmoid) was hardcoded.

This page was made to serve as reference for future students, should the course project remain similar. Do not copy the code directly, since it is not the best. If you do, remember to reference it properly.
