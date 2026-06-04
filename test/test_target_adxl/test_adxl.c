#include "unity.h"
#include "adxl345.h"
#include "util/delay.h"

extern uint8_t MOSI_value;
extern uint8_t MISO_value; 
extern uint8_t MISO_set_value; // This variable will be used to set the value of MISO during tests
extern uint8_t spi_transfer(uint8_t data);

void setUp(void)
{
    adxl345_init();
}

void tearDown(void)
{
}

void test_adxl345_read_x(void)
{
    int16_t x, y, z;
    adxl345_read_xyz(&x, &y, &z);
    // Since we can't predict the actual values, we can only check if they are within the expected range
    TEST_ASSERT(x >= -5 && x <= 5);
}

void test_adxl345_read_y(void)
{
    int16_t x, y, z;
    adxl345_read_xyz(&x, &y, &z);
    // Since we can't predict the actual values, we can only check if they are within the expected range
    TEST_ASSERT(y >= -5 && y <= 5);
}

void test_adxl345_read_z(void)
{
    int16_t x, y, z;
    adxl345_read_xyz(&x, &y, &z);
    // Since we can't predict the actual values, we can only check if they are within the expected range
    TEST_ASSERT(z >= 120 && z <= 140);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_adxl345_read_x);
    RUN_TEST(test_adxl345_read_y);
    RUN_TEST(test_adxl345_read_z);
    return UNITY_END();
}
