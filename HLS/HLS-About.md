# HLS Implementation

The folder "HLS Component Files" contains the code we used that decided the logic of the component. 
The folder "Test Code" is the code we used to test the functionality of the board after we created the component. We made hardcoded and non-hardcoded versions, mainly for the sake of testing ease. No difference in function.
Timer files not included, but they can be found online elsewhere.

This is a fairly optimised implementation. Of the four optimisation techniques taught (unrolling, partitioning, pipelining, dataflow), the only one not used was the dataflow optimisation as that would have required reconsidering the logic of the code.
  * Note that pipelining is on by default for the versions we used.
