/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

/* EE4218 Project Software:
    uses the given inputs without adding bias column;
    "bias" treated as being added to the back instead of front (edit weights accordingly);
    no timing implemented;
    otherwise follows given template data
        activation function is hard-coded
*/

// calling libraries
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

// defining variables
#define INPUT_ROWS 64
#define INPUT_COLUMNS 7
#define W_HID_ROWS 8
#define W_HID_COLUMNS 2
#define W_OUT_COLUMNS 1


// data arrays
// input data without bias
int A[INPUT_ROWS][INPUT_COLUMNS] = {
{44, 90, 0, 0, 24, 81, 22},
{159, 250, 140, 176, 121, 183, 138},
{167, 158, 172, 134, 172, 161, 118},
{130, 178, 136, 120, 135, 121, 86},
{34, 112, 142, 112, 163, 159, 43},
{63, 28, 145, 126, 190, 165, 66},
{88, 214, 157, 140, 205, 174, 128},
{185, 203, 170, 110, 211, 138, 126},
{140, 255, 110, 136, 133, 156, 131},
{73, 90, 164, 118, 75, 121, 111},
{16, 93, 187, 88, 162, 101, 27},
{31, 110, 120, 77, 166, 101, 45},
{183, 181, 155, 175, 135, 211, 185},
{129, 193, 104, 159, 184, 183, 161},
{190, 183, 143, 124, 139, 138, 145},
{79, 113, 115, 111, 164, 101, 88},
{213, 213, 149, 159, 133, 199, 183},
{13, 94, 127, 146, 132, 150, 89},
{103, 175, 164, 139, 187, 147, 128},
{42, 82, 104, 83, 167, 50, 71},
{26, 90, 109, 143, 135, 220, 106},
{69, 90, 101, 180, 125, 222, 108},
{143, 225, 125, 147, 196, 197, 121},
{73, 87, 125, 29, 8, 87, 67},
{100, 136, 254, 119, 170, 140, 77},
{110, 136, 101, 137, 186, 174, 126},
{165, 143, 179, 151, 167, 156, 147},
{182, 207, 131, 105, 130, 101, 124},
{34, 51, 194, 94, 90, 94, 58},
{108, 85, 116, 25, 24, 0, 59},
{18, 70, 110, 118, 179, 138, 50},
{35, 165, 120, 72, 126, 72, 82},
{72, 99, 85, 76, 210, 101, 56},
{27, 85, 85, 109, 122, 133, 54},
{149, 166, 176, 111, 135, 115, 98},
{138, 180, 136, 131, 255, 139, 84},
{120, 97, 115, 96, 110, 128, 44},
{183, 181, 183, 152, 119, 174, 148},
{38, 113, 125, 67, 88, 26, 68},
{180, 188, 169, 137, 188, 124, 145},
{86, 87, 80, 72, 76, 73, 71},
{219, 224, 155, 165, 197, 252, 218},
{31, 65, 145, 38, 112, 32, 78},
{21, 134, 48, 83, 94, 78, 111},
{111, 62, 128, 89, 163, 209, 65},
{224, 206, 128, 155, 167, 170, 150},
{231, 225, 139, 174, 149, 202, 208},
{140, 146, 106, 124, 192, 142, 104},
{116, 191, 196, 136, 192, 170, 108},
{43, 136, 131, 58, 44, 50, 118},
{39, 76, 130, 63, 71, 62, 39},
{103, 166, 170, 115, 236, 131, 75},
{83, 148, 206, 120, 142, 156, 102},
{73, 41, 150, 63, 123, 78, 51},
{159, 136, 93, 153, 140, 149, 198},
{14, 77, 160, 67, 68, 72, 56},
{37, 70, 131, 53, 72, 46, 37},
{225, 171, 136, 148, 136, 161, 188},
{64, 177, 76, 69, 92, 92, 84},
{44, 54, 166, 93, 158, 101, 59},
{153, 170, 150, 125, 152, 171, 166},
{86, 155, 136, 41, 36, 131, 63},
{138, 160, 104, 119, 149, 124, 100},
{19, 56, 140, 139, 217, 161, 51}
};

// w_hid
int w_hid[W_HID_ROWS][W_HID_COLUMNS] = {
{25, 18},
{31, 6},
{29, 26},
{22, 1},
{1, 28},
{11, 9},
{26, 45},
{26, 6}
}; // not same as given, swapped first row to the back

// w_out
int w_out[] = {50, 200, 80}; // not same as given, swapped first row to the back

// labels
int labels[INPUT_ROWS] = {
0, 1, 1, 1, 0, 0, 1, 1,
1, 0, 0, 0, 1, 1, 1, 0,
1, 0, 1, 0, 0, 1, 1, 0,
1, 1, 1, 1, 0, 0, 0, 0,
0, 0, 1, 1, 0, 1, 0, 1,
0, 1, 0, 0, 1, 1, 1, 1,
1, 0, 0, 1, 1, 0, 1, 0,
0, 1, 0, 0, 1, 0, 1, 0
};

