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

#include "QPointMarker.h"
#include "FEProblem.h"
#include "MooseEnum.h"

template <>
InputParameters
validParams<QPointMarker>()
{
  InputParameters params = validParams<Marker>();
  return params;
}

QPointMarker::QPointMarker(const InputParameters & parameters) : QuadraturePointMarker(parameters)
{
}

Marker::MarkerValue
QPointMarker::computeQpMarker()
{
  if (_q_point[_qp](0) > 0.5)
    return REFINE;
  else
    return DONT_MARK;
}
