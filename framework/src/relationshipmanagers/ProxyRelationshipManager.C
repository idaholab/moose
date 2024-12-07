//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProxyRelationshipManager.h"
#include "MooseApp.h"

#include "libmesh/system.h"
#include "libmesh/elem.h"
#include "libmesh/dof_map.h"

using namespace libMesh;

registerMooseObject("MooseApp", ProxyRelationshipManager);

InputParameters
ProxyRelationshipManager::validParams()
{
  InputParameters params = RelationshipManager::validParams();

  params.addRequiredParam<System *>("other_system",
                                    "The libMesh system to mirror the ghosting from");

  return params;
}

ProxyRelationshipManager::ProxyRelationshipManager(const InputParameters & parameters)
  : RelationshipManager(parameters), _other_system(getCheckedPointerParam<System *>("other_system"))
{
}

ProxyRelationshipManager::ProxyRelationshipManager(const ProxyRelationshipManager & others)
  : RelationshipManager(others), _other_system(others._other_system)
{
}

void
ProxyRelationshipManager::operator()(const MeshBase::const_element_iterator & /*range_begin*/,
                                     const MeshBase::const_element_iterator & /*range_end*/,
                                     processor_id_type p,
                                     map_type & coupled_elements)
{
  auto & other_mesh = _other_system->get_mesh();

  auto other_elements_begin = other_mesh.active_local_elements_begin();
  auto other_elements_end = other_mesh.active_local_elements_end();

  map_type other_coupled_elements;

  // If we're geometric - run all the geometric ghosting functors from the other system
  if (isType(Moose::RelationshipManagerType::GEOMETRIC))
  {
    auto gf_it = other_mesh.ghosting_functors_begin();
    const auto gf_end = other_mesh.ghosting_functors_end();

    for (; gf_it != gf_end; ++gf_it)
      if (!dynamic_cast<ProxyRelationshipManager *>(*gf_it)) // Don't recurse!
        (*(*gf_it))(other_elements_begin, other_elements_end, p, other_coupled_elements);
  }

  // If we're algebraic - run all the algebraic ghosting functors from the other system
  if (isType(Moose::RelationshipManagerType::ALGEBRAIC))
  {
    auto gf_it = _other_system->get_dof_map().algebraic_ghosting_functors_begin();
    const auto gf_end = _other_system->get_dof_map().algebraic_ghosting_functors_end();

    for (; gf_it != gf_end; ++gf_it)
      if (!dynamic_cast<ProxyRelationshipManager *>(*gf_it)) // Don't recurse!
        (*(*gf_it))(other_elements_begin, other_elements_end, p, other_coupled_elements);
  }

  // Build unique_id to elem map
  std::map<dof_id_type, const Elem *> unique_id_to_elem_map;

  for (auto elem_it = _moose_mesh->getMesh().active_elements_begin();
       elem_it != _moose_mesh->getMesh().active_elements_end();
       ++elem_it)
    unique_id_to_elem_map[(*elem_it)->unique_id()] = *elem_it;

  // Now translate those back into elements in this system
  for (auto other_coupled_it = other_coupled_elements.begin();
       other_coupled_it != other_coupled_elements.end();
       ++other_coupled_it)
  {
    auto other_system_elem = other_coupled_it->first;

    auto unique_id_to_elem_map_it = unique_id_to_elem_map.find(other_system_elem->unique_id());
    mooseAssert(unique_id_to_elem_map_it != unique_id_to_elem_map.end(), "no matching unique id");

    coupled_elements.emplace(unique_id_to_elem_map_it->second, other_coupled_it->second);
  }
}

std::string
ProxyRelationshipManager::getInfo() const
{
  std::string info = std::string("Proxy for ") + _other_system->name();

  if (useDisplacedMesh())
    info = info + " on displaced";

  return info;
}

bool
ProxyRelationshipManager::operator>=(const RelationshipManager & /*rhs*/) const
{
  // There isn't a need to determine these because only the correct ones will be added
  return false;
}

std::unique_ptr<GhostingFunctor>
ProxyRelationshipManager::clone() const
{
  return _app.getFactory().copyConstruct(*this);
}
