/*Scope test */

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <cstring>

#include "ScopeMimicry.h"

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

bool trig_global; // trigger value returned by trig_global_fun()

// crude trigger function which returns the global trig_global value
bool trig_global_fun(void) {
    return trig_global;
}

/*Test the Scope data acquisition phase on a test sequence
*/
bool test_scope_acquire(uint16_t length, uint16_t nb_pretrig,
                        ScopeInput2ch in, ScopeAcqResults out_exp) {
    bool success=true;
    // Init scope
    int nb_channels = 2;
    ScopeMimicry scope(length, nb_channels);
    scope.set_pretrig_nsamples(nb_pretrig);

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

// Base test. Bug: the last instant is missed
bool test_scope_acquire1() {
    printf("Scope acquisition test 1...\n");
    bool success;

    // Scope parameters
    const uint16_t length = 3;
    const int n_channels = 2;
    uint16_t nb_pretrig = 0;

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
        10, 20, 11, 21, 12, 22
    };
    ScopeAcqResults out_exp = {
        .final_idx = 2,
        .status = status_seq,
        .memory = memory_exp
    };

    success = test_scope_acquire(length, nb_pretrig, in, out_exp);
    return success;
}

// Base variant, with trigger at 2nd instant
bool test_scope_acquire2() {
    printf("Scope acquisition test 2 (trig k=1)...\n");
    bool success;

    // Scope parameters
    const uint16_t length = 3;
    const int n_channels = 2;
    uint16_t nb_pretrig = 0;

    // Input test sequences
    const int length_seq = 5;
    bool start_seq[length_seq]    = { 1,  0,  0,  0,  0};
    bool trig_seq[length_seq]     = { 0,  1,  0,  0,  0};
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
    uint16_t status_seq[length_seq] = {0,1,1,1,2};
    float32_t memory_exp[length*n_channels] = {
        13, 23, 11, 21, 12, 22
    };
    ScopeAcqResults out_exp = {
        .final_idx = 0,
        .status = status_seq,
        .memory = memory_exp
    };

    success = test_scope_acquire(length, nb_pretrig, in, out_exp);
    return success;
}

// Base variant, with trigger at 2nd instant + nb_pretrig
// -> expect result be like Base
bool test_scope_acquire3() {
    printf("Scope acquisition test 3 (trig k=1, nb_pretrig=1)...\n");
    bool success;

    // Scope parameters
    const uint16_t length = 3;
    const int n_channels = 2;
    uint16_t nb_pretrig = 1;

    // Input test sequences
    const int length_seq = 5;
    bool start_seq[length_seq]    = { 1,  0,  0,  0,  0};
    bool trig_seq[length_seq]     = { 0,  1,  0,  0,  0};
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
    uint16_t status_seq[length_seq] = {0,1,1,2,2};
    float32_t memory_exp[length*n_channels] = {
        10, 20, 11, 21, 12, 22 // Same as Base test
    };
    ScopeAcqResults out_exp = {
        .final_idx = 2,
        .status = status_seq,
        .memory = memory_exp
    };

    success = test_scope_acquire(length, nb_pretrig, in, out_exp);
    return success;
}

// Special bug for nb_pretrig=100%: scope never stops!
bool test_scope_acquire4() {
    printf("Scope acquisition test 4 (100%% nb_pretrig)...\n");
    bool success;

    // Scope parameters
    const uint16_t length = 3;
    const int n_channels = 2;
    uint16_t nb_pretrig = length;

    // Input test sequences
    const int length_seq = 7;
    bool start_seq[length_seq]    = { 1,  0,  0,  0,  0,  0,  0};
    bool trig_seq[length_seq]     = { 0,  0,  0,  1,  0,  0,  0};
    float32_t ch1_seq[length_seq] = {10, 11, 12, 13, 14, 15, 16};
    float32_t ch2_seq[length_seq] = {20, 21, 22, 23, 24, 25, 26};

    ScopeInput2ch in = {
        .length = length_seq,
        .start = start_seq,
        .trig = trig_seq,
        .ch1 = ch1_seq,
        .ch2 = ch2_seq
    };

    // Expected output:
    uint16_t status_seq[length_seq] = {0,0,0, 2,2,2,2};
    float32_t memory_exp[length*n_channels] = {
        10, 20, 11, 21, 12, 22 // Same as Base test
    };
    ScopeAcqResults out_exp = {
        .final_idx = 2,
        .status = status_seq,
        .memory = memory_exp
    };

    success = test_scope_acquire(length, nb_pretrig, in, out_exp);
    return success;
}

