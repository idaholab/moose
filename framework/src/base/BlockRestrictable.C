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
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (subdomain) that this kernel will be applied to");
  return params;
}

BlockRestrictable::BlockRestrictable(const std::string & name, InputParameters & parameters) :
    _br_fe_problem(parameters.isParamValid("_fe_problem") ? parameters.get<FEProblem *>("_fe_problem") : NULL),
    _br_mesh(parameters.isParamValid("_mesh") ? parameters.get<MooseMesh *>("_mesh") : NULL)
{

  // If the mesh pointer is not defined, but FEProblem is, get it from there
  if (_br_fe_problem != NULL && _br_mesh == NULL)
    _br_mesh = &_br_fe_problem->mesh();

  // Check that the mesh pointer was defined, it is required for this class to operate
  if (_br_mesh == NULL)
    mooseError("The input paramters must contain a pointer to FEProblem via '_fe_problem' or a pointer to the MooseMesh via '_mesh'");

  // If the 'block' input is not defined, populate it
  if (!parameters.isParamValid("block"))
  {
    // Case when a nonlinear variable limits the blocks
    if (parameters.have_parameter<NonlinearVariableName>("variable") && parameters.isParamValid("variable"))
    {

      // Check that the FEProblem pointer is defined
      if (_br_fe_problem == NULL)
        mooseError("The BlockRestrictable class requires '_fe_problem' to be defined as an input parameter when the object is variable restricted (i.e., contains the 'variable' input parameter)");

      // Get the SystemBase and the thread id
      SystemBase* sys = parameters.get<SystemBase *>("_sys");
      THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

      // Get the subdomains for curent variable
      MooseVariable & var = _br_fe_problem->getVariable(tid, parameters.get<NonlinearVariableName>("variable"));
      _set_ids = sys->getSubdomainsForVar(var.index());
    }

    // General case when the mesh limits the blocks
    else
      _set_ids  = _br_mesh->meshSubdomains();

    // Define a vector that stores the block names
    for (std::set<SubdomainID>::const_iterator it = _set_ids.begin(); it != _set_ids.end(); ++it)
      _blocks.push_back(_br_mesh->getMesh().subdomain_name(*it));

    // Update the std::vector of IDS
    _vec_ids.assign(_set_ids.begin(), _set_ids.end());
  }

  // The 'block' input is defined and the FEProblem pointer is valid
  else
  {
    // Extract the blocks from the input
    _blocks = parameters.get<std::vector<SubdomainName> >("block");

    // Get the IDs from the supplied names
    _vec_ids = _br_mesh->getSubdomainIDs(_blocks);

    // Create a set of IDS
    for (std::vector<SubdomainID>::const_iterator it = _vec_ids.begin(); it != _vec_ids.end(); ++it)
      _set_ids.insert(*it);

    // Check that supplied blocks are within the variable domain
    if (parameters.have_parameter<NonlinearVariableName>("variable") && parameters.isParamValid("variable"))
    {
      // Get the SystemBase and the thread id
      SystemBase* sys = parameters.get<SystemBase *>("_sys");
      THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

      // Get the subdomains for curent variable
      MooseVariable & var = _br_fe_problem->getVariable(tid, parameters.get<NonlinearVariableName>("variable"));
      std::set<SubdomainID> var_ids = sys->getSubdomainsForVar(var.index());

      // Test if the variable Subdomain IDs are valid for this object
      if (!isSubset(var_ids))
        mooseError("In Object '" + name + "': block outside of the domain of the variable");
    }
  }
}

const std::vector<SubdomainName> &
BlockRestrictable::blocks()
{
  return _blocks;
}

const std::set<SubdomainID> &
BlockRestrictable::getSubdomainIDs()
{
  return _set_ids;
}

bool
BlockRestrictable::hasBlocks(SubdomainName name)
{
  // Create a vector and utilize the getSubdomainIDs function, which
  // handles the ANY_BLOCK_ID (getSubdomainID does not)
  std::vector<SubdomainName> names(1);
  names[0] = name;
  return hasBlocks(_br_mesh->getSubdomainIDs(names));
}

bool
BlockRestrictable::hasBlocks(std::vector<SubdomainName> names)
{
  return hasBlocks(_br_mesh->getSubdomainIDs(names));
}

bool
BlockRestrictable::hasBlocks(SubdomainID id)
{
  // Cycle through the stored values, return if the supplied id matches on of the entries
  for (std::vector<SubdomainID>::const_iterator it = _vec_ids.begin(); it != _vec_ids.end(); ++it)
  {
    std::cout << "_vec_ids = " << (*it) << std::endl;

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
    return std::includes(_set_ids.begin(), _set_ids.end(), ids.begin(), ids.end());
}

bool
BlockRestrictable::isSubset(std::set<SubdomainID> ids)
{
  // An empty input is assumed to be ANY_BLOCK_ID
  if (ids.size() == 0 || ids.count(Moose::ANY_BLOCK_ID))
    return true;
  else
    return std::includes(ids.begin(), ids.end(), _set_ids.begin(), _set_ids.end());
}
