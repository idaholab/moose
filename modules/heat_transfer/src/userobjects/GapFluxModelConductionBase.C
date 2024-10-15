//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapFluxModelConductionBase.h"
#include "libmesh/utility.h"

InputParameters
GapFluxModelConductionBase::validParams()
{
  InputParameters params = GapFluxModelBase::validParams();
  params.addParam<Real>("gap_conductivity", 1.0, "Gap conductivity value");
  params.addRangeCheckedParam<Real>(
      "min_gap", 1e-6, "min_gap>0", "A minimum gap (denominator) size");
  params.addRangeCheckedParam<unsigned int>(
      "min_gap_order",
      0,
      "min_gap_order<=1",
      "Order of the Taylor expansion below min_gap for GapFluxModelConductionBase");
  params.addParamNamesToGroup("gap_conductivity min_gap min_gap_order", "Gap conductive flux");
  return params;
}

GapFluxModelConductionBase::GapFluxModelConductionBase(const InputParameters & parameters)
  : GapFluxModelBase(parameters),
    _gap_conductivity(getParam<Real>("gap_conductivity")),
    _min_gap(getParam<Real>("min_gap")),
    _min_gap_order(getParam<unsigned int>("min_gap_order"))
{
}

ADReal
GapFluxModelConductionBase::computeConductionFlux(const ADReal & secondary_T,
                                                  const ADReal & primary_T,
                                                  const ADReal & gap_conductivity_multiplier) const
{
  const auto gap_conductivity = _gap_conductivity * gap_conductivity_multiplier;

  return (primary_T - secondary_T) * gap_conductivity * gapAttenuation();
}

ADReal
GapFluxModelConductionBase::gapAttenuation() const
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
            "Invalid Taylor expansion order for gap attenuation in GapFluxModelConductionBase");
    }
}
