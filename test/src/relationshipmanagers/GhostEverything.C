//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// App includes
#include "GhostEverything.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseApp.h"

// libMesh includes
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/boundary_info.h"

registerMooseObject("MooseTestApp", GhostEverything);

using namespace libMesh;

InputParameters
GhostEverything::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  params.set<bool>("attach_geometric_early") = false;
  return params;
}

GhostEverything::GhostEverything(const InputParameters & params) : RelationshipManager(params) {}

GhostEverything::GhostEverything(const GhostEverything & other) : RelationshipManager(other) {}

std::string
GhostEverything::getInfo() const
{
  return "GhostEverything";
}

void
GhostEverything::operator()(const MeshBase::const_element_iterator & range_begin,
                            const MeshBase::const_element_iterator & range_end,
                            const processor_id_type p,
                            map_type & coupled_elements)
{
  if (std::distance(range_begin, range_end) == 0)
    return;

  static const CouplingMatrix * const null_mat = nullptr;

  for (const Elem * const elem : _mesh->active_element_ptr_range())
    if (elem->processor_id() != p)
      coupled_elements.emplace(elem, null_mat);
}

bool
GhostEverything::operator>=(const RelationshipManager & other) const
{
  return dynamic_cast<const GhostEverything *>(&other);
}
