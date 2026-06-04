/***********************************************
 * adxl345.c
 *  ADXL345 accelerometer implementation
 *  Author:  Erland Larsen
 *  Date:    2026-06-04
 *  Project: SPE4_API
 **********************************************/
#include "adxl345.h"
#include "avr/io.h"
#include "util/delay.h"
#include "avr/interrupt.h"

#define CS_BIT PB1
#define CS_DDR DDRB
#define CS_PORT PORTB

#define MISO_BIT PB2 // SDO to MISO (Pin 50)
#define MISO_DDR DDRB
#define MISO_PIN PINB
#define MISO_PORT PORTB

#define MOSI_BIT PB3 // SDA/SDI to MOSI
#define MOSI_DDR DDRB
#define MOSI_PORT PORTB

#define SCL_BIT PB0
#define SCL_DDR DDRB
#define SCL_PORT PORTB

// ADXL345 Register Addresses
#define ADXL345_DEVID 0x00
#define ADXL345_POWER_CTL 0x2D
#define ADXL345_DATA_FORMAT 0x31
#define ADXL345_DATAX0 0x32

// ADXL345 specific constants
#define ADXL345_RANGE_2G 0x00
#define ADXL345_RANGE_4G 0x01
#define ADXL345_RANGE_16G 0x03
#define ADXL345_MEASURE_MODE 0x08

uint8_t spi_transfer(uint8_t data);

void adxl345_init(void)
{
    CS_DDR |= (1 << CS_BIT);
    //Chip select high, because its not selected.
    CS_PORT |= (1 << CS_BIT);

    SCL_DDR |= (1 << SCL_BIT);
    // Clock is default high.
    SCL_PORT |= (1 << SCL_BIT);

    MOSI_DDR |= (1 << MOSI_BIT);
    MISO_DDR &= ~(1 << MISO_BIT);
    MISO_PORT |= (1 << MISO_BIT); // Enable pull-up on MISO

    _delay_ms(2);

    adxl345_write_register(ADXL345_POWER_CTL, ADXL345_MEASURE_MODE);
    adxl345_write_register(ADXL345_DATA_FORMAT, ADXL345_RANGE_4G);
}

uint8_t spi_transfer(uint8_t data)
{
    uint8_t received_data = 0;
    uint8_t sreg = SREG; // Save the current state of interrupts
    cli(); // Disable interrupts during SPI transfer
    for (uint8_t i = 0; i < 8; i++)
    {
        if (data & (1 << (7 - i)))
        {
            MOSI_PORT |= (1 << MOSI_BIT);
        }
        else
        {
            MOSI_PORT &= ~(1 << MOSI_BIT);
        }

        SCL_PORT &= ~(1 << SCL_BIT);
        _delay_us(4);   // Asymetric delays on SCL to compensate for loop control execution time

        received_data <<= 1;
        if (MISO_PIN & (1 << MISO_BIT))
        {
            received_data |= 1;
        }

        SCL_PORT |= (1 << SCL_BIT);
        _delay_us(1);   // Asymetric delays on SCL to compensate for loop control execution time
    }

    MOSI_PORT |= (1 << MOSI_BIT); // Set MOSI high after transfer
    SREG = sreg; // Restore the state of interrupts
    return received_data;
}

void adxl345_write_register(uint8_t reg, uint8_t value)
{
    CS_PORT &= ~(1 << CS_BIT);
    _delay_us(1);

    spi_transfer(reg);
    spi_transfer(value);

    CS_PORT |= (1 << CS_BIT);
}

uint8_t adxl345_read_register(uint8_t reg)
{
    CS_PORT &= ~(1 << CS_BIT);
    spi_transfer(0x80 | reg);
    uint8_t value = spi_transfer(0x00);
    CS_PORT |= (1 << CS_BIT);
    
    return value;
}

void adxl345_read_xyz(int16_t *x, int16_t *y, int16_t *z)
{
    CS_PORT &= ~(1 << CS_BIT);
    spi_transfer(0xC0 | 0x32); // Read=1, Multibyte=1, Start Address=0x32

    *x = spi_transfer(0x00);
    *x |= spi_transfer(0x00) << 8;

    *y = spi_transfer(0x00);
    *y |= spi_transfer(0x00) << 8;

    *z = spi_transfer(0x00);
    *z |= spi_transfer(0x00) << 8;

    CS_PORT |= (1 << CS_BIT);
}
