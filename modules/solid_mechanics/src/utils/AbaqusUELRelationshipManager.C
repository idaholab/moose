//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUELRelationshipManager.h"
#include "AbaqusUELMesh.h"

#include "libmesh/coupling_matrix.h"
#include "libmesh/elem.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/remote_elem.h"
#include "libmesh/int_range.h"
#include "libmesh/libmesh_logging.h"

// C++ Includes
#include <unordered_set>

registerMooseObject("MooseApp", AbaqusUELRelationshipManager);

using namespace libMesh;

InputParameters
AbaqusUELRelationshipManager::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  return params;
}

AbaqusUELRelationshipManager::AbaqusUELRelationshipManager(const InputParameters & params)
  : RelationshipManager(params), _uel_mesh(dynamic_cast<AbaqusUELMesh *>(_moose_mesh))
{
  if (!_uel_mesh)
    mooseError("Must use an AbaqusUELMesh with the AbaqusUELRelationshipManager.");
}

AbaqusUELRelationshipManager::AbaqusUELRelationshipManager(
    const AbaqusUELRelationshipManager & other)
  : RelationshipManager(other), _uel_mesh(other._uel_mesh)
{
}

std::string
AbaqusUELRelationshipManager::getInfo() const
{
  static const std::string info = "AbaqusUELRelationshipManager";
  return info;
}

void
AbaqusUELRelationshipManager::operator()(const MeshBase::const_element_iterator & range_begin,
                                         const MeshBase::const_element_iterator & range_end,
                                         processor_id_type p,
                                         map_type & coupled_elements)
{
  LOG_SCOPE("operator()", "AbaqusUELRelationshipManager");

  const auto & elements = _uel_mesh->getElements();
  const auto & node_to_uel_map = _uel_mesh->getNodeToUELMap();

  // loop over each NodeElement
  for (const Elem * const elem : as_range(range_begin, range_end))
  {
    // find the list of UEL Elements the NodeElment is part of
    const auto it = node_to_uel_map.find(elem->id());
    if (it == node_to_uel_map.end())
      mooseError("Element not found in NodeToUELMap.");

    // iterate over the UEL Elements connected to the current node elem
    for (const auto uel_elem_index  : it->second)
      // iterate over the NodeElements
      for (const auto nodeelem_index : elements[uel_elem_index]._nodes)
      {
        const auto coupled_elem = _mesh->elem_ptr(nodeelem_index);
        mooseAssert(coupled_elem, "Element not found. Internal error");
        if (coupled_elem->processor_id() != p)
          coupled_elements.emplace(coupled_elem, nullptr);
      }
  }
}

bool
AbaqusUELRelationshipManager::operator>=(const RelationshipManager & /*other*/) const
{
  return false;
}

std::unique_ptr<GhostingFunctor>
AbaqusUELRelationshipManager::clone() const
{
  return _app.getFactory().copyConstruct(*this);
}
