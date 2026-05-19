# HDL Implementation

The folder "EE4218 Project HDL" contains the Vivado code we used to create the component, while the folder "EE4218 HDL Test Code" contains the C++ code run in Vitis to test the functionality of the board.
Timer files not included, but they can be found elsewhere online.

This is not a very well optimised code, since we were busy with other courses. Some optimisations could include:
  1) Dataflow optimisations between the hidden layer and activation function.
  2) Combining states in the hidden layer, as some states are redundant and could possibly have been implemented together.
  3) Combining the hidden layer and activation layer to reduce the need to write to the RAM.

Other notes:
  1) The inputs are given in hexdecimal, which is different from the other implementations where we give the inputs in decimal.

## Files

tb_myip_v1_0.v: testbench.

  myip_v1_0_ethan.v: main/top module.

    hidden.v: hidden layer matrix mult.

    sigmoid.v: sigmoid LUT and activation function layer.

    output.v: output layer matrix mult.

    memory_RAM.v: RAM.

test_input.mem: input array.

test_result_expected.mem: expected outputs.

EE4218_HDL_wrapper.xsa: wrapper created with HDL component

