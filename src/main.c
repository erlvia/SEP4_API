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

// #include "uart.h"
#define MAX_STRING_LENGTH 100
#define MAX_MENU_OPTIONS 5

static int led2_timer_id = 0;
static char _tcp_receive_buff[MAX_STRING_LENGTH] = {0};
static bool _tcp_string_received = false;
static char _stdio_input_buff[MAX_STRING_LENGTH] = {0};

uint8_t menu(void)
{
    int choice = 0;
    puts("VIA UNIVERSITY COLLEGE SEP4 IoT Hardware DRIVERS DEMO");
    puts("\tMenu:");
    puts("\t1. LEDs");
    puts("\t2. PIR Sensor");
    puts("\t3. Display");
    puts("\t4. WiFi");
    puts("\t5. stdio");

    printf("Choose a driver to test (1-%d): ", MAX_MENU_OPTIONS);
    stdin_flush(); // Flush any leftover '\n' input from the buffer
    do
    {
        scanf("%d", &choice);
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
    if (pir_get_state() == PIR_NO_MOTION)
    {
        led_off(1);
    }
    else
    {
        led_on(1);
    }
}

void led2_callback(uint8_t id)
{
    static int8_t led_no = 2;
    led_toggle(led_no);
}

void start_stop_callback(uint8_t id)
{
    led_off(3);

    if (timer_get_state(led2_timer_id)) // Check if LED2 timer is active
        timer_pause(led2_timer_id);     // Pause the LED2 timer
    else
        timer_resume(led2_timer_id); // Resume the LED2 timer
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

int main(void)
{
    char welcome_text[] = "Welcome from SEP4 IoT hardware!\n";
    char prompt_text[] = "Type text to send: ";

    led_init();
    pir_init(pir_callback);
    display_init();
    wifi_init();

    // Initialize UART stdio at 115200 baud. Must be same on terminal.
    if (UART_OK != uart_stdio_init(115200)) // Prøv 115200 først, hvis det ikke virker prøv 9600
    {
        led_on(4); // Tænd LED1 for at indikere fejl ved UART init
    }
    sei(); // Enable global interrupts

    // printf("UART stdio klar.\n");
    // printf("Skriv et tal og tryk Enter.\n");

    // if ((led2_timer_id = timer_create_sw(led2_callback, 100)) < 0)
    // {
    //     printf("Fejl ved oprettelse af LED timer!\n");
    // }

    // if (timer_create_sw(start_stop_callback, 5000) < 0)
    // {
    //     printf("Fejl ved oprettelse af LED timer!\n");
    // }


    while (1)
    {
        int x = 0;
        switch (menu())
        {
        case 1:
            printf("Du valgte LED driveren.\n");
            led_blink(3, 500);  // Blink LED3 with 500ms periode
            led_blink(4, 5000); // Blink LED4 with 5000ms periode

            // scanf er blokkerende og venter på input
            if (scanf("%d", &x) == 1)
            {
                display_int(x);
                printf("Du skrev: %d\n", x);
                if ((x > 0) && (x < 5))
                {
                    led_toggle((int8_t)x);
                }
                else
                {
                    printf("Tallet skal være mellem 1 og 4 for at toggle en LED.\n");
                }
            }
            break;
        case 2:
            printf("PIR sensor driver. LED 1 should turn on when motion is detected.\n");
            _delay_ms(10000); // Wait for 10 seconds to allow testing the PIR sensor
            break;
        case 3:
            printf("Display driver. Type a number between -999 and 9999\n");
            if (scanf("%d", &x) == 1)
            {
                display_int(x);
                printf("Du skrev: %d\n", x);
            }
            break;
        case 4:
            printf("WiFi driver demo. Press Reset to exit.\n");
            wifi_command_join_AP("Erlands SEP4", "ViaUC1234");
            wifi_command_create_TCP_connection("10.184.216.102", 23, wifi_line_callback, _tcp_receive_buff);
            wifi_command_TCP_transmit((uint8_t *)welcome_text, strlen(welcome_text));

            while (1)
            {
                if (_tcp_string_received)
                {
                    printf("TCP modtaget: %s\n", _tcp_receive_buff);
                    _tcp_receive_buff[0] = '\0';
                    _tcp_string_received = false;
                }
                if (gets_nonblocking(_stdio_input_buff, MAX_STRING_LENGTH) > 0)
                {
                    printf("Du skrev: %s\n", _stdio_input_buff);
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
        default:
            printf("Ugyldigt valg.\n");
            continue; // Gå tilbage til menuen
        }


//        _delay_ms(200);
    }
    return 0;
}