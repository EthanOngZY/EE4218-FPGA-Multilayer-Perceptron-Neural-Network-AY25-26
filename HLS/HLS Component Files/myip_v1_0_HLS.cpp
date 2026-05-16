/*
----------------------------------------------------------------------------------
--	(c) Rajesh C Panicker, NUS,
--  Description : AXI Stream Coprocessor (HLS), implementing the sum of 4 numbers
--	License terms :
--	You are free to use this code as long as you
--		(i) DO NOT post a modified version of this on any public repository;
--		(ii) use it only for educational purposes;
--		(iii) accept the responsibility to ensure that your implementation does not violate any intellectual property of any entity.
--		(iv) accept that the program is provided "as is" without warranty of any kind or assurance regarding its suitability for any particular purpose;
--		(v) send an email to rajesh.panicker@ieee.org briefly mentioning its use (except when used for the course EE4218/CEG5203 at the National University of Singapore);
--		(vi) retain this notice in this file or any files derived from this.
----------------------------------------------------------------------------------
*/

/* 
HLS code to do everything up until converting final output to labels (comparing do in C code)
	activation function hardcoded
	assumes input array given in order of w_out -> w_hid -> input array
		biases of weights move to back
		input array no need add bias terms
*/


#include "hls_stream.h"
#include "ap_int.h"
#include "ap_axi_sdata.h"

// matrix dimensions
#define INPUT_ROWS		64
#define INPUT_COLUMNS	7
#define W_HID_ROWS		8
#define W_HID_COLUMNS	2
#define W_OUT_ROWS		3
#define W_OUT_COLUMNS	1

// array lengths
#define NUMBER_OF_INPUT_WORDS		INPUT_ROWS*INPUT_COLUMNS // length of input matrix
#define NUMBER_OF_W_HID_WORDS 		W_HID_ROWS*W_HID_COLUMNS  // length of hidden layer weights
#define NUMBER_OF_W_OUT_WORDS		W_OUT_ROWS*W_OUT_COLUMNS	// length of output layer weights
#define NUMBER_OF_TOTAL_IN_WORDS	NUMBER_OF_INPUT_WORDS+NUMBER_OF_W_HID_WORDS+NUMBER_OF_W_OUT_WORDS	// total input words
#define NUMBER_OF_OUTPUT_WORDS		INPUT_ROWS*W_OUT_COLUMNS  // length of an output vector

// ACLK, ARESETN, TREADY, TDATA, TVALID are essential signals for AXIS. New version of AXI DMA seems to expect TSTRB and/or TKEEP as well.

typedef ap_axis<32,0,0,0> AXIS;  //data, user, id, dest

// sigmoid array
int sig_arr[] = {12,12,12,12,13,13,13,14,14,14,15,15,15,16,16,16,17,17,18,18,18,19,19,20,20,21,21,21,22,22,23,23,24,24,25,26,26,27,27,28,28,29,30,30,31,32,32,33,34,34,35,36,36,37,38,39,39,40,41,42,43,44,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,66,67,68,69,70,72,73,74,75,76,78,79,80,82,83,84,86,87,88,90,91,92,94,95,97,98,99,101,102,104,105,107,108,110,111,113,114,116,117,119,120,122,123,125,126,128,129,130,132,133,135,136,138,139,141,142,144,145,147,148,150,151,153,154,156,157,158,160,161,163,164,165,167,168,169,171,172,173,175,176,177,179,180,181,182,183,185,186,187,188,189,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,211,212,213,214,215,216,216,217,218,219,219,220,221,221,222,223,223,224,225,225,226,227,227,228,228,229,229,230,231,231,232,232,233,233,234,234,234,235,235,236,236,237,237,237,238,238,239,239,239,240,240,240,241,241,241,242,242,242,243,243,243};

//supporting functions
int clamp_u8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

void sigmoid(int *inp_arr, int rows, int cols){
	#pragma HLS pipeline 
    for (int i = 0; i < rows*cols; i++){
        inp_arr[i] = sig_arr[inp_arr[i]];
    }
}

void matrix_mult_1(int *inp_arr, int *bias_arr, int *output_arr, int inp_rows, int inp_cols, int output_cols){
	#pragma HLS pipeline
    for (int i = 0; i < inp_rows; i++){
        for (int z = 0; z < output_cols; z++){
		#pragma HLS unroll
            int sum = 0;
            for (int j = 0; j < inp_cols; j++){
				#pragma HLS unroll factor=8 skip_exit_check
                sum += (int)inp_arr[i*inp_cols + j] * (int)bias_arr[j*output_cols + z];
            }
            sum += 255*(int)bias_arr[inp_cols*output_cols + z];
            output_arr[i*output_cols + z] = clamp_u8(sum >> 8);
        }
    }
}

