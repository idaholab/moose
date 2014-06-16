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
    _mesh(_subproblem.mesh()),
    _point(getParam<Point>("point")),
    _point_vec(1, _point),
    _value(0),
    _root_id(0)
{
}

PointValue::~PointValue()
{
}

void
PointValue::initialize()
{
}

void
PointValue::execute()
{
  std::set<MooseVariable *> var_list;
  var_list.insert(&_var);

  AutoPtr<PointLocatorBase> pl = _mesh.getMesh().sub_point_locator();

  // First find the element the hit lands in
  const Elem * elem = (*pl)(_point);
  mooseAssert(elem, "No element located at the specified point");
  _root_id = elem->processor_id();

  // Compute the value at the point
  if (elem && elem->processor_id() == processor_id())
  {
    _fe_problem.setActiveElementalMooseVariables(var_list, _tid);
    _subproblem.reinitElemPhys(elem, _point_vec, 0);
    mooseAssert(_u.size() == 1, "No values in u!");
    _value = variableValue();
  }
}

Real
PointValue::getValue()
{
  _communicator.broadcast(_value, _root_id);
  return _value;
}

Real
PointValue::variableValue()
{
  return _u[0];
}
