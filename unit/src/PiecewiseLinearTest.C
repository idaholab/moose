//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseLinearTest.h"
#include "FunctionTestUtils.h"

InputParameters
PiecewiseLinearTest::getDefaultParams()
{
  InputParameters params = _factory.getValidParams("PiecewiseLinear");
  params.set<Real>("scale_factor") = 0.4;
  params.set<std::vector<Real>>("x") = {-1.0, 1.0, 4.0, 6.0, 9.0};
  params.set<std::vector<Real>>("y") = {2.0, 0.0, -3.0, 4.0, 2.0};
  return params;
}

const Function &
PiecewiseLinearTest::buildFunction(InputParameters & params, const std::string & obj_name)
{
  _fe_problem->addFunction("PiecewiseLinear", obj_name, params);
  return _fe_problem->getFunction(obj_name);
}

TEST_F(PiecewiseLinearTest, testTimeIntegral)
{
  InputParameters params_t_axis = getDefaultParams();
  const auto & f_t_axis = buildFunction(params_t_axis, "taxis");

  InputParameters params_t_axis_extrap = getDefaultParams();
  params_t_axis_extrap.set<bool>("extrap") = true;
  const auto & f_t_axis_extrap = buildFunction(params_t_axis, "taxis_extrap");

  InputParameters params_x_axis = getDefaultParams();
  params_x_axis.set<MooseEnum>("axis") = "x";
  const auto & f_x_axis = buildFunction(params_x_axis, "xaxis");

  const Real t1 = 2.0;
  const Real t2 = 5.0;
  const Point p = Point(0.5, 2.0, 3.0);
  const Real n_int = 1000;
  const Real rel_tol = 1e-6;

  FunctionTestUtils::testTimeIntegral(f_t_axis, t1, t2, p, n_int, rel_tol);
  FunctionTestUtils::testTimeIntegral(f_t_axis, t1, 2.5, p, n_int, rel_tol);
  FunctionTestUtils::testTimeIntegral(f_t_axis, t2, t1, p, n_int, rel_tol);
  FunctionTestUtils::testTimeIntegral(f_t_axis, -5.0, 11.0, p, n_int, rel_tol);
  FunctionTestUtils::testTimeIntegral(f_t_axis_extrap, -5.0, 11.0, p, n_int, rel_tol);
  FunctionTestUtils::testTimeIntegral(f_x_axis, t1, t2, p, n_int, rel_tol);
}
