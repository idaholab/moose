//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalScalarKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

template <>
InputParameters
validParams<NodalScalarKernel>()
{
  InputParameters params = validParams<ScalarKernel>();
  params.addRequiredParam<std::vector<dof_id_type>>("nodes", "Node ids");
  return params;
}

NodalScalarKernel::NodalScalarKernel(const InputParameters & parameters)
  : ScalarKernel(parameters),
    Coupleable(this, true),
    MooseVariableDependencyInterface(),
    _node_ids(getParam<std::vector<dof_id_type>>("nodes"))
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}

void
NodalScalarKernel::reinit()
{
  _subproblem.reinitNodes(_node_ids, _tid); // compute variables at nodes
  _assembly.prepareOffDiagScalar();
}

void
NodalScalarKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
}
