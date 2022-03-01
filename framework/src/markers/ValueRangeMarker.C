//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ValueRangeMarker.h"
#include "FEProblem.h"
#include "MooseEnum.h"

registerMooseObject("MooseApp", ValueRangeMarker);

InputParameters
ValueRangeMarker::validParams()
{
  InputParameters params = QuadraturePointMarker::validParams();

  params.addRequiredParam<Real>("lower_bound", "The lower bound value for the range.");
  params.addRequiredParam<Real>("upper_bound", "The upper bound value for the range.");
  params.addParam<Real>("buffer_size",
                        0.0,
                        "A buffer zone value added to both ends of the range "
                        "where a third_state marker can be returned.");

  params.addClassDescription("Mark elements for adaptivity based on the supplied upper and lower "
                             "bounds and the specified variable.");
  return params;
}

ValueRangeMarker::ValueRangeMarker(const InputParameters & parameters)
  : QuadraturePointMarker(parameters),
    _lower_bound(parameters.get<Real>("lower_bound")),
    _upper_bound(parameters.get<Real>("upper_bound")),
    _buffer_size(parameters.get<Real>("buffer_size")),
    _inside(getParam<bool>("invert") ? COARSEN : REFINE),
    _outside(getParam<bool>("invert") ? REFINE : COARSEN)
{
  if (_upper_bound < _lower_bound)
    mooseError("Invalid bounds specified (upper_bound < lower_bound)");

  if (_buffer_size < 0.0)
    mooseError("Buffer size must be non-negative: ", _buffer_size);
}

Marker::MarkerValue
ValueRangeMarker::computeQpMarker()
{
  // Is the variable value inside the range?
  if (_u[_qp] >= _lower_bound && _u[_qp] <= _upper_bound)
    return _inside;

  // How about the buffer zone?
  if (_u[_qp] >= _lower_bound - _buffer_size && _u[_qp] <= _upper_bound + _buffer_size)
    return _third_state;

  // Must be outside the range
  return _outside;
}
