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

#include "PointValue.h"
#include "Function.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PointValue>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<VariableName>("variable", "The name of the variable that this postprocessor operates on.");
  params.addRequiredParam<Point>("point", "The physical point where the solution will be evaluated.");
  return params;
}

PointValue::PointValue(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),
    _u(_var.sln()),
    _mesh(_subproblem.mesh().getMesh()),
    _point_vec(1, getParam<Point>("point")),
    _value(0),
    _root_id(0),
    _elem_id(DofObject::invalid_id)
{
}

void
PointValue::execute()
{
  // Locate the element and store the id
  // We can't store the actual Element pointer here b/c PointLocatorBase returns a const Elem *
  UniquePtr<PointLocatorBase> pl = _mesh.sub_point_locator();
  const Elem * elem = (*pl)(_point_vec[0]);

  // Error if the element cannot be located
  if (!elem)
    mooseError("No element located at " << _point_vec[0] << " in PointValue Postprocessor named: " << _name);

  // Store the element id and processor id that owns the located element
  _elem_id = elem->id();
  _root_id = elem->processor_id();
}

void
PointValue::finalize()
{
  // Gather a consist id for broadcasting the computed value
  gatherMin(_root_id);

  // Compute the value at the point
  if (_root_id == processor_id())
  {
    const Elem * elem = _mesh.elem(_elem_id);
    std::set<MooseVariable *> var_list;
    var_list.insert(&_var);

    _fe_problem.setActiveElementalMooseVariables(var_list, _tid);
    _subproblem.reinitElemPhys(elem, _point_vec, 0);
    mooseAssert(_u.size() == 1, "No values in u!");
    _value = _u[0];
  }

  // Make sure all processors have the correct computed values
  _communicator.broadcast(_value, _root_id);
}

Real
PointValue::getValue()
{
  return _value;
}
