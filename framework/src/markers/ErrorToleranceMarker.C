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

#include "ErrorToleranceMarker.h"

template<>
InputParameters validParams<ErrorToleranceMarker>()
{
  InputParameters params = validParams<IndicatorMarker>();
  params.addParam<Real>("coarsen", 0, "Elements with error less than this will be coarsened.");
  params.addParam<Real>("refine", std::numeric_limits<Real>::max(), "Elements with error more than this will be refined.");
  return params;
}


ErrorToleranceMarker::ErrorToleranceMarker(const std::string & name, InputParameters parameters) :
    IndicatorMarker(name, parameters),
    _coarsen(parameters.get<Real>("coarsen")),
    _refine(parameters.get<Real>("refine"))
{
}

int
ErrorToleranceMarker::computeElementMarker()
{
  Real error = _error_vector[_current_elem->id()];

  if(error > _refine)
    return Elem::REFINE;
  else if(error < _coarsen)
    return Elem::COARSEN;

  return Elem::DO_NOTHING;
}