// sigmoid array
int sig_arr[] = {12,12,12,12,13,13,13,14,14,14,15,15,15,16,16,16,17,17,18,18,18,19,19,20,20,21,21,21,22,22,23,23,24,24,25,26,26,27,27,28,28,29,30,30,31,32,32,33,34,34,35,36,36,37,38,39,39,40,41,42,43,44,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,66,67,68,69,70,72,73,74,75,76,78,79,80,82,83,84,86,87,88,90,91,92,94,95,97,98,99,101,102,104,105,107,108,110,111,113,114,116,117,119,120,122,123,125,126,128,129,130,132,133,135,136,138,139,141,142,144,145,147,148,150,151,153,154,156,157,158,160,161,163,164,165,167,168,169,171,172,173,175,176,177,179,180,181,182,183,185,186,187,188,189,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,211,212,213,214,215,216,216,217,218,219,219,220,221,221,222,223,223,224,225,225,226,227,227,228,228,229,229,230,231,231,232,232,233,233,234,234,234,235,235,236,236,237,237,237,238,238,239,239,239,240,240,240,241,241,241,242,242,242,243,243,243};

//supporting functions
// UART Helpers
static u8 clamp_u8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (u8)v;
}

static void send_res_csv(int *RES64, int inp_rows, int inp_cols) {
    for (int i = 0; i < inp_rows*inp_cols; i++) {
        xil_printf("%lu\r\n", (unsigned long)(RES64[i] & 0xFF));
    }
}

static u8 uart_getc_blocking(void) {
    extern char inbyte(void);
    return (u8)inbyte();
}


//sigmoid activation function
void sigmoid(int *inp_arr, int rows, int cols){
    for (int i = 0; i < rows*cols; i++){
        inp_arr[i] = sig_arr[inp_arr[i]];
    }
}

// matrix mult (generic)
void matrix_mult(int *inp_arr, int *bias_arr, int *output_arr, int inp_rows, int inp_cols, int output_cols){
    for (int i = 0; i < inp_rows; i++){
        for (int z = 0; z < output_cols; z++){
            int sum = 0;
            for (int j = 0; j < inp_cols; j++){
                sum += (int)inp_arr[i*inp_cols + j] * (int)bias_arr[j*output_cols + z];
            }
            sum += 255*(int)bias_arr[inp_cols*output_cols + z];
            output_arr[i*output_cols + z] = clamp_u8(sum >> 8);
        }
    }
}

// convert to labels
void convert(int *output_arr){
    for (int i = 0; i < 64; i++){
        if (output_arr[i] < 128){
            output_arr[i] = 0;
        }
        else{
            output_arr[i] = 1;
        }
    }
}

// comparison function
void compare(int *created_arr, int *label_arr){
    int count = 0;
    for (int i = 0; i < 64; i++){
        if (created_arr[i] != label_arr[i]){
            xil_printf("Entry %d wrong.\r\n", i);
            count++;
        }
    }
    xil_printf("Number of Incorrect Results: %d.\r\n", count);
}


// main function
int main()
{
    init_platform();
    int A_1[128];
    int A_2[64];

    // 1st Layer
    matrix_mult(*A, *w_hid, A_1, INPUT_ROWS, INPUT_COLUMNS, W_HID_COLUMNS);
    
    /* // For testing...
    xil_printf("\r\n--- layer 1 Done ---\r\n");
    xil_printf("START RealTerm CAPTURE NOW, THEN PRESS ANY KEY...\r\n");
    uart_getc_blocking();
    send_res_csv(A_1, INPUT_ROWS, W_HID_COLUMNS);*/
    
    // Sigmoid
    sigmoid(A_1,INPUT_ROWS, W_HID_COLUMNS);
    
    /* //For testing...
    xil_printf("\r\n--- Sigmoid Done ---\r\n");
    xil_printf("START RealTerm CAPTURE NOW, THEN PRESS ANY KEY...\r\n");
    uart_getc_blocking();
    send_res_csv(A_1, INPUT_ROWS, W_HID_COLUMNS);*/

    // 2nd Layer
    matrix_mult(A_1, w_out, A_2, INPUT_ROWS, W_HID_COLUMNS, W_OUT_COLUMNS);

    /*//For testing...
    xil_printf("\r\n--- Layer 2 Done ---\r\n");
    xil_printf("START RealTerm CAPTURE NOW, THEN PRESS ANY KEY...\r\n");
    uart_getc_blocking();
    send_res_csv(A_2, INPUT_ROWS, W_OUT_COLUMNS);*/
    
    // Final Label Conversion
    convert(A_2);
    compare(A_2, labels);
    
    // sending results
    xil_printf("\r\n--- Labeling Done ---\r\n");
    xil_printf("START RealTerm CAPTURE NOW, THEN PRESS ANY KEY...\r\n");
    uart_getc_blocking();
    send_res_csv(A_2, INPUT_ROWS, W_OUT_COLUMNS);
    
    cleanup_platform();
    return 0;
}

