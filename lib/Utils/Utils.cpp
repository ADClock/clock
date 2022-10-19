#include "Utils.h"

#define MAX_STEPS 1705 // TODO Import from Config.h i guess

/**
 * @brief Calculates the step difference between two points using the direction.
 *
 * If start and finish are equal, 0 is returned.
 *
 * ```cpp
 * diff(20, 30, true);       // returns:   10
 * diff(1650, 1660, false);  // returns: 1695
 * ```
 *
 * @param start Start Point for calculation.
 * @param finish Finish Point for calculation.
 * @param direction Count the steps in forward or backward direction. True means forward.
 * @return size_t Steps between both points. Number between [0, MAX_STEPS)
 */
size_t diff(size_t start, size_t finish, bool direction)
{
  if (direction) // Rotating forwards
  {

    if (finish >= start)
      return finish - start;
    else
      return MAX_STEPS - start + finish;
  }
  else // Rotating backwards
  {
    if (start >= finish)
      return start - finish;
    else
      return MAX_STEPS - finish + start;
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
  return (direction ? fieldWidth / 2 : MAX_STEPS - fieldWidth / 2);
}

/**
 * @brief Get the shortest direction between to positions.
 *
 * @param from Position to start from.
 * @param to Target position to reach.
 * @return true Rotating forwards is the shortest direction.
 * @return false Rotating backwards is the shortest direction.
 */
bool getShortestDirection(size_t from, size_t to)
{
  return (diff(from, to, true) < diff(from, to, false));
}