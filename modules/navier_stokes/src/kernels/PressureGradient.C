//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PressureGradient.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PressureGradient);

InputParameters
PressureGradient::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar(NS::pressure, "pressure");
  params.addRequiredParam<unsigned int>("component", "number of component (0 = x, 1 = y, 2 = z)");
  params.addClassDescription(
      "Implements the pressure gradient term for one of the Navier Stokes momentum equations.");

  return params;
}

PressureGradient::PressureGradient(const InputParameters & parameters)
  : Kernel(parameters),
    _component(getParam<unsigned int>("component")),
    _pressure(coupledValue(NS::pressure))
{
}

Real
PressureGradient::computeQpResidual()
{
  return -_pressure[_qp] * _grad_test[_i][_qp](_component);
}

Real
PressureGradient::computeQpJacobian()
{
  return 0.;
}
