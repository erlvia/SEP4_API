#pragma once
#include <stdint.h>

void proximity_init();
uint16_t proximity_measure();
uint16_t proximity_get_distance();