void matrix_mult_2(int *inp_arr, int *bias_arr, int *output_arr, int inp_rows, int inp_cols, int output_cols){
	#pragma HLS pipeline
    for (int i = 0; i < inp_rows; i++){
	#pragma HLS unroll factor=8
        int sum = 0;
        for (int j = 0; j < inp_cols; j++){
			#pragma HLS unroll factor=2 skip_exit_check
            sum += (int)inp_arr[i*inp_cols + j] * (int)bias_arr[j*output_cols];
        }
        sum += 255*(int)bias_arr[inp_cols*output_cols];
        output_arr[i*output_cols] = clamp_u8(sum >> 8);
    }
}

void convert(int *output_arr){
	#pragma HLS pipeline II=4
    for (int i = 0; i < 64; i++){
        if (output_arr[i] < 128){
            output_arr[i] = 0;
        }
        else{
            output_arr[i] = 1;
        }
    }
}

void init_arr(int inp_arr[], int A_arr[], int w_hid_arr[], int w_out_arr[], int A_rows, int A_cols, int hid_cols, int out_rows) {
	int w_hid_offset = out_rows;
	int A_offset	 = out_rows + (A_cols+1)*hid_cols;
	int w_hid_size	 = (A_cols+1)*hid_cols;
	int A_size		 = A_rows * A_cols;

	for (int i = 0; i < out_rows; i++){
	#pragma HLS pipeline 
		w_out_arr[i] = inp_arr[i];
	}
	for (int j = 0; j < w_hid_size; j++) {
	#pragma HLS pipeline 
		w_hid_arr[j] = inp_arr[j + w_hid_offset];
	}
	for (int z = 0; z < A_size; z++) {
	#pragma HLS pipeline 
	#pragma HLS unroll factor=8
		A_arr[z] = inp_arr[z + A_offset];
	}
}

// main function
void myip_v1_0_HLS(hls::stream<AXIS>& S_AXIS, hls::stream<AXIS>& M_AXIS){
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=S_AXIS
#pragma HLS INTERFACE axis port=M_AXIS

	int word_cnt;
	int inp_arr[NUMBER_OF_TOTAL_IN_WORDS];
	int A[NUMBER_OF_INPUT_WORDS];
	#pragma HLS array_partition type=cyclic factor=7 variable=A
	int w_hid[NUMBER_OF_W_HID_WORDS];
	#pragma HLS array_partition variable=w_hid
	int w_out[NUMBER_OF_W_OUT_WORDS];
	#pragma HLS array_partition variable=w_out
	int A_1[INPUT_ROWS*W_HID_COLUMNS];
	#pragma HLS array_partition type=cyclic factor=2 variable=A_1
	int A_2[NUMBER_OF_OUTPUT_WORDS];
	#pragma HLS array_partition type=cyclic factor=8 variable=A_2
	
	
	ap_uint<8> sum = 0; // using arbitrary precision
	//int sum = 0;		 // using 32 bit precision
	AXIS read_input, write_output;

		myip_v1_0_HLS_for1:for(word_cnt = 0; word_cnt < NUMBER_OF_TOTAL_IN_WORDS; word_cnt++){
		#pragma HLS pipeline
			read_input = S_AXIS.read();
			// read_input is the element (data + other signals) received by our ip through S_AXIS in one clock cycle (which contains one word).
			// read() extracts it from the stream. Overloaded operator >> can also be used.
			inp_arr[word_cnt] = read_input.data; //extracting that word
			// We are not making using of S_AXIS_TLAST in this example.
			// S_AXIS_TLAST is required only when we are receiving an unknown number of words.
		}

		init_arr(inp_arr, A, w_hid, w_out, INPUT_ROWS, INPUT_COLUMNS, W_HID_COLUMNS, W_OUT_ROWS);
		
		matrix_mult_1(A, w_hid, A_1, INPUT_ROWS, INPUT_COLUMNS, W_HID_COLUMNS);

		sigmoid(A_1, INPUT_ROWS, W_HID_COLUMNS);

		matrix_mult_2(A_1, w_out, A_2, INPUT_ROWS, W_HID_COLUMNS, W_OUT_COLUMNS);

		convert(A_2);
		
		myip_v1_0_HLS_for2:for(word_cnt = 0; word_cnt < NUMBER_OF_OUTPUT_WORDS; word_cnt++){
		#pragma HLS pipeline
			//write_output.data = sum.to_int() + word_cnt;	// using arbitrary precision internally but int for interfacing
			write_output.data = A_2[word_cnt];	// using 32 bit precision or arbitrary precision all the way
			// write_output is the element sent by our ip through M_AXIS in one clock cycle.
			write_output.last = 0;
			write_output.keep = 0xFU;
            write_output.strb = 0xFU;
			if(word_cnt==NUMBER_OF_OUTPUT_WORDS-1)
			{
				write_output.last = 1;
				// M_AXIS_TLAST is required to be asserted for the last word.
				// Else, the AXI Stream FIFO / AXI DMA will not know if all the words have been received from the co-processor.
			}
			M_AXIS.write(write_output);
			// write() inserts it into the stream. Overloaded operator << can also be used.
		}
}
