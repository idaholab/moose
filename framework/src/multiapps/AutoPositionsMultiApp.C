//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AutoPositionsMultiApp.h"
#include "MooseMesh.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AutoPositionsMultiApp>()
{
  InputParameters params = validParams<TransientMultiApp>();

  params += validParams<BoundaryRestrictable>();

  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<std::vector<FileName>>("positions_file");

  return params;
}

AutoPositionsMultiApp::AutoPositionsMultiApp(const InputParameters & parameters)
  : TransientMultiApp(parameters), BoundaryRestrictable(this, true) // true for applying to nodesets
{
}

void
AutoPositionsMultiApp::fillPositions()
{
  MooseMesh & master_mesh = _fe_problem.mesh();

  const std::set<BoundaryID> & bids = boundaryIDs();

  for (const auto & boundary_id : bids)
  {
    // Grab the nodes on the boundary ID and add a Sub-App at each one.
    const std::vector<dof_id_type> & boundary_node_ids = master_mesh.getNodeList(boundary_id);

    for (const auto & boundary_node_id : boundary_node_ids)
    {
      Node & node = master_mesh.nodeRef(boundary_node_id);

      _positions.push_back(node);
    }
  }
}
