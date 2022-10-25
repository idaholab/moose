//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MultiAppFieldTransfer.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "MooseVariableFEBase.h"
#include "MooseMesh.h"
#include "SystemBase.h"
#include "MooseAppCoordTransform.h"
#include "MooseMeshUtils.h"

#include "libmesh/system.h"

InputParameters
MultiAppFieldTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addParam<TagName>(
      "from_solution_tag",
      "The tag of the solution vector to be transferred (default to the solution)");
  params.addParam<TagName>(
      "to_solution_tag",
      "The tag of the solution vector to be transferred to (default to the solution)");

  // Block restrictions
  params.addParam<std::vector<SubdomainName>>(
      "from_blocks",
      "Subdomain restriction to transfer from (defaults to all the origin app domain)");
  params.addParam<std::vector<SubdomainName>>(
      "to_blocks", "Subdomain restriction to transfer to, (defaults to all the target app domain)");

  return params;
}

MultiAppFieldTransfer::MultiAppFieldTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _has_block_restrictions(isParamValid("from_blocks") || isParamValid("to_blocks"))
{
}

void
MultiAppFieldTransfer::initialSetup()
{
  MultiAppTransfer::initialSetup();

  if (_current_direction == TO_MULTIAPP)
    for (auto & to_var : getToVarNames())
      variableIntegrityCheck(to_var);
  else if (_current_direction == FROM_MULTIAPP)
    for (auto & from_var : getFromVarNames())
      variableIntegrityCheck(from_var);
  else
  {
    for (auto & to_var : getToVarNames())
      variableIntegrityCheck(to_var);
    for (auto & from_var : getFromVarNames())
      variableIntegrityCheck(from_var);
  }

  // Convert block names to block IDs, fill with ANY_BLOCK_ID if unspecified
  if (_has_block_restrictions)
  {
    const FEProblemBase * from_problem;
    const FEProblemBase * to_problem;
    MeshBase * from_mesh;
    MeshBase * to_mesh;

    if (_current_direction == FROM_MULTIAPP)
    {
      // Subdomain and variable type information is shared on all subapps
      from_problem = &getFromMultiApp()->appProblemBase(0);
      from_mesh = &getFromMultiApp()->appProblemBase(0).mesh().getMesh();

      to_problem = &getFromMultiApp()->problemBase();
      to_mesh = &getFromMultiApp()->problemBase().mesh().getMesh();
    }
    else if (_current_direction == TO_MULTIAPP)
    {
      from_problem = &getToMultiApp()->problemBase();
      from_mesh = &getToMultiApp()->problemBase().mesh().getMesh();

      to_problem = &getToMultiApp()->appProblemBase(0);
      to_mesh = &getToMultiApp()->appProblemBase(0).mesh().getMesh();
    }
    else
    {
      from_problem = &getFromMultiApp()->appProblemBase(0);
      from_mesh = &getFromMultiApp()->appProblemBase(0).mesh().getMesh();

      to_problem = &getToMultiApp()->appProblemBase(0);
      to_mesh = &getToMultiApp()->appProblemBase(0).mesh().getMesh();
    }

    const auto & from_block_names = getParam<std::vector<SubdomainName>>("from_blocks");
    for (const auto & b : from_block_names)
      if (!MooseMeshUtils::hasSubdomainName(*from_mesh, b))
        paramError("from_blocks", "The block '", b, "' was not found in the mesh");

    if (from_block_names.size())
    {
      const auto block_vec = from_problem->mesh().getSubdomainIDs(from_block_names);
      _from_blocks = std::set<SubdomainID>(block_vec.begin(), block_vec.end());
    }
    else
      _from_blocks = {Moose::ANY_BLOCK_ID};

    const auto & to_block_names = getParam<std::vector<SubdomainName>>("to_blocks");
    for (const auto & b : to_block_names)
      if (!MooseMeshUtils::hasSubdomainName(*to_mesh, b))
        paramError("to_blocks", "The block '", b, "' was not found in the mesh");

    if (to_block_names.size())
    {
      const auto block_vec = to_problem->mesh().getSubdomainIDs(to_block_names);
      _to_blocks = std::set<SubdomainID>(block_vec.begin(), block_vec.end());
    }
    else
      _to_blocks = {Moose::ANY_BLOCK_ID};
  }
}

EquationSystems &
MultiAppFieldTransfer::getEquationSystem(FEProblemBase & problem, bool use_displaced) const
{
  if (use_displaced)
  {
    if (!problem.getDisplacedProblem())
      mooseError("No displaced problem to provide a displaced equation system");
    return problem.getDisplacedProblem()->es();
  }
  else
    return problem.es();
}
