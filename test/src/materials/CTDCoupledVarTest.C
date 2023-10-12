//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CTDCoupledVarTest.h"

registerMooseObject("MooseTestApp", CTDCoupledVarTest);

InputParameters
CTDCoupledVarTest::validParams()
{
  InputParameters params = CompileTimeDerivativesMaterial<3, false, 3>::validParams();
  params.addRequiredCoupledVar("x", "First variable");
  params.addRequiredCoupledVar("y", "Second variable");
  params.addRequiredCoupledVar("z", "Third variable");
  return params;
}

CTDCoupledVarTest::CTDCoupledVarTest(const InputParameters & parameters)
  : CompileTimeDerivativesMaterial<3, false, 3>(parameters, {"x", "y", "z"})
{
}

void
CTDCoupledVarTest::computeQpProperties()
{
  using namespace CompileTimeDerivatives;

  const auto & [a, b, c] = _refs;
  const auto F = pow<3>(a) * pow<4>(b) * pow<5>(c) + sin(a) * cos(b) / (c + 0.1) +
                 log(a + 0.1) * sin(b) * cos(c);

  evaluate(F);
}
