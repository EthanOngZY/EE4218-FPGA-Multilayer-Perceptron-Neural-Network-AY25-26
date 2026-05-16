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
#define W_OUT_ROWS 3
#define W_OUT_COLUMNS 1


// data arrays
// input data without bias
int A[INPUT_ROWS][INPUT_COLUMNS];

// w_hid
int w_hid[W_HID_ROWS][W_HID_COLUMNS]; // not same as given, swapped first row to the back

// w_out
int w_out[W_OUT_ROWS]; // not same as given, swapped first row to the back

// labels
int expected_labels[INPUT_ROWS];

// sigmoid array
int sig_arr[] = {12,12,12,12,13,13,13,14,14,14,15,15,15,16,16,16,17,17,18,18,18,19,19,20,20,21,21,21,22,22,23,23,24,24,25,26,26,27,27,28,28,29,30,30,31,32,32,33,34,34,35,36,36,37,38,39,39,40,41,42,43,44,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,66,67,68,69,70,72,73,74,75,76,78,79,80,82,83,84,86,87,88,90,91,92,94,95,97,98,99,101,102,104,105,107,108,110,111,113,114,116,117,119,120,122,123,125,126,128,129,130,132,133,135,136,138,139,141,142,144,145,147,148,150,151,153,154,156,157,158,160,161,163,164,165,167,168,169,171,172,173,175,176,177,179,180,181,182,183,185,186,187,188,189,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,211,212,213,214,215,216,216,217,218,219,219,220,221,221,222,223,223,224,225,225,226,227,227,228,228,229,229,230,231,231,232,232,233,233,234,234,234,235,235,236,236,237,237,237,238,238,239,239,239,240,240,240,241,241,241,242,242,242,243,243,243};

//supporting functions
// UART Helpers
static u8 clamp_u8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (u8)v;
}

static u8 uart_getc_blocking(void) {
    extern char inbyte();
    return (u8)inbyte();
}

static void send_res_csv(int *RES64, int inp_rows, int inp_cols) {
    for (int i = 0; i < inp_rows*inp_cols; i++) {
        xil_printf("%lu\r\n", (unsigned long)(RES64[i] & 0xFF));
    }
}

static int uart_read_u32(u32 *out) {
    u32 val = 0;
    int got_digit = 0;

    while (1) {
        u8 c = uart_getc_blocking();

        if (c >= '0' && c <= '9') {
            got_digit = 1;
            val = val * 10 + (c - '0');
        } else {
            if (got_digit) {
                *out = val;
                return 1;
            }
        }
    }
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

    // Receiving Data
    xil_printf("\r\n--- READY TO RECEIVE MATRIX  ---\r\n");
    xil_printf("WAITING FOR W_HID...\r\n");
    // Receive w_hid 
    for (int i = 0; i < W_HID_ROWS; i++) {
        for (int j = 0; j < W_HID_COLUMNS; j++) {
            u32 x;
            uart_read_u32(&x);
            if (x > 255) x = 255;
            w_hid[i][j] = (u8)x;
        }
    }

    xil_printf("\r\n--- W_HID RECEIVED  ---\r\n");
    xil_printf("WAITING FOR W_OUT...\r\n");
    // Receive w_out 
    for (int i = 0; i < W_OUT_ROWS; i++) {
        u32 x;
        uart_read_u32(&x);
        if (x > 255) x = 255;
        w_out[i] = (u8)x;
    }
    
    xil_printf("\r\n--- W_OUT RECEIVED  ---\r\n");
    xil_printf("WAITING FOR EXPECTED LABELS...\r\n");
    // Receive expected_labels
    for (int i = 0; i < INPUT_ROWS; i++) {
        u32 x;
        uart_read_u32(&x);
        if (x > 255) x = 255;
        expected_labels[i] = (u8)x;
    }
    
    xil_printf("\r\n--- EXPECTED LABELS RECEIVED  ---\r\n");
    xil_printf("WAITING FOR STARTING INPUT...\r\n");
    // Receive A 
    for (int i = 0; i < INPUT_ROWS; i++) {
        for (int j = 0; j < INPUT_COLUMNS; j++) {
            u32 x;
            uart_read_u32(&x);
            if (x > 255) x = 255;
            A[i][j] = (u8)x;
        }
    }
    
    xil_printf("\r\n--- STARTING INPUT RECEIVED ---\r\n");
    xil_printf("\r\nCOMPUTING...\r\n");
    // 1st Layer
    matrix_mult(*A, *w_hid, A_1, INPUT_ROWS, INPUT_COLUMNS, W_HID_COLUMNS);
    
    /*// For testing...
    xil_printf("\r\n--- layer 1 Done ---\r\n");
    xil_printf("START RealTerm CAPTURE NOW, THEN PRESS ANY KEY...\r\n");
    uart_getc_blocking();
    send_res_csv(A_1, INPUT_ROWS, W_HID_COLUMNS);*/
    
    // Sigmoid
    sigmoid(A_1,INPUT_ROWS, W_HID_COLUMNS);
    
    /*//For testing...
    xil_printf("\r\n--- Sigmoid Done ---\r\n");
    xil_printf("START RealTerm CAPTURE NOW, THEN PRESS ANY KEY...\r\n");
    uart_getc_blocking();
    send_res_csv(A_1, INPUT_ROWS, W_HID_COLUMNS);*/

    // 2nd Layer
    matrix_mult(A_1, w_out, A_2, INPUT_ROWS, W_HID_COLUMNS, W_OUT_COLUMNS);
    xil_printf("\r\n--- COMPUTE END ---\r\n");

    /*//For testing...
    xil_printf("\r\n--- Layer 2 Done ---\r\n");
    xil_printf("START RealTerm CAPTURE NOW, THEN PRESS ANY KEY...\r\n");
    uart_getc_blocking();
    send_res_csv(A_2, INPUT_ROWS, W_OUT_COLUMNS);*/
    
    // Final Label Conversion
    convert(A_2);
    compare(A_2, expected_labels);
    
    // sending results
    xil_printf("\r\n--- Labeling Done ---\r\n");
    xil_printf("START RealTerm CAPTURE NOW, THEN PRESS ANY KEY...\r\n");
    uart_getc_blocking(); // extra here that picks up some junk input (idk but it works)
    uart_getc_blocking();
    
    send_res_csv(A_2, INPUT_ROWS, W_OUT_COLUMNS);
    
    xil_printf("--- PROGRAMME END ---\r\n");
    
    cleanup_platform();
    return 0;
}
