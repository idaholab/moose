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
  InputParameters params = emptyInputParameters();

  // Add the user-facing 'block' input parameter
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (SubdomainID) that this kernel will be applied to");

  // Add the private parameter that is populated by this class that contains valid block ids for the
  // object inheriting from this class
  params.addPrivateParam<std::vector<SubdomainID> >("_block_ids", std::vector<SubdomainID>());

  // A parameter for disabling error message for objects restrictable by boundary and block,
  // if the parameter is valid it was already set so don't do anything
  if (!params.isParamValid("_dual_restrictable"))
    params.addPrivateParam<bool>("_dual_restrictable", false);

  // Return the parameters
  return params;
}

BlockRestrictable::BlockRestrictable(const std::string name, InputParameters & parameters) :
    _blk_dual_restrictable(parameters.get<bool>("_dual_restrictable")),
    _blk_feproblem(parameters.isParamValid("_fe_problem") ?
                   parameters.get<FEProblem *>("_fe_problem") : NULL),
    _blk_mesh(parameters.isParamValid("_mesh") ?
              parameters.get<MooseMesh *>("_mesh") : NULL)
{
  // If the mesh pointer is not defined, but FEProblem is, get it from there
  if (_blk_feproblem != NULL && _blk_mesh == NULL)
    _blk_mesh = &_blk_feproblem->mesh();

  // Check that the mesh pointer was defined, it is required for this class to operate
  if (_blk_mesh == NULL)
    mooseError("The input paramters must contain a pointer to FEProblem via '_fe_problem' or a pointer to the MooseMesh via '_mesh'");

  // The 'block' input is defined
  if (parameters.isParamValid("block"))
  {
    // Extract the blocks from the input
    _blocks = parameters.get<std::vector<SubdomainName> >("block");

    // Get the IDs from the supplied names
    std::vector<SubdomainID> vec_ids = _blk_mesh->getSubdomainIDs(_blocks);

    // Create a set of IDS
    for (std::vector<SubdomainID>::const_iterator it=vec_ids.begin(); it != vec_ids.end(); ++it)
      _blk_ids.insert(*it);

    // Check that supplied blocks are within the variable domain
    if (parameters.isParamValid("variable") &&
        (parameters.have_parameter<NonlinearVariableName>("variable") ||
         parameters.have_parameter<AuxVariableName>("variable")))
    {
      // A pointer to the variable class
      std::set<SubdomainID> var_ids = variableSubdomianIDs(parameters);

      // Test if the variable blockIDs are valid for this object
      if (!isBlockSubset(var_ids))
        mooseError("In object " << name << " the defined blocks are outside of the domain of the variable");
    }
  }

  // The 'block' input parameter is undefined
  else
  {
    // If the object contains a variable, set the subdomain ids to those of the variable
    if (parameters.isParamValid("variable") &&
        (parameters.have_parameter<NonlinearVariableName>("variable") ||
         parameters.have_parameter<AuxVariableName>("variable")))
    {
      _blk_ids = variableSubdomianIDs(parameters);
    }
  }

  // Produce error if the object is not allowed to be both block and boundary restrictable
  if (!_blk_dual_restrictable && !_blk_ids.empty())
    if (parameters.isParamValid("_boundary_ids"))
    {
      std::vector<BoundaryID> bnd_ids = parameters.get<std::vector<BoundaryID> >("_boundary_ids");
      if (!bnd_ids.empty()
          && std::find(bnd_ids.begin(), bnd_ids.end(), Moose::ANY_BOUNDARY_ID) != bnd_ids.end())
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
BlockRestrictable::blockIDs() const
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
  return hasBlocks(_blk_mesh->getSubdomainIDs(names));
}

bool
BlockRestrictable::hasBlocks(std::vector<SubdomainName> names)
{
  return hasBlocks(_blk_mesh->getSubdomainIDs(names));
}

bool
BlockRestrictable::hasBlocks(SubdomainID id)
{
  return _blk_ids.find(id) != _blk_ids.end();
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
  if (ids.empty() || ids.count(Moose::ANY_BLOCK_ID) > 0)
    return true;
  else
    return std::includes(_blk_ids.begin(), _blk_ids.end(), ids.begin(), ids.end());
}

bool
BlockRestrictable::isBlockSubset(std::set<SubdomainID> ids)
{
  // An empty input is assumed to be ANY_BLOCK_ID
  if (ids.empty() || ids.count(Moose::ANY_BLOCK_ID)  > 0)
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

std::set<SubdomainID>
BlockRestrictable::variableSubdomianIDs(InputParameters & parameters)
{
  // Return an empty set if _sys is not defined
  if (!parameters.isParamValid("_sys"))
    return std::set<SubdomainID>();

  // Get the SystemBase and the thread id
  SystemBase* sys = parameters.get<SystemBase *>("_sys");
  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

  // Pointer to MooseVariable
  MooseVariable * var = NULL;

  // Get the variable based on the type
  if (parameters.have_parameter<NonlinearVariableName>("variable"))
    var = &_blk_feproblem->getVariable(tid, parameters.get<NonlinearVariableName>("variable"));
  else if (parameters.have_parameter<AuxVariableName>("variable"))
    var = &_blk_feproblem->getVariable(tid, parameters.get<AuxVariableName>("variable"));

  // Return the block ids for the variable
  return sys->getSubdomainsForVar(var->index());
}

bool
BlockRestrictable::hasBlockMaterialProperty(const std::string & name)
{
  // Get reference to the blocks for the material
  const std::set<SubdomainID> & mat_blk = _blk_feproblem->getMaterialPropertyBlocks(name);

  // If material blocks are empty return false, otherwise test if the materials are a subset
  // of the blocks for this object
  if (mat_blk.empty())
    return false;
  else
    return isBlockSubset(mat_blk);
}
