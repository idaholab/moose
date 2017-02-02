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

#include "ValueRangeMarker.h"
#include "FEProblem.h"
#include "MooseEnum.h"

template<>
InputParameters validParams<ValueRangeMarker>()
{
  InputParameters params = validParams<Marker>();

  MooseEnum third_state("DONT_MARK=-1 COARSEN DO_NOTHING REFINE", "DONT_MARK");
  params.addParam<MooseEnum>("third_state", third_state, "The Marker state to apply to values in the buffer zone (both ends of the range).");
  params.addRequiredParam<Real>("lower_bound", "The lower bound value for the range.");
  params.addRequiredParam<Real>("upper_bound", "The upper bound value for the range.");
  params.addParam<Real>("buffer_size", 0.0, "A buffer zone value added to both ends of the range where a third_state marker can be returned.");
  params.addParam<bool>("invert", false, "If this is true then values inside the range will be coarsened, and values outside the range will be refined.");
  params.addRequiredCoupledVar("variable", "The variable whose values are used in this marker.");
  params.addClassDescription("Mark elements for adaptivity based on the supplied upper and lower bounds and the specified variable.");
  return params;
}


ValueRangeMarker::ValueRangeMarker(const InputParameters & parameters) :
    QuadraturePointMarker(parameters),
    _lower_bound(parameters.get<Real>("lower_bound")),
    _upper_bound(parameters.get<Real>("upper_bound")),
    _buffer_size(parameters.get<Real>("buffer_size")),
    _third_state(getParam<MooseEnum>("third_state").getEnum<MarkerValue>()),
    _inside(getParam<bool>("invert") ? COARSEN : REFINE),
    _outside(getParam<bool>("invert") ? REFINE : COARSEN),

    _u(coupledValue("variable"))
{
  if (_upper_bound < _lower_bound)
    mooseError2("Invalid bounds specified (upper_bound < lower_bound)");

  if (_buffer_size < 0.0)
    mooseError2("Buffer size must be non-negative: ", _buffer_size);
}

Marker::MarkerValue
ValueRangeMarker::computeQpMarker()
{
  // Is the variable value inside the range?
  if (_u[_qp] >= _lower_bound && _u[_qp] <= _upper_bound)
    return _inside;

  // How about the buffer zone?
  if (_u[_qp] >= _lower_bound-_buffer_size && _u[_qp] <= _upper_bound+_buffer_size)
    return _third_state;

  // Must be outside the range
  return _outside;
}
