/**
 * @file
 * @brief ScopeMimicry: mimic a digital scope inside the cpu with a ram array.
 * 
 * @author Régis Ruelland <regis.ruelland@laas.fr>
 * @author Jean Alinei <jean.alinei@laas.fr>
 * @author Pierre Haessig <pierre.haessig@centralesupelec.fr>
 */

#ifndef _SCOPEMIMICRY_H_
#define _SCOPEMIMICRY_H_

#include <cstdint>
typedef float float32_t;

/* Scope data dump state*/
enum e_dump_state {
	initialized,
	names,
	final_idx,
	datas,
	finished
};

/*Scope acquisition state*/
enum ScopeAcqState {
	ACQ_UNTRIG, // data being recorded, before trigger
	ACQ_TRIG,   // data being recorded, after trigger
	ACQ_DONE    // data recording finished
};

class ScopeMimicry {
public:
	/**
	 * @brief create a scope with RAM storage for recorded data
	 *
	 * @param length number of samples to record, for each variable
	 * @param nb_channel number of variables to record
	 */
	ScopeMimicry(uint16_t length, uint16_t nb_channel);
	
	/**
	 * @brief destry scope instance and release RAM
	 */
	~ScopeMimicry();
	
	/**
	 * @brief store a reference to static variable (channel), associated to a channel name
	 * 
	 * @details 
	 * a reference is stored be able to record its value at each acquire() call.
	 *
	 * @param channel static float variable (a reference to which is stored)
	 * @param name channel name (when retrieving scope data)
	 * 
	 * @return true if channel addition was successful
	 */
	bool connectChannel(float &channel, const char name[]);
	
	/**
	 * @brief store one value of each channel into internal memory if scope is recording
	 * 
	 * @details 
	 * data recording happens after scope start in two cases:
	 * - before trigger, to allow recording nb_pretrig instants
	 * - after trigger, as long as less than (length) sample have been recorded
	 *
	 * @return scope acquistion state (ACQ_UNTRIG, ACQ_TRIG or ACQ_DONE)
	 */
	ScopeAcqState acquire();
	
	/**
	 * @brief get number of data samples recorded *before* trigger
	 *
	 * @return number of samples in [0, length]
	 */
	
	uint16_t get_pretrig_nsamples();
	
	/**
	 * @brief set number of data samples recorded *before* trigger
	 *
	 * @param n number of samples in [0, length]
	 */
	void set_pretrig_nsamples(uint16_t n);
	
	/**
	 * @brief set amount of data recorded *before* trigger, as a fraction of scope length
	 *
	 * @param r fraction of scope length in [0.0, 1.0]
	 */
	void set_pretrig_ratio(float r);
	
	/**
	 * @brief force (re)start scope data acquisition (go to state ACQ_UNTRIG)
	 */
	void start();
	
	/**
	 * @brief define the trigger function
	 *
	 * @param func handle to a function without arguments which returns a boolean trigger value
	 */
	void set_trigger(bool (*func)());

	/**
	 * @brief return true if scope has been trigged since the last (re)start
	 */
	bool has_trigged();

	/**
	 * @brief return data acquision state (ACQ_UNTRIG, ACQ_TRIG or ACQ_DONE)
	 */
	ScopeAcqState acq_state();
	
	/**
	 * @brief memory index of the last data points recorded
	 * 
	 * @details
	 * scope memory is used as a circular buffer, so the last point is at final_idx,
	 * while the first one is at (final_idx+1) % length
	 *
	 * @return index in [0, lenght-1]
	 */
	uint16_t get_final_idx();

	/**
	 * @brief memory index of the data point recorded at first trigger instant
	 *
	 * @return index in [0, lenght-1]
	 */
	uint16_t get_trig_idx();

	/**
	 * @brief get pointer to the memory of data recorded, seen as bytes
	 */
	uint8_t *get_buffer();

	/**
	 * @brief return number of bytes used by the memory
	 */
	uint16_t get_buffer_size();
	
	/**
	 * @brief return the length of one channel recording
	 */
	uint16_t get_length();
	
	/**
	 * @brief number of channels that the scope can record
	 */
	uint16_t get_nb_channel();

	/**
	 * @brief effective number of channels being recorded
	 */
	uint16_t get_nb_channel_effective();
	
	/**
	 * @brief return name of the channel "idx"
	 *
	 * @param idx : number of the channel, in [0, nb_channel-1]
	 * @return a c-string (const char*)
	 */
	const char *get_channel_name(uint16_t idx);
	
	/**
	 * @brief get one value of one channel
	 *
	 * @param index : data index in the circular buffer, in [0, length-1]
	 * @param channel_idx : number of the channel, in [0, nb_channel-1]
	 * @return a float number
	 */
	float get_channel_value(uint32_t index, uint32_t channel_idx);
	
	/**
	 * @brief to call one time before using dump_datas()
	 */
	void reset_dump();
	
	/**
	 * @brief dump_state can be: initialized, names, final_idx, datas, and finished.
	 *
	 * @return current dump_state value
	 */
	e_dump_state get_dump_state();

	/**
	 * @brief help to get all the data recorded in a csv format.
	 * you must call scope.reset_dump() before, then
	 * it must be called in a while loop until the scope.dump_state == finished
	 *
	 * @return string.
	 */
	char *dump_datas();

private:
	const uint16_t _length;
	const uint16_t _nb_channel;
	uint16_t _nb_channel_effective;

	/* Variables for acquisition logic */
	uint16_t _nb_pretrig; // number of samples to be recorded before trigger (parameter)
	ScopeAcqState _acq_state; // data acquisition state
	uint16_t _acq_count; // number of samples already recorded
	uint16_t _mem_idx; // memory index where to record the *next* samples in memory
	uint16_t _trig_idx; // memory index value when trigger happened (once triggered)
	uint16_t _final_idx; // memory index value where last sample was recorded (once acquision is finished)
	bool (*_triggFunc)(); // trigger function called in each data acquisition call

	/* Variables for scope data storage */
	float **_channels;
	float *_channel_numbers;
	const char **_names;
	float *_memory;

	/* Variables for dump_datas method */
	e_dump_state dump_state;
	char hash[2] = "#";
	char nullchar[2] = " ";
	char char_name[256];
	/* each data is a float written in hexa so eight chars with \n and \0 chars at end */
	char char_data[10]; // string buffer for hexa reprensentation of 1 float32 value
	char *data_dumped;
	uint16_t _idx_name;
	uint16_t _idx_datas;
};

#endif
