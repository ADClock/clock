#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <Arduino.h>

class Motor
{
public:
  Motor(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4);
  void stepForward();
  void stepBackward();
  void planStepForward();
  void planStepBackward();
  bool tryStep();
  void reset();

  size_t getCurrentPosition();
  bool isRotatingForwards();
  void recalibrate(size_t target_pos, size_t steps_off, bool correction_direction);

private:
  void writeNewCoilState();
  void disableAllCoils();

  const uint8_t pin1;
  const uint8_t pin2;
  const uint8_t pin3;
  const uint8_t pin4;

  size_t current_pos;
  bool current_direction;
  int planned_steps;

  unsigned long last_step_micros;
  size_t coil_state;
};

#endif