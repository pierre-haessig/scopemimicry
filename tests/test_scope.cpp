/*Scope test */

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "ScopeMimicry.h"

static float32_t Ilow1;
static float32_t Ilow2;

const int n_channels = 2;

bool trig_global;

// crude trigger function which returns the global trig_global value
bool trig_global_fun(void) {
    return trig_global;
}

bool mytrigger(void) {
    return true;
    return (Ilow1 > 0.1);
}

/* Scope acquisition test input data*/
struct ScopeInput2ch {
    int length;// length of test sequences
    bool *start;// point to start events sequence
    bool *trig;// point to trigger events sequence
    float32_t *ch1; // pointer to ch1 sequence
    float32_t *ch2; // pointer to ch1 sequence
};

/* Scope acquisition test data for expected output  */
struct ScopeAcqResults {
    uint16_t final_idx; // index of the last acquisition in the memory
    uint16_t *status;// point to acquisition status (size = test sequence length)
    float32_t *memory; // pointer to expected scope memory content (size nb_channel * scope length)
};


/*Test the Scope data acquisition phase on a test sequence
*/
bool test_scope_acquire(uint16_t length, float32_t delay,
                        ScopeInput2ch in, ScopeAcqResults out_exp) {
    bool success=true;
    // Init scope
    int nb_channels = 2;
    ScopeMimicry scope(length, nb_channels);
    scope.set_delay(delay);

    static float32_t ch1, ch2; // is static needed?
    scope.connectChannel(ch1, "ch1");
    scope.connectChannel(ch2, "ch2");
    scope.set_trigger(&trig_global_fun);

    // Acquisition status sequence
    uint16_t *status_seq = (uint16_t*)malloc(in.length * sizeof(uint16_t));
    for (int k=0; k<in.length; k++) {
        if (in.start[k]) {
            scope.start();
        }
        trig_global = in.trig[k];
        ch1 = in.ch1[k];
        ch2 = in.ch2[k];
        status_seq[k] = scope.acquire();
    }

    // Compare to expected results
    // 0. Index of the last acquisition in the memory
    if (scope.get_final_idx() != out_exp.final_idx) {
        success = false;
        printf("- Final index mismatch: is %d, expected %d\n",
            scope.get_final_idx(), out_exp.final_idx);
    }
    // 1. Acquisition status sequence
    for (int k=0; k<in.length; k++) {
        if (status_seq[k] != out_exp.status[k]) {
            success = false;
            printf("- Acquisition status mismatch (k=%d): was %d, expected %d\n",
                k, status_seq[k], out_exp.status[k]);
        }
    }

    // 2. Scope memory content
    float32_t *memory = (float32_t*) scope.get_buffer();
    for (int i=0; i<length*nb_channels; i++) {
        if (memory[i] != out_exp.memory[i]) {
            success = false;
            printf("- Memory content mismatch (i=%d): is %g, expected %g\n",
                i, memory[i], out_exp.memory[i]);
        }
    }

    free(status_seq);
    if (success) {
        printf("-> SUCCESS\n");
    } else {
        printf("-> FAILED\n");
    }
    return success;
}

bool test_scope_acquire1() {
    printf("Scope acquisition test 1...\n");
    bool success;

    // Scope parameters
    const uint16_t length = 3;
    const int n_channels = 2;
    float32_t delay = 0.0;

    // Input test sequences
    const int length_seq = 5;
    bool start_seq[length_seq]    = { 1,  0,  0,  0,  0};
    bool trig_seq[length_seq]     = { 1,  0,  0,  0,  0};
    float32_t ch1_seq[length_seq] = {10, 11, 12, 13, 14};
    float32_t ch2_seq[length_seq] = {20, 21, 22, 23, 24};

    ScopeInput2ch in = {
        .length = length_seq,
        .start = start_seq,
        .trig = trig_seq,
        .ch1 = ch1_seq,
        .ch2 = ch2_seq
    };

    // Expected output:
    uint16_t status_seq[length_seq] = {1,1,1,2,2};
    float32_t memory_exp[length*n_channels] = {
        12, 22, 10, 20, 11, 21,
    };
    ScopeAcqResults out_exp = {
        .final_idx = 0,
        .status = status_seq,
        .memory = memory_exp
    };

    success = test_scope_acquire(length, delay, in, out_exp);
    return success;
}

/* Legacy test which runs the scope and then print memory content
but doesn't compare against a reference
*/
void test_legacy() {
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

int main(void) {
    bool success = true;
    //test_legacy();
    success &= test_scope_acquire1();

    if (success) {
        printf("GLOBAL SUCCESS\n");
    } else {
        printf("GLOBAL FAILED\n");
    }
}
