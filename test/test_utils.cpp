#include <unity.h>
#include "Utils.h"

void test_forward_diff()
{
  TEST_ASSERT_EQUAL(10, diff(20, 30, true));
  TEST_ASSERT_EQUAL(10, diff(1650, 1660, true));
  TEST_ASSERT_EQUAL(10, diff(1700, 5, true));
}

void test_backward_diff()
{
  TEST_ASSERT_EQUAL(10, diff(30, 20, false));
  TEST_ASSERT_EQUAL(10, diff(1660, 1650, false));
  TEST_ASSERT_EQUAL(10, diff(5, 1700, false));
}

void test_target_pos()
{
  TEST_ASSERT_EQUAL(100, calculateFieldLeavePosition(200, true));
  TEST_ASSERT_EQUAL(1605, calculateFieldLeavePosition(200, false));
  TEST_ASSERT_EQUAL(100, calculateFieldLeavePosition(201, true));
  TEST_ASSERT_EQUAL(1605, calculateFieldLeavePosition(201, false));
}

void test_shortest_direction()
{
  TEST_ASSERT_EQUAL(true, getShortestDirection(0, 10));
  TEST_ASSERT_EQUAL(true, getShortestDirection(1700, 10));
  TEST_ASSERT_EQUAL(false, getShortestDirection(1600, 1500));
  TEST_ASSERT_EQUAL(false, getShortestDirection(10, 1700));
}

void setUp(void)
{
  // set stuff up here
}

void tearDown(void)
{
  // clean stuff up here
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();

  RUN_TEST(test_forward_diff);
  RUN_TEST(test_backward_diff);
  RUN_TEST(test_target_pos);
  RUN_TEST(test_shortest_direction);

  UNITY_END();
}