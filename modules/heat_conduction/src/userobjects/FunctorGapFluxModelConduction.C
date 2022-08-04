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
#include "Function.h"

registerMooseObject("HeatConductionApp", FunctorGapFluxModelConduction);

InputParameters
FunctorGapFluxModelConduction::validParams()
{
  InputParameters params = GapFluxModelBase::validParams();
  params.addClassDescription("Gap flux model for varying gap conductance");
  params.addRequiredParam<MooseFunctorName>("temperature", "The name of the temperature functor");
  params.addParam<Real>("gap_conductivity", 1.0, "Gap conductivity value");
  params.addParam<MooseFunctorName>(
      "gap_conductivity_functor",
      1,
      "Thermal conductivity of the gap material as an ADReal functor.  Multiplied "
      "by the constant gap_conductivity.");
  params.addRangeCheckedParam<Real>(
      "min_gap", 1e-6, "min_gap>0", "A minimum gap (denominator) size");
  params.addRangeCheckedParam<unsigned int>(
      "min_gap_order",
      0,
      "min_gap_order<=1",
      "Order of the Taylor expansion below min_gap for FunctorGapFluxModelConduction");

  return params;
}

FunctorGapFluxModelConduction::FunctorGapFluxModelConduction(const InputParameters & parameters)
  : GapFluxModelBase(parameters),
    _T(getFunctor<ADReal>("temperature")),
    _gap_conductivity(getParam<Real>("gap_conductivity")),
    _gap_conductivity_functor(getFunctor<ADReal>("gap_conductivity_functor")),
    _min_gap(getParam<Real>("min_gap")),
    _min_gap_order(getParam<unsigned int>("min_gap_order"))

{
}

ADReal
FunctorGapFluxModelConduction::computeFlux() const
{
  ADReal gap_conductivity = _gap_conductivity;

  gap_conductivity *= 0.5 * (_gap_conductivity_functor(_secondary_point) +
                             _gap_conductivity_functor(_primary_point));

  return (_T(_primary_point) - _T(_secondary_point)) * gap_conductivity * gapAttenuation();
}

ADReal
FunctorGapFluxModelConduction::gapAttenuation() const
{

  mooseAssert(_min_gap > 0, "min_gap must be larger than zero.");

  if (_adjusted_length > _min_gap)
  {
    return 1.0 / _adjusted_length;
  }
  else
    switch (_min_gap_order)
    {
      case 0:
        return 1.0 / _min_gap;

      case 1:
        return 1.0 / _min_gap - (_adjusted_length - _min_gap) / (_min_gap * _min_gap);

      default:
        mooseError(
            "Invalid Taylor expansion order for gap attenuation in FunctorGapFluxModelConduction");
    }
}
