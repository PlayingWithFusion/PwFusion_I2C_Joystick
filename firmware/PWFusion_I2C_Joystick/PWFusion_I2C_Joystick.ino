/***************************************************************************
* File Name: PwFusion_I2C_Joystick.ino
* Processor/Platform: ATtiny841-MMHR (tested)
* Development Environment: Arduino 2.1.1
*
* Designed for use with with Playing With Fusion IFB-40002 I2C Joystick
* Copyright � 2023 Playing With Fusion, Inc.
* SOFTWARE LICENSE AGREEMENT: This code is released under the MIT License.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
* **************************************************************************
* REVISION HISTORY:
* Author		    Date		    Comments
* N. Johnson    2023Oct1   Original version
*
* Playing With Fusion, Inc. invests time and resources developing open-source
* code. Please support Playing With Fusion and continued open-source
* development by buying products from Playing With Fusion!
*
* **************************************************************************
* ADDITIONAL NOTES:
* This file contains functions to flash an AtTiny841-MMHR included in
* the Playing with Fusion i2c joystick interface board (IFB-40002).
* Funcionality is as described below:
*   - Read values from the joystick componet
*   - Pack values into a register struct
*   - Send values over the i2c bus when requested
***************************************************************************/

#include <WireS.h>
#include <ezButton.h>

uint8_t primaryAddress = 0x03;
uint8_t secondaryAddress = 0x04;

// Define pins
#define VRX A2
#define VRY A1
#define SW 0

// Address selecter pin
#define ADR_SEL 7

ezButton button(SW);

struct memoryMap {
  uint8_t vrx;
  uint8_t vry;
  uint8_t sw;
};

// Define register addresses
volatile memoryMap registerMap = {
  .vrx = 0x00,
  .vry = 0x01,
  .sw = 0x02,
};

// Create a pointer so the register number refers to data in the registerMap.
uint8_t *registerPointer = (uint8_t *)&registerMap;

volatile byte registerPosition;

int joystickData[] = {0, 0, 0};
int btnState;

void setup() {
  pinMode(ADR_SEL, INPUT);
  startI2C();

  pinMode(SW, INPUT);
  button.setDebounceTime(5); // set debounce time to 50 milliseconds
};

void startI2C() {

  // Select the i2c address based on the state of the jumper
  if (digitalRead(ADR_SEL) == LOW) {
    Wire.begin(primaryAddress);
  } else {
    Wire.begin(secondaryAddress);
  }

  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}


void loop() {
  // Collect data from the joystick button
  button.loop();
  if(button.isPressed())
    btnState = 1;

  if(button.isReleased())
    btnState = 0;
  
  // Store the mapped values from the joystick in the registerMap struct
  registerMap.vrx = map(analogRead(VRX), 0, 1023, 0, 255);
  registerMap.vry = map(analogRead(VRY), 0, 1023, 0, 255);
  registerMap.sw = btnState;
}
 
// Gets called when the ATtiny receives an i2c request
void requestEvent() {
  Wire.write(*(registerPointer + registerPosition));
}

// Gets called when the ATtiny revieves an i2c transmission
void receiveEvent(byte howMany) {
  if (howMany < 1) return;

  // Set the register offset position
  registerPosition = Wire.read();
  howMany--;
  if (!howMany) return;
}