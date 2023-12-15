//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TurbulentConductivityAux.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", TurbulentConductivityAux);

InputParameters
TurbulentConductivityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Calculates the turbulent heat conductivity.");
  params.addRequiredParam<MooseFunctorName>(NS::cp, "Specific Heat Capacity functor");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "The turbulent viscosity.");
  params.addParam<MooseFunctorName>("Pr_t", 0.9, "Turbulent Prandtl number functor.");
  return params;
}

TurbulentConductivityAux::TurbulentConductivityAux(const InputParameters & params)
  : AuxKernel(params),
    _cp(getFunctor<ADReal>(NS::cp)),
    _Pr_t(getFunctor<ADReal>("Pr_t")),
    _mu_t(getFunctor<ADReal>(NS::mu_t))
{
}

Real
TurbulentConductivityAux::computeValue()
{
  const auto current_argument = makeElemArg(_current_elem);
  const auto state = determineState();

  return raw_value(_cp(current_argument, state) * _mu_t(current_argument, state) /
                   _Pr_t(current_argument, state));
}
