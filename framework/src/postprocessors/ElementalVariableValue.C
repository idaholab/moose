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
#include "MooseMesh.h"
#include "SubProblem.h"

// libMesh
#include "boundary_info.h"

template<>
InputParameters validParams<ElementalVariableValue>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<VariableName>("variable", "The variable to be monitored");
  params.addRequiredParam<unsigned int>("elementid", "The ID of the element where we monitor");
  return params;
}

ElementalVariableValue::ElementalVariableValue(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _mesh(_subproblem.mesh()),
    _var_name(parameters.get<VariableName>("variable")),
    _element(_mesh.elem(parameters.get<unsigned int>("elementid")))
{
}

Real
ElementalVariableValue::getValue()
{
  Real value = 0;

  if(_element->processor_id() == libMesh::processor_id())
  {
    _subproblem.prepare(_element, _tid);
    _subproblem.reinitElem(_element, _tid);

    MooseVariable & var = _subproblem.getVariable(_tid, _var_name);
    VariableValue & u = var.sln();
    unsigned int n = u.size();
    for (unsigned int i = 0; i < n; i++)
      value += u[i];
    value /= n;
  }

  gatherSum(value);

  return value;
}
