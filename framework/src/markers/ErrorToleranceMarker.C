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

// libMesh includes
#include "libmesh/error_vector.h"

template <>
InputParameters
validParams<ErrorToleranceMarker>()
{
  InputParameters params = validParams<IndicatorMarker>();
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
