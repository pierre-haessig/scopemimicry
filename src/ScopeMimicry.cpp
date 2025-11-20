/**
 * @file
 * @brief ScopeMimicry: mimic a digital scope inside the cpu with a ram array.
 * 
 * @author Régis Ruelland <regis.ruelland@laas.fr>
 * @author Jean Alinei <jean.alinei@laas.fr>
 * @author Pierre Haessig <pierre.haessig@centralesupelec.fr>
 */

#include "ScopeMimicry.h"
#include <stdio.h>
#include <string.h>

Scope::Scope(uint16_t length, uint16_t nb_channel, float Ts):
    _length(length),
    _nb_channel(nb_channel),
    _Ts(Ts),
    _nb_channel_effective(0),
    /* Variables for acquisition logic */
    _nb_pretrig(0),
    _acq_state(ACQ_UNTRIG),
    _acq_count(0),
    _mem_idx(0),
    _trig_idx(0),
    _final_idx(length-1),
    _triggFunc(NULL)
{
    _memory = new float[_length*_nb_channel]();
    _names = new const char*[nb_channel];
    _channels = new float32_t *[nb_channel];
};

Scope::~Scope() {
    delete(_memory);
    delete(_names);
    // missing delete _channels?
}

bool Scope::connectChannel(float &channel, const char name[]) {
    if (_nb_channel_effective < _nb_channel) {
        // capture the reference of the variable.
        _channels[_nb_channel_effective] = &channel;
        _names[_nb_channel_effective] = name;
        _nb_channel_effective++;
        return true;
    } else {
        return false;
    }
}

ScopeAcqState Scope::acquire() {
    // Call trigger function if set, else trigger always true
    bool trigg_value;
    if (_triggFunc != NULL) {
        trigg_value = (*_triggFunc)();
    } else {
        trigg_value = true;
    }
    // Aquisition status state machine, state change logic:
    if (_acq_state == ACQ_UNTRIG) {
        if (trigg_value) {
            // State change: ACQ_UNTRIG -> ACQ_TRIG
            _acq_state = ACQ_TRIG;
            // ACQ_UNTRIG exit actions: keep at most nb_pretrig samples + save trigger instant
            _acq_count = (_acq_count<_nb_pretrig) ? _acq_count : _nb_pretrig; //min(_acq_count, _nb_pretrig), i.e. keep at most _nb_pretrig samples before trigger
            _trig_idx = _mem_idx;
        }
    }
    if (_acq_state == ACQ_TRIG) { // including the case if status was changed just before
        if (_acq_count == _length) {
            _acq_state = ACQ_DONE;
            // ACQ_TRIG exit action: save last record index
            _final_idx = (_mem_idx + _length - 1) % _length; // (idx - 1) % _length breaks with unsigned int16 when idx=0
        }

    }
    // Remark: ACQ_DONE state can only be left by calling start()

    // State machine actions:
    if (_acq_state == ACQ_DONE) {
        return _acq_state;

    } else { // _acq_state == ACQ_UNTRIG or ACQ_TRIG, i.e. data being recorded
        // Record channel values at _mem_idx*_nb_channel index
        for (int k_ch=0; k_ch < _nb_channel; k_ch++) {
            int16_t mem_idx_effective = _mem_idx * _nb_channel;
            if (k_ch < _nb_channel_effective) {
                _memory[mem_idx_effective + k_ch] = *(_channels[k_ch]);
            } else {
                _memory[mem_idx_effective + k_ch] = 0.0/0.0; // NaN
            }
        }
        // Increment memory index and acquisition counter
        _mem_idx = (_mem_idx + 1) % _length;
        _acq_count = (_acq_count+1 < _length) ? _acq_count+1 : _length; //min(_acq_count+1, _length)
    }
    return _acq_state;
}

uint16_t Scope::get_pretrig_nsamples() {
    return _nb_pretrig;
}

void Scope::set_pretrig_nsamples(uint16_t n) {
    if (n < 0) _nb_pretrig = 0;
    else if (n > _length) _nb_pretrig = _length;
    else _nb_pretrig = n;
}

void Scope::set_pretrig_ratio(float r) {
    if (r < 0.0) _nb_pretrig = 0;
    else if (r >= 1.0) _nb_pretrig = _length;
    else _nb_pretrig = r *_length;
}

