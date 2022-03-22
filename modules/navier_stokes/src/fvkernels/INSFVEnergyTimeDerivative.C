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
  params.addRequiredParam<MooseFunctorName>(NS::cp, "Specific heat capacity");
  params.addRequiredParam<MooseFunctorName>(NS::time_deriv(NS::cp),
                                            "Specific heat capacity time derivative functor");
  return params;
}

INSFVEnergyTimeDerivative::INSFVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params),
    _rho(getFunctor<ADReal>(getParam<MooseFunctorName>(NS::density))),
    _cp(getFunctor<ADReal>(getParam<MooseFunctorName>(NS::cp))),
    _cp_dot(getFunctor<ADReal>(getParam<MooseFunctorName>(NS::time_deriv(NS::cp))))
{
}

ADReal
INSFVEnergyTimeDerivative::computeQpResidual()
{
  const auto & elem_arg = makeElemArg(_current_elem);
  return _rho(elem_arg) * _cp(elem_arg) * FVTimeKernel::computeQpResidual() +
         _rho(elem_arg) * _cp_dot(elem_arg) * _var(elem_arg);
}
