#include <iostream> 
#include <math.h> 
#include <stdio.h> 
#include <stdint.h>

#include "ScopeMimicry.h"

static float32_t Ilow1;
static float32_t Ilow2;

bool mytrigger(void) {
    return (Ilow1 > 0);
}

int main(void) {
    float32_t f0 = 50.0;
    float32_t t;
    const float32_t Ts = 100e-6;

    ScopeMimicry scope(201, 2);
    scope.connectChannel(Ilow1, "iLow1");
    scope.connectChannel(Ilow2, "iLow2");
    scope.set_trigger(mytrigger);
    scope.set_delay(0.0);
    for (t=0; t <  3./f0; t+=Ts)  {
        Ilow1 = (t < 1/f0) ? cos(2 * M_PI * f0 * t): 2*cos(2 * M_PI * f0 * t); 
        Ilow2 = sin(2 * M_PI * f0 * t);
        uint16_t status = scope.acquire();
        printf("status = %d\n", status);
    }
}
