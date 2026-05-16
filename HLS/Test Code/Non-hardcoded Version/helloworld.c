/**********************************************************************************
EE4218 Project HLS main.c (KV260)
- UART receives w_out (3x1), w_hid (8x2), input_data (64x7)
- Runs HW coprocessor via AXI DMA (467 words in, 64 words out)
- Prints timing cycles for HW version
- edited off Lab 3 main.c

Assumptions:
- Data is provided as one singular array, with w_out, w_hid, input_data in that sequence
    - Also assumes that bias rows of w_out and w_hid are the last row
    - Also assumes that no bias column is added to the input_data
    - Also assumes a space or newline after last input
- Assumes that provided data ranges from 0 to 255

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

// defining variables
#define INPUT_ROWS      64
#define INPUT_COLUMNS   7
#define W_HID_ROWS      8
#define W_HID_COLUMNS   2
#define W_OUT_ROWS      3
#define W_OUT_COLUMNS   1

#define total_in_size   (INPUT_ROWS*INPUT_COLUMNS + W_HID_ROWS*W_HID_COLUMNS + W_OUT_ROWS*W_OUT_COLUMNS)
#define total_out_size  (INPUT_ROWS*W_OUT_COLUMNS)

// data arrays
int Aflat[total_in_size]; // (448 + 16 +3) = 467 inputs

int test_result_expected_memory[total_out_size];// 64 outputs


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

static u8 uart_getc_blocking(void) {
    extern char inbyte(void);
    return (u8)inbyte();
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
   Packs TX = [Aflat(467)] into TX DDR buffer
   Receives total_out_size (64) into RX DDR buffer
*/
static int dma_matrix_coprocessor(const u32 *Aflat, u32 *RES_HW)
{
    int Status;
    u32 t1, t2, diff_hw;

    u32 *tx_buf = (u32 *)TX_BUFFER_BASE;
    u32 *rx_buf = (u32 *)RX_BUFFER_BASE;

    /* Pack input */
    for (int i = 0; i < 467; i++){
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

    int RES_HW[INPUT_ROWS];

    xil_printf("\r\n--- READY TO RECEIVE MATRIX  ---\r\n");
    xil_printf("WAITING FOR DATA...\r\n");
    // Receiving Aflat
    for (int i = 0; i < total_in_size; i++) {
        u32 x;
        uart_read_u32(&x);
        Aflat[i] = (u8)x;
    }

    xil_printf("\r\n--- DATA RECEIVED  ---\r\n");
    xil_printf("WAITING FOR EXPECTED LABELS...\r\n");
    // Receive expected_labels
    for (int i = 0; i < total_out_size; i++) {
        u32 x;
        uart_read_u32(&x);
        test_result_expected_memory[i] = (u8)x;
    }

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
