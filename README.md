# EE4218-FPGA-Multilayer-Perceptron-Neural-Network-AY25-26
Final project for EE4218 Sem 2 AY25/26, done by me and my teammates. Implemented on the Kria KV260 SOM Vision Starter Kit, with Vitis/Vivado versions 2025.1 and 2025.2.

In this project we implemented a simple MLP through software, Hardware Description Language (HDL), and High Level Synthesis (HLS) to work on an FPGA. This MLP takes in a 64x7 input matrix, an 8x2 hidden layer weights matrix, and a 3x1 output layer weights matrix and returns a 64x1 output matrix. In the original template data, the bias component of the weights matrices were the first element/row. For our implementation, we moved that bias term to the last element/row. The activation function (in this case, sigmoid) was hardcoded.
  * In order of cycles taken, software took the most (~7840), followed by HDL (~5800), and lastly HLS (~1440).
  * With the inputs and weights we used, we expect one error (at entry 9) from our expected results.

We used Realterm to send data and receive messages from the board.<br/>

This page was made to serve as reference for future students, should the course project remain similar. If you directly copy any code, remember to reference it properly.
