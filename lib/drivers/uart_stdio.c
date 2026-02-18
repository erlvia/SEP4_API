/***********************************************
 * uart_stdio.c
 *  Low-level UART stdio implementation
 *  Author:  Erland Larsen
 *  Date:    2026-01-30
 *  Project: SPE4_API
 **********************************************/
#include "uart_stdio.h"
#include <avr/io.h>
#include <stdio.h>

static int uart0_putchar(char c, FILE *stream);
static int uart0_getchar(FILE *stream);

// Stream-objekt for stdio
static FILE uart0_stream;

uart_t uart_stdio_init(uint32_t baud)
{
    uart_t result;

    #ifdef UART_STDIO_RX_BUFFER_SIZE
    {
    result = uart_init(UART0_ID, baud, UART_STDIO_RX_BUFFER_SIZE);
    }
    #else
    result = uart_init(UART0_ID, baud, 0);
    #endif

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
    uart_write_byte(UART0_ID, (uint8_t)c);
    return 0;
}

static int uart0_getchar(FILE *stream)
{
    (void)stream;
    uint8_t c;
    uart_t result;

    #ifdef UART_STDIO_RX_BUFFER_SIZE
    do
    {
        result = uart_read_byte(UART0_ID, &c);
    } while(UART_OK != result); // Wait until a byte is available in the ring buffer
    #else
        result = uart_read_byte_blocking(UART0_ID, &c);
        if (UART_OK != result) 
        {
            return EOF;
        }
    #endif

    // Convert CR to NL (makes enter key work as expected)
    if (c == '\r') c = '\n';

    return (int)c;
}
