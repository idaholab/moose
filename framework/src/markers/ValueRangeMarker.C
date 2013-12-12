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

  MooseEnum third_state("DONT_MARK = -1, COARSEN, DO_NOTHING, REFINE", "DONT_MARK");
  params.addParam<MooseEnum>("third_state", third_state, "The Marker state to apply to values in the buffer zone (both ends of the range).");
  params.addRequiredParam<Real>("lower_bound", "The lower bound value for the range.");
  params.addRequiredParam<Real>("upper_bound", "The upper bound value for the range.");
  params.addParam<Real>("buffer_size", 0.0, "A buffer zone value added to both ends of the range where a third_state marker can be returned.");
  params.addParam<bool>("invert", false, "If this is true then values inside the range will be coarsened, and values outside the range will be refined.");
  params.addRequiredParam<VariableName>("variable", "The variable whose values are used in this marker.");
  return params;
}


ValueRangeMarker::ValueRangeMarker(const std::string & name, InputParameters parameters) :
    Marker(name, parameters),
    _lower_bound(parameters.get<Real>("lower_bound")),
    _upper_bound(parameters.get<Real>("upper_bound")),
    _buffer_size(parameters.get<Real>("buffer_size")),

    _third_state((MarkerValue)(int)getParam<MooseEnum>("third_state")),
    _inside(getParam<bool>("invert") ? COARSEN : REFINE),
    _outside(getParam<bool>("invert") ? REFINE : COARSEN),

    _variable_name(parameters.get<VariableName>("variable")),
    _variable(_fe_problem.getVariable(_tid, _variable_name)),
    _variable_sys(_variable.sys()),
    _variable_sys_solution(_variable_sys.currentSolution()),
    _variable_fe_type(_variable.feType())
{
  if(_variable_fe_type.family != LAGRANGE && _variable_fe_type != FEType(CONSTANT, MONOMIAL))
    mooseError("ValueRangeMarker can only be used with variables of type Lagrange or Constant Monomial!");
  if (_upper_bound < _lower_bound)
    mooseError("Invalid bounds specified (upper_bound < lower_bound)");
  if (_buffer_size < 0.0)
    mooseError("Buffer size must be non-negative: " << _buffer_size);

  addMooseVariableDependency(&_variable);
}

Marker::MarkerValue
ValueRangeMarker::computeElementMarker()
{
  _variable.getDofIndices(_current_elem, _variable_dof_indices);

  Real max_value = -std::numeric_limits<Real>::max();

  for(unsigned int i=0; i<_variable_dof_indices.size(); i++)
  {
    Real value = (*_variable_sys_solution)(_variable_dof_indices[i]);
    max_value = std::max(max_value, value);
  }

  // Is the variable value inside the range?
  if (max_value >= _lower_bound && max_value <= _upper_bound)
    return _inside;

  // How about the buffer zone?
  if (max_value >= _lower_bound-_buffer_size && max_value <= _upper_bound+_buffer_size)
    return _third_state;

  // Must be outside the range
  return _outside;
}


