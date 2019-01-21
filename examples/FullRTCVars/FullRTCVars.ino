extern "C" {
  #include <user_interface.h>
}
#include <TimeLib.h>

// By default, the maximum number of variables is 32, it can be set to a higher value if needed
// More needs more RAM, (about 6 Bytes per Variable), is a definition in lib's .h file
// number is limited by total RTC memory
// Use getFreeRTCMem() and getFreeRTCMem() to check at runtime

#include "RTCVars.h"

RTCVars state;
time_t oldTime;

// test variables
// variables for RTC memory should be global or valid at any time saveToRTC() is called 
int i = 1234;
int j = 23456;
byte k = 0;
long l = 123456789;
float m = 3.141592683;
int int_array[] = {1, 2, 0};

int autoreset = 20; // this will count down to zero and reset the esp

void setup() {
  // let's inform the user about what type of reset has happened
  rst_info *resetInfo;
  resetInfo = ESP.getResetInfoPtr();
  
  Serial.begin(115200);
  Serial.println("Booting...");
  Serial.print("Reset Reason was: ");
  Serial.println(resetInfo->reason);

  // register each variable by sending a pointer to it. valid types are: byte, char, int, long, float
  // register preferably in setup call
  state.registerVar(&i);
  state.registerVar(&j);
  state.registerVar(&k);
  state.registerVar(&l);
  state.registerVar(&m);
  // Note: 
  // Registered vars cannot be "thrown away" (e.g. unregistered).
  // If you really need a different set of vars to be saved in RTC mem, create another RTCVars object
  // and throw away the first one. These different sets have different signatures so to recover those
  // you need to identify the state set with a byte number:
  // state.setStateID(3);

  // Since recovery needs to know the kind of state in memory, request the saved ID first:
  // byte id_in_rtc = state.getStateIDFromRTC();
  // Afterwards you can register this state's variables and read all from RTC.
  // Be aware that you need to set the state ID (setStateID) to the ID in RTC memory, otherwise the
  // call to loadFromRTC() will fail. 
  byte id_in_rtc = state.getStateIDFromRTC(); // =255 if no valid signature is found
  Serial.print("The RTCVars state set in RTC memory has the id ");
  Serial.println(id_in_rtc);
  // since we change the id for demonstration purpose (!) every boot, let's accept the id in RTC memory
  if (id_in_rtc != 255) state.setStateID(id_in_rtc);
  
  j = 0;
  // arrays can be inserted by registering all elements in a loop
  for (int n = 0; n < 3; n++) state.registerVar(&(int_array[n]));
  // double registering is senseless (it is the same memory address!) but possible
  state.registerVar(&i);

  // debug output
  Serial.println("This is the set of vars BEFORE trying to load these from RTC"); 
  state.debugOutputRTCVars();

  // try to load from RTC memory
  // be aware that you should keep registering the same variables in the same order between to resets
  // otherwise you'll end up with data written in wrong variables
  if (state.loadFromRTC()) {
    DPRINTLN(F("Data successfully read from RTC"));
  } else {
    switch (state.getReadError()) {
      Serial.print("Couldn't load an old state because of ");
      case RTC_ERROR_MAGIC_BYTES:
        Serial.println("wrong magic bytes.");
        break;
      case RTC_ERROR_SIZE:
        Serial.println("a different state size than expected.");
        break;
      case RTC_ERROR_READING_FAILED:
        Serial.println("errors while reading RTC memory.");
        break;
      case RTC_ERROR_CHECKSUM:
        Serial.println("a wrong checksum.");
        break;
      case RTC_ERROR_STATE_ID:
        Serial.println("a different state id.");
        break;
      case RTC_ERROR_OTHER:
        Serial.println("unknown reasons.");
        break;
    }
  }

  // once again let's do some debug output to see what we loaded here
  Serial.println(); 
  Serial.println("And this is the set of vars AFTER trying to load these from RTC"); 
  state.debugOutputRTCVars();

  // once per reboot action
  // some fiddling in the array
  for (int n = 0; n < 3; n++) {
    // next element of fibonacci sequence with every reboot 
    int_array[int_array[2]] = int_array[0] + int_array[1];
    int_array[2] = (int_array[2] + 1) & 0x01;
  }

  // simulate different state ids
  // note: don't do this at home. only use different ids if you use a different setup of vars
  k = (k + 1) & 0x7f;
  state.setStateID(k);
}

void loop() {
  // do some action every second
  if (now()!=oldTime) {
    oldTime = now();
    
    // let us change one of the saved vars
    j += 1;
    
    // show results
    Serial.print(F("This counter will reset soon: "));
    Serial.print(now());
    Serial.print(F(", this won't: "));
    Serial.println(j);
 
    // make sure to call this function after each essential change of state variables
    // if you don't the RTC memory does not reflect the latest changes
    state.saveToRTC();
    
    // perform a reset after defined number of seconds
    if (autoreset--<=0) ESP.reset();
  }
  
  // this one changes approx. 20 times per second
  i += 1;
  delay(50);
}
