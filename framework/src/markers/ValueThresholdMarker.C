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

#include "ValueThresholdMarker.h"
#include "FEProblem.h"

template<>
InputParameters validParams<ValueThresholdMarker>()
{
  InputParameters params = validParams<Marker>();

  params.addParam<Real>("coarsen", "The threshold value for coarsening.  Elements with variable values beyond this will be marked for coarsening.");
  params.addParam<Real>("refine", "The threshold value for refinement.  Elements with variable values beyond this will be marked for refinement.");
  params.addParam<bool>("invert", false, "If this is true then values _below_ 'refine' will be refined and _above_ 'coarsen' will be coarsened.");
  params.addRequiredParam<VariableName>("variable", "The values of this variable will be compared to 'refine' and 'coarsen' to see what should be done with the element");
  params.addParam<bool>("dont_mark", true, "If this is true than any element not marked for refinement or coarsening will _not_ be marked for anything.  If it is false then elements not marked for refinement or coarsening will be marked to 'do_nothing'");

  return params;
}


ValueThresholdMarker::ValueThresholdMarker(const std::string & name, InputParameters parameters) :
    Marker(name, parameters),
    _coarsen_set(parameters.isParamValid("coarsen")),
    _coarsen(parameters.get<Real>("coarsen")),
    _refine_set(parameters.isParamValid("refine")),
    _refine(parameters.get<Real>("refine")),
    _invert(parameters.get<bool>("invert")),
    _dont_mark(parameters.get<bool>("dont_mark")),
    _variable_name(parameters.get<VariableName>("variable")),
    _variable(_fe_problem.getVariable(_tid, _variable_name)),
    _variable_sys(_variable.sys()),
    _variable_sys_solution(_variable_sys.currentSolution()),
    _variable_number(_variable.number()),
    _variable_dof_map(_variable.dofMap()),
    _variable_fe_type(_variable.feType())
{
  if(_variable_fe_type.family != LAGRANGE && _variable_fe_type != FEType(CONSTANT, MONOMIAL))
    mooseError("ValueThresholdMarker can only be used with variables of type Lagrange or Constant Monomial!");
}

Marker::MarkerValue
ValueThresholdMarker::computeElementMarker()
{
  _variable_dof_map.dof_indices(_current_elem, _variable_dof_indices, _variable_number);

  Real max_value = -std::numeric_limits<Real>::max();

  for(unsigned int i=0; i<_variable_dof_indices.size(); i++)
  {
    Real value = (*_variable_sys_solution)(_variable_dof_indices[i]);
    max_value = std::max(max_value, value);
  }

  if(!_invert)
  {
    if(_refine_set && max_value > _refine)
      return REFINE;

    if(_coarsen_set && max_value < _coarsen)
      return COARSEN;
  }
  else
  {
    if(_refine_set && max_value < _refine)
      return REFINE;

    if(_coarsen_set && max_value > _coarsen)
      return COARSEN;
  }

  if(_dont_mark)
    return DONT_MARK;
  else
    return DO_NOTHING;
}