// Test with restart: works, albeit with same bug as in Base
bool test_scope_acquire5() {
    printf("Scope acquisition test 5 (restart k=4)...\n");
    bool success;

    // Scope parameters
    const uint16_t length = 3;
    const int n_channels = 2;
    uint16_t nb_pretrig = 0;

    // Input test sequences
    const int length_seq = 9;
    bool start_seq[length_seq]    = { 1,  0,  0,  0,  1,  0,  0,  0,  0};
    bool trig_seq[length_seq]     = { 1,  1,  1,  1,  1,  1,  1,  1,  1};
    float32_t ch1_seq[length_seq] = {10, 11, 12, 13, 14, 15, 16, 17, 18};
    float32_t ch2_seq[length_seq] = {20, 21, 22, 23, 24, 25, 26, 27, 28};

    ScopeInput2ch in = {
        .length = length_seq,
        .start = start_seq,
        .trig = trig_seq,
        .ch1 = ch1_seq,
        .ch2 = ch2_seq
    };

    // Expected output:
    uint16_t status_seq[length_seq] = {1,1,1, 2, 1,1,1, 2,2};
    float32_t memory_exp[length*n_channels] = {
        14, 24, 15, 25, 16, 26
    };
    ScopeAcqResults out_exp = {
        .final_idx = 2,
        .status = status_seq,
        .memory = memory_exp
    };

    success = test_scope_acquire(length, nb_pretrig, in, out_exp);
    return success;
}

// Forgotten trigger function
bool test_scope_acquire_notrigger() {
    printf("Scope acquisition test without trigger function...\n");
    bool success=true;

    // Scope parameters
    const uint16_t length = 1;
    int nb_channels = 2;
    ScopeMimicry scope(length, nb_channels);
    static float32_t ch1=10, ch2=20; // is static needed?
    scope.connectChannel(ch1, "ch1");
    scope.connectChannel(ch2, "ch2");
    // missing setting trigger function
    //scope.set_trigger(&trig_global_fun);

    ScopeAcqState acq_state = scope.acquire();

    // 2. Scope memory content
    float32_t *memory = (float32_t*) scope.get_buffer();
    if (acq_state != ACQ_TRIG) {
        success = false;
        printf("- Acquisition status mismatch: was %d, expected %d\n",
               acq_state, ACQ_TRIG);
    }
    if (memory[0] != 10. && memory[1] != 20.) {
        success = false;
        printf("- Memory content mismatch: is [%g, %g], expected [10.0, 20.0]\n",
               memory[0], memory[1]);
    }
    if (success) {
        printf("-> SUCCESS\n");
    } else {
        printf("-> FAILED\n");
    }
    return success;
}

/***  Scope data dump test 
 * See https://www.h-schmidt.net/FloatConverter/IEEE754.html on how numbers are stored as float32
 * 
 * note: in principle, Intel and ARM processors stores float numbers in Little Endian format
 * however, the hexademical formatting is done by reading each float32 as an int32
 * which is formatted as a string with the "%08x" format string.
 * This write the number in the occidental left to right, which corresponds to Big Endian format!
 * ***/
