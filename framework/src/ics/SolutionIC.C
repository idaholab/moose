//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionIC.h"
#include "SolutionUserObject.h"
#include "MooseMesh.h"
#include "SystemBase.h"

registerMooseObject("MooseApp", SolutionIC);
registerMooseObjectRenamed("MooseApp", SolutionInitialCondition, "06/30/2024 24:00", SolutionIC);

InputParameters
SolutionIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredParam<UserObjectName>("solution_uo",
                                          "The SolutionUserObject to extract data from.");
  params.addRequiredParam<VariableName>(
      "from_variable", "The name of the variable in the file that is to be extracted");
  params.addClassDescription(
      "Sets the initial condition from a field variable stored in an Exodus file, "
      "retrieved by a SolutionUserObject");
  return params;
}

SolutionIC::SolutionIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _solution_object(getUserObject<SolutionUserObject>("solution_uo")),
    _solution_object_var_name(getParam<VariableName>("from_variable"))
{
}

void
SolutionIC::initialSetup()
{
  // can only check blocks when the solution UO uses Exodus
  if (_solution_object.getSolutionFileType() == "exodusII")
  {
    // remap block names this IC is defined on into the ExodusII file block IDs
    const auto & block_names_to_ids_from = _solution_object.getBlockNamesToIds();

    const std::vector<SubdomainID> all_block_ids(meshBlockIDs().begin(), meshBlockIDs().end());
    const auto blocks_to_check =
        blockRestricted() ? blocks() : _sys.mesh().getSubdomainNames(all_block_ids);

    for (auto & blk_name : blocks_to_check)
    {
      auto it = block_names_to_ids_from.find(blk_name);
      if (it != block_names_to_ids_from.end())
        _exo_block_ids.insert(it->second);
      else
      {
        auto blk_id = _sys.mesh().getSubdomainID(blk_name);
        // use the ids, it may be that the source file does not have block names
        // and that we are using the id as the block name (block = 0 for example)
        const auto & block_ids_to_names_from = _solution_object.getBlockIdsToNames();
        if (block_ids_to_names_from.find(blk_id) != block_ids_to_names_from.end() &&
            block_ids_to_names_from.find(blk_id)->second.empty())
          _exo_block_ids.insert(blk_id);
        else
          mooseError("Block '",
                     blk_name,
                     "' does not exist in the file '",
                     _solution_object.getMeshFileName(),
                     "'.");
      }
    }
  }
}

Real
SolutionIC::value(const Point & p)
{
  return _solution_object.pointValue(0., p, _solution_object_var_name, &_exo_block_ids);
}
