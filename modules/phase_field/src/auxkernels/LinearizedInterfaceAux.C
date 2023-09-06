//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearizedInterfaceAux.h"

registerMooseObject("PhaseFieldApp", LinearizedInterfaceAux);

InputParameters
LinearizedInterfaceAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates the order parameter from the linearized interface function");
  params.addRequiredCoupledVar("nonlinear_variable",
                               "The variable used in the linearized interface function");
  return params;
}

LinearizedInterfaceAux::LinearizedInterfaceAux(const InputParameters & parameters)
  : AuxKernel(parameters), _phi(coupledValue("nonlinear_variable"))
{
}

Real
LinearizedInterfaceAux::computeValue()
{
  return (1.0 + std::tanh(_phi[_qp] / std::sqrt(2.0))) / 2.0;
}
