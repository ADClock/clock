#include "Calibration.h"
#include "Utils.h"
#include <FastGPIO.h>
#include "Config.h"

Calibration::Calibration(Motor &m, size_t hall_pin) : motor(m), hall_pin(hall_pin)
{
  pinMode(hall_pin, INPUT_PULLUP);
}

/**
 * @brief Starts the calibration process.
 *
 */
void Calibration::startCalibration()
{
  this->steps = 0;
  this->state = FIND_MAGNET;
}

/**
 * @brief Calibrates the motor.
 *
 * The calibration process is divided into several steps:
 * 1. Preparation: If the motor is inside or very close to the magnetic field, it must leave the field first. (FIND_MAGNET)
 *   1. Check if the motor is inside the magnetic field and the steps are less than MIN_STEPS_OUTSIDE_FIELD.
 *   2. Forcing the motor to leave the magnetic field by rotating it backwards until MIN_STEPS_OUTSIDE_FIELD is reached. (LEAVE_MAGNET)
 * 2. The motor must find the magnetic field (FIND_MAGNET) by stepping forward.
 * 3. The motor must walk through the magnetic field (INFIELD) and count the width of the field.
 * 4. The motor must rotate backwards by half the width of the field (CENTERING).
 * 5. The motor is calibrated (CALIBRATED).
 *
 * @return true
 * @return false
 */
bool Calibration::calibrate()
{
  if (this->state == CALIBRATED)
    return true;

  bool in_field = isInField();

  switch (this->state)
  {
  case FIND_MAGNET:
    if (in_field && this->steps <= MIN_STEPS_OUTSIDE_FIELD)
    {
      this->steps = MIN_STEPS_OUTSIDE_FIELD + 1;
      this->state = LEAVE_MAGNET;
    }
    else if (in_field && this->steps > MIN_STEPS_OUTSIDE_FIELD)
    {
      this->steps = 0;
      this->state = INFIELD;
    }
    else
    {
      this->motor.stepForward();
      this->steps++;
    }
    break;

  case LEAVE_MAGNET:
    if (this->steps == 0)
    {
      this->state = FIND_MAGNET;
    }
    else if (in_field)
    {
      this->motor.stepBackward();
    }
    else
    {
      this->motor.stepBackward();
      this->steps--;
    }
    break;

  case INFIELD:
    if (!in_field)
    {
      this->state = CENTERING;
      this->steps = this->steps / 2;
    }
    else
    {
      this->motor.stepForward();
      this->steps++;
    }
    break;

  case CENTERING:
    if (this->steps == 0)
    {
      this->state = CALIBRATED;
      this->motor.reset();
      this->recal_ignore_next_field = true;
    }
    else
    {
      this->steps--;
      this->motor.stepBackward();
    }
    break;

  default:
    break;
  }

  return false;
}

/**
 * @brief Checks if the hall sensor detects the magnet.
 *
 * @return true The motor is in the magnet field.
 * @return false The motor is not in the magnet field.
 */
bool Calibration::isInField()
{
  return digitalRead(this->hall_pin) == LOW;
}

/**
 * @brief 100 % bug free
 *
 * This method is called on every tick in main loop.
 * It checks if the hand/motor interacts with the magnet field and saves the enter and leave position.
 * With those values the magnet field width is calculated and the motor position is recalibrated.
 */
void Calibration::checkForCalibrationAfterStep()
{
  bool in_field = isInField();
  if (!this->recal_infield && in_field)
  {
    this->recal_infield = true;
    this->recal_enter_pos = this->motor.getCurrentPosition();
    this->recal_enter_direction = motor.isRotatingForwards(); // true = forwards, false = backwards
  }
  else if (this->recal_infield && !in_field)
  {
    this->recal_infield = false;

    if (this->recal_enter_direction != motor.isRotatingForwards())
    {
      return; // The field was left on the same side. Do not recalibrate
    }

    if (this->recal_ignore_next_field)
    {
      this->recal_ignore_next_field = false;
      return; // Ignore the first field after calibration. Do not recalibrate
    }

    this->recal_leave_pos = this->motor.getCurrentPosition();

    size_t field_width = diff(this->recal_enter_pos, this->recal_leave_pos, this->recal_enter_direction);

    // Serial.print("Field width: ");
    // Serial.println(field_width);
    // Serial.print(" Enter pos: ");
    // Serial.println(this->recal_enter_pos);
    // Serial.print(" Leave pos: ");
    // Serial.println(this->recal_leave_pos);
    // Serial.print(" Enter dir: ");
    // Serial.println(this->recal_enter_direction);

    if (field_width < MIN_WIDTH_FOR_RECALIBRATION)
    {
      return; // The tracked magnet field was smaller then the minimum magnet field width. Do not recalibrate
    }

    size_t target_leave_pos = calculateFieldLeavePosition(field_width, this->recal_enter_direction);
    bool correction_direction = getShortestDirection(this->motor.getCurrentPosition(), target_leave_pos);
    size_t steps_off = diff(this->motor.getCurrentPosition(), target_leave_pos, correction_direction);

    if (steps_off < MIN_STEPS_OFF_FOR_RECALIBRATION)
    {
      return; // The motor is not far enough off target position. Do not recalibrate
    }

    steps_off /= 2; // Soft recalibration (slowly pull hand back to correct position)

    this->motor.recalibrate(target_leave_pos, steps_off, correction_direction);
    this->recal_ignore_next_field = true;
  }
}