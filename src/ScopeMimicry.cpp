#include "ScopeMimicry.h"

ScopeMimicry::ScopeMimicry(uint16_t length, uint16_t nb_channel): 
    _length(length),
    _nb_channel(nb_channel),
    _acq_counter(0),
    _effective_chan_number(0),
    _trigged(false),
    _delay(0),
    _final_idx(length-1)
{
    _memory = new float[_length*_nb_channel]; 
    _names = new const char*[nb_channel];
    _channels = new float32_t *[nb_channel];
};

ScopeMimicry::~ScopeMimicry() {
    delete(_memory);
    delete(_names);
}

/**
 * @brief it get the reference of a static variable (channel) to be able to record its values when
 * it is necessary.
 *
 * @param channel : reference to a static variable
 * @param name: name of the variable you want to have after retrieving. 
 */
void ScopeMimicry::connectChannel(float &channel, const char name[]) {
    if (_effective_chan_number < _nb_channel) {
        // capture the reference of the variable.
        _channels[_effective_chan_number] = &channel;
        _names[_effective_chan_number] = name;
        _effective_chan_number++;
    }
    // TODO: return errno code ?
}

/**
 * @brief add one value of each channel into its internal memory if the trigger has been
 * activated
 */
uint16_t ScopeMimicry::acquire() {

    if (!_trigged) {
        _trigged =  _triggFunc();
        _trigged_counter = 0;
    } else {
        _trigged_counter++;
    }

    if (_trigged_counter < _delay_complement) {
        for (int k_ch=0; k_ch < _effective_chan_number; k_ch++) {
            _memory[(_acq_counter * _nb_channel) + k_ch] = *(_channels[k_ch]);
        }
        _acq_counter = (_acq_counter + 1)% _length;
    } else {
        _final_idx = _acq_counter;
    }
    uint16_t status= (_trigged_counter*100) / _delay_complement;

    return  (status < 100) ? status:100;
}

/**
 * @brief define delay to record data before trigger between. delay is in [0, 1] 
 *
 * @param d delay
 */
void ScopeMimicry::set_delay(float32_t d) {
    if (d < 0.0) _delay = 0.0;
    else if (d >= 1.0) _delay = _length;
    else _delay = d *_length;
    _delay_complement = _length - _delay + 1; 
}

/**
 * @brief reset the trigger and enable to record new data
 */
void ScopeMimicry::start() {
    _acq_counter = 0;
    _trigged = false;
}

uint16_t ScopeMimicry::get_final_idx() {
    return _final_idx;
}


/**
 * @brief get a function without arguments which should return a boolean to enable
 * recording
 *
 * @param func: handle to a function which shall return a boolean. 
 */
void ScopeMimicry::set_trigger(bool (*func)()) {
    _triggFunc = func;
}

uint8_t * ScopeMimicry::get_buffer() {
    return (uint8_t * )_memory;
}

uint16_t ScopeMimicry::get_buffer_size() {
    return (_length * _nb_channel * sizeof(float32_t));
}

uint16_t ScopeMimicry::get_length() {
    return _length;
}

uint16_t ScopeMimicry::get_nb_channel() {
    return _nb_channel;
}

bool ScopeMimicry::has_trigged() {
    return _trigged;
}

const char *ScopeMimicry::get_channel_name(uint16_t idx) {
    if (idx < _nb_channel) {
        return this->_names[idx];
    }
    else {
        return "BadIdx";
    }
}
