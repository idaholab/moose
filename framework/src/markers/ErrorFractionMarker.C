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

template<>
InputParameters validParams<ErrorFractionMarker>()
{
  InputParameters params = validParams<IndicatorMarker>();
  params.addParam<Real>("coarsen", 0, "Elements within this percentage of the min error will be coarsened.  Must be between 0 and 1!");
  params.addParam<Real>("refine", 0, "Elements within this percentage of the max error will be refined.  Must be between 0 and 1!");
  return params;
}


ErrorFractionMarker::ErrorFractionMarker(const std::string & name, InputParameters parameters) :
    IndicatorMarker(name, parameters),
    _coarsen(parameters.get<Real>("coarsen")),
    _refine(parameters.get<Real>("refine"))
{
  mooseAssert(_coarsen <= 1, "coarsen amount in " + _name + " not less than 1!");
  mooseAssert(_coarsen >= 0, "coarsen amount in " + _name + " not greater than 0!");
  mooseAssert(_refine <= 1, "refine amount in " + _name + " not less than 1!");
  mooseAssert(_refine >= 0, "refine amount in " + _name + " not greater than 0!");
}

void
ErrorFractionMarker::markerSetup()
{
  _min = std::numeric_limits<Real>::max();
  _max = 0;

  // First find the max and min error
  for(unsigned int i=0; i<_error_vector.size(); i++)
  {
    _min = std::min(_min, static_cast<Real>(_error_vector[i]));
    _max = std::max(_max, static_cast<Real>(_error_vector[i]));
  }

  _delta = _max-_min;
  _refine_cutoff = (1.0-_refine)*_max;
  _coarsen_cutoff = _coarsen*_delta + _min;
}

Marker::MarkerValue
ErrorFractionMarker::computeElementMarker()
{
  Real error = _error_vector[_current_elem->id()];

  if(error > _refine_cutoff)
    return REFINE;
  else if(error < _coarsen_cutoff)
    return COARSEN;

  return DO_NOTHING;
}

