#include "Utils.h"
#include "Config.h"

/**
 * @brief Calculates the step difference between two points using the direction.
 *
 * If from and to are equal, 0 is returned.
 *
 * ```cpp
 * diff(20, 30, true);       // returns:   10
 * diff(1650, 1660, false);  // returns: 1695
 * ```
 *
 * @param from Start Point for calculation.
 * @param to Finish Point for calculation.
 * @param direction Count the steps in forward (true) or backward (false) direction.
 * @return size_t Steps between both points. Number between [0, MAX_STEPS)
 */
size_t diff(size_t from, size_t to, bool direction)
{
  if (direction) // Rotating forwards
  {
    if (to >= from)
      return to - from;
    else
      return MAX_STEPS - from + to;
  }
  else // Rotating backwards
  {
    if (from >= to)
      return from - to;
    else
      return MAX_STEPS - to + from;
  }
}

/**
 * @brief Calculates the position where the motor should leave the magnet field for a given field width and direction.
 *
 * @param fieldWidth The width of the detected magnet field.
 * @param direction  The direction where the motor left the magnet field. True means forward.
 * @return size_t Position the motor should be at when leaving the magnet field.
 */
size_t calculateFieldLeavePosition(size_t fieldWidth, bool direction)
{
  return (direction ? fieldWidth / 2 : MAX_STEPS - (fieldWidth / 2));
}

/**
 * @brief Get the shortest direction between to positions.
 *
 * @param from Stating from position.
 * @param to Target position to reach.
 * @return true Rotating forwards is the shortest direction.
 * @return false Rotating backwards is the shortest direction.
 */
bool getShortestDirection(size_t from, size_t to)
{
  return (diff(from, to, true) < diff(from, to, false));
}