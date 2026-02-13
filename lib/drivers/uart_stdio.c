/***********************************************
 * uart_stdio.c
 *  Low-level UART stdio implementation
 *  Author:  Erland Larsen
 *  Date:    2026-01-30
 *  Project: SPE4_API
 **********************************************/
#include "uart_stdio.h"
#include "ringbuffer_static.h"
#include <avr/io.h>
#include <stdio.h>

static int uart0_putchar(char c, FILE *stream);
static int uart0_getchar(FILE *stream);

// Stream-objekt for stdio
static FILE uart0_stream;

// Ring buffer for incoming bytes on UART0
static ringbuffer_t uart0_rx_buffer;   // Pointer to buffer object
static uint8_t uart0_rx_storage[UART_STDIO_RX_BUFFER_SIZE];  // Backing storage for ring buffer

void uart0_rx_callback(uint8_t byte)
{
    // Push received byte into ring buffer.
    ringbuffer_push(&uart0_rx_buffer, &byte);
}

uart_t uart_stdio_init(uint32_t baud)
{
    // Create ringbuffer for incoming bytes on UART0
    if(!ringbuffer_init_static(&uart0_rx_buffer, uart0_rx_storage, 
                      sizeof(uart0_rx_storage), UART_STDIO_RX_BUFFER_SIZE, 1 ))
    {
        return UART_ERROR_INIT_FAILED; // Failed to initialize ringbuffer
    }

    uart_t result = uart_init(UART0_ID, baud, uart0_rx_callback, uart0_rx_buffer);
    if (UART_OK == result)
    {
        // Bind stdio to UART
        fdev_setup_stream(&uart0_stream, uart0_putchar, uart0_getchar, _FDEV_SETUP_RW);
        stdin = &uart0_stream;
        stdout = &uart0_stream;
        stderr = &uart0_stream;
    }
    return result;
}

static int uart0_putchar(char c, FILE *stream)
{
    (void)stream;

    // Many terminals expect CRLF
    if (c == '\n') {
        uart_write_byte(UART0_ID, '\r');
    }
    uart_write_byte(UART0_ID, (int8_t)c);
    return 0;
}

static int uart0_getchar(FILE *stream)
{
    (void)stream;

    int8_t c;
    volatile bool in_val = false;   //Must be volatile to prevent optimization issues in loop below

    do
    {
        in_val = ringbuffer_pop(&uart0_rx_buffer, &c);
    } while(!in_val); // Wait until a byte is available in the ring buffer

    // Convert CR to NL (makes enter key work as expected)
    if (c == '\r') c = '\n';

    return (int)c;
}
