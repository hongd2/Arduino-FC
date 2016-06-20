#include <Wire.h> 
// Project is done with some help from http://www.brokking.net/ymfc-3d_main.html


/////////////////////////////////////
// Variables for Transmitter ISR() //
/////////////////////////////////////
byte last_ch_1, last_ch_2, last_ch_3, last_ch_4;
unsigned long timer_1, timer_2, timer_3, timer_4, current_time;
int rec_input_timer_1, rec_input_timer_2, rec_input_timer_3, rec_input_timer_4;

////////////////////////////////
// Variables for Reading Gyro //
////////////////////////////////


void setup(){
  Serial.begin(9600);
  //////////////////////////////////////////////
  // PCICR: Enable PCMSK0 scan for interrupts //
  // PCINT0: Digital Pin 8 trigger interrupt  //
  // PCINT1: Digital Pin 9 trigger interrupt  //
  // PCINT2: Digital Pin 10 trigger interrupt //
  // PCINT3: Digital Pin 11 trigger interrupt //
  //////////////////////////////////////////////
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT0);
  PCMSK0 |= (1 << PCINT1);
  PCMSK0 |= (1 << PCINT2);
  PCMSK0 |= (1 << PCINT3);
  
  
  /////////////////////////////////////
  // Setup Gyro Ports and I2C Buffer //
  /////////////////////////////////////
  Wire.begin();
  // Gyro I2C Addr: https://www.pololu.com/file/0J563/L3GD20.pdf
  // Write to Register 0x20:
  //   -Set gyro to 'enabled' (disabled by default)
  //   -Set gyro xyz axis to enabled (enabed by default)
  Wire.beginTransmission(110101xb);
  Wire.write(0x20);
  Wire.write(00001111xb);
  Wire.endTransmission();
  // Write to Register 0x23:
  //   -Set Block Data Update to 'active' (disabled by default)
  //   (This is because I2C sends in 2 Bytes (lowByte and highByte)
  Wire.beginTransmission(110101xb);
  Wire.write(0x23);
  Wire.write(10000000xb);
  Wire.endTransmission();
  delay(250);
  
}

void loop(){
  delay(250);
  //read_signals(); //Read Tx Controls
}


///////////////////////////////////////////////////
// Interrupt Subroutine for transmitter controls //
///////////////////////////////////////////////////
ISR(PCINT0_vect){
  current_time = micros();
  if(last_ch_1 == 0 && PINB & B00000001){
    last_ch_1 = 1;
    timer_1 = current_time;
  }
  else if(last_ch_1 == 1 && !(PINB & B00000001)){
    last_ch_1 = 0;
    rec_input_timer_1 = current_time - timer_1;
  }
  
  if(last_ch_2 == 0 && PINB & B00000010){
    last_ch_2 = 1;
    timer_2 = current_time;
  }
  else if(last_ch_2 == 1 && !(PINB & B00000010)){
    last_ch_2 = 0;
    rec_input_timer_2 = current_time - timer_2;
  }
  
  if(last_ch_3 == 0 && PINB & B00000100){
    last_ch_3 = 1;
    timer_3 = current_time;
  }
  else if(last_ch_3 == 1 && !(PINB & B00000100)){
    last_ch_3 = 0;
    rec_input_timer_3 = current_time - timer_3;
  }
  
  if(last_ch_4 == 0 && PINB & B00001000){
    last_ch_4 = 1;
    timer_4 = current_time;
  }
  else if(last_ch_4 == 1 && !(PINB & B00001000)){
    last_ch_4 = 0;
    rec_input_timer_4 = current_time - timer_4;
  } 
}

////////////////////////////////////////////////
// Print signals from ISR transmitter signals //
////////////////////////////////////////////////
void read_signals(){
  Serial.print("Roll:");
  if(rec_input_timer_1 - 1480 < 0) Serial.print("<<<");
  else if(rec_input_timer_1 - 1520 > 0) Serial.print(">>>");
  else Serial.print("-+-");
  Serial.print(rec_input_timer_1);
  
  Serial.print("  Pitch:");
  if(rec_input_timer_2 - 1480 < 0) Serial.print("^^^");
  else if(rec_input_timer_2 - 1520 > 0) Serial.print("vvv");
  else Serial.print("-+-");
  Serial.print(rec_input_timer_2);
  
  Serial.print("  Gas:");
  if(rec_input_timer_3 - 1480 < 0) Serial.print("vvv");
  else if(rec_input_timer_3 - 1520 > 0) Serial.print("^^^");
  else Serial.print("-+-");
  Serial.print(rec_input_timer_3);
  
  Serial.print("  Yaw:");
  if(rec_input_timer_4 - 1480 < 0) Serial.print("<<<");
  else if(rec_input_timer_4 - 1520 > 0) Serial.print(">>>");
  else Serial.print("-+-");
  Serial.println(rec_input_timer_4);
}


