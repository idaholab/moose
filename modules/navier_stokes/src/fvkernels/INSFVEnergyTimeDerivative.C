//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVEnergyTimeDerivative.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVEnergyTimeDerivative);

InputParameters
INSFVEnergyTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes energy equation.");
  params.addRequiredParam<MaterialPropertyName>(NS::density, "Density");
  params.addRequiredParam<MaterialPropertyName>(NS::cp, "Specific heat capacity");
  params.addParam<MaterialPropertyName>(NS::time_deriv(NS::cp),
                                        "Specific heat capacity time derivative functor");
  return params;
}

INSFVEnergyTimeDerivative::INSFVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params),
    _rho(getFunctor<ADReal>(getParam<MaterialPropertyName>(NS::density))),
    _cp(getFunctor<ADReal>(getParam<MaterialPropertyName>(NS::cp))),
    _cp_dot(isParamValid(NS::time_deriv(NS::cp))
                ? &getFunctor<ADReal>(getParam<MaterialPropertyName>(NS::time_deriv(NS::cp)))
                : nullptr)
{
}

ADReal
INSFVEnergyTimeDerivative::computeQpResidual()
{
  const auto & elem_arg = makeElemArg(_current_elem);
  auto time_derivative = _rho(elem_arg) * _cp(elem_arg) * FVTimeKernel::computeQpResidual();

  if (_cp_dot)
    time_derivative += _rho(elem_arg) * (*_cp_dot)(elem_arg) * _var(elem_arg);

  return time_derivative;
}
