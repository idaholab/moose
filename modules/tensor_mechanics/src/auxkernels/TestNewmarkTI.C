//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestNewmarkTI.h"

registerMooseObject("TensorMechanicsApp", TestNewmarkTI);

InputParameters
TestNewmarkTI::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Assigns the velocity/acceleration calculated by time integrator to "
                             "the velocity/acceleration auxvariable.");
  params.addRequiredCoupledVar(
      "displacement",
      "The variable whose first/second derivative needs to be copied to the provided auxvariable.");
  params.addParam<bool>("first",
                        true,
                        "Set to true to copy the first derivative to the auxvariable. If false, "
                        "the second derivative is copied.");
  return params;
}

TestNewmarkTI::TestNewmarkTI(const InputParameters & parameters)
  : AuxKernel(parameters),
    _first(getParam<bool>("first")),
    _value(_first ? coupledDot("displacement") : coupledDotDot("displacement"))
{
}

Real
TestNewmarkTI::computeValue()
{
  if (!isNodal())
    mooseError("must run on a nodal variable");

  // assigns the first/second time derivative of displacement to the provided auxvariable
  return _value[_qp];
}
