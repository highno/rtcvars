RTCVars
=======
This library eases the storage of variables in reset-safe RTC memory. Variables stored there survive all kinds of resets as long as there is no hard reset. It provides boilerplate code to securely store relevant state data in RTC memory so it may survive (unexpected) reboots or deep sleeps. Supports ESP only at this time, will change in the future.

Aim of the library is to make an integration into existing projects as easy as possible.

# Help wanted
My own main target is the ESP8266 platform. I am happy owner of a brand new ESP32 so expect support for it soon. Other plattforms can be supported too, but since its basic function relies on a real time clock (RTC) with additional RAM, it would require a plattform including such a real time clock or using an external module for it. I don't have that at home right now, so it would help a lot, if you own an rtc module, if you contact me or develop the integration yourself and make a pull request.  

# Installation
At the moment, this library is not part of the "official" repository within the arduino gui. Therefore you need to download the library as a .zip-file and install it by hand using the arduino gui.

# Quick examples
## Pseudo code
The major aim of the library is seamless integration but it still requires a little change to your existing programming pattern. In pseudo code it would look like this:

    // pseudo code example
    global variable definition: a, b, c
    
    setup {
      registerVariablesForStorageInRTC(a);
      if tryToLoadFromRTC==true then
        do some work to recreate/enter saved state
      else
        do "cold boot" work
      endif
    }
    
    loop {
      do loop work
      if stateRelevantChangeOf(a) then
        storeVarsIntoRTC
      endif
    }

The magic lies in the first <code>if</code> statement. If the loading of a valid set of values works, you will (most likely) need to add some code, so that your look will enter the correct state. 

## Basic example
Let's see some real action:

    #include <RTCVars.h>
    RTCVars state; // create the state object
    
    int reset_counter;                      // we want to keep these values after reset
    int program_step;
    
    void setup() {
      Serial.begin(115200);                 // allow debug output
      
      state.registerVar( &reset_counter );  // we send a pointer to each of our variables
      state.registerVar( &program_step );

      if (state.loadFromRTC()) {            // we load the values from rtc memory back into the registered variables
        reset_counter++;
        Serial.println("This is reset no. " + (String)reset_counter);
        state.saveToRTC();                  // since we changed a state relevant variable, we store the new values
      } else {
        reset_counter = 0;                  // cold boot part
        Serial.println("This seems to be a cold boot. We don't have a valid state on RTC memory");
        program_step = 0;
      }
    }
    
    void loop() {
      // do your work here - try to reset your chip externally or internally but don't power off...
      Serial.println("Current state is " + (String)program_step);
      program_step = (program_step == 7) ? 1 : program_step + 1;
      Serial.println("New state is " + (String)program_step);
      state.saveToRTC();                    // there are no parameters because it only needs the vars' definiton once 
      delay(1000);
    }

First the library is included by <code>#include <RTCVars.h></code>. Then an RTCVars object named <code>state</code> is created via <code>RTCVars state;</code>. This object now is the interface to all functionality. Be sure to register all variables before reading the old state from RTC memory. The state is invalid if the registered variables differ in total size to the saved state. It seems good practice to use globally defined vars only and register all of them in the <code>setup()</code> function. If you really need to keep track of different, state-specific variables, look at the advanced usage.
  
Registering does nothing but keeping track of where the variables are to find in memory. Of course reading the state from RTC memory would require to save the values in the corresponding variables. Therefore you need to register them even before you call <code>state.loadFromRTC()</code>. If everything works out, the call returns <code>true</code>. If not there are problems with the state or it has been a cold boot. See advanced usage for further information on error handling.

Later on, everytime a change is made to some of the vars registered and have a consistent state is esablished, <code>state.saveToRTC()</code> is called to push these values to RTC memory.

## Supported types and size
Please note, that there are two limits in this lib: memory and the number of managed variables. Due to the fact how storage of vars is organized, there is a fixed upper boundry other than memory. By default, the limit is 32 variables, the types are <code>byte</code>, <code>char</code>, <code>int</code>, <code>long</code>, <code>float</code>. Since these types do not exceed 8 bytes each, the RTC memory is not the limit here (512 bytes - 28 starting offset - 7 header/checksum = 477 bytes). See advanced usage for more functionality to control these settings.  

## Keep track of space

# Advanced Usage
This part will be filled later. Sorry for now - just have a look at the example.

## Error handling
The same goes here...

## Multiple sets of state vars
Basically it is a bad idea to use different sets of variables to store in RTC memory. Typically this is not needed but due to memory constraints sometimes there is the need for it. Problem is, to setting up a state starts by registering the vars. Restoring these only works if vars are registered with the same type in the same order. What if these are different between two states: state one needs to store 30 ints and state two 12 floats. It's needed to know before setting up vars, which state is to be restored. This is what /state id/s are for:
To keep two (or more) distinct sets of vars for different states, instantiate an additional <code>RTCVars</code> object and register different vars. Initally all <code>RTCVars</code> objects have the <code>state_id</code> 1. This can be changed by calling the objects method <code>setStateId(byte new_state_id)</code>. 
Using different ids is essential for a correct recovery after reset. The <code>state_id</code> of an <code>RTCVars</code> object is returned when calling <code>getStateId()</code> function.
In a program's <code>setup()</code> method, after creating the <code>RTCVars</code> object, <code>getStateIdFromRTC()</code> function should be called. It reads the <code>state_id</code> of the state in RTC memory ahead of other checks or the need to register vars. Be aware though, that this call might fail (see error handling) because of wrong checksum or other reasons (e.g. cold boot). 
Check the returned <code>state_id</code> against valid states and register the vars relevant for this state. Afterwards it is needed to set the object's <code>state_id</code> to the one in the RTC (for safety reasons). You can then load the state from RTC via <code>loadFromRTC()</code>.

## Memory and variable accounting
As stated in Supported types and size, there are two limits to check for. 
First one is the RTC memory. This is a fixed size depending on the RTC type used. On the ESP8266 e.g. it is 512 bytes with a known good offset to use the memory freely of 28 bytes. Additionally the library uses 7 bytes for management purposes. At any time available RTC memory can be checked by using the <code>getFreeRTCMem()</code> call of the <code>RTCVars</code> instance. It returns the number of free bytes as <code>int</code>.
Second limit is the number of registered vars which is set within the lib as <code>32</code>. To change that limit, a change in the <code>RTCVars.h</code> file is needed for now, as this limit is set by a <code>#define</code> statement. Use the <code>getFreeRTCVars()</code> function of the <code>RTCVars</code> instance. It returns the number of unused slots for registered vars.

## Advanced example
Aim of this example is to show all features developed so far.
