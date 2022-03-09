//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionInitialCondition.h"
#include "SolutionUserObject.h"
#include "MooseMesh.h"

registerMooseObject("ThermalHydraulicsApp", SolutionInitialCondition);

InputParameters
SolutionInitialCondition::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredParam<UserObjectName>("solution_uo",
                                          "The SolutionUserObject to extract data from.");
  params.addRequiredParam<VariableName>(
      "from_variable", "The name of the variable in the file that is to be extracted");
  return params;
}

SolutionInitialCondition::SolutionInitialCondition(const InputParameters & parameters)
  : InitialCondition(parameters),
    _solution_object(getUserObject<SolutionUserObject>("solution_uo")),
    _solution_object_var_name(getParam<VariableName>("from_variable"))
{
}

void
SolutionInitialCondition::initialSetup()
{
  // remap block names this IC is defined on into the ExodusII file block IDs
  const std::map<SubdomainName, SubdomainID> & block_names_to_ids_from =
      _solution_object.getBlockNamesToIds();
  for (auto & blk_name : blocks())
  {
    auto jt = block_names_to_ids_from.find(blk_name);
    if (jt != block_names_to_ids_from.end())
      _exo_block_ids.insert(jt->second);
    else
      mooseError("Block '",
                 blk_name,
                 "' does not exist in the file '",
                 _solution_object.getMeshFileName(),
                 "'.");
  }
}

Real
SolutionInitialCondition::value(const Point & p)
{
  return _solution_object.pointValue(0., p, _solution_object_var_name, &_exo_block_ids);
}
