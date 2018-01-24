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

#include "ElementalVariableValue.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

template <>
InputParameters
validParams<ElementalVariableValue>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<VariableName>("variable", "The variable to be monitored");
  params.addRequiredParam<unsigned int>("elementid", "The ID of the element where we monitor");
  params.addClassDescription("Outputs an elemental variable value at a particular location");
  return params;
}

ElementalVariableValue::ElementalVariableValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _var_name(parameters.get<VariableName>("variable")),
    _element(_mesh.getMesh().query_elem_ptr(parameters.get<unsigned int>("elementid")))
{
  // This class may be too dangerous to use if renumbering is enabled,
  // as the nodeid parameter obviously depends on a particular
  // numbering.
  if (_mesh.getMesh().allow_renumbering())
    mooseError("ElementalVariableValue should only be used when node renumbering is disabled.");
}

Real
ElementalVariableValue::getValue()
{
  Real value = 0;

  if (_element && (_element->processor_id() == processor_id()))
  {
    _subproblem.prepare(_element, _tid);
    _subproblem.reinitElem(_element, _tid);

    MooseVariable & var = _subproblem.getVariable(_tid, _var_name);
    const VariableValue & u = var.sln();
    unsigned int n = u.size();
    for (unsigned int i = 0; i < n; i++)
      value += u[i];
    value /= n;
  }

  gatherSum(value);

  return value;
}
