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

InputParameters
NodalScalarKernel::validParams()
{
  InputParameters params = ScalarKernel::validParams();
  params.addParam<std::vector<dof_id_type>>("nodes", "Supply nodes using node ids");
  params.addParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs  from the mesh where this nodal kernel applies");

  return params;
}

NodalScalarKernel::NodalScalarKernel(const InputParameters & parameters)
  : ScalarKernel(parameters),
    Coupleable(this, true),
    MooseVariableDependencyInterface(this),
    _node_ids(getParam<std::vector<dof_id_type>>("nodes")),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary"))
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);

  // Check if node_ids and/or node_bc_names given
  if ((_node_ids.size() == 0) && (_boundary_names.size() == 0))
    mooseError("Must provide either 'nodes' or 'boundary' parameter.");

  if ((_node_ids.size() != 0) && (_boundary_names.size() != 0))
    mooseError("Both 'nodes' and 'boundary' parameters were specified. Use the 'boundary' "
               "parameter only.");

  // nodal bc names provided, append the nodes in each bc to _node_ids
  if ((_node_ids.size() == 0) && (_boundary_names.size() != 0))
  {
    std::vector<dof_id_type> nodelist;

    for (auto & boundary_name : _boundary_names)
    {
      nodelist = _mesh.getNodeList(_mesh.getBoundaryID(boundary_name));
      for (auto & node_id : nodelist)
        _node_ids.push_back(node_id);
    }
  }
  else
  {
    mooseDeprecated("Using the 'nodes' parameter is deprecated. Please update your input file to "
                    "use the 'boundary' parameter.");
  }
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
