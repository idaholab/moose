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

#include "BlockRestrictable.h"

template<>
InputParameters validParams<BlockRestrictable>()
{
  // Create InputParameters object that will be appended to the parameters for the inheriting object
  InputParameters params = validParams<RestrictableBase>();

  // Add the user-facing 'block' input parameter
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (SubdomainID) that this kernel will be applied to");

  // Add the private parameter that is populated by this class that contains valid block ids for the
  // object inheriting from this class
  params.addPrivateParam<std::vector<SubdomainID> >("_block_ids", std::vector<SubdomainID>());

  // Return the parameters
  return params;
}

BlockRestrictable::BlockRestrictable(const std::string name, InputParameters & parameters) :
    RestrictableBase(name, parameters)
{
  // The 'block' input is defined and the FEProblem pointer is valid
  if (parameters.isParamValid("block"))
  {
    // Extract the blocks from the input
    _blocks = parameters.get<std::vector<SubdomainName> >("block");

    // Get the IDs from the supplied names
    std::vector<SubdomainID> vec_ids = _r_mesh->getSubdomainIDs(_blocks);

    // Create a set of IDS
    for (std::vector<SubdomainID>::const_iterator it = vec_ids.begin(); it != vec_ids.end(); ++it)
      _blk_ids.insert(*it);

    // Check that supplied blocks are within the variable domain
    if (parameters.isParamValid("variable") &&
        (parameters.have_parameter<NonlinearVariableName>("variable") ||
         parameters.have_parameter<AuxVariableName>("variable")))
    {
      // Get the SystemBase and the thread id
      SystemBase* sys = parameters.get<SystemBase *>("_sys");
      THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

  // A pointer to the variable class
      MooseVariable * var;

  // Get the variable based on the type
      if (parameters.have_parameter<NonlinearVariableName>("variable"))
        var = &_r_feproblem->getVariable(tid, parameters.get<NonlinearVariableName>("variable"));
      else if (parameters.have_parameter<AuxVariableName>("variable"))
        var = &_r_feproblem->getVariable(tid, parameters.get<AuxVariableName>("variable"));
      else
        mooseError("The variable input has an unknown Type");

      // Return the block ids for the variable
      std::set<SubdomainID> var_ids = sys->getSubdomainsForVar(var->index());

      // Test if the variable blockIDs are valid for this object
      if (!isBlockSubset(var_ids))
        mooseError("In object " << name << " the defined blocks are outside of the domain of the variable");
    }
  }

  // Produce error if the object is not allowed to be both block and boundary restrictable
  if (!_dual_restrictable && !_blk_ids.empty())
    if (parameters.isParamValid("_boundary_ids"))
    {
      std::vector<BoundaryID> bnd_ids = parameters.get<std::vector<BoundaryID> >("_boundary_ids");
      if (!bnd_ids.empty() && bnd_ids[0] != Moose::ANY_BOUNDARY_ID)
        mooseError("Attempted to restrict the object '" << name << "' to a block, but the object is already restricted by boundary");
    }

  // If no blocks were defined above, specify that it is valid on all blocks
  if (_blk_ids.empty())
    _blk_ids.insert(Moose::ANY_BLOCK_ID);

  // Store the private parameter that contains the set of block ids
  parameters.set<std::vector<SubdomainID> >("_block_ids") = std::vector<SubdomainID>(_blk_ids.begin(), _blk_ids.end());
}

const std::vector<SubdomainName> &
BlockRestrictable::blocks()
{
  return _blocks;
}

const std::set<SubdomainID> &
BlockRestrictable::blockIDs()
{
  return _blk_ids;
}

unsigned int
BlockRestrictable::numBlocks()
{
  return (unsigned int) _blk_ids.size();
}

bool
BlockRestrictable::hasBlocks(SubdomainName name)
{
  // Create a vector and utilize the getSubdomainIDs function, which
  // handles the ANY_BLOCK_ID (getSubdomainID does not)
  std::vector<SubdomainName> names(1);
  names[0] = name;
  return hasBlocks(_r_mesh->getSubdomainIDs(names));
}

bool
BlockRestrictable::hasBlocks(std::vector<SubdomainName> names)
{
  return hasBlocks(_r_mesh->getSubdomainIDs(names));
}

bool
BlockRestrictable::hasBlocks(SubdomainID id)
{
  // Cycle through the stored values, return if the supplied id matches on of the entries
  for (std::set<SubdomainID>::const_iterator it = _blk_ids.begin(); it != _blk_ids.end(); ++it)
  {
    if (id == *it)
      return true;
  }

  // If you make it here, there was no match
  return false;
}

bool
BlockRestrictable::hasBlocks(std::vector<SubdomainID> ids)
{
  std::set<SubdomainID> ids_set(ids.begin(), ids.end());
  return hasBlocks(ids_set);
}

bool
BlockRestrictable::hasBlocks(std::set<SubdomainID> ids)
{
  // An empty input is assumed to be ANY_BLOCK_ID
  if (ids.size() == 0 || ids.count(Moose::ANY_BLOCK_ID))
    return true;
  else
    return std::includes(_blk_ids.begin(), _blk_ids.end(), ids.begin(), ids.end());
}

bool
BlockRestrictable::isBlockSubset(std::set<SubdomainID> ids)
{
  // An empty input is assumed to be ANY_BLOCK_ID
  if (ids.size() == 0 || ids.count(Moose::ANY_BLOCK_ID))
    return true;
  else
    return std::includes(ids.begin(), ids.end(), _blk_ids.begin(), _blk_ids.end());
}

bool
BlockRestrictable::isBlockSubset(std::vector<SubdomainID> ids)
{
  std::set<SubdomainID> ids_set(ids.begin(), ids.end());
  return isBlockSubset(ids_set);
}
