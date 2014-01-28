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
  InputParameters params = emptyInputParameters();

  // Create user-facing 'boundary' input for restricting inheriting object to boundaries
  params.addParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");

  // Create a private parameter for storing the boundary IDs (used by MaterialPropertyInterface)
  params.addPrivateParam<std::vector<BoundaryID> >("_boundary_ids", std::vector<BoundaryID>());

  // A parameter for disabling error message for objects restrictable by boundary and block,
  // if the parameter is valid it was already set so don't do anything
  if (!params.isParamValid("_dual_restrictable"))
    params.addPrivateParam<bool>("_dual_restrictable", false);

  return params;
}

BoundaryRestrictable::BoundaryRestrictable(const std::string name, InputParameters & parameters) :
    _boundary_names(parameters.get<std::vector<BoundaryName> >("boundary")),
    _bnd_feproblem(parameters.isParamValid("_fe_problem") ?
                   parameters.get<FEProblem *>("_fe_problem") : NULL),
    _bnd_mesh(parameters.isParamValid("_mesh") ?
              parameters.get<MooseMesh *>("_mesh") : NULL),
    _boundary_id(_bnd_feproblem->getActiveBoundaryID()),
    _bnd_dual_restrictable(parameters.get<bool>("_dual_restrictable"))

{

  // If the mesh pointer is not defined, but FEProblem is, get it from there
  if (_bnd_feproblem != NULL && _bnd_mesh == NULL)
    _bnd_mesh = &_bnd_feproblem->mesh();

  // Check that the mesh pointer was defined, it is required for this class to operate
  if (_bnd_mesh == NULL)
    mooseError("The input paramters must contain a pointer to FEProblem via '_fe_problem' or a pointer to the MooseMesh via '_mesh'");

  // If the user supplies boundary IDs
  if (!_boundary_names.empty())
  {
    std::vector<BoundaryID> ids = _bnd_mesh->getBoundaryIDs(_boundary_names, true);
    _bnd_ids.insert(ids.begin(), ids.end());
  }

  // Produce error if the object is not allowed to be both block and boundary restrictable
  if (!_bnd_dual_restrictable && !_bnd_ids.empty())
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
BoundaryRestrictable::numBoundaryIDs()
{
  return (unsigned int) _bnd_ids.size();
}

bool
BoundaryRestrictable::hasBoundary(BoundaryName name)
{
  // Create a vector and utilize the getBoundaryIDs function, which
  // handles the ANY_BOUNDARY_ID (getBoundaryID does not)
  std::vector<BoundaryName> names(1);
  names[0] = name;
  return hasBoundary(_bnd_mesh->getBoundaryIDs(names));
}

bool
BoundaryRestrictable::hasBoundary(std::vector<BoundaryName> names)
{
  return hasBoundary(_bnd_mesh->getBoundaryIDs(names));
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
  if (ids.empty() || ids.count(Moose::ANY_BOUNDARY_ID) > 0)
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
  if (ids.empty() || ids.count(Moose::ANY_BOUNDARY_ID) > 0)
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

const BoundaryID &
BoundaryRestrictable::getActiveBoundaryID()
{
  mooseAssert(_boundary_id != Moose::INVALID_BOUNDARY_ID, "BoundaryID is invalid");
  return _boundary_id;
}
