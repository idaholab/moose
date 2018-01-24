/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MultiAuxVariablesAction.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<MultiAuxVariablesAction>()
{
  InputParameters params = validParams<AddAuxVariableAction>();
  params.addClassDescription("Set up auxvariables for components of "
                             "MaterialProperty<std::vector<data_type> > for polycrystal sample.");
  params.addRequiredParam<unsigned int>(
      "grain_num", "Specifies the number of grains to create the aux varaivles for.");
  params.addRequiredParam<std::vector<std::string>>(
      "variable_base", "Vector that specifies the base name of the variables.");
  MultiMooseEnum data_type("Real RealGradient", "Real");
  params.addRequiredParam<MultiMooseEnum>(
      "data_type",
      data_type,
      "Specifying data type of the materials property, variables are created accordingly");
  return params;
}

MultiAuxVariablesAction::MultiAuxVariablesAction(InputParameters params)
  : AddAuxVariableAction(params),
    _grain_num(getParam<unsigned int>("grain_num")),
    _var_name_base(getParam<std::vector<std::string>>("variable_base")),
    _num_var(_var_name_base.size()),
    _data_type(getParam<MultiMooseEnum>("data_type")),
    _data_size(_data_type.size())
{
}

void
MultiAuxVariablesAction::act()
{
  if (_num_var != _data_size)
    mooseError("Data type not provided for all the AuxVariables in MultiAuxVariablesAction");

  // Blocks from the input
  std::set<SubdomainID> blocks = getSubdomainIDs();

  // mesh dimension & components required for gradient variables
  const unsigned int dim = _mesh->dimension();
  const std::vector<char> suffix = {'x', 'y', 'z'};

  // Loop through the number of order parameters
  for (unsigned int val = 0; val < _num_var; ++val)
    for (unsigned int gr = 0; gr < _grain_num; ++gr)
    {
      /// for exatrcting data from MaterialProperty<std::vector<Real> >
      if (_data_type[val] == "Real")
      {
        // Create variable names with variable name base followed by the order parameter it applies
        // to.
        std::string var_name = _var_name_base[val] + Moose::stringify(gr);

        if (blocks.empty())
          _problem->addAuxVariable(var_name, _fe_type);
        else
          _problem->addAuxVariable(var_name, _fe_type, &blocks);
      }
      /// for exatrcting data from MaterialProperty<std::vector<RealGradient> >
      if (_data_type[val] == "RealGradient")
        for (unsigned int x = 0; x < dim; ++x)
        {
          /**
           * The name of the variable is the variable name base followed by
           * the order parameter and a suffix mentioning dimension it applies to.
           */
          std::string var_name = _var_name_base[val] + Moose::stringify(gr) + "_" + suffix[x];

          if (blocks.empty())
            _problem->addAuxVariable(var_name, _fe_type);
          else
            _problem->addAuxVariable(var_name, _fe_type, &blocks);
        }
    }
}
