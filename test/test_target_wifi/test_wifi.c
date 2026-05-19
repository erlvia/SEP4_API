#include "unity.h"
#include "wifi.h"
#include "uart.h"
#include <string.h>
#include <stddef.h>
#include "../fff.h"

DEFINE_FFF_GLOBALS

FAKE_VALUE_FUNC(uart_t, uart_init, uart_id_t, uint32_t, rx_callback_t, uint16_t);
FAKE_VALUE_FUNC(uart_t, uart_send_string_blocking, uart_id_t, const char*);
FAKE_VALUE_FUNC(uart_t, uart_write_bytes, uart_id_t, uint8_t*, uint16_t);
FAKE_VOID_FUNC(_delay_ms, unsigned long);

char test_response_buffer[100];

uart_t uart_send_string_blocking_custom_fake(uart_id_t uart_id, const char* str)
{
    char *response_ptr = test_response_buffer;

    // Feed the next byte of the test response into the wifi command callback
    while(*response_ptr++ != '\0')
    {
        wifi_test_feed_response_byte();
    } 
    return UART_OK;
}

void setUp(void)
{
    RESET_FAKE(uart_init);
    RESET_FAKE(uart_send_string_blocking);
    RESET_FAKE(uart_write_bytes);
    RESET_FAKE(_delay_ms);
    FFF_RESET_HISTORY();

    uart_init_fake.return_val = UART_OK;
    uart_send_string_blocking_fake.return_val = UART_OK;
    uart_write_bytes_fake.return_val = UART_OK;
    test_response_buffer[0] = '\0';
    wifi_test_set_response(NULL);
    uart_send_string_blocking_fake.custom_fake = uart_send_string_blocking_custom_fake;
}

void tearDown(void)
{
}

void test_wifi_init_calls_uart_init(void)
{
    wifi_init();
    TEST_ASSERT_EQUAL_UINT(1, uart_init_fake.call_count);
    TEST_ASSERT_EQUAL_INT(UART2_ID, uart_init_fake.arg0_val);
    TEST_ASSERT_EQUAL_UINT32(115200, uart_init_fake.arg1_val);
    TEST_ASSERT_NOT_NULL(uart_init_fake.arg2_val);
    TEST_ASSERT_EQUAL_UINT16(0, uart_init_fake.arg3_val);
}

void test_wifi_command_AT_returns_ok_when_response_is_ok(void)
{
    strncpy(test_response_buffer, "OK\r\n", sizeof(test_response_buffer));
    wifi_test_set_response(test_response_buffer);

    WIFI_ERROR_MESSAGE_t result = wifi_command_AT();

    TEST_ASSERT_EQUAL_UINT(1, uart_send_string_blocking_fake.call_count);
    TEST_ASSERT_EQUAL_STRING("AT\r\n", uart_send_string_blocking_fake.arg1_val);
    TEST_ASSERT_EQUAL(WIFI_OK, result);
}

void test_wifi_command_AT_returns_error_when_response_is_error(void)
{
    strncpy(test_response_buffer, "ERROR", sizeof(test_response_buffer));
    wifi_test_set_response(test_response_buffer);

    WIFI_ERROR_MESSAGE_t result = wifi_command_AT();

    TEST_ASSERT_EQUAL_UINT(1, uart_send_string_blocking_fake.call_count);
    TEST_ASSERT_EQUAL_STRING("AT\r\n", uart_send_string_blocking_fake.arg1_val);
    TEST_ASSERT_EQUAL(WIFI_ERROR_RECEIVED_ERROR, result);
}

void test_wifi_command_AT_times_out_when_no_response(void)
{
    wifi_test_set_response(NULL);

    WIFI_ERROR_MESSAGE_t result = wifi_command_AT();

    TEST_ASSERT_EQUAL_UINT(1, uart_send_string_blocking_fake.call_count);
    TEST_ASSERT_EQUAL(WIFI_ERROR_NOT_RECEIVING, result);
}

