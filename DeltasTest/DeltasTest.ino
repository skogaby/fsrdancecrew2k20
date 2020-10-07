#include "RunningSum.h"

// define the number of players
#define NUM_PLAYERS 2
#define NUM_INPUTS 4
int inputs[] = { A0, A1, A2, A3, A4 };
int outputs[] = { 12, 11, 10, 9, 8, 7, 6, 5, 4, 3 };

// handle mux configuration for 2 player mode
#define MUX_INPUT A5
#define MUX_0 A6
#define MUX_1 A7
#define MUX_2 2 

// for hit detection, we keep a running record of the last X number
// of arrow readings and the deltas from their last readings
int deltaThreshold = 40;
int numReadings = 20;
int releaseHysteresis = 8;

bool states[NUM_PLAYERS][NUM_INPUTS];
int releaseThresholds[NUM_PLAYERS][NUM_INPUTS];
int lastReadings[NUM_PLAYERS][NUM_INPUTS];
RunningSum* readings[NUM_PLAYERS][NUM_INPUTS];
RunningSum* deltas[NUM_PLAYERS][NUM_INPUTS];

// keep track of poll rate
uint16_t pollRate;
unsigned long lastMillis = 0;
uint16_t loops = 0;

void setup() {
  Serial.begin(115200);

  // init IO pins
  for (int i = 0; i < NUM_INPUTS * 2; i++) {
    pinMode(inputs[i], INPUT);
    pinMode(outputs[i], OUTPUT);
  }

  // setup some initial values
  for (int player = 0; player < NUM_PLAYERS; player++) {
    for (int i = 0; i < NUM_INPUTS; i++) {
      readings[player][i] = new RunningSum(numReadings);
      deltas[player][i] = new RunningSum(numReadings);
    }
  }
}

void loop() {
  // keep track of our poll rate
  long currentMillis = millis();
  long deltaMillis = currentMillis - lastMillis;
  loops++;

  // every certain number of milliseconds, update the poll rate and send the readouts to the ESP module
  if (deltaMillis >= 1000) {
    pollRate = (loops / (deltaMillis / 1000.0f));

    Serial.print("Current poll rate: ");
    Serial.print(pollRate);
    Serial.println(" Hz");

    lastMillis = currentMillis;
    loops = 0;
  }
     
  int pressure;

  // check if any of the buttons are pressed
  for (int player = 0; player < NUM_PLAYERS; player++) {
    for (int i = 0; i < NUM_INPUTS; i++) {
      // P1 is read directly from analog pins, P2 is read from a mux
      if (player == 0) {
        pressure = analogRead(inputs[i]);
      } else {
        digitalWrite(MUX_0, bitRead(i, 0));
        digitalWrite(MUX_1, bitRead(i, 1));
        digitalWrite(MUX_2, bitRead(i, 2));
        
        pressure = analogRead(MUX_INPUT);
      }
  
      // keep a running record of the last X readings
      readings[player][i]->addValue(pressure);
  
      // calculate the newest delta
      deltas[player][i]->addValue(pressure - lastReadings[player][i]);
      lastReadings[player][i] = pressure;

      // check the readout data and the running sum of the deltas;
      // if running sum of deltas goes above our defined threshold,
      // trigger a press and set new lower bounds for release,
      // otherwise, check for a release
      if (deltas[player][i]->getSum() > deltaThreshold) {
        // set a new release threshold based on the reading at the time we detected
        // the press (minus the delta + a small hysteresis factor)
        releaseThresholds[player][i] = pressure - deltaThreshold + releaseHysteresis;

        // this is a new press
        if (!states[player][i]) {
          states[player][i] = true;
          
          // turn on the panel once we calculate the release threshold
          digitalWrite(outputs[i], LOW);
  
          // test output for my one test sensor
          if (player == 0 && i == 0) {
            Serial.println("ON");
          }
        }
      } else if (pressure < releaseThresholds[player][i]) {
        // new release
        if (states[player][i]) {
          states[player][i] = false;
          releaseThresholds[player][i] = 0;
          
          // if the value is below the release threshold, turn off the panel
          digitalWrite(outputs[i], HIGH);

          // test output for my one test sensor
          if (player == 0 && i == 0) {
            Serial.println("OFF");
          }
        }
      }
    } 
  }
}
