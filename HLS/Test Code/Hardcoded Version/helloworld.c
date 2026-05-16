/**********************************************************************************
EE4218 Project HLS main.c (KV260)
- UART receives w_out (3x1), w_hid (8x2), input_data (64x7)
- Runs HW coprocessor via AXI DMA (467 words in, 64 words out)
- Prints timing cycles for HW version
- edited off Lab 3 main.c

Differences to Template Data:
- Data is provided as one singular array, with w_out, w_hid, input_data in that sequence
    - Also assumes that bias rows of w_out and w_hid are the last row
    - Also assumes that no bias column is added to the input_data
**********************************************************************************/

#include "platform.h"
#include <stdio.h>
#include <math.h>
#include "timer.h"

/* DMA headers */
#include "xaxidma.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xstatus.h"
#include "xil_printf.h"

/* constants */
#define INPUT_ROWS      64
#define INPUT_COLUMNS   7
#define W_HID_ROWS      8
#define W_HID_COLUMNS   2
#define W_OUT_ROWS      3
#define W_OUT_COLUMNS   1

#define total_in_size   (INPUT_ROWS*INPUT_COLUMNS + W_HID_ROWS*W_HID_COLUMNS + W_OUT_ROWS*W_OUT_COLUMNS)
#define total_out_size  (INPUT_ROWS*W_OUT_COLUMNS)

// data arrays
u32 Aflat[total_in_size] = {
50, 200, 80,
25, 18, 31, 6, 29, 26, 22, 1, 1, 28, 11, 9, 26, 45, 26, 6,
44, 90, 0, 0, 24, 81, 22, 159, 250, 140, 176, 121, 183, 138, 167, 158, 172, 134, 172, 161, 118, 130, 178, 136, 120, 135, 121, 86, 34, 112, 142, 112, 163, 159, 43, 63, 28, 145, 126, 190, 165, 66, 88, 214, 157, 140, 205, 174, 128, 185, 203, 170, 110, 211, 138, 126, 140, 255, 110, 136, 133, 156, 131, 73, 90, 164, 118, 75, 121, 111, 16, 93, 187, 88, 162, 101, 27, 31, 110, 120, 77, 166, 101, 45, 183, 181, 155, 175, 135, 211, 185, 129, 193, 104, 159, 184, 183, 161, 190, 183, 143, 124, 139, 138, 145, 79, 113, 115, 111, 164, 101, 88, 213, 213, 149, 159, 133, 199, 183, 13, 94, 127, 146, 132, 150, 89, 103, 175, 164, 139, 187, 147, 128, 42, 82, 104, 83, 167, 50, 71, 26, 90, 109, 143, 135, 220, 106, 69, 90, 101, 180, 125, 222, 108, 143, 225, 125, 147, 196, 197, 121, 73, 87, 125, 29, 8, 87, 67, 100, 136, 254, 119, 170, 140, 77, 110, 136, 101, 137, 186, 174, 126, 165, 143, 179, 151, 167, 156, 147, 182, 207, 131, 105, 130, 101, 124, 34, 51, 194, 94, 90, 94, 58, 108, 85, 116, 25, 24, 0, 59, 18, 70, 110, 118, 179, 138, 50, 35, 165, 120, 72, 126, 72, 82, 72, 99, 85, 76, 210, 101, 56, 27, 85, 85, 109, 122, 133, 54, 149, 166, 176, 111, 135, 115, 98, 138, 180, 136, 131, 255, 139, 84, 120, 97, 115, 96, 110, 128, 44, 183, 181, 183, 152, 119, 174, 148, 38, 113, 125, 67, 88, 26, 68, 180, 188, 169, 137, 188, 124, 145, 86, 87, 80, 72, 76, 73, 71, 219, 224, 155, 165, 197, 252, 218, 31, 65, 145, 38, 112, 32, 78, 21, 134, 48, 83, 94, 78, 111, 111, 62, 128, 89, 163, 209, 65, 224, 206, 128, 155, 167, 170, 150, 231, 225, 139, 174, 149, 202, 208, 140, 146, 106, 124, 192, 142, 104, 116, 191, 196, 136, 192, 170, 108, 43, 136, 131, 58, 44, 50, 118, 39, 76, 130, 63, 71, 62, 39, 103, 166, 170, 115, 236, 131, 75, 83, 148, 206, 120, 142, 156, 102, 73, 41, 150, 63, 123, 78, 51, 159, 136, 93, 153, 140, 149, 198, 14, 77, 160, 67, 68, 72, 56, 37, 70, 131, 53, 72, 46, 37, 225, 171, 136, 148, 136, 161, 188, 64, 177, 76, 69, 92, 92, 84, 44, 54, 166, 93, 158, 101, 59, 153, 170, 150, 125, 152, 171, 166, 86, 155, 136, 41, 36, 131, 63, 138, 160, 104, 119, 149, 124, 100, 19, 56, 140, 139, 217, 161, 51
}; // 467 inputs

int test_result_expected_memory[total_out_size] = {
0,
1,
1,
1,
0,
0,
1,
1,
1,
0,
0,
0,
1,
1,
1,
0,
1,
0,
1,
0,
0,
1,
1,
0,
1,
1,
1,
1,
0,
0,
0,
0,
0,
0,
1,
1,
0,
1,
0,
1,
0,
1,
0,
0,
1,
1,
1,
1,
1,
0,
0,
1,
1,
0,
1,
0,
0,
1,
0,
0,
1,
0,
1,
0};// 64 outputs


