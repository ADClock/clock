#include "Motor.h"
#include <FastGPIO.h>
#include "Config.h"

void quickWrite(uint8_t pin, bool state)
{
  switch (pin)
  {
  case 6:
    FastGPIO::Pin<6>::setOutputValue(state);
    break;
  case 7:
    FastGPIO::Pin<7>::setOutputValue(state);
    break;
  case 8:
    FastGPIO::Pin<8>::setOutputValue(state);
    break;
  case 9:
    FastGPIO::Pin<9>::setOutputValue(state);
    break;
  case 10:
    FastGPIO::Pin<10>::setOutputValue(state);
    break;
  case 11:
    FastGPIO::Pin<11>::setOutputValue(state);
    break;
  case 12:
    FastGPIO::Pin<12>::setOutputValue(state);
    break;
  case 13:
    FastGPIO::Pin<13>::setOutputValue(state);
    break;

  default:
    break;
  }
}

Motor::Motor(uint8_t _pin1, uint8_t _pin2, uint8_t _pin3, uint8_t _pin4) : pin1(_pin1), pin2(_pin2), pin3(_pin3), pin4(_pin4)
{
  this->coil_state = 1;

  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);

  this->disableAllCoils();

  this->reset();
}

void Motor::planStepForward()
{
  this->planned_steps++; // plan one step in positive direction (forward)
}

void Motor::planStepBackward()
{
  this->planned_steps--; // plan one step in negative direction (backward)
}

void Motor::stepForward()
{
  // Move hand
  this->coil_state++;
  if (this->coil_state > 4)
  {
    this->coil_state = 1;
  }
  this->writeNewCoilState();

  // Update position
  if (this->current_pos == MAX_STEPS - 1)
  {
    this->current_pos = 0;
  }
  else
  {
    this->current_pos++;
  }
  this->current_direction = true;

  // Decrease planned steps because we did one in positive direction
  this->planned_steps--;
}

void Motor::stepBackward()
{
  // Move hand
  this->coil_state--;
  if (this->coil_state < 1)
  {
    this->coil_state = 4;
  }
  this->writeNewCoilState();

  // Update position
  if (this->current_pos == 0)
  {
    this->current_pos = MAX_STEPS - 1;
  }
  else
  {
    this->current_pos--;
  }
  this->current_direction = false;

  // Increase planned steps because we did one in negative direction
  this->planned_steps++;
}

void Motor::writeNewCoilState()
{
  switch (this->coil_state)
  {
  case 1:
    quickWrite(pin2, HIGH);
    quickWrite(pin1, LOW);
    break;
  case 2:
    quickWrite(pin3, HIGH);
    quickWrite(pin2, LOW);
    break;
  case 3:
    quickWrite(pin4, HIGH);
    quickWrite(pin3, LOW);
    break;
  case 4:
    quickWrite(pin1, HIGH);
    quickWrite(pin4, LOW);
    break;
  }
}

bool Motor::tryStep()
{
  if (this->planned_steps != 0)
  {
    long micros_since_last_step = micros() - this->last_step_micros; // always positive. Overflow save
    if (micros_since_last_step > MIN_STEP_DELAY)
    {
      // We can execute the next step!
      if (this->planned_steps < 0)
      {
        this->stepBackward();
      }
      else if (this->planned_steps > 0)
      {
        this->stepForward();
      }
      this->last_step_micros = micros();
      return true;
    } // else do nothing and wait until the MIN_STEP_DELAY is reached
  }
  else
  {
    // This might be to fast. Maybe we need MIN_STEP_DELAY before turning the pins off?
    this->disableAllCoils();
  }
  return false; // No step was executed
}

void Motor::reset()
{
  this->current_pos = 0;
  this->last_step_micros = 0;
  this->planned_steps = 0;
}

void Motor::disableAllCoils()
{
  quickWrite(pin1, LOW);
  quickWrite(pin2, LOW);
  quickWrite(pin3, LOW);
  quickWrite(pin4, LOW);
}

size_t Motor::getCurrentPosition()
{
  return this->current_pos;
}

bool Motor::isRotatingForwards()
{
  return this->current_direction;
}

void Motor::recalibrate(size_t target_pos, size_t steps_off, bool correction_direction)
{
  if (correction_direction)
  {
    this->planned_steps += steps_off;
    this->current_pos = (target_pos - steps_off + MAX_STEPS) % MAX_STEPS;
  }
  else
  {
    this->planned_steps -= steps_off;
    this->current_pos = (target_pos + steps_off) % MAX_STEPS;
  }
}