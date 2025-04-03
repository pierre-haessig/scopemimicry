# OwnTech Scope library

OwnTech Scope library allows recording over time the live value of variables from within a microcontroller application loop (typically control and measurement variables). The data is recorded in the controller memory and can later be sent over the serial monitor to the host computer for futher analysis (data plot).

Here is a plot of the recorded data for a motor control application:

![image](https://github.com/owntech-foundation/scopemimicry/assets/22010135/2f9d3ffe-c2f5-4192-a7df-3b2b085cbab1)

##  Scope library configuration

### 1\. Platformio project configuration

Add the following to `lib_deps` in `platformio.ini`  
```scopemimicry = https://github.com/owntech-foundation/scopemimicry.git```  

###  2\. Include the library

In the microcontroller application code:

```cpp
#include "ScopeMimicry.h"
```

## Scope library usage

### 1\. Scope setup

Within the microcontroller application setup rountine, create your scope object:

```cpp
ScopeMimicry scope(sample_lenght, number_of_channels);
```

- Sample length is the number of data points per channel.  
- Number of channel is the total number of channels acquired by the scope.
  

For example:
```cpp
ScopeMimicry scope(512, 4);
```
Defines a scope with 4 channels and 512 data points per channel.

#### Channels connection

Channels are connected to variables using:

```cpp
scope.connectChannel(variable, "channel_name");
```
with `variable` being a `static` `float` variable. For example:
```cpp
scope.connectChannel(I1_low_value, "I1_low");
```
Remarks:

- Because `connectChannel` stores a reference to the variable, make sure that the variable to be recorded is a persistent `static` variable
- Make sure to connect as many channels as the number of channels set up in scope object, otherwise `NaN` values are recorded for missing channels.

### 2\. Data acquisition and trigger
Data acquisition is done by **repeatedly** calling:  

```cpp
scope.acquire();
```

This is typically done at a periodic rate within the application **control loop** and it captures one sample value for each channel variable.

#### Setting Trigger

Data recording by `scope.acquire()` can be controlled by a user-provided trigger function so that the recording only happens after a trigger condition held true at least for one instant.

To do that, define a trigger function taking no argument. It should return a `bool` value that will trigger the scope when returning `true`. 

Trigger function example:  
```cpp
bool a_trigger()
    return (myVariable == 1)
```
will trigger the scope when `myVariable = 1` (with `myVariable` being globally defined)  

Once the trigger function created, attach the trigger function to the scope:
```cpp
scope.set_trigger(&a_trigger);
```

Remark: if no trigger function is set, the scope uses an always true trigger.

#### Optional pretrigger recording

The scope already records value before the trigger happened, but with the default setting, these older values get erased by the more recent posttrigger values.

It is possible to use a fraction of the record window length to preserve the latest samples before trigger (at the expense of recording less values after the trigger, because the total record length is fixed).

That number of pretrigger samples can be specified in two ways:

- as an integer number of samples (smaller than `length`):

```cpp
scope.set_pretrig_nsamples(10);
```

- as a fraction of the length (between 0.0 and 1.0):

```cpp
scope.set_pretrig_ratio(0.5F); // 50% of samples 
```

Minor detail: if the trigger happens too soon, that is before the prescribed number of pretrigger sample were recorded, then the scope records more posttrigger samples to reach the total of length samples before stopping acquisition.

#### Scope data acquisition state

The scope, in relation to the trigger function, obeys an acquisition state machine with the following states:

- `ACQ_UNTRIG` (initial state): data being recorded, before trigger (to allow the pretrigger recording)
- `ACQ_TRIG`: data being recorded, after trigger
- `ACQ_DONE`: data recording finished, once a total of `length` sample has been recorded

Those states are defined in the `ScopeAcqState` `enum`. This state is returned by the `scope.acquire()` function and can also be probbed by `scope.acq_state()`.

Minor detail: the scope only enters the ACQ_DONE state one extra call to`scope.acquire()`  after the final sample got recorded.

#### (Re)starting the Scope

Start the scope to begin fresh new data acquisition sequence:
```cpp
scope.start();
```

This is not needed the first time after scope initialization (start is automatic), but is needed to restart a fresh acquisition once the scope was triggered. It brings the scope back to its initial `ACQ_UNTRIG` state.

### 3\. Scope data retrieval

Once data acquisition is finished (preferably), the content of the scope memory can be retrieved and sent over the serial link to the host computer by repeatedly calling `scope.dump_datas()` (outside the critical control task, yielding execution between each call):

```C++
scope.reset_dump();
while (scope.get_dump_state() != finished) {
    printk("%s", scope.dump_datas());
    task.suspendBackgroundUs(200);
}
```

