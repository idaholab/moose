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
  // Create instance of InputParameters
  InputParameters params = validParams<RestrictableBase>();

  // Create user-facing 'boundary' input for restricting inheriting object to boundaries
  params.addParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");

  // Create a private parameter for storing the boundary IDs
  params.addPrivateParam<std::vector<BoundaryID> >("_boundary_ids", std::vector<BoundaryID>());

  return params;
}

BoundaryRestrictable::BoundaryRestrictable(const std::string name, InputParameters & parameters) :
    RestrictableBase(name, parameters),
    _boundary_names(parameters.get<std::vector<BoundaryName> >("boundary")),
    _boundary_id(parameters.isParamValid("_boundary_id") ? parameters.get<BoundaryID>("_boundary_id") : Moose::ANY_BOUNDARY_ID)
{
  // If the user supplies boundary IDs
  if (!_boundary_names.empty())
  {
    std::vector<BoundaryID> ids = _r_mesh->getBoundaryIDs(_boundary_names, true);
    _bnd_ids.insert(ids.begin(), ids.end());
  }

  // Produce error if the object is not allowed to be both block and boundary restrictable
  if (!_dual_restrictable && !_bnd_ids.empty())
    if (parameters.isParamValid("_block_ids"))
    {
       std::vector<SubdomainID> blk_ids = parameters.get<std::vector<SubdomainID> >("_block_ids");
       if (!blk_ids.empty() && blk_ids[0] != Moose::ANY_BLOCK_ID)
         mooseError("Attempted to restrict the object '" << name << "' to a boundary, but the object is already restricted by block(s)");

    }

  // Store ANY_BOUNDARY_ID if empty
  if (_bnd_ids.empty())
    _bnd_ids.insert(Moose::ANY_BOUNDARY_ID);

  // Store the ids in the input parameters
  parameters.set<std::vector<BoundaryID> >("_boundary_ids") = std::vector<BoundaryID>(_bnd_ids.begin(), _bnd_ids.end());
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
BoundaryRestrictable::boundaryIDs()
{
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
  // Cycle through the stored values, return if the supplied id matches one of the entries
  for (std::set<BoundaryID>::const_iterator it = _bnd_ids.begin(); it != _bnd_ids.end(); ++it)
  {
    if (id == *it)
      return true;
  }

  // If you make it here, there was no match
  return false;
}

bool
BoundaryRestrictable::hasBoundary(std::vector<BoundaryID> ids, TEST_TYPE type)
{
  std::set<BoundaryID> ids_set(ids.begin(), ids.end());
  return hasBoundary(ids_set, type);
}

bool
BoundaryRestrictable::hasBoundary(std::set<BoundaryID> ids, TEST_TYPE type)
{
  // An empty input is assumed to be ANY_BOUNDARY_ID
  if (ids.size() == 0 || ids.count(Moose::ANY_BOUNDARY_ID) > 0)
    return true;

  // All supplied IDs must match those of the object
  else if (type == ALL)
    return std::includes(_bnd_ids.begin(), _bnd_ids.end(), ids.begin(), ids.end());

  // Any of the supplid IDs must match those of the object
  else
  {
    // Loop through the supplied ids
    for (std::set<BoundaryID>::const_iterator it = ids.begin(); it != ids.end(); ++it)
    {
      // Test the current supplied id
      bool test = hasBoundary(*it);

      // If the id exists in the stored ids, then return true, otherwise
      if (test)
        return true;
    }
    return false;
  }
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

bool
BoundaryRestrictable::isBoundarySubset(std::vector<BoundaryID> ids)
{
  std::set<BoundaryID> ids_set(ids.begin(), ids.end());
  return isBoundarySubset(ids_set);
}
