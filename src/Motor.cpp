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
    digitalWrite(pin, state);
    break;
  }
}

Motor::Motor(uint8_t _pin1, uint8_t _pin2, uint8_t _pin3, uint8_t _pin4) : pin1(_pin1), pin2(_pin2), pin3(_pin3), pin4(_pin4)
{
  this->coil_state = 1;
  this->previous_coil_state = 0;

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
  if (this->coil_state > MAX_COIL_STATE)
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
}

void Motor::stepBackward()
{
  // Move hand
  this->coil_state--;
  if (this->coil_state < 1)
  {
    this->coil_state = MAX_COIL_STATE;
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
}

void Motor::writeNewCoilState()
{
  this->coils_active = true;

#ifdef COIL_MODE_SINGLE
  switch (this->coil_state)
  {
  case 1:
    quickWrite(pin4, LOW);
    quickWrite(pin1, HIGH);
    quickWrite(pin2, LOW);
    break;
  case 2:
    quickWrite(pin1, LOW);
    quickWrite(pin2, HIGH);
    quickWrite(pin3, LOW);
    break;
  case 3:
    quickWrite(pin2, LOW);
    quickWrite(pin3, HIGH);
    quickWrite(pin4, LOW);
    break;
  case 4:
    quickWrite(pin3, LOW);
    quickWrite(pin4, HIGH);
    quickWrite(pin1, LOW);
    break;
  case 0:
    this->disableAllCoils();
    break;
  }
#else
  // Enable new coil
  switch (this->coil_state)
  {
  case 1:
    quickWrite(pin4, LOW);
    quickWrite(pin1, HIGH);
    quickWrite(pin2, LOW);
    break;
  case 2:
    quickWrite(pin1, HIGH);
    quickWrite(pin2, HIGH);
    break;
  case 3:
    quickWrite(pin1, LOW);
    quickWrite(pin2, HIGH);
    quickWrite(pin3, LOW);
    break;
  case 4:
    quickWrite(pin2, HIGH);
    quickWrite(pin3, HIGH);
    break;
  case 5:
    quickWrite(pin2, LOW);
    quickWrite(pin3, HIGH);
    quickWrite(pin4, LOW);
    break;
  case 6:
    quickWrite(pin3, HIGH);
    quickWrite(pin4, HIGH);
    break;
  case 7:
    quickWrite(pin3, LOW);
    quickWrite(pin4, HIGH);
    quickWrite(pin1, LOW);
    break;
  case 8:
    quickWrite(pin4, HIGH);
    quickWrite(pin1, HIGH);
    break;
  case 0:
    this->disableAllCoils();
    break;
  }
#endif
}

bool Motor::tryStep()
{
  if (this->coils_active == false && this->planned_steps == 0 && this->recal_steps == 0)
  {
    return false;
  }

  unsigned long micros_since_last_step = micros() - this->last_step_micros; // always positive. Overflow save

  // No planned steps? Prevent motor from overheating by disabling coils
  if (this->planned_steps == 0 && micros_since_last_step > MIN_STANDSTILL_DELAY)
  {
    this->disableAllCoils();
    this->last_step_micros = 0;
    return false;
  }

  // If we are not ready for the next step, skip current iteration
  if (micros_since_last_step < MIN_STEP_DELAY)
  {
    return false;
  }

  // // If there is recalibration needed, we might have to update the movement
  // if (this->recal_steps != 0)
  // {
  //   if (this->planned_steps == 0)
  //   {
  //     // Current plan complete: Now execute the remaining recalibration steps
  //     this->planned_steps = this->recal_steps;
  //     this->recal_steps = 0;
  //   }
  //   else if (this->planned_steps > 0 && this->recal_steps > 0)
  //   {
  //     // Both planned and recalibration steps are positive: Skip one planned step
  //     this->planned_steps--;
  //     this->recal_steps--;
  //   }
  //   else if (this->planned_steps < 0 && this->recal_steps < 0)
  //   {
  //     // Both planned and recalibration steps are negative: Skip one planned step
  //     this->planned_steps++;
  //     this->recal_steps++;
  //   }
  //   else if (this->planned_steps > 0 && this->recal_steps < 0)
  //   {
  //     // Planned steps are positive, recalibration steps are negative: Skip one planned step
  //     this->planned_steps--;
  //     this->recal_steps++;
  //   }
  //   else if (this->planned_steps < 0 && this->recal_steps > 0)
  //   {
  //     // Planned steps are negative, recalibration steps are positive: Skip one planned step
  //     this->planned_steps++;
  //     this->recal_steps--;
  //   }
  // }

  // We can execute the next step!
  if (this->planned_steps < 0)
  {
    this->stepBackward();
    this->planned_steps++;
  }
  else if (this->planned_steps > 0)
  {
    this->stepForward();
    this->planned_steps--;
  }
  else
  {
    return false;
  }
  this->last_step_micros = micros();
  return true;
}

void Motor::reset()
{
  this->current_pos = 0;
  this->last_step_micros = 0;
  this->planned_steps = 0;
  this->recal_steps = 0;
  this->disableAllCoils();
}

void Motor::disableAllCoils()
{
  quickWrite(pin1, LOW);
  quickWrite(pin2, LOW);
  quickWrite(pin3, LOW);
  quickWrite(pin4, LOW);
  this->coils_active = false;
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
  // Serial.print("Recalibrating motor. Target position: ");
  // Serial.print(target_pos);
  // Serial.print(" Steps off: ");
  // Serial.print(steps_off);
  // Serial.print(" Correction direction: ");
  // Serial.println(correction_direction);
  // Serial.print("Current position: ");
  // Serial.print(this->current_pos);
  if (correction_direction)
  {
    this->planned_steps -= steps_off;
    this->current_pos = (target_pos + steps_off) % MAX_STEPS;
  }
  else
  {
    this->planned_steps += steps_off;
    this->current_pos = (target_pos - steps_off + MAX_STEPS) % MAX_STEPS;
  }
  // Serial.print("New Current position: ");
  // Serial.println(this->current_pos);
}