//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// App includes
#include "GhostLowerDElems.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseApp.h"

// libMesh includes
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/boundary_info.h"

registerMooseObject("MooseApp", GhostLowerDElems);

using namespace libMesh;

InputParameters
GhostLowerDElems::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  params.set<bool>("attach_geometric_early") = false;
  return params;
}

GhostLowerDElems::GhostLowerDElems(const InputParameters & params) : RelationshipManager(params) {}

GhostLowerDElems::GhostLowerDElems(const GhostLowerDElems & other) : RelationshipManager(other) {}

std::string
GhostLowerDElems::getInfo() const
{
  std::ostringstream oss;
  oss << "GhostLowerDElems";
  return oss.str();
}

void
GhostLowerDElems::operator()(const MeshBase::const_element_iterator & range_begin,
                             const MeshBase::const_element_iterator & range_end,
                             const processor_id_type p,
                             map_type & coupled_elements)
{
  mooseAssert(_moose_mesh,
              "The MOOSE mesh must be non-null in order for this relationship manager to work.");

  static const CouplingMatrix * const null_mat = nullptr;

  for (const Elem * const elem : as_range(range_begin, range_end))
    for (const auto s : elem->side_index_range())
    {
      const Elem * const neighbor = elem->neighbor_ptr(s);
      const bool elem_owns_lowerd = !neighbor || elem->id() < neighbor->id();

      const Elem * const lower_d_elem =
          elem_owns_lowerd
              ? _moose_mesh->getLowerDElem(elem, s)
              : _moose_mesh->getLowerDElem(neighbor, neighbor->which_neighbor_am_i(elem));

      if (lower_d_elem && lower_d_elem->processor_id() != p)
        coupled_elements.emplace(lower_d_elem, null_mat);
    }
}

bool
GhostLowerDElems::operator>=(const RelationshipManager & other) const
{
  return dynamic_cast<const GhostLowerDElems *>(&other);
}