bool test_scope_dump() {
    bool success=true;
    printf("Scope data dump test...\n");

    // Setup: a short scope acquisition to fill the memory
    const uint16_t length = 3;
    int nb_channels = 2;
    ScopeMimicry scope(length, nb_channels);
    static float32_t ch1,ch2; // is static needed?
    scope.connectChannel(ch1, "ch1");
    scope.connectChannel(ch2, "ch2");
    trig_global = true; // always true trigger
    scope.set_trigger(&trig_global_fun);

    ScopeAcqState acq_state;
    ch1=10; ch2=20; // 1.25*2^3,  1.25*2^4.   Big Endian hexa: 41200000, 41a00000
    acq_state = scope.acquire();
    ch1=11; ch2=21; // 1.375*2^3, 1.3125*2^4. Big Endian hexa: 41300000, 41a80000
    acq_state = scope.acquire();
    ch1=12; ch2=22; // 1.5*2^3,   1.375*2^4.  Big Endian hexa: 41400000, 41b00000
    acq_state = scope.acquire();
    // After this, scope memory is expected to contain
    // [10, 20, 11, 21, 12, 22], with final_idx=2

    // Expected data dump:
    const char* dump_exp = 
        "#ch1,ch2,\n" //10 chars
        "## 2\n" // 5 chars
        "41200000\n" // 9 * 6 chars
        "41a00000\n"
        "41300000\n"
        "41a80000\n"
        "41400000\n"
        "41b00000\n";
    //printf("str len: %d\n", (int)strlen(dump_exp)); len==69

    // Test start: dump scope data
    char dump[100]; // string to store the dump, which length should be larger than expected dump
    scope.reset_dump();

    int dump_strlen = 0;
	while (scope.get_dump_state() != finished) {
		dump_strlen += sprintf(dump+dump_strlen, "%s", scope.dump_datas());
	}
    //printf("%s", dump); // always print data dump

    success &= (strcmp(dump, dump_exp) == 0);

    if (success) {
        printf("-> SUCCESS\n");
    } else {
        printf("Difference in dump data\n");
        printf("- Expected:\n");
        printf("%s", dump_exp);
        printf("- Got instead:\n");
        printf("%s", dump);

        // find the first character difference
        int i = 0;
        while (dump[i] && dump_exp[i] && (dump[i] == dump_exp[i])) {
            i++;
        }
        printf("Difference at character %d ('%c' vs '%c')\n", i+1, dump[i], dump_exp[i]);

        printf("-> FAILED\n");
    }
    return success;
}



/*** Scope demo ***/

// Static variables the the scope acquisitions will read
static float32_t Ilow1;
static float32_t Ilow2;

// trigger function for the demo
bool mytrigger(void) {
    return true;
    return (Ilow1 > 0.1);
}

/* Runs scope acquisition and then print memory content
This is a demo rather than a test because it doesn't compare the result against a reference
*/
void scope_demo() {
    printf("Scope acquisition demo...\n");
    float32_t t;
    const float32_t Ts = 1e-3; // sampling time (s)
    uint8_t *buffer;
    uint16_t buffer_size = 0;
    int length_sim = 40; // simulation length -> 40 ms
    uint16_t length = 10; // scope capture length
    const int n_channels = 2;
    ScopeMimicry scope(length, n_channels);

    scope.connectChannel(Ilow1, "iLow1");
    scope.connectChannel(Ilow2, "iLow2");
    scope.set_trigger(&mytrigger);
    //scope.set_pretrig_ratio(0.5); // defaults to 0.0
    //scope.start(); // not needed: automatic after first initialization

    for (int k=0; k<length_sim; k++)  {
        t = k*Ts;
        Ilow1 = t*1000;
        Ilow2 = (k % 10)*0.1; // sawtooth 0 -> 0.9
        if (k == 20) {
            scope.start();
        }
        ScopeAcqState acq_state = scope.acquire();
        printf("k=%2d, t=%.3f: ", k, t);
        printf("Ilow1=%.1f (acq_state=%d)\n", Ilow1, acq_state);
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

    //scope_demo();

    printf("\nScope acquisition tests...\n");
    bool success = true;
    success &= test_scope_acquire1();
    success &= test_scope_acquire2();
    success &= test_scope_acquire3();
    success &= test_scope_acquire4();
    success &= test_scope_acquire5();
    success &= test_scope_acquire_notrigger();
    success &= test_scope_dump();

    if (success) {
        printf("GLOBAL SUCCESS\n");
    } else {
        printf("GLOBAL FAILED\n");
    }
}
