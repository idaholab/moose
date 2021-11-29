//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseGhostPointNeighbors.h"
#include "MooseMesh.h"
#include "Conversion.h"
#include "MooseApp.h"

#include "libmesh/ghost_point_neighbors.h"

registerMooseObject("MooseApp", MooseGhostPointNeighbors);

InputParameters
MooseGhostPointNeighbors::validParams()
{
  InputParameters params = FunctorRelationshipManager::validParams();
  return params;
}

MooseGhostPointNeighbors::MooseGhostPointNeighbors(const InputParameters & parameters)
  : FunctorRelationshipManager(parameters)
{
  if (_rm_type != Moose::RelationshipManagerType::GEOMETRIC)
    mooseError("The MooseGhostPointNeighbors relationship manager should only be used for "
               "geometric ghosting");
}

MooseGhostPointNeighbors::MooseGhostPointNeighbors(const MooseGhostPointNeighbors & others)
  : FunctorRelationshipManager(others)
{
  if (_rm_type != Moose::RelationshipManagerType::GEOMETRIC)
    mooseError("The MooseGhostPointNeighbors relationship manager should only be used for "
               "geometric ghosting");
}

std::unique_ptr<GhostingFunctor>
MooseGhostPointNeighbors::clone() const
{
  return std::make_unique<MooseGhostPointNeighbors>(*this);
}

std::string
MooseGhostPointNeighbors::getInfo() const
{
  std::ostringstream oss;

  oss << "GhostPointNeighbors";

  return oss.str();
}

bool
MooseGhostPointNeighbors::operator>=(const RelationshipManager & rhs) const
{
  const auto * rm = dynamic_cast<const MooseGhostPointNeighbors *>(&rhs);
  if (!rm)
    return false;
  else
    return baseGreaterEqual(*rm);
}

void
MooseGhostPointNeighbors::internalInitWithMesh(const MeshBase & mesh)
{
  auto functor = std::make_unique<GhostPointNeighbors>(mesh);

  _functor = std::move(functor);
}
