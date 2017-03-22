/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ErrorFractionMarker.h"

// libMesh includes
#include "libmesh/error_vector.h"

template <>
InputParameters
validParams<ErrorFractionMarker>()
{
  InputParameters params = validParams<IndicatorMarker>();
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

  params.addClassDescription("Marks elements for refinement or coarsening based on the fraction of "
                             "the total error from the supplied indicator.");
  return params;
}

ErrorFractionMarker::ErrorFractionMarker(const InputParameters & parameters)
  : IndicatorMarker(parameters),
    _coarsen(parameters.get<Real>("coarsen")),
    _refine(parameters.get<Real>("refine"))
{
}

void
ErrorFractionMarker::markerSetup()
{
  _min = std::numeric_limits<Real>::max();
  _max = 0;

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
