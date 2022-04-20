//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AugmentSparsityBetweenElements.h"
#include "libmesh/elem.h"

registerMooseObject("ThermalHydraulicsApp", AugmentSparsityBetweenElements);

using namespace libMesh;

InputParameters
AugmentSparsityBetweenElements::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  params.addRequiredParam<std::map<dof_id_type, std::vector<dof_id_type>> *>(
      "_elem_map", "Element to element augmentation map");
  return params;
}

AugmentSparsityBetweenElements::AugmentSparsityBetweenElements(const InputParameters & params)
  : RelationshipManager(params),
    _elem_map(*getParam<std::map<dof_id_type, std::vector<dof_id_type>> *>("_elem_map"))
{
}

AugmentSparsityBetweenElements::AugmentSparsityBetweenElements(
    const AugmentSparsityBetweenElements & other)
  : RelationshipManager(other), _elem_map(other._elem_map)
{
}

std::unique_ptr<GhostingFunctor>
AugmentSparsityBetweenElements::clone() const
{
  return std::make_unique<AugmentSparsityBetweenElements>(*this);
}

void
AugmentSparsityBetweenElements::mesh_reinit()
{
  RelationshipManager::mesh_reinit();
}

void
AugmentSparsityBetweenElements::redistribute()
{
}

void
AugmentSparsityBetweenElements::internalInitWithMesh(const MeshBase &)
{
}

std::string
AugmentSparsityBetweenElements::getInfo() const
{
  std::ostringstream oss;
  oss << "AugmentSparsityBetweenElements";
  return oss.str();
}

void
AugmentSparsityBetweenElements::operator()(const MeshBase::const_element_iterator & range_begin,
                                           const MeshBase::const_element_iterator & range_end,
                                           processor_id_type p,
                                           map_type & coupled_elements)
{
  const CouplingMatrix * const null_mat = libmesh_nullptr;
  for (const auto & elem : as_range(range_begin, range_end))
  {
    auto it = _elem_map.find(elem->id());
    if (it != _elem_map.end())
    {
      for (auto & coupled_elem_id : it->second)
      {
        auto coupled_elem = _moose_mesh->elemPtr(coupled_elem_id);
        if (coupled_elem->processor_id() != p)
          coupled_elements.insert(std::make_pair(coupled_elem, null_mat));
      }
    }
  }
}

bool
AugmentSparsityBetweenElements::operator>=(const RelationshipManager & rhs) const
{
  const auto * const rm = dynamic_cast<const AugmentSparsityBetweenElements *>(&rhs);
  if (!rm)
    return false;

  return (_elem_map == rm->_elem_map) && baseGreaterEqual(rhs);
}
