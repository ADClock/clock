#include "ClockCommunication.h"
#include <FastGPIO.h>
#include "Config.h"
#include <Arduino.h>

ClockCommunication::ClockCommunication(OwnInstruction &own) : own(own)
{
  pinMode(COMM_OUT_DATA1, OUTPUT);
  pinMode(COMM_OUT_DATA2, OUTPUT);
  pinMode(COMM_OUT_DATA3, OUTPUT);
  pinMode(COMM_OUT_DATA4, OUTPUT);
  pinMode(COMM_OUT_CLOCK, OUTPUT);
  FastGPIO::Pin<COMM_OUT_CLOCK>::setOutputLow();

  pinMode(COMM_IN_DATA1, INPUT);
  pinMode(COMM_IN_DATA2, INPUT);
  pinMode(COMM_IN_DATA3, INPUT);
  pinMode(COMM_IN_DATA4, INPUT);
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
  if (!this->pass_on_instructions)
  {
    // if no messages are forwarded, there is nothing to do for this procedure
    return;
  }

  // unsigned long micros_since_last_input = micros() - this->last_instruction_read_micros; // always positive. Overflow save

  unsigned long current_micros = micros();
  unsigned long current_last_instruction_read_micros = this->last_instruction_read_micros;
  if (current_micros < current_last_instruction_read_micros)
  {
    // Overflow or isr has interrupted and updated last_instruction_read_micros
    current_last_instruction_read_micros = current_micros;
  }
  else if (current_micros - current_last_instruction_read_micros > DELAY_BETWEEN_INSTRUCTIONS)
  {
    // No new instruction for a long time
    this->pass_on_instructions = false;
  }
}

/**
 * @brief Reads the instruction from the data pins and stores it as own instruction or passes it on to the next clock.
 *
 * This function takes approximately 9 microseconds to execute.
 * ~ 2 us for reading the data pins
 * ~ 3 us for reading micros()
 * ~ 4 us (CLOCK_OUT_HIGH) if sending the instruction to the next clock
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
    this->pass_on_instructions = true; // Next instructions should be pass on to next clock
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
}

void ClockCommunication::passOnInstruction()
{
  FastGPIO::Pin<COMM_OUT_DATA1>::setOutputValue(FastGPIO::Pin<COMM_IN_DATA1>::isInputHigh());
  FastGPIO::Pin<COMM_OUT_DATA2>::setOutputValue(FastGPIO::Pin<COMM_IN_DATA2>::isInputHigh());
  FastGPIO::Pin<COMM_OUT_DATA3>::setOutputValue(FastGPIO::Pin<COMM_IN_DATA3>::isInputHigh());
  FastGPIO::Pin<COMM_OUT_DATA4>::setOutputValue(FastGPIO::Pin<COMM_IN_DATA4>::isInputHigh());
  FastGPIO::Pin<COMM_OUT_CLOCK>::setOutputHigh();
  delayMicroseconds(CLOCK_OUT_HIGH);
  FastGPIO::Pin<COMM_OUT_CLOCK>::setOutputLow();
}

void ClockCommunication::sendTestInstruction(Instruction &instruction)
{
  this->pass_on_instructions = true;
  FastGPIO::Pin<COMM_OUT_DATA1>::setOutputValue(instruction.hourBackward);
  FastGPIO::Pin<COMM_OUT_DATA2>::setOutputValue(instruction.hourForward);
  FastGPIO::Pin<COMM_OUT_DATA3>::setOutputValue(instruction.minuteBackward);
  FastGPIO::Pin<COMM_OUT_DATA4>::setOutputValue(instruction.minuteForward);
  FastGPIO::Pin<COMM_OUT_CLOCK>::setOutputHigh();
  this->clock_out_high = true;
  this->last_instruction_send_micros = micros();
}