/* ---------------- DDR buffers for DMA (With thr aid of AI)---------------- */
#if defined(XPAR_DDR_MEM_BASEADDR)
  #define MEM_BASE_ADDR   (XPAR_DDR_MEM_BASEADDR + 0x01000000)
#elif defined(XPAR_PSU_DDR_0_S_AXI_BASEADDR)
  #define MEM_BASE_ADDR   (XPAR_PSU_DDR_0_S_AXI_BASEADDR + 0x01000000)
#elif defined(XPAR_PS7_DDR_0_S_AXI_BASEADDR)
  #define MEM_BASE_ADDR   (XPAR_PS7_DDR_0_S_AXI_BASEADDR + 0x01000000)
#elif defined(DDR_BASE_ADDR)
  #define MEM_BASE_ADDR   (DDR_BASE_ADDR + 0x01000000)
#else
  #warning "No DDR base macro found in xparameters.h."
  #define MEM_BASE_ADDR   0x01000000
#endif

#define TX_BUFFER_BASE       (MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE       (MEM_BASE_ADDR + 0x00300000)

/* ---------------- DMA base address and ID selection---------------- */
#if defined(XPAR_XAXIDMA_0_BASEADDR)
  #define DMA_BASEADDR XPAR_XAXIDMA_0_BASEADDR
#elif defined(XPAR_AXI_DMA_0_BASEADDR)
  #define DMA_BASEADDR XPAR_AXI_DMA_0_BASEADDR
#endif

static XAxiDma AxiDma;

/* ---------------- helpers ---------------- */

void compare(int created_arr[], int label_arr[]){
    int count = 0;
    for (int i = 0; i < INPUT_ROWS; i++){
        if (created_arr[i] != label_arr[i]){
            xil_printf("Entry %d wrong.\r\n", i);
            count++;
        }
    }
    xil_printf("Number of Incorrect Results: %d.\r\n", count);
}

/* ---------------- DMA init ---------------- */
static int dma_init(void)
{
    XAxiDma_Config *CfgPtr;
    int Status;

    /* Correct: LookupConfig takes DEVICE_ID */
    CfgPtr = XAxiDma_LookupConfig(DMA_BASEADDR);
    if (!CfgPtr) {
        xil_printf("[ERR] No DMA config found for DMA_DEV_ID=%d\r\n", DMA_BASEADDR);
        return XST_FAILURE;
    }

    Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
    if (Status != XST_SUCCESS) {
        xil_printf("[ERR] DMA init failed %d\r\n", Status);
        return XST_FAILURE;
    }

    if (XAxiDma_HasSg(&AxiDma)) {
        xil_printf("[ERR] DMA is SG mode, expected Simple mode\r\n");
        return XST_FAILURE;
    }

    /* Polling mode */
    XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);
    XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

    return XST_SUCCESS;
}

/* ---------------- HW coprocessor via DMA ----------------
   Packs TX = [Aflat(512)][Bflat(8)] into TX DDR buffer
   Receives OUT_WORDS (64) into RX DDR buffer
*/
static int dma_matrix_coprocessor(const u32 *Aflat, u32 *RES_HW)
{
    int Status;
    u32 t1, t2, diff_hw;

    u32 *tx_buf = (u32 *)TX_BUFFER_BASE;
    u32 *rx_buf = (u32 *)RX_BUFFER_BASE;

    /* Pack input */
    for (int i = 0; i < total_in_size; i++){
        tx_buf[i] = Aflat[i];
    }

    /* Cache maintenance */
    Xil_DCacheFlushRange((UINTPTR)tx_buf, total_in_size * 4);

    Xil_DCacheInvalidateRange((UINTPTR)rx_buf, total_out_size * 4);

    initTimer();
    t1 = startTimer();

    /* Device to memory */
    Status = XAxiDma_SimpleTransfer(&AxiDma,
                                   (UINTPTR)rx_buf,
                                   total_out_size * 4,
                                   XAXIDMA_DEVICE_TO_DMA);
    if (Status != XST_SUCCESS) return XST_FAILURE;

    /* Memory to device */
    Status = XAxiDma_SimpleTransfer(&AxiDma,
                                   (UINTPTR)tx_buf,
                                   total_in_size * 4,
                                   XAXIDMA_DMA_TO_DEVICE);
    if (Status != XST_SUCCESS) return XST_FAILURE;

    /* Poll completion */
    while (XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE)) {}
    while (XAxiDma_Busy(&AxiDma, XAXIDMA_DEVICE_TO_DMA)) {}

    /* Invalidate RX then copy out */
    Xil_DCacheInvalidateRange((UINTPTR)rx_buf, total_out_size * 4);

    for (int i = 0; i < 64; i++) RES_HW[i] = rx_buf[i];

    diff_hw = endTimer(t1, &t2);
    xil_printf("\r\nHardware Time taken: %lu cycles\r\n", (unsigned long)diff_hw);
    return XST_SUCCESS;
}

int main(void)
{
    init_platform();

    int RES_HW[INPUT_ROWS*W_OUT_COLUMNS];

    /* Init DMA once */
    if (dma_init() != XST_SUCCESS) {
        xil_printf("dma_init failed\r\n");
        cleanup_platform();
        return 1;
    }

    /* ---------------- HW timing ---------------- */

    if (dma_matrix_coprocessor(Aflat, RES_HW) != XST_SUCCESS) {
        xil_printf("dma_matrix_coprocessor failed\r\n");
        cleanup_platform();
        return 1;
    }

    /* Output HW results (numbers only) */
    //send_res_csv_only_u32(RES_HW);
    compare(RES_HW, test_result_expected_memory);

    xil_printf("\r\n--- PROGRAM END ---\r\n");

    cleanup_platform();
    return 0;
}