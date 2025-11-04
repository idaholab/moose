//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVEnergyTimeDerivative.h"
#include "INSFVEnergyVariable.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVEnergyTimeDerivative);

InputParameters
INSFVEnergyTimeDerivative::validParams()
{
  InputParameters params = FVFunctorTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes energy equation.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addParam<MooseFunctorName>(NS::time_deriv(NS::specific_enthalpy),
                                    NS::time_deriv(NS::specific_enthalpy),
                                    "The time derivative of the specific enthalpy");
  return params;
}

INSFVEnergyTimeDerivative::INSFVEnergyTimeDerivative(const InputParameters & params)
  : FVFunctorTimeKernel(params),
    _rho(getFunctor<ADReal>(NS::density)),
    _h_dot(getFunctor<ADReal>(NS::time_deriv(NS::specific_enthalpy)))
{
  if (!dynamic_cast<INSFVEnergyVariable *>(&_var))
    paramError("variable", "The supplied variable should be of INSFVEnergyVariable type.");
}

ADReal
INSFVEnergyTimeDerivative::computeQpResidual()
{
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  return _rho(elem_arg, state) * _h_dot(elem_arg, state);
}
