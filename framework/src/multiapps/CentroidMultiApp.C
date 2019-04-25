//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "CentroidMultiApp.h"
#include "MooseMesh.h"
#include "FEProblem.h"

// libMesh includes
#include "libmesh/parallel_algebra.h"

registerMooseObject("MooseApp", CentroidMultiApp);

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

  for (const auto & elem_ptr : master_mesh.getMesh().active_local_element_ptr_range())
    if (hasBlocks(elem_ptr->subdomain_id()))
      _positions.push_back(elem_ptr->centroid());

  // Use the comm from the problem this MultiApp is part of
  libMesh::ParallelObject::comm().allgather(_positions);

  if (_positions.empty())
    mooseError("No positions found for CentroidMultiapp ", _name);

  // An attempt to try to make this parallel stable
  std::sort(_positions.begin(), _positions.end());
}
