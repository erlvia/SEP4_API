//#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "uart_stdio.h"
#include "led.h"
#include "pir.h"
#include "display.h"
#include "timer.h"

static int led2_timer_id = 0;

void pir_callback(void)
{
        if(pir_get_state() == PIR_NO_MOTION)
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

    if(timer_get_state(led2_timer_id)) // Check if LED2 timer is active
        timer_pause(led2_timer_id); // Pause the LED2 timer
    else
        timer_resume(led2_timer_id); // Resume the LED2 timer
}

int main(void)
{
    led_init();
    pir_init(pir_callback);
    display_init();
    // Initialize UART stdio at 115200 baud. Must be same on terminal.
    if(UART_OK != uart_stdio_init(115200))
    {
        led_on(4); // Tænd LED1 for at indikere fejl ved UART init
    }

    printf("UART stdio klar.\n");
    printf("Skriv et tal og tryk Enter.\n");

    // if((led2_timer_id = timer_create_sw(led2_callback, 100)) < 0)
    // {
    //     printf("Fejl ved oprettelse af LED timer!\n");
    // }

    // if(timer_create_sw(start_stop_callback, 5000) < 0)
    // {
    //     printf("Fejl ved oprettelse af LED timer!\n");
    // }

    // led_blink(3, 500); // Blink LED3 with 500ms periode
    // led_blink(4, 5000); // Blink LED4 with 5000ms periode

    sei(); // Enable global interrupts

    while (1)
    {
        int x = 0;

        printf("\n> ");
        fflush(stdout); // typisk ikke nødvendigt, men ok

        // scanf er blokkerende og venter på input
        if (scanf("%d", &x) == 1) 
        {
            display_int(x);
            printf("Du skrev: %d\n", x);
            if((x>0) && (x<5))
            {
                led_toggle((int8_t)x);
            }
            else
            {
                printf("Tallet skal være mellem 1 og 4 for at toggle en LED.\n");
            }
        } 
        else 
        {
            // Hvis scanf fejler (f.eks. bogstaver), ryd input-linjen
            printf("Ugyldigt input. Prøv igen.\n");
            int ch;
            do 
            { 
                ch = getchar(); 
                putchar(ch);
            } while (ch != '\n' && ch != EOF);
        }


        _delay_ms(200);
    }
    return 0;
}