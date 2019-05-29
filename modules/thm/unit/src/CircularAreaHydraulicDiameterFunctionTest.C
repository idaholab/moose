#include "CircularAreaHydraulicDiameterFunctionTest.h"
#include "FluidPropertiesTestUtils.h"

TEST_F(CircularAreaHydraulicDiameterFunctionTest, test)
{
  const Function & fn = _fe_problem->getFunction(_Dh_name);

  const Point p(0.5, 0, 0);
  ABS_TEST(fn.value(0, p), 1.3819765978853, 1e-13);
}
