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

#include "CohesiveZoneMeshManualSplit_2DJunction.h"
#include "Parser.h"
#include "MooseUtils.h"
#include "Moose.h"
#include "MooseApp.h"
#include "MooseError.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/nemesis_io.h"
#include "libmesh/parallel_mesh.h"
#include "libmesh/serial_mesh.h"

registerMooseObject("TensorMechanicsApp", CohesiveZoneMeshManualSplit_2DJunction);

template <>
InputParameters
validParams<CohesiveZoneMeshManualSplit_2DJunction>()
{
  InputParameters params = validParams<CohesiveZoneMeshManualSplitBase>();
  params.addParam<MeshFileName>("file", "4ElementJunction.e", "The name of the mesh file to read");
  params.addClassDescription("Manually split the mesh in 4ElementJunction.e");
  return params;
}

CohesiveZoneMeshManualSplit_2DJunction::CohesiveZoneMeshManualSplit_2DJunction(
    const InputParameters & parameters)
  : CohesiveZoneMeshManualSplitBase(parameters)
{
  getMesh().set_mesh_dimension(getParam<MooseEnum>("dim"));
}

CohesiveZoneMeshManualSplit_2DJunction::CohesiveZoneMeshManualSplit_2DJunction(
    const CohesiveZoneMeshManualSplit_2DJunction & other_mesh)
  : CohesiveZoneMeshManualSplitBase(other_mesh)
{
}

CohesiveZoneMeshManualSplit_2DJunction::~CohesiveZoneMeshManualSplit_2DJunction() {}

std::unique_ptr<MooseMesh>
CohesiveZoneMeshManualSplit_2DJunction::safeClone() const
{
  return libmesh_make_unique<CohesiveZoneMeshManualSplit_2DJunction>(*this);
}

void
CohesiveZoneMeshManualSplit_2DJunction::init()
{

  /*
  this file routine uses a 4 element mesh
  */

  if (_pars.isParamSetByUser("file"))
    mooseError("CohesiveZoneMeshManualSplit_2DJunction is intended to work "
               "only on the default parameter value");

  MooseMesh::init();

  checkInputParameter();

  updateElements();

  addInterfaceBoundary();
}

void
CohesiveZoneMeshManualSplit_2DJunction::updateElements()
{
  // specyfing nodes to duplciate

  dof_id_type curr_elem, local_node;

  curr_elem = 1;
  local_node = 0;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 2;
  local_node = 0;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 3;
  local_node = 0;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 1;
  local_node = 1;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 2;
  local_node = 2;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 3;
  local_node = 1;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 3;
  local_node = 2;
  duplicateAndSetLocalNode(curr_elem, local_node);
}

void
CohesiveZoneMeshManualSplit_2DJunction::addInterfaceBoundary()
{
  // construct boundary 100, which will contain the cohesive interface
  Elem * elem;
  BoundaryInfo & boundary_info = getMesh().get_boundary_info();

  if (!_split_interface){
  elem = getMesh().elem(0);
  boundary_info.add_side(elem->id(), 0, _interface_id);
  boundary_info.add_side(elem->id(), 2, _interface_id);

  elem = getMesh().elem(1);
  boundary_info.add_side(elem->id(), 2, _interface_id);

  elem = getMesh().elem(2);
  boundary_info.add_side(elem->id(), 0, _interface_id);

  // rename the boundary
  boundary_info.sideset_name(_interface_id) =
                             _interface_name;
} else {
  elem = getMesh().elem(0);
  boundary_info.add_side(elem->id(), 0, 0);
  boundary_info.sideset_name(0) = "czm_bM1_bS2";

  boundary_info.add_side(elem->id(), 2, 5);
  boundary_info.sideset_name(5) = "czm_bM1_bS3";

  elem = getMesh().elem(1);
  boundary_info.add_side(elem->id(), 2, 6);
  boundary_info.sideset_name(6) = "czm_bM2_bS4";

  elem = getMesh().elem(2);
  boundary_info.add_side(elem->id(), 0, 7);
  boundary_info.sideset_name(7) = "czm_bM3_bS4";

}
}
