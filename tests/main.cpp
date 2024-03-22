#include <iostream> 
#include <math.h> 
#include <stdio.h> 
#include <stdint.h>

#include "ScopeMimicry.h"

static float32_t Ilow1;
static float32_t Ilow2;

bool mytrigger(void) {
    return (Ilow1 > 0.1);
}

int main(void) {
    float32_t f0 = 50.0;
    float32_t t;
    const float32_t Ts = 100e-6;
    uint8_t *buffer;
    uint16_t buffer_size = 0;
    ScopeMimicry scope(201, 2);
    scope.connectChannel(Ilow1, "iLow1");
    scope.connectChannel(Ilow2, "iLow2");
    scope.set_trigger(&mytrigger);
    scope.set_delay(0.5);
    for (t=0; t <  4./f0; t+=Ts)  {
        Ilow1 = (t < .5/f0) ? cos(2 * M_PI * f0 * t): 2*cos(2 * M_PI * f0 * t); 
        Ilow2 = sin(2 * M_PI * f0 * t);
        if (t > 2.0/f0) scope.start();
        uint16_t status = scope.acquire();
    }

    buffer = scope.get_buffer();
    buffer_size = scope.get_buffer_size()>>2;
    printf("%s,", scope.get_channel_name(0));
    printf("%s\n", scope.get_channel_name(1));
    for (uint32_t k=0;k < (buffer_size);k=k+2)
    {
        printf("%08x, %08x\n", *((uint32_t *)buffer+k), *((uint32_t *)buffer+k+1));
    }
}
