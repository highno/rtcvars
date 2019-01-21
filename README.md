RTCVars
=======
This library eases the storage of variables in reset-safe RTC memory. Variables stored there survive all kinds of resets as long as there is no hard reset. It provides boilerplate code to securely store relevant state data in RTC memory so it may survive (unexpected) reboots or deep sleeps. Supports ESP only at this time, will change in the future.

Aim of the library is to make an integration into existing projects as easy as possible.

# Help wanted
My own main target is the ESP8266 platform. I am happy owner of a brand new ESP32 so expect support for it soon. Other plattforms can be supported too, but since its basic function relies on a real time clock (RTC) with additional RAM, it would require a plattform including such a real time clock or using an external module for it. I don't have that at home right now, so it would help a lot, if you own an rtc module, if you contact me or develop the integration yourself and make a pull request.  

# Installation
At the moment, this library is not part of the "official" repository within the arduino gui. Therefore you need to download the library as a .zip-file and install it by hand using the arduino gui.

# Quick Examples
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

# Advanced Usage
This part will be filled later. Sorry for now - just have a look at the example.

## Error handling
The same goes here...

## Multiple sets of state vars
... and here.
