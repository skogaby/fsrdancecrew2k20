#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

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

// how far above the current baseline the readings need
// to go to trigger a hit
int deltaThresholds[2][5] = { { 80, 50, 80, 80, 80 }, { 80, 50, 80, 80, 80 } };

// the percentage to multiple the delta threshold by
// to add to the baseline as a release threshold
double releaseThreshold = 0.4;

// how many milliseconds to wait between re-calibrations if
// no inputs are detected
long calibrationPeriod = 10000;

// keep track of the thresholds for trigger and release for each arrow
// according to their own resting values
bool states[NUM_PLAYERS][NUM_INPUTS];
int triggerThresholds[NUM_PLAYERS][NUM_INPUTS];
int releaseThresholds[NUM_PLAYERS][NUM_INPUTS];
unsigned long lastTriggerTimes[NUM_PLAYERS][NUM_INPUTS];

// keep track of poll rate
uint16_t pollRate;
unsigned long lastMillis = 0;
uint16_t loops = 0;

int readPressure(int player, int sensor);

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

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
      pinMode(outputs[player][i], OUTPUT);
      digitalWrite(outputs[player][i], HIGH);
      outputPortMapsLow[player][i] = ~outputPortMapsHigh[player][i];
      lastTriggerTimes[player][i] = millis();
    }
  }

  for (int i = 0; i < 3; i++) {
    muxSelectPortMapsLow[i] = ~muxSelectPortMapsHigh[i];
  }

  delay(1000);

  int pressure;
  // take some initial readings before the main loop so we can establish baselines
  for (int player = 0; player < NUM_PLAYERS; player++) {
    for (int i = 0; i < NUM_INPUTS; i++) {
      // determine the current thresholds for trigger
      // and release based on the configured delta threhsold
      // and the current readings
      pressure = readPressure(player, i);
      
      states[player][i] = false;
      triggerThresholds[player][i] = pressure + deltaThresholds[player][i];
      releaseThresholds[player][i] = pressure + (deltaThresholds[player][i] * releaseThreshold);
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

      if (!states[player][i]) {
        // new press detected
        if (pressure > triggerThresholds[player][i]) {
          lastTriggerTimes[player][i] = currentMillis;
          states[player][i] = true;
          
          // turn on the panel once we calculate the release threshold
          *(outputPorts[player]) &= outputPortMapsLow[player][i];
        // the panel hasn't been triggered, so we'll check how long it's been since
        // the last trigger. if it's been more than the configured period, re-establish
        // baselines for thresholds
        } else if (currentMillis - lastTriggerTimes[player][i] > calibrationPeriod) {          
          triggerThresholds[player][i] = pressure + deltaThresholds[player][i];
          releaseThresholds[player][i] = pressure + (deltaThresholds[player][i] * releaseThreshold);
          lastTriggerTimes[player][i] = currentMillis;
        }
      // new release detected
      } else if (states[player][i] && (pressure < releaseThresholds[player][i])) {
        states[player][i] = false;
        lastTriggerTimes[player][i] = currentMillis;
        
        // if the value is below the release threshold, turn off the panel
        *(outputPorts[player]) |= outputPortMapsHigh[player][i];
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
