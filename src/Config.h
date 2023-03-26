#ifndef _CONFIG_H_
#define _CONFIG_H_

// Calibration
#define MIN_STEPS_OUTSIDE_FIELD 20
#define MIN_WIDTH_FOR_RECALIBRATION 10

// Communication parameters
#define MIN_OUT_CLOCK_HIGH 5            // us
#define DELAY_BETWEEN_INSTRUCTIONS 1500 // us

// Motor
#define MAX_STEPS 1705             // steps per rotation
#define MIN_STEP_DELAY 2500        // us
#define MIN_STANDSTILL_DELAY 10000 // us

// Pins
#define HALL_DATA_PIN_1 A1
#define HALL_DATA_PIN_2 A0

#define MOTOR_1_PIN_1 10
#define MOTOR_1_PIN_2 11
#define MOTOR_1_PIN_3 12
#define MOTOR_1_PIN_4 13

#define MOTOR_2_PIN_1 9
#define MOTOR_2_PIN_2 8
#define MOTOR_2_PIN_3 7
#define MOTOR_2_PIN_4 6

// Pins for DataSender (Sending to next Arduino)
#define COMM_OUT_CLOCK A3
#define COMM_OUT_DATA1 A2
#define COMM_OUT_DATA2 A4
#define COMM_OUT_DATA3 1 // tx
#define COMM_OUT_DATA4 0 // rx

// Pins for DataReceiver (Receiving from previous Arduino or Raspberry Pi Zero)
#define COMM_IN_CLOCK 2  // interrupt pin
#define COMM_IN_DATA1 A5 // 1
#define COMM_IN_DATA2 3
#define COMM_IN_DATA3 4
#define COMM_IN_DATA4 5

// Enable Debug messages for Serial
// #define DEBUG

#endif