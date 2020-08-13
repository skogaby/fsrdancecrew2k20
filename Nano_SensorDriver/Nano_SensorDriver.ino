#include "SerialUtils.h"

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
// #define __DEBUG_LOG__

// uncomment this for 5-panel (PIU) mode
// #define __5_PANEL__ 

// define the number of players
#define NUM_PLAYERS 2

// these are the default sensitivity values we'll use, until
// overridden by the controller board
#define DEFAULT_SENSITIVITY 800

// set the serial address
#define SERIAL_ADDR 0x0A

// keep track of the current sensor values and what the thresholds
// are before we trigger a button press
#ifdef __5_PANEL__
  #define NUM_INPUTS 5
  int inputs[] = { A0, A1, A2, A3, A4 };
  int outputs[] = { 12, 11, 10, 9, 8, 7, 6, 5, 4, 3 };
#else
  #define NUM_INPUTS 4
  int inputs[] = { A0, A1, A2, A3 };
  int outputs[] = { 12, 11, 10, 9, 7, 6, 5, 4 };
#endif

// handle mux configuration for 2 player mode
#define MUX_INPUT A5
#define MUX_0 A6
#define MUX_1 A7
#define MUX_2 2 

// the current thresholds for each arrow
struct THRESHOLD_DATA {
  uint16_t thresholds[NUM_INPUTS * 2];
};

// the current values of each sensor, and the last poll rate
struct PAD_READOUT_DATA {
  uint16_t pressures[NUM_INPUTS * 2];
  uint16_t pollRate;
};

THRESHOLD_DATA thresholdData;
PAD_READOUT_DATA padReadoutData;

// keep track of poll rate
long lastMillis = 0;
int loops = 0;

/**
 * Setup the IO pins
 */
void setup() {
  Serial.begin(115200);

  // init IO pins
  for (int i = 0; i < NUM_INPUTS * 2; i++) {
    pinMode(inputs[i], INPUT);
    pinMode(outputs[i], OUTPUT);
    thresholdData.thresholds[i] = DEFAULT_SENSITIVITY;
  }
}

/**
 * Main program loop, drives the sensors and
 * updates the ESP32 SPI master
 */
void loop() {
  // keep track of our poll rate
  long currentMillis = millis();
  long deltaMillis = currentMillis - lastMillis;
  loops++;

  // every certain number of milliseconds, update the poll rate and send the readouts to the ESP module
  if (deltaMillis > 500) {
    padReadoutData.pollRate = (loops / (deltaMillis / 1000.0f));

#ifdef __DEBUG_LOG__
    Serial.print("Current poll rate: ");
    Serial.print(padReadoutData.pollRate);
    Serial.println(" Hz");
#endif

    lastMillis = currentMillis;
    loops = 0;

    // update the ESP module
    sendReadouts();
  }

  // check if any of the buttons are pressed on player 1
  for (int i = 0; i < NUM_INPUTS; i++) {
    int pressure = analogRead(inputs[i]);
    padReadoutData.pressures[i] = pressure;

    if (pressure >= thresholdData.thresholds[i]) {
      digitalWrite(outputs[i], LOW);
    } else {
      digitalWrite(outputs[i], HIGH);
    }

    // if applicable, check button presses on player 2
#if NUM_PLAYERS == 2
    digitalWrite(MUX_0, bitRead(i, 0));
    digitalWrite(MUX_1, bitRead(i, 1));
    digitalWrite(MUX_2, bitRead(i, 2));
    
    pressure = analogRead(MUX_INPUT);
    padReadoutData.pressures[i + NUM_INPUTS] = pressure;

    if (pressure >= thresholdData.thresholds[i + NUM_INPUTS]) {
      digitalWrite(outputs[i + NUM_INPUTS], LOW);
    } else {
      digitalWrite(outputs[i + NUM_INPUTS], HIGH);
    }
#endif
  }

  // check for updates from the ESP modules for new sensitivity thresholds and 
  // apply them if there are any
  recvSerialBytes();
  checkSerialData();
}

/**
 * Send the pad readouts and poll rate to the ESP module.
 */
void sendReadouts() {
    //Serial.println("Sending pad readouts to ESP module");
    Serial.write(START_MARKER);

    sendSerialInt(SERIAL_ADDR);

    for (int i = 0; i < NUM_INPUTS * 2; i++) {
      sendSerialInt(padReadoutData.pressures[i]);
    }

    sendSerialInt(padReadoutData.pollRate);
    Serial.write(END_MARKER);
}

/**
 * Receive new threshold values from the ESP module.
 */
void checkSerialData() {
  if (newData) {
    // make sure this message was for our address
    uint16_t addr = receivedBytes[0] | (receivedBytes[1] << 8);

    // read the thresholds
    if (addr == SERIAL_ADDR) {
      for (int i = 0; i < NUM_INPUTS * 2; i++) {
        thresholdData.thresholds[i] = receivedBytes[(i * 2) + 2] | (receivedBytes[(i * 2) + 3] << 8);
      }
      
#ifdef __DEBUG_LOG__
      Serial.println("Reading threshold data from serial master");
      Serial.print("New thresholds: ");
      
      for (int i = 0; i < NUM_INPUTS * 2; i++) {
        Serial.print(thresholdData.thresholds[i]);
        Serial.print(" ");
      }
      
      Serial.println("");
#endif
    }
    
    newData = false;
  }
 }
