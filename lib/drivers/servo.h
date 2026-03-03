/*
 * servo_pwm.h
 *
 * Created: 07/05/2021 14:04:20
 *  Author: ERL
 */ 
#pragma once

#include <stdint.h>

typedef enum {PWM_NORMAL, PWM_INVERTED} pwmMode_t;
typedef enum {PWM_A, PWM_B} pwmChannel_t;

// Initialize servo. Use Timer 4
void servo_init(pwmMode_t mode);

// Start PWM (all channels)
void servo_start();

// Stop PWM (all channels)
void servo_stop();

// servo_setAngle. Position +/-90 degrees
uint16_t servo_setAngle(pwmChannel_t channel, int8_t position);

