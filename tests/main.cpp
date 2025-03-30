/*Scope test */

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "ScopeMimicry.h"

static float32_t Ilow1;
static float32_t Ilow2;

const int n_channels = 2;

bool mytrigger(void) {
    return true;
    return (Ilow1 > 0.1);
}

int main(void) {
    float32_t t;
    const float32_t Ts = 1e-3; // sampling time (s)
    uint8_t *buffer;
    uint16_t buffer_size = 0;
    int length_sim = 40; // simulation length -> 40 ms
    uint16_t length = 10; // scope capture length
    ScopeMimicry scope(length, n_channels);
    scope.connectChannel(Ilow1, "iLow1");
    scope.connectChannel(Ilow2, "iLow2");
    scope.set_trigger(&mytrigger);
    //scope.set_delay(0.5);
    //scope.start();

    for (int k=0; k<length_sim; k++)  {
        t = k*Ts;
        Ilow1 = t*1000;
        Ilow2 = (k % 10)*0.1; // sawtooth 0 -> 0.9
        if (k > 20) {
            scope.start();
        }
        uint16_t status = scope.acquire();
        printf("k=%2d, t=%.3f: ", k, t);
        printf("Ilow1=%.1f (status=%d)\n", Ilow1, status);

    }

    buffer = scope.get_buffer();
    buffer_size = scope.get_buffer_size()>>2;
    printf("%s,", scope.get_channel_name(0));
    printf("%s\n", scope.get_channel_name(1));
    printf("final_idx: %d\n", scope.get_final_idx());

    for (uint32_t k=0;k < (buffer_size);k=k+n_channels)
    {
        printf("%08x, %08x (%.2f A, %.2f A)\n",
            *((uint32_t *)buffer+k), *((uint32_t *)buffer+k+1),
            *((float32_t *)buffer+k), *((float32_t *)buffer+k+1)
        );
    }
}
