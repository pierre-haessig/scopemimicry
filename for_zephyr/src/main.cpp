#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/console/console.h>
#include <zephyr/logging/log.h>
#include "ScopeMimicry.h"
#include <arm_math.h>

static float32_t Ilow1;
static float32_t Ilow2;

LOG_MODULE_REGISTER(test_scope);

bool mytrigger(void) {
    return (Ilow1 > 0);
}

int main(void) {
    float32_t f0 = 50.0;
    float32_t t;
    const float32_t Ts = 100e-6;


    ScopeMimicry scope(1024, 2);


    scope.connectChannel(Ilow1, "iLow1");
    scope.connectChannel(Ilow2, "iLow2");
    scope.set_trigger(mytrigger);
    scope.set_delay(0.0);
    for (t=0; t <  2./f0; t+=Ts)  {
        Ilow1 = (t < 1/f0) ? arm_cos_f32(2 * PI * f0 * t): 2*cos(2 * PI * f0 * t); 
        Ilow2 = arm_sin_f32(2 * PI * f0 * t);
        scope.acquire();
    }
    //scope.dump();
    uint8_t *buf = scope.get_buffer();
    uint16_t size = scope.get_buffer_size();
    printk("float size: %i\n", sizeof(float));
    printk("size: %i\n", size);
    printk("length: %i\n", scope.get_length());
    printk("nb chan: %i\n", scope.get_nb_channel());
    // for (int k=0; k < size; k+=4) {
    //     LOG_RAW("%08x\n", *(uint32_t *)(buf+k));
    // }
    LOG_HEXDUMP_INF(buf, size, "essai");
    return 0;
}
