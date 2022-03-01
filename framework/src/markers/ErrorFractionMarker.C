//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ErrorFractionMarker.h"

#include "libmesh/error_vector.h"

registerMooseObject("MooseApp", ErrorFractionMarker);

InputParameters
ErrorFractionMarker::validParams()
{
  InputParameters params = IndicatorMarker::validParams();
  params.addRangeCheckedParam<Real>("coarsen",
                                    0,
                                    "coarsen>=0 & coarsen<=1",
                                    "Elements within this percentage of the min error "
                                    "will be coarsened.  Must be between 0 and 1!");
  params.addRangeCheckedParam<Real>("refine",
                                    0,
                                    "refine>=0 & refine<=1",
                                    "Elements within this percentage of the max error will "
                                    "be refined.  Must be between 0 and 1!");

  params.addParam<bool>("clear_extremes",
                        true,
                        "Whether or not to clear the extremes during each error calculation. "
                        " Changing this to `false` will result in the global extremes ever "
                        "encountered during the run to be used as the min and max error.");

  params.addClassDescription("Marks elements for refinement or coarsening based on the fraction of "
                             "the min/max error from the supplied indicator.");
  return params;
}

ErrorFractionMarker::ErrorFractionMarker(const InputParameters & parameters)
  : IndicatorMarker(parameters),
    _coarsen(parameters.get<Real>("coarsen")),
    _refine(parameters.get<Real>("refine")),
    _clear_extremes(parameters.get<bool>("clear_extremes")),
    _max(0),
    _min(std::numeric_limits<Real>::max())
{
}

void
ErrorFractionMarker::markerSetup()
{
  if (_clear_extremes)
  {
    _min = std::numeric_limits<Real>::max();
    _max = 0;
  }

  // First find the max and min error
  for (const auto & val : _error_vector)
  {
    _min = std::min(_min, static_cast<Real>(val));
    _max = std::max(_max, static_cast<Real>(val));
  }

  _delta = _max - _min;
  _refine_cutoff = (1.0 - _refine) * _max;
  _coarsen_cutoff = _coarsen * _delta + _min;
}

Marker::MarkerValue
ErrorFractionMarker::computeElementMarker()
{
  Real error = _error_vector[_current_elem->id()];

  if (error > _refine_cutoff)
    return REFINE;
  else if (error < _coarsen_cutoff)
    return COARSEN;

  return DO_NOTHING;
}
