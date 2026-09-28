const uint8_t MY_PINS[] = {A0,A1};
const uint32_t MAX_IDLE = 300000;
const float MIN_VOLTAGE = 0.9;
const float ADC_REFERENCE = 4.39; // The specific board's actual voltage at the 5V pin
const int MIN_ADC = floor((MIN_VOLTAGE * 1023.0) / ADC_REFERENCE); 

template <size_t N>
class DataLogger {

  private:
  uint8_t pins[N];
  uint32_t idleTimer;
  int prevVal[N];
  int curVal[N];
  uint32_t prevTime = 0;
  uint32_t curTime = 0;
  uint32_t lastLogTime = UINT32_MAX; // Prevent curTime == lastLogTime on startup
  bool prevChange = true; // No previous data to print on first log

  public:
  DataLogger(const uint8_t (&inputPins)[N], uint32_t maxIdle) : idleTimer(maxIdle) {
    for(size_t i = 0; i < N; i++) {
      pins[i] = inputPins[i];
      prevVal[i] = curVal[i] = 0;
    }
  }

  bool allAbove(int threshold) {
    for(int i = 0; i < N; i++) {
      if(curVal[i] < threshold) {
        return false;
      }
    }
    return true;
  }

  bool allBelow(int threshold) {
    for(int i = 0; i < N; i++) {
      if(curVal[i] > threshold) {
        return false;
      }
    }
    return true;
  }

  void LogToCSV() {
    bool curChange = false;
    curTime = millis();
    for(int i = 0; i < N; i++) {
      curVal[i] = analogRead(pins[i]);
    }

    for(int i = 0; i < N; i++) {
      if(prevVal[i] != curVal[i]) {
        curChange = true;
        break;
      }
    }

    // No change condition
    if((!curChange && curTime - lastLogTime < idleTimer) || curTime == lastLogTime) {
      prevChange = false;
    }

    else {
      // Single change condition
      if(!prevChange) {
        Serial.print(prevTime);
        for(int i = 0; i < N; i++) {
          Serial.print(",");
          Serial.print(prevVal[i]);
      }
      Serial.println();
      }

      // Common for single and continuous change
      Serial.print(curTime);
      for(int i = 0; i < N; i++) {
        Serial.print(",");
        Serial.print(curVal[i]);
        prevVal[i] = curVal[i];
      }
      Serial.println();

      lastLogTime = curTime;
      prevChange = true;
    }

    prevTime = curTime;
  }

};

DataLogger <sizeof(MY_PINS) / sizeof(MY_PINS[0])> sensors(MY_PINS, MAX_IDLE);
int endCheck = 300; // Batteries must be constantly under threshold for 5 mins to terminate program

void setup() {
  Serial.begin(9600);
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
}

void loop() {
  do {
    sensors.LogToCSV();
    delay(1000);
    if(sensors.allBelow(MIN_ADC)) {
      endCheck--;
    }
    else {
      endCheck = 300;
    }
  } while(endCheck > 0);

  Serial.println("DONE");
  while(true){ // Flash LED to indicate completion
  digitalWrite(8, HIGH);
  delay(200);
  digitalWrite(8, LOW);
  delay(200);
  }
}
