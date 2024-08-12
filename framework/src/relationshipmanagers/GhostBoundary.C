//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// App includes
#include "GhostBoundary.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseApp.h"

// libMesh includes
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/boundary_info.h"

registerMooseObject("MooseApp", GhostBoundary);

using namespace libMesh;

InputParameters
GhostBoundary::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  params.addRequiredParam<std::vector<BoundaryName>>("boundary",
                                                     "The name of the primary boundary sideset.");
  params.addClassDescription("This class constructs a relationship manager system' "
                             "to communicate ghost elements on a boundary.");
  return params;
}

GhostBoundary::GhostBoundary(const InputParameters & params)
  : RelationshipManager(params), _boundary_name(getParam<std::vector<BoundaryName>>("boundary"))
{
}

GhostBoundary::GhostBoundary(const GhostBoundary & other)
  : RelationshipManager(other), _boundary_name(other._boundary_name)
{
}

void
GhostBoundary::internalInitWithMesh(const MeshBase &)
{
}

std::string
GhostBoundary::getInfo() const
{
  std::ostringstream oss;
  oss << "GhostBoundary";
  return oss.str();
}

void
GhostBoundary::operator()(const MeshBase::const_element_iterator & /*range_begin*/,
                          const MeshBase::const_element_iterator & /*range_end*/,
                          const processor_id_type p,
                          map_type & coupled_elements)
{
  // We ask the user to pass boundary names instead of ids to our constraint object.  However, We
  // are unable to get the boundary ids from boundary names until we've attached the MeshBase object
  // to the MooseMesh
  const bool generating_mesh = !_moose_mesh->getMeshPtr();
  const auto boundary_ids = generating_mesh ? std::vector<BoundaryID>{Moose::INVALID_BOUNDARY_ID}
                                            : _moose_mesh->getBoundaryIDs(_boundary_name);

  for (const Elem * const elem : _mesh->active_element_ptr_range())
  {
    if (generating_mesh)
    { // We are still generating the mesh, so it's possible we don't even have the right boundary
      // ids created yet! So we actually ghost all boundary elements and all lower dimensional
      // elements who have parents on a boundary
      if (elem->on_boundary())
        coupled_elements.insert(std::make_pair(elem, _null_mat));
    }
    else
    {
      // We've finished generating our mesh so we can be selective and only ghost elements lying on
      // our boundary
      const BoundaryInfo & binfo = _mesh->get_boundary_info();
      for (auto side : elem->side_index_range())
        for (auto boundary_id : boundary_ids)
          if ((elem->processor_id() != p) && (binfo.has_boundary_id(elem, side, boundary_id)))
          {
            coupled_elements.insert(std::make_pair(elem, _null_mat));
            goto countBreak;
          }
    countBreak:;
    }
  }
}

bool
GhostBoundary::operator>=(const RelationshipManager & other) const
{
  if (auto asoi = dynamic_cast<const GhostBoundary *>(&other); asoi && baseGreaterEqual(*asoi))
  {
    std::set<BoundaryName> our_set(_boundary_name.begin(), _boundary_name.end());
    std::set<BoundaryName> their_set(asoi->_boundary_name.begin(), asoi->_boundary_name.end());
    std::set<BoundaryName> difference;
    std::set_difference(their_set.begin(),
                        their_set.end(),
                        our_set.begin(),
                        our_set.end(),
                        std::inserter(difference, difference.end()));
    if (difference.empty())
      return true;
  }
  return false;
}

std::unique_ptr<GhostingFunctor>
GhostBoundary::clone() const
{
  return _app.getFactory().copyConstruct(*this);
}
