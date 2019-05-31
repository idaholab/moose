#include "TimeRampFunctionTest.h"
#include "THMTestUtils.h"
#include "Function.h"

TEST_F(TimeRampFunctionTest, test)
{
  const Function & fn = _fe_problem->getFunction(_fn_name);
  const Point p(0, 0, 0);

  // during ramp
  {
    const Real test_time = 3.0;
    const Real slope = (_final_value - _initial_value) / _ramp_duration;
    const Real elapsed_time = test_time - _initial_time;
    const Real test_value = _initial_value + slope * elapsed_time;

    ABS_TEST(fn.value(test_time, p), test_value, ABS_TOL_ROUNDOFF);
  }

  // after ramp
  {
    const Real test_time = 10.0;
    ABS_TEST(fn.value(test_time, p), _final_value, ABS_TOL_ROUNDOFF);
  }
}
