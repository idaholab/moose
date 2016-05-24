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
  params.addRequiredParam<unsigned int>("op_num", "Specifies the number of order parameters to create");
  params.addRequiredParam<unsigned int>("dim", "The dimensions of the mesh");
  params.addRequiredParam<std::vector<std::string> >("var_name_base", "Vector that specifies the base name of the variables");
  params.addParam<std::vector<SubdomainName> >("block", "The block id where this variable lives");
  params.addParam<std::vector<OutputName> >("outputs", "Vector of output names where you would like to restrict the output of variable(s) associated with this object");

  return params;
}

MultiAuxVariablesAction::MultiAuxVariablesAction(InputParameters params) :
    AddAuxVariableAction(params)
{
}

/*
The name of the variable is the variable name base followed by two digits.
The first digit in the name is the dimension it applies to:
  0 = x, 1 = y, 2 = z (or some other criteria as user needs).
The second digit in the name is the order parameter it applies to.
*/

void
MultiAuxVariablesAction::act()
{
  MooseEnum order = getParam<MooseEnum>("order");
  MooseEnum family = getParam<MooseEnum>("family");
  unsigned int op_num = getParam<unsigned int>("op_num");
  std::vector<std::string> var_name_base = getParam<std::vector<std::string> >("var_name_base");
  std::set<SubdomainID> blocks = getSubdomainIDs();
  unsigned int dim = getParam<unsigned int>("dim");

  unsigned int size_v = var_name_base.size();

  // Loop through the number of order parameters
  for (unsigned int val = 0; val < size_v; ++val)
    for (unsigned int op = 0; op < op_num; ++op)
      for (unsigned int x = 0; x < dim; ++x)
      {
        //Create variable names
        std::string var_name = var_name_base[val] + Moose::stringify(x) + Moose::stringify(op);
        if (blocks.empty())
          _problem->addAuxVariable(var_name, _fe_type);
        else
          _problem->addAuxVariable(var_name, _fe_type, &blocks);
      }
}
