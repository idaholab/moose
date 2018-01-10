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
#include "CentroidMultiApp.h"
#include "MooseMesh.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/parallel_algebra.h"

template <>
InputParameters
validParams<CentroidMultiApp>()
{
  InputParameters params = validParams<TransientMultiApp>();

  params += validParams<BlockRestrictable>();

  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<std::vector<FileName>>("positions_file");

  return params;
}

CentroidMultiApp::CentroidMultiApp(const InputParameters & parameters)
  : TransientMultiApp(parameters), BlockRestrictable(this)
{
}

void
CentroidMultiApp::fillPositions()
{
  MooseMesh & master_mesh = _fe_problem.mesh();

  auto & bids = blockIDs();

  // Detect if it can be on any block
  bool any_bid = bids.find(Moose::ANY_BLOCK_ID) != bids.end();

  for (const auto & elem_ptr : master_mesh.getMesh().active_local_element_ptr_range())
  {
    if (any_bid || bids.find(elem_ptr->subdomain_id()) != bids.end())
      _positions.push_back(elem_ptr->centroid());
  }

  libMesh::ParallelObject::comm().allgather(_positions);

  // An attempt to try to make this parallel stable
  std::sort(_positions.begin(), _positions.end());
}
