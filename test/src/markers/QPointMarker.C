//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QPointMarker.h"
#include "FEProblem.h"
#include "MooseEnum.h"

registerMooseObject("MooseTestApp", QPointMarker);

InputParameters
QPointMarker::validParams()
{
  InputParameters params = QuadraturePointMarker::validParams();
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
