# Code for a clock of the ADClock

The ADClock consists of 24 individual clocks. The special feature of the individual clocks is that the hands can be moved forwards and backwards independently of one another using two stepper motors. A microcontroller is built into every watch to control the motors and calibrate the hands. The Arduino Atmega328p receives 4 bits of data for each tick, which includes the stepping instruction. The beat is transmitted by an additional clock. Further data packets, which are sent directly after your own step instruction, are passed on to the next clock.


## How it works

- General clock layout

### Step Motors
- Calibration
- Step
- Connection
- Power

### Controller
