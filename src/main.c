// #define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include "uart_stdio.h"
#include "led.h"
#include "pir.h"
#include "display.h"
#include "timer.h"
#include "wifi.h"
#include "button.h"
#include "buzzer.h"
#include "dht11.h"
#include "proximity.h"
#include "servo.h"

// #include "uart.h"
#define MAX_STRING_LENGTH 100
#define MAX_MENU_OPTIONS 10

static bool _pir_active = false;
static int x = 0;
static char tmp_buff1[15];

static char _tcp_receive_buff[MAX_STRING_LENGTH] = {0};
static bool _tcp_string_received = false;
static char _stdio_input_buff[MAX_STRING_LENGTH] = {0};

char welcome_text[] = "Welcome from SEP4 IoT hardware!\n";

uint8_t menu(void)
{
    int choice = 0;
    puts("VIA UNIVERSITY COLLEGE SEP4 IoT Hardware DRIVERS DEMO");
    puts("\tMenu:");
    puts("\t 1. Button and LED");
    puts("\t 2. PIR Sensor (HC-SR501)");
    puts("\t 3. Display");
    puts("\t 4. WiFi (ESP8266)");
    puts("\t 5. stdio");
    puts("\t 6. Timer");
    puts("\t 7. Buzzer");
    puts("\t 8. Temperature and humiduty sensor (DHT11)");
    puts("\t 9. Proximity sensor (HC-SR04)");
    puts("\t10. Servo motor (SG90)");

    printf("Choose a driver to test (1-%d): ", MAX_MENU_OPTIONS);
    stdin_flush(); // Flush any leftover '\n' input from the buffer
    do
    {
        scanf("%d", &choice);
        printf("%d\n", choice);
        if(choice < 1 || choice > MAX_MENU_OPTIONS)
        {
            // Invalid input, clear the input buffer
            while (getchar() != '\n')
                ;
            printf("Invalid input. Please enter a number between 1 and %d: ", MAX_MENU_OPTIONS);
        }
    } while (choice < 1 || choice > MAX_MENU_OPTIONS);

    return (uint8_t)choice;
}

void pir_callback(void)
{
    if (_pir_active)
    {
        if (pir_get_state() == PIR_NO_MOTION)
        {
            led_off(1);
        }
        else
        {
            led_on(1);
        }
    }
}

void led2_callback(uint8_t id)
{
    static int8_t led_no = 2;
    led_toggle(led_no);
}

void start_stop_timer(uint8_t id)
{
    if (timer_get_state(id)) // Check if LED2 timer is active
        timer_pause(id);     // Pause the LED2 timer
    else
        timer_resume(id); // Resume the LED2 timer
}

void wifi_line_callback(const char *line)
{
    uint8_t _index;
    _index = strlen(_tcp_receive_buff);
    _tcp_receive_buff[_index] = '\r';
    _tcp_receive_buff[_index + 1] = '\n';
    _tcp_receive_buff[_index + 2] = '\0';
    _tcp_string_received = true;
}

static bool _quit()
{
    return (gets_nonblocking(tmp_buff1, sizeof(tmp_buff1)) > 0 && tmp_buff1[0] == 'q');
}

