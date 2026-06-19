# HDL Improvements

Some simple improvements made to the original files my group submitted for the project. I am unable to check the functionality of these code on the actual board, since I do not have access to the board as of writing. Hence, I am checking the optimisation via simulation in Vivado. 

I am not going to check the correctness of intermediate results, but the final outputs are as expected.

## Version Differences

hidden_merged_states.v: merges the read1, wait1, mult1 and read2, wait2, mult2 into read1, wait1, mult1. This reduces the number of redudant cycles by concurrently calculating the required values instead of sequentially. The write1 and write2 states remain separate, since the memory RAM is single port.

Hidden and Sigmoid Merged: merges the previously separate hidden.v and sigmoid.v submodules into one submodule. Reduces number of RAMs required and simplifies the sigmoid activation function from the original number of states to reduce cycle-time.

## Time taken
Original: 502625 ns

hidden_merged_states.v: 349025 ns

Hidden and Sigmoid Merged: 310225 ns
