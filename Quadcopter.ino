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
int cal_count;
double gyro_roll, gyro_roll_c, gyro_pitch, gyro_pitch_c, gyro_yaw, gyro_yaw_c;
byte lowByte, highByte;


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
  Wire.beginTransmission(107);
  Wire.write(0x20);
  Wire.write(0x0F);
  Wire.endTransmission();
  // Write to Register 0x23:
  //   -Set Block Data Update to 'active' (disabled by default)
  //   (This is because I2C sends in 2 Bytes (lowByte and highByte)
  Wire.beginTransmission(107);
  Wire.write(0x23);
  Wire.write(0x80);
  Wire.endTransmission();
  delay(250);
  // Calibrate the Gyro Readings by averaging among samples
  Serial.print("Calibrating Gyro");
  for(cal_count = 0; cal_count < 2000; cal_count++){
    read_gyro();
    gyro_roll_c += gyro_roll;
    gyro_pitch_c += gyro_pitch;
    gyro_yaw_c += gyro_yaw;
    if(cal_count%100 == 0)Serial.print(".");  
    delay(4);
  }
  Serial.print("Done Calibrating");
  gyro_roll_c /= 2000;
  gyro_pitch_c /= 2000;
  gyro_yaw_c /= 2000;
}  
  
void loop(){
  delay(250);
  //print_tx_signals(); //print Tx Controls
  read_gyro();
  //print_gyro_signals();
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
//   Read Gyro Signals on XYZ Axis via I2C as //
// initiated in setup().                      //
////////////////////////////////////////////////
void read_gyro(){
  Wire.beginTransmission(107);
  Wire.write(168);
  Wire.endTransmission();
  // Get 6 Bytes from buffer
  Wire.requestFrom(107, 6);
  // Wait for the Bytes to arrive
  while(Wire.available() < 6);
  // Read X-axis Bytes
  lowByte = Wire.read();
  highByte = Wire.read();
  gyro_roll = ((highByte << 8) | lowByte);
  // Read Y-axis Bytes
  lowByte = Wire.read();
  highByte = Wire.read();
  gyro_pitch = ((highByte << 8) | lowByte);
  gyro_pitch *= -1;
  // Read Z-axis Bytes
  lowByte = Wire.read();
  highByte = Wire.read();
  gyro_yaw = ((highByte << 8) | lowByte);
  gyro_yaw *= -1;
  if(cal_count == 2000){
    gyro_roll -= gyro_roll_c;
    gyro_pitch -= gyro_pitch_c;
    gyro_yaw -= gyro_yaw_c;
  }
 
}

////////////////////////////////////////////////
// Print signals from ISR transmitter signals //
////////////////////////////////////////////////
void print_tx_signals(){
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

//////////////////////////////////
// Print signals from Gyro axes //
//////////////////////////////////
void print_gyro_signals(){
  // 57.14286 converts rad/s to deg/s
  Serial.print("Roll:");
  if(gyro_roll >= 0)Serial.print("+");
  Serial.print(gyro_roll/57.14286, 0);
  if(gyro_roll/57.14286 - 2 > 0)Serial.print(" RwD");
  else if(gyro_roll/57.14286 + 2 < 0)Serial.print(" RwU");
  else Serial.print(" ---");
  Serial.print("  Pitch:");
  if(gyro_pitch >= 0)Serial.print("+");
  Serial.print(gyro_pitch/57.14286, 0);
  if(gyro_pitch/57.14286 - 2 > 0)Serial.print(" NoU");
  else if(gyro_pitch/57.14286 + 2 < 0)Serial.print(" NoD");
  else Serial.print(" ---");
  Serial.print("  Yaw:");
  if(gyro_yaw >= 0)Serial.print("+");
  Serial.print(gyro_yaw/57.14286, 0);
  if(gyro_yaw/57.14286 - 2 > 0)Serial.println(" NoR");
  else if(gyro_yaw/57.14286 + 2 < 0)Serial.println(" NoL");
  else Serial.println(" ---");
}




