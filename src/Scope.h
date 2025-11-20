/**
 * @file
 * @brief Scope: a software oscilloscope library, to record over time the value of selected variables in embedded applications.
 * 
 * @author Régis Ruelland <regis.ruelland@laas.fr>
 * @author Jean Alinei <jean.alinei@laas.fr>
 * @author Pierre Haessig <pierre.haessig@centralesupelec.fr>
 */

#ifndef _SCOPEMIMICRY_H_
#define _SCOPEMIMICRY_H_

#include <cstdint>
typedef float float32_t;

/*Scope acquisition state*/
enum ScopeAcqState {
	ACQ_UNTRIG, // data being recorded, before trigger
	ACQ_TRIG,   // data being recorded, after trigger
	ACQ_DONE    // data recording finished
};

/* Scope data dump state*/
enum ScopeDumpState {
	DUMP_READY,
	DUMP_NAMES,
	DUMP_DATA,
	DUMP_FINISHED
};


class Scope {
public:
	/**
	 * @brief create a scope with RAM storage for recorded data
	 *
	 * @param length number of samples to record, for each variable
	 * @param nb_channel number of variables to record
	 * @param Ts sampling time period, for time vector computation
	 */
	Scope(uint16_t length, uint16_t nb_channel, float Ts);
	
	/**
	 * @brief destry scope instance and release RAM
	 */
	~Scope();
	
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
	 * @brief Initialize the scope data dump process.
	 * 
	 * Must be called before the first dump() call, or start a new dump process.
	 */
	void init_dump();
	
	/**
	 * @brief dump_state can be: DUMP_READY, DUMP_NAMES, DUMP_DATA, and DUMP_FINISHED.
	 *
	 * @return current dump_state value
	 */
	ScopeDumpState get_dump_state();

	/**
	 * @brief dump a chunk of the recorded data.
	 * 
	 * Data is dumped in small chunks of char arrays, to be printed on a serial port.
	 * To retrieve all recorded data, init_dump() must be called first,
	 * and then dump() must be called in loop until scope.dump_state == DUMP_FINISHED.
	 * 
	 * Dumped data starts with a CSV header of channel names, including time as first column.
	 * It then continues with the float32 channel values, in their hexadecimal representation.
	 *
	 * @return string.
	 */
	char *dump();

private:
	const uint16_t _length; // maximum number of sampling instants
	const uint16_t _nb_channel; // maximum number of channels
	const float _Ts; // sampling time period
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
	float *_memory; // Scope memory array, of size length*nb_channels

	/* Variables for dump_datas method */
	ScopeDumpState dump_state;
	bool _dump_time; // sub state of DUMP_NAMES and DUMP_DATA
	char _dumped_data[102]; // string buffer for dumped data
	// buffer should hold either:
	// - channel name (up to 100 chars) + ',' + '\0' = 102 chars
	// - hexadecimal representation of 1 float32 value (8 chars) + '\n' + '\0' = 10 chars
	uint16_t _dump_count; // number of samples instants having been dumped
	uint16_t _dump_channel_idx; // index of current channel being dumped
};

#endif
