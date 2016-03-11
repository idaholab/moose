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

// MOOSE includes
#include "AutoPositionsMultiApp.h"
#include "MooseMesh.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AutoPositionsMultiApp>()
{
  InputParameters params = validParams<TransientMultiApp>();

  params += validParams<BoundaryRestrictable>();

  params.suppressParameter<std::vector<Point> >("positions");
  params.suppressParameter<std::vector<FileName> >("positions_file");

  return params;
}


AutoPositionsMultiApp::AutoPositionsMultiApp(const InputParameters & parameters):
    TransientMultiApp(parameters),
    BoundaryRestrictable(parameters)
{
}

AutoPositionsMultiApp::~AutoPositionsMultiApp()
{
}

void
AutoPositionsMultiApp::fillPositions()
{
  MooseMesh & master_mesh = _fe_problem.mesh();

  const std::set<BoundaryID> & bids = boundaryIDs();

  for (std::set<BoundaryID>::iterator bid_it = bids.begin();
       bid_it != bids.end();
       ++bid_it)
  {
    BoundaryID boundary_id = *bid_it;

    // Grab the nodes on the boundary ID and add a Sub-App at each one.
    const std::vector<dof_id_type> & boundary_node_ids = master_mesh.getNodeList(boundary_id);

    for (unsigned int i=0; i<boundary_node_ids.size(); i++)
    {
      Node & node = master_mesh.node(boundary_node_ids[i]);

      _positions.push_back(node);
    }
  }
}