void test_wifi_command_join_AP_sends_expected_join_command(void)
{
    strncpy(test_response_buffer, "OK\r\n", sizeof(test_response_buffer));
    wifi_test_set_response(test_response_buffer);
    WIFI_ERROR_MESSAGE_t result = wifi_command_join_AP("MySSID", "MyPass");

    TEST_ASSERT_EQUAL_STRING("AT+CWJAP=\"MySSID\",\"MyPass\"\r\n", uart_send_string_blocking_fake.arg1_val);
    TEST_ASSERT_EQUAL(WIFI_OK, result);
}

void test_wifi_command_get_ip_from_URL_parses_ip_address(void)
{
    strncpy(test_response_buffer, "CIPDOMAIN:192.168.1.10\r\nOK\r\n", sizeof(test_response_buffer));
    wifi_test_set_response(test_response_buffer);
    char ip[32] = {0};

    WIFI_ERROR_MESSAGE_t result = wifi_command_get_ip_from_URL("example.com", ip);

    TEST_ASSERT_EQUAL_STRING("AT+CIPDOMAIN=\"example.com\"\r\n", uart_send_string_blocking_fake.arg1_val);
    TEST_ASSERT_EQUAL_STRING("192.168.1.10", ip);
    TEST_ASSERT_EQUAL(WIFI_OK, result);
}

void test_wifi_command_create_TCP_connection_sends_cipstart(void)
{
    strncpy(test_response_buffer, "OK\r\n", sizeof(test_response_buffer));
    wifi_test_set_response(test_response_buffer);
    char buffer[32] = {0};

    WIFI_ERROR_MESSAGE_t result = wifi_command_create_TCP_connection("1.2.3.4", 80, NULL, buffer);

    TEST_ASSERT_EQUAL_STRING("AT+CIPSTART=\"TCP\",\"1.2.3.4\",80\r\n", uart_send_string_blocking_fake.arg1_val);
    TEST_ASSERT_EQUAL(WIFI_OK, result);
}

void test_wifi_command_create_TCP_connection_n_sends_cipstart(void)
{
    strncpy(test_response_buffer, "OK\r\n", sizeof(test_response_buffer));
    wifi_test_set_response(test_response_buffer);
    char buffer[32] = {0};

    WIFI_ERROR_MESSAGE_t result = wifi_command_create_TCP_connection_n("1.2.3.4", 80, NULL, buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL_STRING("AT+CIPSTART=\"TCP\",\"1.2.3.4\",80\r\n", uart_send_string_blocking_fake.arg1_val);
    TEST_ASSERT_EQUAL(WIFI_OK, result);
}

void test_wifi_command_TCP_transmit_writes_bytes_after_cipsend(void)
{
    strncpy(test_response_buffer, "OK\r\n", sizeof(test_response_buffer));
    wifi_test_set_response(test_response_buffer);
    uint8_t payload[] = { 'H', 'e', 'l', 'l', 'o' };
    
    WIFI_ERROR_MESSAGE_t result = wifi_command_TCP_transmit(payload, sizeof(payload));

    TEST_ASSERT_EQUAL(WIFI_OK, result);
    TEST_ASSERT_EQUAL_UINT(1, uart_send_string_blocking_fake.call_count);
    TEST_ASSERT_EQUAL_UINT(1, uart_write_bytes_fake.call_count);
    TEST_ASSERT_EQUAL_UINT16(sizeof(payload), uart_write_bytes_fake.arg2_val);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, uart_write_bytes_fake.arg1_val, sizeof(payload));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_wifi_init_calls_uart_init);
    RUN_TEST(test_wifi_command_AT_returns_ok_when_response_is_ok);
    RUN_TEST(test_wifi_command_AT_returns_error_when_response_is_error);
    RUN_TEST(test_wifi_command_AT_times_out_when_no_response);
    RUN_TEST(test_wifi_command_join_AP_sends_expected_join_command);
    RUN_TEST(test_wifi_command_get_ip_from_URL_parses_ip_address);
    RUN_TEST(test_wifi_command_create_TCP_connection_sends_cipstart);
    RUN_TEST(test_wifi_command_create_TCP_connection_n_sends_cipstart);
    RUN_TEST(test_wifi_command_TCP_transmit_writes_bytes_after_cipsend);
    return UNITY_END();
}