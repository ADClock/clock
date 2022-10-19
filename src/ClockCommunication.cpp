#include "ClockCommunication.h"
#include <FastGPIO.h>
#include "Config.h"
#include <Arduino.h>

ClockCommunication::ClockCommunication(OwnInstruction &own) : own(own)
{
  pinMode(COMM_OUT_CLOCK, OUTPUT);
  pinMode(COMM_OUT_DATA1, OUTPUT);
  pinMode(COMM_OUT_DATA2, OUTPUT);
  pinMode(COMM_OUT_DATA3, OUTPUT);
  pinMode(COMM_OUT_DATA4, OUTPUT);
}

/**
 * @brief Handles the receive and send process of instructions.
 *
 * Task 1:
 * It might be necessary to turn the output clock low again, after instruction was passed on to next clock by ISR processDataInput().
 *
 * Task 2:
 * Reset the pass_on_instructions variable after no new instruction for a long time, so the next instruction is interpreted as own instruction.
 *
 * All timings are configured inside the Config.h file.
 */
void ClockCommunication::tick()
{
  if (this->pass_on_instructions)
  {
    if (FastGPIO::Pin<COMM_OUT_CLOCK>::isOutputValueHigh())
    {
      unsigned long micros_since_clock_high = micros() - this->last_instruction_send_micros; // always positive. Overflow save
      if (micros_since_clock_high > MIN_OUT_CLOCK_HIGH)
      {
        FastGPIO::Pin<COMM_OUT_CLOCK>::setOutputLow();
      }
    }
    else
    {
      unsigned long micros_since_last_input = micros() - this->last_instruction_read_micros; // always positive. Overflow save
      if (micros_since_last_input > DELAY_BETWEEN_INSTRUCTIONS)
      {
        this->pass_on_instructions = false;
      }
    }
  }
}

/**
 * @brief Reads the instruction from the data pins and stores it as own instruction or passes it on to the next clock.
 *
 */
void ClockCommunication::processDataInput()
{
  if (this->pass_on_instructions)
  {
    this->passOnInstruction();
  }
  else
  {
    this->updateOwnInstruction();
  }

  this->last_instruction_read_micros = micros();
}

void ClockCommunication::updateOwnInstruction()
{
  this->own.data.hourBackward = FastGPIO::Pin<COMM_IN_DATA1>::isInputHigh();
  this->own.data.hourForward = FastGPIO::Pin<COMM_IN_DATA2>::isInputHigh();
  this->own.data.minuteBackward = FastGPIO::Pin<COMM_IN_DATA3>::isInputHigh();
  this->own.data.minuteForward = FastGPIO::Pin<COMM_IN_DATA4>::isInputHigh();
  this->own.pending = true;
  this->pass_on_instructions = true; // Next instructions should be pass on to next clock
}

void ClockCommunication::passOnInstruction()
{
  // Maybe we need a buffer here
  FastGPIO::Pin<COMM_OUT_DATA1>::setOutputValue(FastGPIO::Pin<COMM_IN_DATA1>::isInputHigh());
  FastGPIO::Pin<COMM_OUT_DATA2>::setOutputValue(FastGPIO::Pin<COMM_IN_DATA2>::isInputHigh());
  FastGPIO::Pin<COMM_OUT_DATA3>::setOutputValue(FastGPIO::Pin<COMM_IN_DATA3>::isInputHigh());
  FastGPIO::Pin<COMM_OUT_DATA4>::setOutputValue(FastGPIO::Pin<COMM_IN_DATA4>::isInputHigh());
  FastGPIO::Pin<COMM_OUT_CLOCK>::setOutputHigh();
  this->last_instruction_send_micros = micros();
}