//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// App includes
#include "AugmentSparsityOnInterface.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseApp.h"

// libMesh includes
#include "libmesh/elem.h"

registerMooseObject("MooseApp", AugmentSparsityOnInterface);

using namespace libMesh;

defineLegacyParams(AugmentSparsityOnInterface);

InputParameters
AugmentSparsityOnInterface::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  params.addRequiredParam<BoundaryName>("master_boundary",
                                        "The name of the master boundary sideset.");
  params.addRequiredParam<BoundaryName>("slave_boundary",
                                        "The name of the slave boundary sideset.");
  params.addRequiredParam<SubdomainName>("master_subdomain",
                                         "The name of the master lower dimensional subdomain.");
  params.addRequiredParam<SubdomainName>("slave_subdomain",
                                         "The name of the slave lower dimensional subdomain.");
  return params;
}

AugmentSparsityOnInterface::AugmentSparsityOnInterface(const InputParameters & params)
  : RelationshipManager(params),
    _master_boundary_name(getParam<BoundaryName>("master_boundary")),
    _slave_boundary_name(getParam<BoundaryName>("slave_boundary")),
    _master_subdomain_name(getParam<SubdomainName>("master_subdomain")),
    _slave_subdomain_name(getParam<SubdomainName>("slave_subdomain"))
{
}

void
AugmentSparsityOnInterface::mesh_reinit()
{
}

std::string
AugmentSparsityOnInterface::getInfo() const
{
  std::ostringstream oss;
  oss << "AugmentSparsityOnInterface";
  return oss.str();
}

void
AugmentSparsityOnInterface::operator()(const MeshBase::const_element_iterator &,
                                       const MeshBase::const_element_iterator &,
                                       processor_id_type p,
                                       map_type & coupled_elements)
{
  const CouplingMatrix * const null_mat = libmesh_nullptr;

  // The MeshBase object
  const auto & libmesh_mesh = _mesh.getMesh();

  auto slave_boundary_id = _mesh.getBoundaryID(_slave_boundary_name);
  auto master_boundary_id = _mesh.getBoundaryID(_master_boundary_name);
  auto slave_subdomain_id = _mesh.getSubdomainID(_slave_subdomain_name);
  auto master_subdomain_id = _mesh.getSubdomainID(_master_subdomain_name);

  // Build up the boundary information so we know which higher-dimensional elements are on the
  // mortar interface
  std::set<dof_id_type> higher_d_elems_to_ghost;

  std::vector<dof_id_type> elem_ids;
  std::vector<Point> elem_centroids;
  std::vector<processor_id_type> elem_proc_ids;
  for (const auto & elem : libmesh_mesh.active_element_ptr_range())
  {
    elem_ids.push_back(elem->id());
    elem_centroids.push_back(elem->centroid());
    elem_proc_ids.push_back(elem->processor_id());
  }

  auto side_list = libmesh_mesh.get_boundary_info().build_active_side_list();
  for (auto & tuple : side_list)
  {
    auto boundary_id = std::get<2>(tuple);
    if (boundary_id == slave_boundary_id || boundary_id == master_boundary_id)
      higher_d_elems_to_ghost.insert(std::get<0>(tuple));
  }

  for (const auto & elem : libmesh_mesh.active_element_ptr_range())
    if (elem->processor_id() != p &&
        (elem->subdomain_id() == slave_subdomain_id ||
         elem->subdomain_id() == master_subdomain_id ||
         higher_d_elems_to_ghost.find(elem->id()) != higher_d_elems_to_ghost.end()))
      coupled_elements.insert(std::make_pair(elem, null_mat));
}

bool
AugmentSparsityOnInterface::operator==(const RelationshipManager & other) const
{
  if (auto asoi = dynamic_cast<const AugmentSparsityOnInterface *>(&other))
  {
    if (_master_boundary_name == asoi->_master_boundary_name &&
        _slave_boundary_name == asoi->_slave_boundary_name &&
        _master_subdomain_name == asoi->_master_subdomain_name &&
        _slave_subdomain_name == asoi->_slave_subdomain_name)
      return true;
  }
  return false;
}
