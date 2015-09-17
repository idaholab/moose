/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "TestControl.h"

template<>
InputParameters validParams<TestControl>()
{
  InputParameters params = validParams<Control>();

  MooseEnum test_type("real variable point");
  params.addRequiredParam<MooseEnum>("test_type", test_type, "Indicates the type of test to perform");
  params.addRequiredParam<std::string>("parameter", "The input parameter(s) to control. Specify a single parameter name and all parameters in all objects matching the name will be updated");

  return params;
}

TestControl::TestControl(const InputParameters & parameters) :
    Control(parameters),
    _test_type(getParam<MooseEnum>("test_type"))
{
  if (_test_type == "real")
    getControlParam<Real>("parameter");

  else if (_test_type == "variable")
    getControlParam<NonlinearVariableName>("parameter");

  else if (_test_type != "point")
    mooseError("Unknown test type.");
}

void
TestControl::execute()
{
  Point new_pt(0.25, 0.25);
  std::vector<Point *> pts = getControlParamVector<Point>("parameter");
  for (unsigned int i = 0; i < pts.size(); ++i)
    *pts[i] = new_pt;
}
