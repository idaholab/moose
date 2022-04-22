//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapFluxModelSimple.h"

registerMooseObject("HeatConductionApp", GapFluxModelSimple);

InputParameters
GapFluxModelSimple::validParams()
{
  InputParameters params = GapFluxModelBase::validParams();
  params.addClassDescription("Gap flux model with a constant conductance");
  params.addRequiredCoupledVar("temperature", "The name of the temperature variable");
  params.addRequiredParam<Real>("k", "Gap conductance");
  params.addParam<Real>("min_gap",
                        1e-6,
                        "The minimum gap distance allowed. This helps with preventing the heat "
                        "flux from going to infinity as the gap approaches zero.");
  return params;
}

GapFluxModelSimple::GapFluxModelSimple(const InputParameters & parameters)
  : GapFluxModelBase(parameters),
    _k(getParam<Real>("k")),
    _min_gap(getParam<Real>("min_gap")),
    _primary_T(adCoupledNeighborValue("temperature")),
    _secondary_T(adCoupledValue("temperature"))
{
}

ADReal
GapFluxModelSimple::computeFlux() const
{
  const auto l = std::max(_gap_width, _min_gap);

  return _k * (_primary_T[_qp] - _secondary_T[_qp]) / l;
}
