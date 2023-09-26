//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorGapFluxModelRadiation.h"

registerMooseObject("HeatConductionApp", FunctorGapFluxModelRadiation);

InputParameters
FunctorGapFluxModelRadiation::validParams()
{
  InputParameters params = GapFluxModelRadiationBase::validParams();
  params.addClassDescription("Gap flux model for heat transfer across a gap due to radiation, "
                             "based on the diffusion approximation. Uses a temperature functor.");
  params.addRequiredParam<MooseFunctorName>("temperature", "The name of the temperature functor");
  return params;
}

FunctorGapFluxModelRadiation::FunctorGapFluxModelRadiation(const InputParameters & parameters)
  : GapFluxModelRadiationBase(parameters), _T(getFunctor<ADReal>("temperature"))
{
}

ADReal
FunctorGapFluxModelRadiation::computeFlux() const
{
  return computeRadiationFlux(_T(_secondary_point, determineState()),
                              _T(_primary_point, determineState()));
}
