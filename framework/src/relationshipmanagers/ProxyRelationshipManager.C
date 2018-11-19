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

registerMooseObject("MooseApp", ProxyRelationshipManager);

template <>
InputParameters
validParams<ProxyRelationshipManager>()
{
  InputParameters params = validParams<RelationshipManager>();

  params.addRequiredParam<System *>("other_system",
                                    "The libMesh system to mirror the ghosting from");

  return params;
}

ProxyRelationshipManager::ProxyRelationshipManager(const InputParameters & parameters)
  : RelationshipManager(parameters), _other_system(getCheckedPointerParam<System *>("other_system"))
{
}

void
ProxyRelationshipManager::operator()(const MeshBase::const_element_iterator & range_begin,
                                     const MeshBase::const_element_iterator & range_end,
                                     processor_id_type p,
                                     map_type & coupled_elements)
{
  std::vector<Elem *> other_system_elems;

  auto & other_mesh = _other_system->get_mesh();

  // First, create a vector that contains all of the elements of _other_ system that correspond
  // to the range passed in for _this_ system.
  for (auto this_system_elem_it = range_begin; this_system_elem_it != range_end;
       ++this_system_elem_it)
  {
    auto this_system_elem_ptr = *this_system_elem_it;

    auto other_system_elem_ptr = other_mesh.elem_ptr(this_system_elem_ptr->id());

    other_system_elems.push_back(other_system_elem_ptr);
  }

  Elem * const * elempp = other_system_elems.data();
  Elem * const * elemend = elempp + other_system_elems.size();

  // Now run the other system's ghosting functors over this new vector
  const MeshBase::const_element_iterator other_elements_begin = MeshBase::const_element_iterator(
                                             elempp, elemend, Predicates::NotNull<Elem *const *>()),
                                         other_elements_end = MeshBase::const_element_iterator(
                                             elemend,
                                             elemend,
                                             Predicates::NotNull<Elem *const *>());

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

  // If we're geometric - run all the algebraic ghosting functors from the other system
  if (isType(Moose::RelationshipManagerType::ALGEBRAIC))
  {
    auto gf_it = _other_system->get_dof_map().algebraic_ghosting_functors_begin();
    const auto gf_end = _other_system->get_dof_map().algebraic_ghosting_functors_end();

    for (; gf_it != gf_end; ++gf_it)
      if (!dynamic_cast<ProxyRelationshipManager *>(*gf_it)) // Don't recurse!
        (*(*gf_it))(other_elements_begin, other_elements_end, p, other_coupled_elements);
  }

  // Now translate those back into elements in this system
  for (auto other_coupled_it = other_coupled_elements.begin();
       other_coupled_it != other_coupled_elements.end();
       ++other_coupled_it)
  {
    auto other_system_elem_it = other_coupled_it->first;

    coupled_elements.emplace(_mesh.getMesh().elem_ptr(other_system_elem_it->id()),
                             other_coupled_it->second);
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
ProxyRelationshipManager::operator==(const RelationshipManager & /*rhs*/) const
{
  // There isn't a need to determine these because only the correct ones will be added
  return false;
}
