#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#include <Arduino.h>
#include "Motor.h"

enum CalibrationState
{
  LEAVEMAGNET,
  FINDMAGNET,
  INFIELD,
  CENTERING,
  CALIBRATED
};

enum RecalibrationState
{
  WAITINGFORMAGNET,
  WAIT_EXIT_FORWARD,
  WAIT_EXIT_BACKWARD
};

class Calibration
{
public:
  Calibration(Motor &m, size_t hall_pin);
  void startCalibration();
  bool calibrate();

  void checkForCalibrationAfterStep();

private:
  bool isInField();

  Motor &motor;
  size_t hall_pin;
  CalibrationState state;
  size_t steps;

  bool recal_infield;
  bool recal_enter_direction;
  size_t recal_enter_pos;
  size_t recal_leave_pos;
};

#endif