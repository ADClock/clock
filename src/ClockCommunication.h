#ifndef _CLOCK_COMMUNICATION_H_
#define _CLOCK_COMMUNICATION_H_
#include "Instruction.h"

class ClockCommunication
{

public:
  ClockCommunication(OwnInstruction &instruction); // The pins are set directly by Config.h in order to use the maximum speed advantage

  void tick();

  void processDataInput();

private:
  void updateOwnInstruction();
  void passOnInstruction();

  OwnInstruction &own;
  bool pass_on_instructions;
  unsigned long last_instruction_read_micros;
  unsigned long last_instruction_send_micros;
};

#endif