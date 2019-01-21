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
