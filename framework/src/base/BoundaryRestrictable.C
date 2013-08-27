/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "BoundaryRestrictable.h"

template<>
InputParameters validParams<BoundaryRestrictable>()
{
  InputParameters params = emptyInputParameters();

  params.addParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");

  // Used to control active boundary...
  // The following causes Exodiffs:
  //   tranfers/multiapp_userobject_transfer_test
  //   materials/stateful_prop.spatial_adaptivity
  //params.addPrivateParam<BoundaryID>("_boundary_id");

  return params;
}

BoundaryRestrictable::BoundaryRestrictable(InputParameters & parameters) :
    RestrictableBase(parameters),
    _boundary_names(parameters.get<std::vector<BoundaryName> >("boundary")),
    _boundary_id(parameters.isParamValid("_boundary_id") ? parameters.get<BoundaryID>("_boundary_id") : Moose::ANY_BOUNDARY_ID)
{

  // Extract the boundary ids from the mesh
  std::vector<BoundaryID> ids = _r_mesh->getBoundaryIDs(_boundary_names, true);
  _bnd_ids.insert(ids.begin(), ids.end());
}

BoundaryRestrictable::~BoundaryRestrictable()
{
}

BoundaryID
BoundaryRestrictable::boundaryID()
{
  return _boundary_id;
}

const std::set<BoundaryID> &
BoundaryRestrictable::boundaryIDs(){
  return _bnd_ids;
}

const std::vector<BoundaryName> &
BoundaryRestrictable::boundaryNames()
{
  return _boundary_names;
}

unsigned int
BoundaryRestrictable::numBoundary()
{
  return (unsigned int) _bnd_ids.size();
}

bool
BoundaryRestrictable::hasBoundary(BoundaryName name)
{
  // Create a vector and utilize the getBoundaryIDs function, which
  // handles the ANY_BLOCK_ID (getBoundaryID does not)
  std::vector<BoundaryName> names(1);
  names[0] = name;
  return hasBoundary(_r_mesh->getBoundaryIDs(names));
}

bool
BoundaryRestrictable::hasBoundary(std::vector<BoundaryName> names)
{
  return hasBoundary(_r_mesh->getBoundaryIDs(names));
}

bool
BoundaryRestrictable::hasBoundary(BoundaryID id)
{
  // Cycle through the stored values, return if the supplied id matches on of the entries
  for (std::set<BoundaryID>::const_iterator it = _bnd_ids.begin(); it != _bnd_ids.end(); ++it)
  {
    if (id == *it)
      return true;
  }

  // If you make it here, there was no match
  return false;
}

bool
BoundaryRestrictable::hasBoundary(std::vector<BoundaryID> ids)
{
  std::set<BoundaryID> ids_set(ids.begin(), ids.end());
  return hasBoundary(ids_set);
}

bool
BoundaryRestrictable::hasBoundary(std::set<BoundaryID> ids)
{
  // An empty input is assumed to be ANY_BOUNDARY_ID
  if (ids.size() == 0 || ids.count(Moose::ANY_BOUNDARY_ID))
    return true;
  else
    return std::includes(_bnd_ids.begin(), _bnd_ids.end(), ids.begin(), ids.end());
}

bool
BoundaryRestrictable::isBoundarySubset(std::set<BoundaryID> ids)
{
  // An empty input is assumed to be ANY_BOUNDARY_ID
  if (ids.size() == 0 || ids.count(Moose::ANY_BOUNDARY_ID))
    return true;
  else
    return std::includes(ids.begin(), ids.end(), _bnd_ids.begin(), _bnd_ids.end());
}
