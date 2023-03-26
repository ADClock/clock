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
#ifdef DEBUG
  Serial.println("isr_data_receiving complete");
#endif
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
    delay(3);

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

void testCommunicationWithInstruction(Instruction &instruction, size_t clocks, size_t repeats, size_t delayBetweenClocks, size_t delayBetweenInstructions)
{
  unsigned long lastInstructionSendMicros = 0;
  for (size_t i = 0; i < repeats; i++)
  {
    // for (size_t j = 0; j < clocks; j++)
    //{
    lastInstructionSendMicros = micros();
    comm.sendTestInstruction(instruction);
    //  while (micros() - lastInstructionSendMicros < delayBetweenClocks)
    //  {
    //    comm.tick();
    //  }
    //}
    while (micros() - lastInstructionSendMicros < delayBetweenInstructions)
    {
      comm.tick();
    }
  }
}

/**
 * @brief Test the communication between the clocks.
 *
 * This function is used to test the communication between the clocks.
 * It is not used in the final product.
 *
 */
void testCommunication()
{
  for (size_t i = 0; i < 5; i++)
  {
    motor1.stepForward();
    delay(1000);
  }

  Instruction instruction;
  instruction.hourBackward = true;
  instruction.hourForward = false;
  instruction.minuteBackward = true;
  instruction.minuteForward = false;
  testCommunicationWithInstruction(instruction, 1, 1000, 50, 4000);
  motor2.stepForward();

  delay(500);
  instruction.hourBackward = false;
  instruction.hourForward = true;
  instruction.minuteBackward = false;
  instruction.minuteForward = true;
  testCommunicationWithInstruction(instruction, 1, 1000, 50, 4000);

  delay(500);
  instruction.hourBackward = true;
  instruction.hourForward = false;
  instruction.minuteBackward = false;
  instruction.minuteForward = true;
  testCommunicationWithInstruction(instruction, 1, 500, 50, 8000);

  delay(500);
  instruction.hourBackward = false;
  instruction.hourForward = false;
  instruction.minuteBackward = true;
  instruction.minuteForward = false;
  testCommunicationWithInstruction(instruction, 1, 500, 50, 4000);

  // Reset for next time
  motor2.stepForward();
  motor1.stepForward();
  delay(1000);
  instruction.hourBackward = true;
  instruction.hourForward = true;
  instruction.minuteBackward = true;
  instruction.minuteForward = true;
  testCommunicationWithInstruction(instruction, 1, 1, 50, 4000);
  motor2.stepForward();
  motor1.stepForward();
  delay(5000);
}

void testMotorSpeed(size_t delay_us)
{
  for (size_t i = 0; i < MAX_STEPS * 6; i++)
  {
    motor1.stepForward();
    motor2.stepBackward();
    delayMicroseconds(delay_us);
  }
  delay(5000);
}

void setup()
{
  // Serial.begin(115200);
  // Serial.println("ADClock clock setup. For more information visit https://github.com/ADClock/clock");
  // Serial.println("For debugging information, define DEBUG in Config.h!");

  calibrateMotors();

  // Test communication
  // testCommunication();

  // Test different motor speeds
  // delay(2000);
  // testMotorSpeed(4000);
  // testMotorSpeed(3500);
  // testMotorSpeed(3000);

  // ISR for Data Input
  attachInterrupt(digitalPinToInterrupt(COMM_IN_CLOCK), isr_data_receiving, RISING);

#ifdef DEBUG
  Serial.println("> Debugging is enabled. Setup finished.");
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
    else
    {
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
