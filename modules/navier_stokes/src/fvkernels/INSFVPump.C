//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVPump.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", INSFVPump);

InputParameters
INSFVPump::validParams()
{
  auto params = FVElementalKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription(
      "Effective body force for a pump that contributes to the Rhie-Chow interpolation");
  params.addParam<MooseFunctorName>(
      "pump_volume_force", "pump_volume_force", "Functor for the pump force.");
  return params;
}

INSFVPump::INSFVPump(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    INSFVMomentumResidualObject(*this),
    _pump_volume_force(getFunctor<Real>("pump_volume_force"))
{
}

ADReal
INSFVPump::computeQpResidual()
{
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();

  return _pump_volume_force(elem_arg, state);
}
