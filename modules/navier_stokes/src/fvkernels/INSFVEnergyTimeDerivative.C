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
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addParam<MooseFunctorName>(NS::time_deriv(NS::density), "Density time derivative functor");
  params.addRequiredParam<MooseFunctorName>(NS::cp, "Specific heat capacity");
  params.addParam<MooseFunctorName>(NS::time_deriv(NS::cp),
                                    "Specific heat capacity time derivative functor");
  return params;
}

INSFVEnergyTimeDerivative::INSFVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params),
    _rho(getFunctor<ADReal>(NS::density)),
    _rho_dot(isParamValid(NS::time_deriv(NS::density))
                 ? &getFunctor<ADReal>(NS::time_deriv(NS::density))
                 : nullptr),
    _cp(getFunctor<ADReal>(NS::cp)),
    _cp_dot(isParamValid(NS::time_deriv(NS::cp)) ? &getFunctor<ADReal>(NS::time_deriv(NS::cp))
                                                 : nullptr)
{
}

ADReal
INSFVEnergyTimeDerivative::computeQpResidual()
{
  auto time_derivative = _rho(makeElemArg(_current_elem)) * _cp(makeElemArg(_current_elem)) *
                         FVTimeKernel::computeQpResidual();
  if (_rho_dot)
    time_derivative += (*_rho_dot)(makeElemArg(_current_elem)) * _cp(makeElemArg(_current_elem)) *
                       _var(makeElemArg(_current_elem));
  if (_cp_dot)
    time_derivative += _rho(makeElemArg(_current_elem)) * (*_cp_dot)(makeElemArg(_current_elem)) *
                       _var(makeElemArg(_current_elem));
  return time_derivative;
}
