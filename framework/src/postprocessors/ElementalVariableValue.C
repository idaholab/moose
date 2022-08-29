//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementalVariableValue.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

registerMooseObject("MooseApp", ElementalVariableValue);

InputParameters
ElementalVariableValue::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable", "The variable to be monitored");
  params.addRequiredParam<unsigned int>("elementid", "The ID of the element where we monitor");
  params.addClassDescription("Outputs an elemental variable value at a particular location");
  return params;
}

ElementalVariableValue::ElementalVariableValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _var_name(parameters.get<VariableName>("variable")),
    _element(_mesh.getMesh().query_elem_ptr(parameters.get<unsigned int>("elementid"))),
    _value(0)
{
  // This class may be too dangerous to use if renumbering is enabled,
  // as the nodeid parameter obviously depends on a particular
  // numbering.
  if (_mesh.getMesh().allow_renumbering())
    mooseError("ElementalVariableValue should only be used when node renumbering is disabled.");
}
void
ElementalVariableValue::execute()
{
  _value = 0;
  if (_element && (_element->processor_id() == processor_id()))
  {
    _fe_problem.setCurrentSubdomainID(_element, 0);
    _subproblem.prepare(_element, _tid);
    _subproblem.reinitElem(_element, _tid);

    MooseVariableField<Real> & var = static_cast<MooseVariableField<Real> &>(
        _subproblem.getActualFieldVariable(_tid, _var_name));
    const VariableValue & u = var.sln();
    unsigned int n = u.size();
    for (unsigned int i = 0; i < n; i++)
      _value += u[i];
    _value /= n;
  }
}

Real
ElementalVariableValue::getValue()
{
  return _value;
}

void
ElementalVariableValue::finalize()
{
  gatherSum(_value);
}
