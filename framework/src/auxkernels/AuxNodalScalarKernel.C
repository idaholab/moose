//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AuxNodalScalarKernel.h"
#include "SystemBase.h"

InputParameters
AuxNodalScalarKernel::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<std::vector<dof_id_type>>("nodes", "Node ids");
  return params;
}

AuxNodalScalarKernel::AuxNodalScalarKernel(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    Coupleable(this, true),
    MooseVariableDependencyInterface(this),
    _node_ids(getParam<std::vector<dof_id_type>>("nodes"))
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}

void
AuxNodalScalarKernel::compute()
{
  _subproblem.reinitNodes(_node_ids, _tid); // compute variables at nodes
  AuxScalarKernel::compute();
}
