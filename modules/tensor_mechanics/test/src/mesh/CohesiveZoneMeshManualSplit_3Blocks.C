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

#include "CohesiveZoneMeshManualSplit_3Blocks.h"
#include "Parser.h"
#include "MooseUtils.h"
#include "Moose.h"
#include "MooseApp.h"
#include "MooseError.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/nemesis_io.h"
#include "libmesh/parallel_mesh.h"
#include "libmesh/serial_mesh.h"

registerMooseObject("TensorMechanicsApp", CohesiveZoneMeshManualSplit_3Blocks);

template <>
InputParameters
validParams<CohesiveZoneMeshManualSplit_3Blocks>()
{
  InputParameters params = validParams<CohesiveZoneMeshManualSplitBase>();
  params.addParam<MeshFileName>(
      "file", "coh3D_3Blocks.e", "The name of the mesh file to read");
  params.addClassDescription("Manually split the mesh in coh3D_3blocks.e");
  return params;
}

CohesiveZoneMeshManualSplit_3Blocks::CohesiveZoneMeshManualSplit_3Blocks(
    const InputParameters & parameters)
  : CohesiveZoneMeshManualSplitBase(parameters)

{
  getMesh().set_mesh_dimension(getParam<MooseEnum>("dim"));
}

CohesiveZoneMeshManualSplit_3Blocks::CohesiveZoneMeshManualSplit_3Blocks(
    const CohesiveZoneMeshManualSplit_3Blocks & other_mesh)
  : CohesiveZoneMeshManualSplitBase(other_mesh)
{
}

CohesiveZoneMeshManualSplit_3Blocks::~CohesiveZoneMeshManualSplit_3Blocks() {}

std::unique_ptr<MooseMesh>
CohesiveZoneMeshManualSplit_3Blocks::safeClone() const
{
  return libmesh_make_unique<CohesiveZoneMeshManualSplit_3Blocks>(*this);
}

void
CohesiveZoneMeshManualSplit_3Blocks::init()
{
  if (_pars.isParamSetByUser("file"))
    mooseError("CohesiveZoneMeshManualSplit_3Blocks is intended to work "
               "only on the default parameter value");

  MooseMesh::init();

  checkInputParameter();

  updateElements();

  addInterfaceBoundary();

}

void
CohesiveZoneMeshManualSplit_3Blocks::updateElements()
{
  // specyfing nodes to duplciate

  dof_id_type curr_elem, local_node, global_node;

  // node 0
  curr_elem = 4;
  local_node = 1; // new node 27
  duplicateAndSetLocalNode(curr_elem, local_node);
  // node 3
  curr_elem = 4;
  local_node = 2; // new node 28
  duplicateAndSetLocalNode(curr_elem, local_node);
  curr_elem = 7;
  local_node = 1;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 29

  // node 4
  curr_elem = 4;
  local_node = 5;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 30
  curr_elem = 5;
  local_node = 1;
  global_node = 30;
  setElemNode(curr_elem, local_node, global_node);

  // node 7
  curr_elem = 4;
  local_node = 6;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 31
  curr_elem = 5;
  local_node = 2;
  global_node = 31;
  setElemNode(curr_elem, local_node, global_node);
  curr_elem = 6;
  local_node = 1;
  global_node = 31;
  setElemNode(curr_elem, local_node, global_node);

  curr_elem = 7;
  local_node = 5;
  duplicateAndSetLocalNode(curr_elem, local_node);  // new node 32

  // node 9
  curr_elem = 7;
  local_node = 2;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 33

  // node 11
  curr_elem = 6;
  local_node = 2;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 34
  curr_elem = 7;
  local_node = 6;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 35

  // node 12
  curr_elem = 5;
  local_node = 5;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 36

  // node 15
  curr_elem = 5;
  local_node = 6;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 37
  curr_elem = 6;
  local_node = 5;
  global_node = 37;
  setElemNode(curr_elem, local_node, global_node);

  // node 17
  curr_elem = 6;
  local_node = 6;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 38

  // node 19
  curr_elem = 7;
  local_node = 0;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 39

  // node 21
  curr_elem = 7;
  local_node = 4;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 40

  // node 24
  curr_elem = 7;
  local_node = 7;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 41
}

void
CohesiveZoneMeshManualSplit_3Blocks::addInterfaceBoundary()
{
  // construct boundary 100, which will contain the cohesive interface
  Elem * elem;
  BoundaryInfo & boundary_info = getMesh().get_boundary_info();

  if (!_split_interface){
    elem = getMesh().elem(0);
    boundary_info.add_side(elem->id(), 4, _interface_id);

    elem = getMesh().elem(1);
    boundary_info.add_side(elem->id(), 4, _interface_id);

    elem = getMesh().elem(2);
    boundary_info.add_side(elem->id(), 4, _interface_id);

    elem = getMesh().elem(3);
    boundary_info.add_side(elem->id(), 4, _interface_id);

    elem = getMesh().elem(4);
    boundary_info.add_side(elem->id(), 3, _interface_id);

    elem = getMesh().elem(6);
    boundary_info.add_side(elem->id(), 0, _interface_id);

    // rename the boundary
    boundary_info.sideset_name(_interface_id) =
                               _interface_name;
  } else {
    elem = getMesh().elem(0);
    boundary_info.add_side(elem->id(), 4, 0);

    elem = getMesh().elem(2);
    boundary_info.add_side(elem->id(), 4, 0);

    elem = getMesh().elem(3);
    boundary_info.add_side(elem->id(), 4, 0);

    // rename the boundary
    boundary_info.sideset_name(0) = "czm_bM1_bS2";

    elem = getMesh().elem(1);
    boundary_info.add_side(elem->id(), 4, 4);

    // rename the boundary
    boundary_info.sideset_name(4) = "czm_bM1_bS3";

    elem = getMesh().elem(4);
    boundary_info.add_side(elem->id(), 3, 5);

    elem = getMesh().elem(6);
    boundary_info.add_side(elem->id(), 0, 5);

    // rename the boundary
    boundary_info.sideset_name(5) = "czm_bM2_bS3";
  }
}
