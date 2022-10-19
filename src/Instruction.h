#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_

struct Instruction
{
  bool hourBackward;
  bool hourForward;
  bool minuteBackward;
  bool minuteForward;
};

struct OwnInstruction
{
  bool pending;
  Instruction data;
};

#endif