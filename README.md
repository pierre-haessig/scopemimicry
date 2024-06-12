# ScopeMimicry

ScopeMimicry Library allows to capture live data and variables.

![image](https://github.com/owntech-foundation/scopemimicry/assets/22010135/2f9d3ffe-c2f5-4192-a7df-3b2b085cbab1)

##  Load library  

Add the following to `lib_deps` in `platformio.ini`  
```scopemimicry = https://github.com/owntech-foundation/scopemimicry.git```  

##  Include the library  

```cpp
#include "ScopeMimicry.h"
```

## ScopeMimicry Usage

### Connecting Channels

Create your scope object  

```cpp
ScopeMimicry scope(sample_lenght, number_of_channels);
```

- Sample lenght is the number of data points per channel.  
- Number of channel is the total number of channel acquired by the scope.
  
For example :
```cpp
ScopeMimicry scope(512, 4);
```  
Defines a scope with 4 channels and 512 data points per channel.

Channels are connected to variables using:
```cpp
scope.connectChannel(variable, "channel_name");
```
For example:
```cpp
scope.connectChannel(I1_low_value, "I1_low");
```
Make sure to connect as many channels as the number of channels set up in scope object.

## Launching the acquisition  
Acquisition can be launched using :  

```cpp
scope.acquire();
```

### Setting Triggers and Delays

Instead of using scope.acquire(), it is possible to trigger the scope with a given logic 
condition.  

To do that :  

Define a trigger function. The trigger function shoud not have any arguments. It should 
return a bool value, that will trigger the scope when returning 1. 

For example :  
```cpp
bool a_trigger()
    return (myVariable == 1)
```  
Will trigger the scope when `myVariable = 1`  

Once the trigger function created, set the trigger function
```cpp
scope.set_trigger(&a_trigger);
```

A delay can be added to acquire a given amount of data before the trigger instant.
`delay` should be a float between `0` and `1`.
```cpp
scope.set_delay(0.0F);
```

## Starting the Scope

Start the scope to begin data acquisition:
```cpp
scope.start();
```
