#ifndef _UTILS_H_
#define _UTILS_H_

#include <Arduino.h>

size_t diff(size_t start, size_t finish, bool direction);

size_t calculateFieldLeavePosition(size_t fieldWidth, bool direction);

bool getShortestDirection(size_t from, size_t to);

#endif