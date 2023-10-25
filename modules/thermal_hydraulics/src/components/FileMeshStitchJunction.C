//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshStitchJunction.h"
#include "FileMeshComponent.h"
#include "FileMeshWCNSFVComponent.h"
#include "THMMesh.h"
#include "WCNSFVPhysicsBase.h"
#include "NS.h"

#include "libmesh/boundary_info.h"

registerMooseObject("ThermalHydraulicsApp", FileMeshStitchJunction);

InputParameters
FileMeshStitchJunction::validParams()
{
  InputParameters params = FileMeshComponentJunction::validParams();
  params.addPrivateParam<std::string>("component_type", "flow_junction");

  return params;
}

FileMeshStitchJunction::FileMeshStitchJunction(const InputParameters & params)
  : FileMeshComponentJunction(params)
{
}

void
FileMeshStitchJunction::setupMesh()
{
  FileMeshComponentJunction::setupMesh();
  // The mesh contains all the components already, but the components are not stitched together
  THMMesh & comp_meshes = mesh();
  comp_meshes.buildNodeListFromSideList();
  comp_meshes.buildNodeList();

  for (const auto & conn_i : index_range(_connections))
  {
    if (conn_i == 0)
      continue;
    const auto & b1_name = _connections[0]._boundary_name;
    const auto & b2_name = _connections[conn_i]._boundary_name;
    MooseMeshUtils::stitchNodesets(comp_meshes, b1_name, b2_name, false, true);
    // TODO boundary infos from the component should be invalidated now
  }
}

void
FileMeshStitchJunction::init()
{
  FileMeshComponentJunction::init();
  mooseAssert(_connections.size(), "Should have a connection");
}

void
FileMeshStitchJunction::check() const
{
  FileMeshComponentJunction::check();

  for (const auto & comp_name : _connected_component_names)
    checkComponentOfTypeExistsByName<FileMeshComponent>(comp_name);
  // for (const auto i : index_range(_connections))
  //   getConnectedComponent(i).hasBoundary(_connections[i]._boundary_name);
}
