//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorGapFluxModelConduction.h"
#include "libmesh/utility.h"

registerMooseObject("HeatConductionApp", FunctorGapFluxModelConduction);

InputParameters
FunctorGapFluxModelConduction::validParams()
{
  InputParameters params = GapFluxModelConductionBase::validParams();
  params.addClassDescription(
      "Gap flux model for varying gap conductance using a functor for temperature.");
  params.addRequiredParam<MooseFunctorName>("temperature", "The name of the temperature functor");
  params.addParam<MooseFunctorName>("gap_conductivity_multiplier",
                                    1,
                                    "Thermal conductivity multiplier. Multiplied by the constant "
                                    "gap_conductivity to form the final conductivity");
  return params;
}

FunctorGapFluxModelConduction::FunctorGapFluxModelConduction(const InputParameters & parameters)
  : GapFluxModelConductionBase(parameters),
    _T(getFunctor<ADReal>("temperature")),
    _gap_conductivity_multiplier(getFunctor<ADReal>("gap_conductivity_multiplier"))
{
}

ADReal
FunctorGapFluxModelConduction::computeFlux() const
{
  const auto state = determineState();
  return computeConductionFlux(_T(_secondary_point, state),
                               _T(_primary_point, state),
                               0.5 * (_gap_conductivity_multiplier(_secondary_point, state) +
                                      _gap_conductivity_multiplier(_primary_point, state)));
}
