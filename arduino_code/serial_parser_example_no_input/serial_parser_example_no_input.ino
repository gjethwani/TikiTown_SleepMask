#include <MPU6050.h>

#include <Wire.h>
#define MAX_BUFF_SIZE 2
#define START_PACKET_SIZE 1
#define STOP_PACKET_SIZE 1
#define QUIT_PACKET_SIZE 1
#define ledPin 9

typedef struct __attribute__((packed)){
    unsigned int n_s; //num of samples
    unsigned int s_d; //sampling delay
    unsigned int p_d; //pulse delay
} start_pkt;

typedef struct __attribute__((packed)){
    unsigned char brightness;
    unsigned int ramptime;
} stop_pkt;

char serialInput[MAX_BUFF_SIZE];
volatile boolean stringComplete = false;  // whether the string is complete
volatile boolean stringStarted = false;
volatile unsigned char cmd = 0;
boolean readdata = false;
volatile int i = 0;
volatile int pktSize = 0;
volatile start_pkt startData;
volatile stop_pkt stopData;
MPU6050 accel;
Vector accelerationVector;
double accelValue;
int cnt = 0;


void setup() {
  // initialize serial:
  pinMode(7, OUTPUT);
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  while(!accel.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
  //  Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  //checkAccelSettings();
}

void loop() {
  // print the string when a newline arrives:
  if (stringComplete || cmd == 1) {
    switch (cmd){
      case(1): 
        cnt = 0;
        for (unsigned int j = 0; j < startData.n_s; j++) {
          accelerationVector = accel.readNormalizeAccel();
          accelValue = pow(pow(accelerationVector.XAxis,2) + pow(accelerationVector.YAxis,2) + pow(accelerationVector.ZAxis,2),0.5);
          if (abs(accelValue - 9.81) > 0.25) {
            cnt += 1;
          }
          if (cnt > (0.3 * startData.n_s)) {
            Serial.write(1);
            break;
          }
          delay(startData.s_d);
        }
        delay(startData.p_d);
        break;
      case(2):
        for (unsigned int j = 0; j < stopData.brightness; j++) {
          analogWrite(ledPin, j);
          delay(stopData.ramptime/stopData.brightness);
        }
        break;
      case(3):
        analogWrite(ledPin, 0);
        cmd = 0;
        break;
      case(30):
        digitalWrite(13, HIGH);
        break;
      default:
        digitalWrite(13, LOW);
        break;
    }
    stringComplete = false;
    stringStarted = false;
  }
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = Serial.read();
    // add it to the inputString:
    
    serialInput[i]= inChar;
    i++;
   
    if (stringStarted == false){
      cmd = inChar;
      stringStarted = true;
      switch (cmd){
        case(1):
          pktSize = START_PACKET_SIZE;
          break;
        case(2):
          pktSize = STOP_PACKET_SIZE;
          break;
        case(3):
          pktSize = QUIT_PACKET_SIZE;
          break;
        default:
          stringStarted = false;
          break;
      }
    }
    
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (i >= pktSize) {
      stringComplete = true;
      switch (cmd){
        case(1): 
          //startData = *((start_pkt *)(serialInput + 1));
          startData.n_s = 10;
          startData.s_d = 100;
          startData.p_d = 500;
          break;
        case(2):
          //stopData = *((stop_pkt *)(serialInput + 1));
          stopData.brightness = 255;
          stopData.ramptime = 5000;
          break;
        default:
          break;
      }
      memset(serialInput, 0, MAX_BUFF_SIZE);
      i=0;
    }
  }
}

void checkAccelSettings()
{
  Serial.println();
  
  Serial.print(" * Sleep Mode:            ");
  Serial.println(accel.getSleepEnabled() ? "Enabled" : "Disabled");
  
  Serial.print(" * Clock Source:          ");
  switch(accel.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
  }
  
  Serial.print(" * Accelerometer:         ");
  switch(accel.getRange())
  {
    case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
    case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
    case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
    case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
  }  

  Serial.print(" * Accelerometer offsets: ");
  Serial.print(accel.getAccelOffsetX());
  Serial.print(" / ");
  Serial.print(accel.getAccelOffsetY());
  Serial.print(" / ");
  Serial.println(accel.getAccelOffsetZ());
  
  Serial.println();
}
