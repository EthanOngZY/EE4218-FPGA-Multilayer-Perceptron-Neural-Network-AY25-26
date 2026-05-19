# HDL Implementation Notable Differences
### Dealing with Data
The input data given is in a 64x7 matrix, while the hidden layer weights are given in a 8x2 matrix, and the output layer weights are given in a 3x1 matrix. Notice that we cannot directly matrix multiply the input data and hidden layer weights. This is because we need to add a bias column to make the dimensions work. However, this takes up more storage space and would have been more annoying so we opted for a different method which is explained below.

### Testbench Module
This testbench is edited off the template provided by Prof. Rajesh. The notable difference here is the change in the checking part of the testbench. The original template expects that the output is completely correct when compared to the expected output. However, we expect that there will be one incorrect output due having to divide our values by 256 and losing the decimals. Hence, the output portion is rewritten to give the number of errors and the index of the errors as opposed to giving a pass/fail.

### Main Module
You'll notice bias terms bias_A, bias_B, bias_C for the bias terms of the hidden and output weights. This was done so that we do not need to add any bias columns to the input 64x7 matrix nor the intermediate 64x2 matrix. Doing it this way, we can instead multiply two "constants" together to add the bias term during the matrix mult of each row and column.

Another thing to notice is how we stored the data. For the hidden layer weights, we opted to save each column into its own RAM so that we could reuse code from earlier lab assignments. For the output layer weights, it's a single column matrix so it's saved in one RAM. For the input data, it's saved in one RAM with the first seven elements making up the first row, the next seven elements making up the second row, and so on.

Something that might look weird is the inclusion of a "Wait" state after the "ComputeHid" state. We did this since the data would not write completely without it in our previous lab assignments, so this state is here to buffer a cycle so that it can be written completely.

### Hidden Layer Matrix Mult Module
This submodule has a lot of states, since we duplicated the read, wait, mult, and write stages for the first column of the weights matrix for the second column of the weights matrix. I believe that it should be possible to combine at least the reading and multiplication stages, but I am not willing to test it at the moment. 

This submodule was also separated from the sigmoid and output submodules, so it saves the intermediate results to another RAM.

### Sigmoid Activation Function Module
Not much to add here, other than the LUT being manually assigned.

### Output Layer Matrix Mult Module
Not much to add here, should be fairly understandable if you understand the previous matrix mult module.

### Memory RAM Module
Not much different from the template RAM module provided. However, remember to edit this too if you are planning to implement dataflow optimisations.
