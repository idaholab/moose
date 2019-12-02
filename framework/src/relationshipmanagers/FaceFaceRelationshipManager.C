//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FaceFaceRelationshipManager.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseApp.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", FaceFaceRelationshipManager);

using namespace libMesh;

defineLegacyParams(FaceFaceRelationshipManager);

InputParameters
FaceFaceRelationshipManager::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  params.addRequiredParam<BoundaryName>("master_boundary",
                                        "The name of the master boundary sideset.");
  params.addRequiredParam<BoundaryName>("slave_boundary",
                                        "The name of the slave boundary sideset.");
  return params;
}

FaceFaceRelationshipManager::FaceFaceRelationshipManager(const InputParameters & params)
  : RelationshipManager(params),
    _master_boundary_name(getParam<BoundaryName>("master_boundary")),
    _slave_boundary_name(getParam<BoundaryName>("slave_boundary"))
{
}

void
FaceFaceRelationshipManager::mesh_reinit()
{
}

std::string
FaceFaceRelationshipManager::getInfo() const
{
  std::ostringstream oss;
  oss << "FaceFaceRelationshipManager";
  return oss.str();
}

void
FaceFaceRelationshipManager::operator()(const MeshBase::const_element_iterator &,
                                        const MeshBase::const_element_iterator &,
                                        processor_id_type p,
                                        map_type & coupled_elements)
{
  const CouplingMatrix * const null_mat = libmesh_nullptr;

  // The MeshBase object
  const auto & libmesh_mesh = _mesh.getMesh();

  auto slave_boundary_id = _mesh.getBoundaryID(_slave_boundary_name);
  auto master_boundary_id = _mesh.getBoundaryID(_master_boundary_name);

  // Build up the boundary information so we know which elements are on the slave or master boundary
  std::set<dof_id_type> boundary_elems_to_ghost;

  auto side_list = libmesh_mesh.get_boundary_info().build_active_side_list();
  for (auto & tuple : side_list)
  {
    auto boundary_id = std::get<2>(tuple);
    if (boundary_id == slave_boundary_id || boundary_id == master_boundary_id)
      boundary_elems_to_ghost.insert(std::get<0>(tuple));
  }

  for (const auto & elem : libmesh_mesh.active_element_ptr_range())
    if (elem->processor_id() != p &&
        boundary_elems_to_ghost.find(elem->id()) != boundary_elems_to_ghost.end())
      coupled_elements.insert(std::make_pair(elem, null_mat));
}

bool
FaceFaceRelationshipManager::operator==(const RelationshipManager & other) const
{
  if (auto asoi = dynamic_cast<const FaceFaceRelationshipManager *>(&other))
    if (_master_boundary_name == asoi->_master_boundary_name &&
        _slave_boundary_name == asoi->_slave_boundary_name)
      return true;
  return false;
}
