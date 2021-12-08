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
#include "MooseMesh.h"
#include "SystemBase.h"

defineLegacyParams(NodalScalarKernel);

InputParameters
NodalScalarKernel::validParams()
{
  InputParameters params = ScalarKernel::validParams();

  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where this nodal kernel applies");

  return params;
}

NodalScalarKernel::NodalScalarKernel(const InputParameters & parameters)
  : ScalarKernel(parameters),
    Coupleable(this, true),
    MooseVariableDependencyInterface(),
    _node_ids(buildNodeIDs())
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}

std::vector<dof_id_type>
NodalScalarKernel::buildNodeIDs() const
{
  std::vector<dof_id_type> node_ids;
  for (const auto & boundary_name : getParam<std::vector<BoundaryName>>("boundary"))
  {
    const auto boundary_node_ids = _mesh.getNodeList(_mesh.getBoundaryID(boundary_name));
    node_ids.insert(node_ids.end(), boundary_node_ids.begin(), boundary_node_ids.end());
  }
  return node_ids;
}

void
NodalScalarKernel::reinit()
{
  _subproblem.reinitNodes(_node_ids, _tid); // compute variables at nodes
  _assembly.prepareOffDiagScalar();
}

void
NodalScalarKernel::computeOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
}
