//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CosineHumpFunctionTest.h"
#include "THMTestUtils.h"
#include "Function.h"

TEST_F(CosineHumpFunctionTest, testPositiveHump)
{
  const Function & fn = _fe_problem->getFunction(_fn_name_positive);

  const Real x = 3.0;
  const Real z = 6.0;
  const Real abs_tol = 1e-13;

  // center
  {
    const Point p(x, _hump_center_position, z);
    ABS_TEST(fn.value(0, p), _hump_center_value_positive, abs_tol);
  }

  // left
  {
    const Point p(x, _hump_center_position - 0.7 * _hump_width, z);
    ABS_TEST(fn.value(0, p), _hump_begin_value, abs_tol);
  }

  // right
  {
    const Point p(x, _hump_center_position + 0.7 * _hump_width, z);
    ABS_TEST(fn.value(0, p), _hump_begin_value, abs_tol);
  }

  // halfway between left end of hump and center of hump
  {
    const Point p(x, _hump_center_position - 0.25 * _hump_width, z);
    ABS_TEST(fn.value(0, p), 0.5 * (_hump_begin_value + _hump_center_value_positive), abs_tol);
  }
}

TEST_F(CosineHumpFunctionTest, testNegativeHump)
{
  const Function & fn = _fe_problem->getFunction(_fn_name_negative);

  const Real x = 3.0;
  const Real z = 6.0;
  const Real abs_tol = 1e-13;

  // center
  {
    const Point p(x, _hump_center_position, z);
    ABS_TEST(fn.value(0, p), _hump_center_value_negative, abs_tol);
  }

  // left
  {
    const Point p(x, _hump_center_position - 0.7 * _hump_width, z);
    ABS_TEST(fn.value(0, p), _hump_begin_value, abs_tol);
  }

  // right
  {
    const Point p(x, _hump_center_position + 0.7 * _hump_width, z);
    ABS_TEST(fn.value(0, p), _hump_begin_value, abs_tol);
  }

  // halfway between left end of hump and center of hump
  {
    const Point p(x, _hump_center_position - 0.25 * _hump_width, z);
    ABS_TEST(fn.value(0, p), 0.5 * (_hump_begin_value + _hump_center_value_negative), abs_tol);
  }
}
