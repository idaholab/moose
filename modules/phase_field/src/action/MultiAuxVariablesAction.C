/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MultiAuxVariablesAction.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<MultiAuxVariablesAction>()
{
  MooseEnum familyEnum = AddAuxVariableAction::getAuxVariableFamilies();
  MooseEnum orderEnum = AddAuxVariableAction::getAuxVariableOrders();

  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up auxvariables for a polycrystal sample");
  params.addParam<MooseEnum>("family", familyEnum, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order", orderEnum, "Specifies the order of the FE shape function to use for this variable");
  params.addRequiredParam<std::vector<unsigned int> >("op_num", "Vector that specifies the number of order parameters to create");
  params.addRequiredParam<std::vector<std::string> >("var_name_base", "Vector that specifies the base name of the variables");
  params.addParam<std::vector<SubdomainName> >("block", "The block id where this variable lives");
  params.addParam<std::vector<OutputName> >("outputs", "Vector of output names where you would like to restrict the output of variable(s) associated with this object");

  return params;
}

MultiAuxVariablesAction::MultiAuxVariablesAction(InputParameters params) :
    AddAuxVariableAction(params)
{
}

void
MultiAuxVariablesAction::act()
{
  MooseEnum order = getParam<MooseEnum>("order");
  MooseEnum family = getParam<MooseEnum>("family");
  std::vector<unsigned int> op_num = getParam<std::vector<unsigned int> >("op_num");
  std::vector<std::string> var_name_base = getParam<std::vector<std::string> >("var_name_base");
  std::set<SubdomainID> blocks = getSubdomainIDs();

  unsigned int size_o = op_num.size();
  unsigned int size_v = var_name_base.size();

  if (size_o != size_v)
    mooseError("op_num and var_name_base must be vectors of the same size");


  // Loop through the number of order parameters
  for (unsigned int val = 0; val < size_o; ++val)
    for (unsigned int op = 0; op < op_num[val]; ++op)
    {
      //Create variable names
      std::string var_name = var_name_base[val] + Moose::stringify(op);
      if (blocks.empty())
        _problem->addAuxVariable(var_name, _fe_type);
      else
        _problem->addAuxVariable(var_name, _fe_type, &blocks);
    }
}