void Scope::start() {
    _acq_state = ACQ_UNTRIG;
    _acq_count = 0;
    _mem_idx = 0;
}

void Scope::set_trigger(bool (*func)()) {
    _triggFunc = func;
}

bool Scope::has_trigged() {
    return _acq_state == ACQ_TRIG || _acq_state == ACQ_DONE;
}

ScopeAcqState Scope::acq_state() {
    return _acq_state;
}

uint16_t Scope::get_final_idx() {
    return _final_idx;
}

uint16_t Scope::get_trig_idx() {
    return _trig_idx;
}

uint8_t * Scope::get_buffer() {
    return (uint8_t * )_memory;
}

uint16_t Scope::get_buffer_size() {
    return (_length * _nb_channel * sizeof(float32_t));
}

uint16_t Scope::get_length() {
    return _length;
}

uint16_t Scope::get_nb_channel() {
    return _nb_channel;
}

uint16_t Scope::get_nb_channel_effective() {
    return _nb_channel_effective;
}

const char *Scope::get_channel_name(uint16_t idx) {
    if (idx < _nb_channel) {
        return this->_names[idx];
    }
    else {
        return "BadIdx";
    }
}

float32_t Scope::get_channel_value(uint32_t index, uint32_t channel_idx) {
    /* to be sure we are not outside the buffer */
    index = (index) % _length;
    return _memory[(index * _nb_channel) + channel_idx];
}

void Scope::reset_dump() {
	dump_state = DUMP_READY;
}

enum ScopeDumpState Scope::get_dump_state() {
	return dump_state;
}

char* Scope::dump_datas() {
	uint16_t n_datas = _length * _nb_channel;
	switch (dump_state) {
		case DUMP_READY:
            // Dump header start
            strcpy(_dumped_data, "#");
            // Next state
            _dump_time = true;
            _dump_channel_idx = 0;
            _dump_count = 0;
			dump_state = DUMP_NAMES;
		break;
		case DUMP_NAMES:
            if (_dump_time) { // Dump time to header
                strcpy(_dumped_data, "time,");
                _dump_time = false;
            } else { // Dump each channel name + "," to header
                if (_dump_channel_idx < this->get_nb_channel())
                {
                    strncpy(_dumped_data, get_channel_name(_dump_channel_idx), 100);
                    _dumped_data[100]='\0'; // make sure to null-terminate the string before concatenate
                    if (_dump_channel_idx < this->get_nb_channel()-1) {
                        strcat(_dumped_data, ","); // append "," except for last channel
                    }
                    // Next sub-state
                    _dump_channel_idx +=1;
                }
                else
                {
                    strcpy(_dumped_data, "\n");
                    // Next state
                    dump_state = DUMP_FINAL_IDX;
                }
            }
		break;
		case DUMP_FINAL_IDX:
			sprintf(_dumped_data, "## %d\n", _length-1); // virtual final_idx, since data is dumped in recording order
            // Next state
			dump_state = DUMP_DATA;
            _dump_time = true;
		break;
		case DUMP_DATA:
            if (_dump_time) { // dump time value
                float32_t time = (_dump_count - _nb_pretrig) * _Ts;
                sprintf(_dumped_data, "%08x\n", *((uint32_t *) &time));
                // next sub-state: dump data, first channel
                _dump_time = false;
                _dump_channel_idx = 0;
            } else { // dump channel data
                // Location of data in _memory buffer: first_idx + dump_count
                uint16_t mem_idx = (_dump_count + get_final_idx() + 1) % _length;
                int16_t mem_idx_effective = mem_idx * _nb_channel + _dump_channel_idx;
                sprintf(_dumped_data, "%08x\n", *((uint32_t *) _memory + mem_idx_effective));
                
                // Next state or sub-state
                if (_dump_channel_idx < _nb_channel - 1) {
                    _dump_channel_idx +=1; // next channel
                }
                else {
                    if (_dump_count < _length - 1) {
                        _dump_count += 1; // next instant
                        _dump_time = true; // dump time
                    }
                    else {
                        dump_state = DUMP_FINISHED;
                    }
                }
            }
		break;
		case DUMP_FINISHED:
            // Dump one last line with " "
			strcpy(_dumped_data, " ");
		break;
		}
	return _dumped_data;
}