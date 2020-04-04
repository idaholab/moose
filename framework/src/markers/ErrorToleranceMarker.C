//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ErrorToleranceMarker.h"

#include "libmesh/error_vector.h"

registerMooseObject("MooseApp", ErrorToleranceMarker);

InputParameters
ErrorToleranceMarker::validParams()
{
  InputParameters params = IndicatorMarker::validParams();
  params.addParam<Real>("coarsen", 0, "Elements with error less than this will be coarsened.");
  params.addParam<Real>("refine",
                        std::numeric_limits<Real>::max(),
                        "Elements with error more than this will be refined.");
  params.addClassDescription("Coarsen or refine elements based on an absolute tolerance allowed "
                             "from the supplied indicator.");
  return params;
}

ErrorToleranceMarker::ErrorToleranceMarker(const InputParameters & parameters)
  : IndicatorMarker(parameters),
    _coarsen(parameters.get<Real>("coarsen")),
    _refine(parameters.get<Real>("refine"))
{
}

Marker::MarkerValue
ErrorToleranceMarker::computeElementMarker()
{
  Real error = _error_vector[_current_elem->id()];

  if (error > _refine)
    return REFINE;
  else if (error < _coarsen)
    return COARSEN;

  return DO_NOTHING;
}
