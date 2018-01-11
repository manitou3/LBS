

/*

 N6QW / KK6FUT Super Simple DDS - Si5351 version
 2015 Feb 02
 
 This is a super simple DDS sketch to run a single band PLL Based Clock Generator.
 Kept super-super simple to make it easy for the non-software folks.
 
 Inputs: Rotary encoder
 Outputs: LCD display, DDS frequency

 Uses a a (16*2) LCD Display on the I2C Interface for YwRobot. 
 Using the Si5351 breakout board from Adafruit.
 
 */

#include<stdlib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //twrobot
#include "Si5351.h"

#define IF           0L

Si5351 si5351;

boolean keystate = 0;
boolean changed_f = 0;
int old_vfo = 0;
int old_bfo = 0;
int val = 0; // used for dip

const uint32_t bandStart = 7000000L;  // start of 40m
const uint32_t bandEnd =   7300000L; // end of 40m
const uint32_t bandInit =  7100000L; // where to initially set the frequency

const uint32_t offset =    4915200L; // amount to add for IF offset
const uint32_t USB = 4916700L;
const uint32_t LSB = 4913600L;

volatile uint32_t freq = bandInit ;  // this is a variable (changes) - set it to the beginning of the band
volatile uint32_t vfo = bandInit+LSB ;  // this is a variable (changes) - set it to the beginning of the band
volatile uint32_t bfo = LSB; // or LSB later make it selectable with the SSB Select Switch
volatile uint32_t radix = 100; // how much to change the frequency by, clicking the rotary encoder will change this.

int blinkpos = 3; // position for blinking cursor


// Set pins for ROTARY ENCODER
const int RotEncAPin = 5;
const int RotEncBPin = 7;
const int RotEncSwPin = A3;

// Display library assumes use of A4 for clock, A5 for data. No code needed.

// Variables for Rotary Encoder handling
boolean OldRotEncA = true;
boolean RotEncA = true;
boolean RotEncB = true;
boolean RotEncSw = true;

// Instantiate the LCD display...

//LiquidCrystal_I2C lcd(0x027, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address TwRobot
LiquidCrystal_I2C lcd(0x3F,16,4);  // set the LCD address to 0x20 for a 16 chars and 2 line display

void setup() {
   Serial.begin(9600);
   Serial.print("In setup");
   // Set up LCD

 // lcd.init();
//  delay(35);
  lcd.begin();   // initialize the lcd for 16 chars 2 lines, turn on backlight TwRobot
 
    // Print a message to the LCD.
 lcd.backlight();

  lcd.setCursor(0, 1);
  lcd.print(" N6QW & KK6FUT");
  lcd.setCursor(13,0);
  lcd.print("LSB");

  // Set up ROTARY ENCODER
  pinMode(RotEncAPin, INPUT);
  pinMode(RotEncBPin, INPUT);
  pinMode(RotEncSwPin, INPUT);
  // set up pull-up resistors on inputs...  
  digitalWrite(RotEncAPin,HIGH);
  digitalWrite(RotEncBPin,HIGH);
  digitalWrite(RotEncSwPin,HIGH); 
  
  
  // Start serial and initialize the Si5351
  si5351.init(SI5351_CRYSTAL_LOAD_8PF);
  //si5351.set_correction(200);
  delay(1000);
  // Set CLK0 to output vfo  frequency with a fixed PLL frequency
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_freq(vfo , SI5351_PLL_FIXED, SI5351_CLK0); 
  si5351.set_freq(bfo , 0, SI5351_CLK1); 
  //set power
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);
  
  // start the oscillator...
  send_frequency(freq);     
  display_frequency(freq);
  Serial.print("end setup");
}

void loop() {

  // Read the inputs...
  RotEncA = digitalRead(RotEncAPin);
  RotEncB = digitalRead(RotEncBPin); 
  RotEncSw = digitalRead(RotEncSwPin);

  // check the rotary encoder values
  if ((RotEncA == HIGH)&&(OldRotEncA == LOW)){
    // adjust frequency
    if (RotEncB == LOW) { 
      freq=constrain(freq+radix,bandStart,bandEnd);
    } else {
      freq=constrain(freq-radix,bandStart,bandEnd);
    }
    OldRotEncA=RotEncA;  // store rotary encoder position for next go around
 
     // Now, update the LCD with frequency 
    display_frequency(freq); // push the frequency to LCD display
    send_frequency(freq);  // set the DDS to the new frequency  
   // delay(400); // let the frequency/voltages settle out after changing the frequency
   }
  
  // check for the click of the rotary encoder 
  if (RotEncSw==LOW){
    // if user clicks rotary encoder, change the size of the increment
    // use an if then loop, but this could be more elegantly done with some shifting math
    
    if (radix == 10000) {
       radix = 100;
    } else if (radix == 1000) {
       radix = 10000;
    } else if (radix == 100) {
       radix = 1000;
    } else if (radix == 10) {
       radix = 100;
     } else { // this is either 100 or we goofed somewhere else, so set it back to the big change
       radix = 10000;
    }
  }    
  
  OldRotEncA=RotEncA; 
  
  
  // End of loop()
}



// subroutine to display the frequency...
void display_frequency(uint32_t frequency) {  
  lcd.noBlink();
  lcd.setCursor(0, 0); //was 17
  if (frequency<10000000){
    lcd.print(" ");
  }  
  lcd.print(frequency/1e6,5);  
  lcd.print(" MHz");
  
  

}  


// Subroutine to generate a positive pulse on 'pin'...
void pulseHigh(int pin) {
  digitalWrite(pin, HIGH); 
  digitalWrite(pin, LOW); 
}

// calculate and send frequency code to Si5351...
void send_frequency(uint32_t frequency) {
  vfo = frequency+LSB;
  //vfo = frequency+offset;
  si5351.set_freq(vfo , SI5351_PLL_FIXED, SI5351_CLK0); 
  si5351.set_freq(bfo , 0, SI5351_CLK1); 
  Serial.print("freq: ");
  Serial.print(frequency);
  Serial.print(" vfo: ");
  Serial.print(vfo);
  Serial.print(" bfo:  ");
  Serial.print(bfo);
  Serial.print(" vfo-bfo: ");
  Serial.print(vfo - bfo);
  Serial.print("\n");



}







