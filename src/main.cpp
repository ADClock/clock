#include <Arduino.h>
#include "Config.h"
#include "Calibration.h"
#include "ClockCommunication.h"

Motor motor1(MOTOR_1_PIN_1, MOTOR_1_PIN_2, MOTOR_1_PIN_3, MOTOR_1_PIN_4);
Motor motor2(MOTOR_2_PIN_1, MOTOR_2_PIN_2, MOTOR_2_PIN_3, MOTOR_2_PIN_4);

Calibration calibration1(motor1, HALL_DATA_PIN_1);
Calibration calibration2(motor2, HALL_DATA_PIN_2);

OwnInstruction ownInstruction;

ClockCommunication comm(ownInstruction);

/**
 * @brief Interrupt Service Routine that is called when the clock receives a tick.
 *
 * The transmitted data is either for the own clock or needs to be passed on to the next clock.
 *
 */
void isr_data_receiving()
{
  comm.processDataInput();
}

/**
 * @brief Calibration of both stepper motors.
 *
 * After executing this function, the clock is calibrated and ready to be used.
 * Both motors are calibrated at the same time.
 *
 * The stepper motors are rotated until the hall sensor detects a magnet. After three rotations without detecting a magnet, the calibration is canceled.
 * There might be a hardware problem if this happens.
 *
 * @return true if the calibration was successful.
 *
 */
bool calibrateMotors()
{
  calibration1.startCalibration();
  calibration2.startCalibration();
  bool motor1Calibrated = false;
  bool motor2Calibrated = false;
  size_t counter = 0;
  do
  {
    motor1Calibrated = calibration1.calibrate();
    motor2Calibrated = calibration2.calibrate();
    counter++;
    delay(5);

    if (counter > MAX_STEPS * 2)
    {
#ifdef DEBUG
      Serial.println("Calibration canceled. Atleast one Magnet not found.");
#endif
      return false;
    }
  } while (!motor1Calibrated || !motor2Calibrated);

  return true;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("ADClock clock setup. For more information visit https://github.com/ADClock/clock");
  Serial.println("For debugging information, define DEBUG in Config.h!");

  calibrateMotors();

  delay(2000);
  for (int i = 0; i < MAX_STEPS * 10; i++)
  {
    motor2.planStepBackward();
    motor1.planStepForward();
  }

  // ISR for Data Input
  attachInterrupt(digitalPinToInterrupt(COMM_IN_CLOCK), isr_data_receiving, RISING);

#ifdef DEBUG
  Serial.println("> Debugging is enabled. Setup finished.");
#else
  Serial.println("> Debugging is disabled. Setup finished. No more messages will be printed to ensure correct timings.");
#endif
}

void loop()
{

  if (ownInstruction.pending)
  {
    // Read own instruction and update motors
    if ((ownInstruction.data.hourBackward && ownInstruction.data.hourForward) || (ownInstruction.data.minuteBackward && ownInstruction.data.minuteForward))
    {
      calibrateMotors();
    }

    // Update motor1
    if (ownInstruction.data.hourBackward)
    {
      motor1.planStepBackward();
    }
    else if (ownInstruction.data.hourForward)
    {
      motor1.planStepForward();
    }

    // Update motor2
    if (ownInstruction.data.minuteBackward)
    {
      motor2.planStepBackward();
    }
    else if (ownInstruction.data.minuteForward)
    {
      motor2.planStepForward();
    }

    ownInstruction.pending = false; // own instruction processed
  }

  comm.tick();

  if (motor1.tryStep())
  {
    calibration1.checkForCalibrationAfterStep();
  }
  if (motor2.tryStep())
  {
    calibration2.checkForCalibrationAfterStep();
  }
}