int main(void)
{
    static int led2_timer_id = 0;

    led_init();
    button_init();
    display_init();
    pir_init(pir_callback);

    // Initialize UART stdio at 115200 baud. Must be same on terminal.
    if (UART_OK != uart_stdio_init(115200))
    {
        led_on(4); // Turn on LED4 to indicate error
        while (1)
            ;
    }
    sei(); // Enable global interrupts

    while (1)
    {
        switch (menu())
        {
        case 1:
            puts("Button and LED driver. Type 'q' to exit.");
            puts("LED 4 vill blink. Push a button to light one of the other LEDs.");
            led_blink(4, 500); // Blink LED4 with 5000ms periode
            do
            {
                (button_get(1)) ? led_on(1) : led_off(1);
                (button_get(2)) ? led_on(2) : led_off(2);
                (button_get(3)) ? led_on(3) : led_off(3);
                _delay_ms(200);
            } while (!_quit());
            led_off(4);
            break;
        case 2:
            _pir_active = true;
            puts("PIR sensor driver. Type 'q' to exit.");
            puts("LED 1 should turn on when motion is detected.");
            do
            {
                _delay_ms(200);
            } while (!_quit());
            _pir_active = false;
            break;
        case 3:
            puts("Display driver. Type 'q' to exit.");
            puts("Type a number between -999 and 9999");
            while (scanf("%d", &x) == 1)
            {
                display_int(x);
                printf("Du skrev: %d\n", x);
            }
            break;
        case 4:
            wifi_init();
            puts("WiFi driver demo. Press Reset to exit.");
            puts("(SSID, password, server IP and port are hardcoded, change if needed)");
            wifi_command_join_AP("Erlands SEP4", "ViaUC1234");
            wifi_command_create_TCP_connection("10.184.216.102", 23, wifi_line_callback, _tcp_receive_buff);
            wifi_command_TCP_transmit((uint8_t *)welcome_text, strlen(welcome_text));

            while (1)
            {
                if (_tcp_string_received)
                {
                    printf("TCP received: %s\n", _tcp_receive_buff);
                    _tcp_receive_buff[0] = '\0';
                    _tcp_string_received = false;
                }
                if (gets_nonblocking(_stdio_input_buff, MAX_STRING_LENGTH) > 0)
                {
                    printf("You wrote: %s\n", _stdio_input_buff);
                    wifi_command_TCP_transmit((uint8_t *)_stdio_input_buff, strlen(_stdio_input_buff));
                }
                _delay_ms(200);
            }
            break;
        case 5:
            printf("stdio driver. Type a text to echo to the terminal.\n");
            int ch;
            stdin_flush(); // Flush any leftover input from the buffer
            do
            {
                ch = getchar();
                putchar(ch);
            } while (ch != '\n' && ch != EOF);
            break;
        case 6:
            puts("Timer driver demo. Type 'q' to exit.");
            puts("LED 2 will toggle every 100ms. Pressing button 2 will pause/resume the blinking.");
            if ((led2_timer_id = timer_create_sw(led2_callback, 100)) < 0)
            {
                puts("Timer create failed");
                _delay_ms(2000);
                break;
            }
            do
            {
                if(button_get(2)) 
                {
                    start_stop_timer(led2_timer_id); // Call the start/stop callback to toggle the timer
                    _delay_ms(300); // Debounce delay
                }
            } while (!_quit());
            led_off(2);
            timer_delete(led2_timer_id);
            break;
        case 7:
            puts("Buzzer driver demo. Press button 2 to hear a beep. Type 'q' to exit.");
            do
            {
                if (button_get(2))
                {
                    buzzer_beep();  // Call the buzzer beep function
                    _delay_ms(200); // Debounce delay
                }
            } while (!_quit());
            break;
        case 8:
            puts("DHT11 driver demo. Type 'q' to exit.");
            puts("Temperature and humidity will be printed every 2 seconds.");
            do
            {
                uint8_t humidity_integer, humidity_decimal, temperature_integer, temperature_decimal;
                DHT11_ERROR_MESSAGE_t error = dht11_get(&humidity_integer, &humidity_decimal, &temperature_integer, &temperature_decimal);
                if (error == DHT11_OK)
                {
                    printf("Temperature: %d.%d°C, Humidity: %d.%d%%\n", temperature_integer, temperature_decimal, humidity_integer, humidity_decimal);
                }
                else
                {
                    printf("Failed to read DHT11 sensor data\n");
                }
                _delay_ms(2000); // Wait 2 seconds before next reading
            } while (!_quit());
            break;
        case 9:
            puts("Proximity sensor driver demo. Type 'q' to exit.");
            puts("Distance in mm will be printed every 2 seconds.");
            proximity_init();
            do
            {
                uint16_t distance = proximity_measure();
                if (distance == UINT16_MAX)
                {
                    printf("Proximity sensor timeout. No object detected within range.\n");
                }
                else
                {
                    printf("Distance: %d mm\n", distance);
                }
                _delay_ms(2000); // Wait 2 seconds before next reading
            } while (!_quit());
            break;
        case 10:
            puts("Servo motor driver demo. Type 'q' to exit.");
            servo_init(PWM_NORMAL);
            servo_start();
            int angle;
            printf("Enter angle (-90 to 90): ");
            while (scanf("%d", &angle) == 1)
            {
                if (angle < -90 || angle > 90)
                {
                    puts("Invalid angle. Please enter a value between -90 and 90.");
                }
                else
                {
                    servo_setAngle(PWM_A, (int8_t)angle);
                    servo_setAngle(PWM_B, (int8_t)angle);
                    printf("Servo set to %d degrees.\n", angle);
                }
            }
            servo_stop();
            break;
        default:
            printf("Error: Invalid selection.\n");
            break;
        }
    }
    return 0;
}