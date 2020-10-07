#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#include "RunningSum.h"

// define the number of players
#define NUM_PLAYERS 2
#define NUM_INPUTS 4
int inputs[] = { A0, A1, A2, A3, A4 };
int outputs[2][5] = { { 12, 11, 10, 9, 8 }, { 7, 6, 5, 4, 3 } };
volatile uint8_t* outputPorts[2] = { &PORTB, &PORTD };
// setup the HIGH portmaps here, the LOWs get set automatically via negation in the setup
// these corresponse to output pin assignment { { 12, 11, 10, 9, 8 }, { 7, 6, 5, 4, 3 } }
uint8_t outputPortMapsHigh[2][5] = { { B00010000, B00001000, B00000100, B00000010, B00000001 }, 
                                     { B10000000, B01000000, B00100000, B00010000, B00001000 } };
uint8_t outputPortMapsLow[2][5];

// handle mux configuration for 2 player mode
int muxInput = A5;
// These correspond to pins A6, A7, and 2 respectively
uint8_t muxSelectPortMapsHigh[] = { B01000000, B10000000, B00000100 };
uint8_t muxSelectPortMapsLow[3];
volatile uint8_t* muxSelectPorts[] = { &PORTC, &PORTC, &PORTD };

// for hit detection, we keep a running record of the last X number
// of arrow readings and the deltas from their last readings
int deltaThreshold = 40;
int numReadings = 20;
int releaseHysteresis = 8;

bool states[NUM_PLAYERS][NUM_INPUTS];
int releaseThresholds[NUM_PLAYERS][NUM_INPUTS];
int lastReadings[NUM_PLAYERS][NUM_INPUTS];
RunningSum* deltas[NUM_PLAYERS][NUM_INPUTS];

// keep track of poll rate
uint16_t pollRate;
unsigned long lastMillis = 0;
uint16_t loops = 0;

int readPressure(int player, int sensor);

void setup() {
  Serial.begin(115200);

  // setup the ADC registers to run more quickly
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  // init IO pins
  for (int i = 0; i < NUM_INPUTS * 2; i++) {
    pinMode(inputs[i], INPUT);
  }

  // setup some initial values
  for (int player = 0; player < NUM_PLAYERS; player++) {
    for (int i = 0; i < NUM_INPUTS; i++) {
      deltas[player][i] = new RunningSum(numReadings);
      pinMode(outputs[player][i], OUTPUT);
      digitalWrite(outputs[player][i], HIGH);
      outputPortMapsLow[player][i] = ~outputPortMapsHigh[player][i];
    }
  }

  for (int i = 0; i < 3; i++) {
    muxSelectPortMapsLow[i] = ~muxSelectPortMapsHigh[i];
  }
  
  int pressure;

  // take some initial readings before the main loop so we can establish baselines
  for (int r = 0; r < numReadings; r++) {
    for (int player = 0; player < NUM_PLAYERS; player++) {
      for (int i = 0; i < NUM_INPUTS; i++) {
        // keep a running record of the last X readings
        pressure = readPressure(player, i);
    
        // calculate the newest delta if it's not the first reading
        if (r > 0) {
          deltas[player][i]->addValue(pressure - lastReadings[player][i]);
        }
        
        lastReadings[player][i] = pressure;
      }
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
      // keep a running record of the last X readings
      pressure = readPressure(player, i);
  
      // calculate the newest delta
      deltas[player][i]->addValue(pressure - lastReadings[player][i]);
      lastReadings[player][i] = pressure;

      // check the readout data and the running sum of the deltas;
      // if running sum of deltas goes above our defined threshold,
      // trigger a press and set new lower bounds for release,
      // otherwise, check for a release
      if (!states[player][i] && (deltas[player][i]->getSum() > deltaThreshold)) {
        // set a new release threshold based on the reading at the time we detected
        // the press (minus the delta + a small hysteresis factor)
        releaseThresholds[player][i] = pressure - deltaThreshold + releaseHysteresis;
        states[player][i] = true;
        
        // turn on the panel once we calculate the release threshold
        *(outputPorts[player]) &= outputPortMapsLow[player][i];

        // test output for my one test sensor
        if (player == 0 && i == 0) {
          Serial.println("1");
        }
      } else if (states[player][i] && (pressure < releaseThresholds[player][i])) {
        // new release
        states[player][i] = false;
        releaseThresholds[player][i] = 0;
        
        // if the value is below the release threshold, turn off the panel
        *(outputPorts[player]) |= outputPortMapsHigh[player][i];

        // test output for my one test sensor
        if (player == 0 && i == 0) {
          Serial.println("0");
        }
      }
    } 
  }
}

int readPressure(int player, int sensor) {
  // P1 is read directly from analog pins, P2 is read from a mux
  if (player == 0) {
    return analogRead(inputs[sensor]);
  } else {
    uint8_t* port0 = muxSelectPorts[0];
    uint8_t* port1 = muxSelectPorts[1];
    uint8_t* port2 = muxSelectPorts[2];

    switch (sensor) {
      case 0: // 000
        *port0 &= muxSelectPortMapsLow[0];
        *port1 &= muxSelectPortMapsLow[1];
        *port2 &= muxSelectPortMapsLow[2];
        break;
      case 1: // 001
        *port0 |= muxSelectPortMapsHigh[0];
        break;
      case 2: // 010
        *port0 &= muxSelectPortMapsLow[0];
        *port1 |= muxSelectPortMapsHigh[1];
        break;
      case 3: // 011
        *port0 |= muxSelectPortMapsHigh[0];
        break;
      case 4: // 100
        *port0 &= muxSelectPortMapsLow[0];
        *port1 &= muxSelectPortMapsLow[1];
        *port2 |= muxSelectPortMapsHigh[2];
        break;
      default:
        break;
    }
    
    return analogRead(muxInput);
  }
}
