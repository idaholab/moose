//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVEnergyTimeDerivative.h"
#include "INSFVEnergyVariable.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVEnergyTimeDerivative);

InputParameters
PINSFVEnergyTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the Navier-Stokes energy equation: "
      "for fluids: d(eps * rho * cp * T)/dt, for solids: (1 - eps) * d(rho * cp * T)/dt"
      "Material property derivatives are ignored if not provided.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addParam<MooseFunctorName>(NS::time_deriv(NS::density), "Density time derivative functor");
  params.addRequiredParam<MooseFunctorName>(NS::cp, "Specific heat capacity");
  params.addParam<MooseFunctorName>(NS::time_deriv(NS::cp),
                                    "Specific heat capacity time derivative functor");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity");

  params.addRequiredParam<bool>("is_solid", "Whether this kernel acts on the solid temperature");
  params.addRangeCheckedParam<Real>("scaling",
                                    1,
                                    "scaling >= 0.0",
                                    "scaling factor to reduce the thermal mass during pseudo "
                                    "transients; this can accelerate convergence to steady state");
  return params;
}

PINSFVEnergyTimeDerivative::PINSFVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params),
    _rho(getFunctor<ADReal>(NS::density)),
    _rho_dot(isParamValid(NS::time_deriv(NS::density))
                 ? &getFunctor<ADReal>(NS::time_deriv(NS::density))
                 : nullptr),
    _cp(getFunctor<ADReal>(NS::cp)),
    _cp_dot(isParamValid(NS::time_deriv(NS::cp)) ? &getFunctor<ADReal>(NS::time_deriv(NS::cp))
                                                 : nullptr),
    _eps(getFunctor<ADReal>(NS::porosity)),
    _is_solid(getParam<bool>("is_solid")),
    _scaling(getParam<Real>("scaling")),
    _zero_scaling(_scaling < 1e-8)
{
}

ADReal
PINSFVEnergyTimeDerivative::computeQpResidual()
{
  if (_zero_scaling)
    return 0.0;
  else
  {
    auto time_derivative = _rho(makeElemArg(_current_elem)) * _cp(makeElemArg(_current_elem)) *
                           FVTimeKernel::computeQpResidual();
    if (_rho_dot)
      time_derivative += (*_rho_dot)(makeElemArg(_current_elem)) * _cp(makeElemArg(_current_elem)) *
                         _var(makeElemArg(_current_elem));
    if (_cp_dot)
      time_derivative += _rho(makeElemArg(_current_elem)) * (*_cp_dot)(makeElemArg(_current_elem)) *
                         _var(makeElemArg(_current_elem));

    return _scaling *
           (_is_solid ? 1 - _eps(makeElemArg(_current_elem)) : _eps(makeElemArg(_current_elem))) *
           time_derivative;
  }
}
