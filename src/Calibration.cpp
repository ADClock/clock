#include "Calibration.h"
#include <Utils.h>
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
  this->state = FINDMAGNET;
#ifdef DEBUG
  Serial.println("Starte Kalibierung");
#endif
}

// Funktionsweise der Kalibierung:
// 0. Der Motor muss das Magnetfeld finden (FINDMAGNET) und zählt die Steps
// 1. Der Motor muss sich mindestens MIN_STEPS_OUTSIDE_FIELD Steps außerhalb des Magnetfelds drehen, damit die initalisierung richtig funktioniert. (LEAVEMAGNET)
// 2. Der Motor muss das Magnetfeld finden (FINDMAGNET) und zählt die Steps
// 3. Der Motor durchwandert das Magnetfeld (INFIELD) und zählt die Steps
// 4. Der Motor verlässt das Magnetfeld nimmt die Stepps von INFIELD/2 und läuft diese zurück (CENTERING)
// 5. Der Motor ist kalibiert (CALIBRATED)
bool Calibration::calibrate()
{
#ifdef SKIPCALIBRATION
#ifdef DEBUG
  Serial.println("Calibration >> Die Kalibierung wurde aufgrund von 'SKIPCALIBRATION' übersprungen.");
#endif
  return true;
#endif

  if (this->state == CALIBRATED)
    return true;

  bool in_field = isInField();

  switch (this->state)
  {
  case FINDMAGNET:
    if (in_field && this->steps <= MIN_STEPS_OUTSIDE_FIELD)
    {
      this->steps = MIN_STEPS_OUTSIDE_FIELD + 1;
      this->state = LEAVEMAGNET;
#ifdef DEBUG
      Serial.println("Calibration >> Magnet in unter 20 Schritten gefunden. Drehe rückwärts..");
#endif
    }
    else if (in_field && this->steps > MIN_STEPS_OUTSIDE_FIELD)
    {
      this->steps = 0;
      this->state = INFIELD;
#ifdef DEBUG
      Serial.println("Calibration >> Magnet in über 20 Schritten gefunden. Durchlaufe das Feld.");
#endif
    }
    else
    {
      this->motor.stepForward();
      this->steps++;
    }
    break;

  case LEAVEMAGNET:
    if (this->steps == 0)
    {
      this->state = FINDMAGNET;
#ifdef DEBUG
      Serial.println("Calibration >> Aus dem Feld gedreht.. Suche Magnet erneut");
#endif
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
#ifdef DEBUG
      Serial.println("Calibration >> Magnetfeld durchlaufen.. Zentriere den Zeiger");
#endif
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

#ifdef DEBUG
      Serial.println("Calibration >> Zeiger zentriert. Kalibrierung abgeschlossen");
#endif
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
  if (!this->recal_infield && this->isInField())
  {
    this->recal_infield = true;
    this->recal_enter_pos = this->motor.getCurrentPosition();
    this->recal_enter_direction = true; // TODO
  }
  else if (this->recal_infield && !this->isInField())
  {
    this->recal_infield = false;
    if (this->recal_enter_direction != true)
    {
      return; // The field was left on the same side. Do not recalibrate
    }

    this->recal_leave_pos = this->motor.getCurrentPosition();

    size_t field_width = diff(this->recal_enter_pos, this->recal_leave_pos, this->recal_enter_direction);

    if (field_width < MIN_WIDTH_FOR_RECALIBRATION)
    {
      return; // The tracked magnet field was smaller then the minimum magnet field width. Do not recalibrate
    }

    size_t target_pos = calculateFieldLeavePosition(field_width, this->recal_enter_direction);
    bool correction_direction = getShortestDirection(this->motor.getCurrentPosition(), target_pos);
    size_t steps_off = diff(this->motor.getCurrentPosition(), target_pos, correction_direction);
    this->motor.recalibrate(target_pos, steps_off, correction_direction);
  }
}