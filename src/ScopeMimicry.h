#ifndef _SCOPEMIMICRY_H_
#define _SCOPEMIMICRY_H_

#include <stdint.h>
typedef float float32_t;

class ScopeMimicry {
public:

    ScopeMimicry(uint16_t length, uint16_t nb_channel);
    ~ScopeMimicry();
    void connectChannel(float &channel, const char name[]);
    /**
     * @brief add one data in memories for all channels if trigger has been true one time.
     *
     * @return an integer percentage of acquisition. then if you have 100 the
     * acquisition is finished.
     */
    uint16_t acquire(); // or trace ?
    // void dump();
    /**
     * @brief reset acquisition trigger
     */
    void start();

    /**
     * @brief define the trigger 
     *
     * @param func must a function without arguments but which return a boolean value
     * when its true the data are recorded in memory by the way of the `acquire()`
     * method.
     */
    void set_trigger(bool (*func)());
    /**
     * @brief define a delay to have some datas before the trigger instant between 
     * the delay is a relative value between [0, 1]
     * delay of 0 : mean no delay after trigger
     *
     * @param delay (float value)
     */
    void set_delay(float32_t delay);
    /**
     * @brief as there's a circular buffer, the last point recorded can be anywhere in
     * this buffer. this function give to us the index of this last value. 
     * the index is between 0 and lenght-1
     *
     * @return 
     */
    uint16_t get_final_idx();

    /**
     * @brief get pointer to the memory of data recorded
     *
     * @return 
     */
    uint8_t *get_buffer();

    /**
     * @brief return number of byte used by the memory
     *
     * @return 
     */
    uint16_t get_buffer_size();
    /**
     * @brief return the length of one channel recording
     *
     * @return 
     */
    uint16_t get_length();
    /**
     * @brief number of channel recorded
     *
     * @return 
     */
    uint16_t get_nb_channel();
    /**
     * @brief return the status of the trigger
     *
     * @return 
     */
    bool has_trigged();
    const char* get_channel_name(uint16_t idx);

private:
    uint16_t _length;
    uint16_t _nb_channel;
    uint16_t _acq_counter;
    uint16_t _effective_chan_number;
    bool _trigged;
    uint16_t _delay;

    float32_t **_channels;
    float32_t *_channel_numbers;
    const char **_names;
    float32_t *_memory;

    uint16_t _decimation;
    bool (*_triggFunc)();
    uint16_t _delay_complement;
    uint16_t _trigged_counter;
    uint16_t _final_idx;
};


#endif
