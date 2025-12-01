/*
 * gpsdo 
 *
 */
 
#include <Adafruit_SI5351.h> 
#include <Adafruit_MCP4725.h>
#include <SoftwareSerial.h>
#include <Wire.h>

// Connect NEO-6M GPS module to pins 4 (RX) and 3 (TX)
SoftwareSerial gpsSerial(4, 3);

Adafruit_SI5351 clockgen = Adafruit_SI5351(); //includes the library
Adafruit_MCP4725 dac;

// CFG-NAV5: Set navigation mode to stationary
byte CFG_NAV5_STATIONARY[] = { 0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4E, 0x60 };

// UBX CFG-CFG: BBR + FLASH:
byte CFG[] = { 0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1D, 0xAB };

// Timepulse 5: 100 kHz, 5 uS (50 % duty cycle), synch to UTC Time, No Putput when unlocked
byte TP5_100K_LOCK[]{ 0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xA0, 0x86, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x36, 0x36 };

void setup() {
  gpsSerial.begin(9600); // GPS default baud rate
  dac.begin(0x60); 
  
  pinMode(LED_BUILTIN, OUTPUT); //led config output
  pinMode(5, OUTPUT);   // Set D5 as a digital output (LED)
  pinMode(6, OUTPUT);   // Set D6 as a digital output (Relay)
  pinMode(A0, INPUT);   // Set A0 as a digital input (Lock)
  // pinMode(A3, INPUT);   // Set A3 as a analog input (dac)

  digitalWrite(5, LOW);   // Turn LED OFF
  digitalWrite(6, LOW);   // Turn Relay OFF
 
  dac.setVoltage(0x8C4, false);  // 0x7FF = 2047 -> mid-scale output

    /* Initialise the sensor */
  if (clockgen.begin() != ERROR_NONE)
  {    /* There was a problem detecting the IC ... check your connections */

    while(1){ //led blinking fast
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(100);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(100);                      // wait for a second
    }
  }

  
  

  clockgen.setupPLL(SI5351_PLL_A, 70,2,10); //set to 702Mhz
  // clockgen.setupMultisynth(0, SI5351_PLL_A, 28,8,100); // 25MHz
  clockgen.setupMultisynth(0, SI5351_PLL_A, 17,55,100); // 40MHz
  // clockgen.setupMultisynth(2, SI5351_PLL_A, 14,4,10); // 50MHz
  /* Enable the clocks */
  clockgen.enableOutputs(true);

  // Set Timepulse output to 100 kHz, 50 % duty-cycle
  gpsSerial.write(TP5_100K_LOCK, sizeof(TP5_100K_LOCK));
  // Set navigation mode to stationary
  gpsSerial.write(CFG_NAV5_STATIONARY, sizeof(CFG_NAV5_STATIONARY));
  // Store settings permanently
  gpsSerial.write(CFG, sizeof(CFG));

  digitalWrite(6, HIGH);   // Turn Relay ON
}


void loop() {

  int adcvalue = analogRead(A3);

  if(digitalRead(A0))
  {
    digitalWrite(5, HIGH);   // Turn LED ON
    digitalWrite(6, LOW);   // Turn Relay OFF
  }
  else
  {
    digitalWrite(5, LOW);   // Turn LED OFF
  }

  uint16_t dacvalue = 0x8C4;   // 2725mV

  Wire.beginTransmission(0x60);  // MCP4725 I2C address
  Wire.write((dacvalue >> 8) & 0x0F);  // Upper data bits (D11–D8)
  Wire.write(dacvalue & 0xFF);        // Lower data bits (D7–D0)
  Wire.endTransmission();

  delay(1000);
  
}
