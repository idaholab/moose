//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MaterialOutputAction.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "AddOutputAction.h"
#include "MaterialBase.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "MooseEnum.h"
#include "MooseVariableConstMonomial.h"

#include "libmesh/utility.h"

// Declare the output helper specializations
template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<Real>(
    const std::string & material_name, const MaterialBase & material, bool get_names_only);

template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<ADReal>(
    const std::string & material_name, const MaterialBase & material, bool get_names_only);

template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<RealVectorValue>(
    const std::string & material_name, const MaterialBase & material, bool get_names_only);

template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<ADRealVectorValue>(
    const std::string & material_name, const MaterialBase & material, bool get_names_only);

template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<RealTensorValue>(
    const std::string & material_name, const MaterialBase & material, bool get_names_only);

template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<RankTwoTensor>(
    const std::string & material_name, const MaterialBase & material, bool get_names_only);

template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<ADRankTwoTensor>(
    const std::string & material_name, const MaterialBase & material, bool get_names_only);

template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<RankFourTensor>(
    const std::string & material_name, const MaterialBase & material, bool get_names_only);

registerMooseAction("MooseApp", MaterialOutputAction, "add_output_aux_variables");

registerMooseAction("MooseApp", MaterialOutputAction, "add_aux_kernel");

InputParameters
MaterialOutputAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

MaterialOutputAction::MaterialOutputAction(const InputParameters & params)
  : Action(params),
    _output_warehouse(_app.getOutputWarehouse()),
    _output_only_on_timestep_end(_app.parameters().get<bool>("use_legacy_material_output"))
{
}

void
MaterialOutputAction::act()
{
  mooseAssert(_problem,
              "FEProblemBase pointer is nullptr, it is needed for auto material property output");

  // Do nothing if the application does not have output
  if (!_app.actionWarehouse().hasActions("add_output"))
    return;

  bool get_names_only = _current_task == "add_output_aux_variables" ? true : false;

  // Set the pointers to the MaterialData objects (Note, these pointers are not available at
  // construction)
  _block_material_data = _problem->getMaterialData(Moose::BLOCK_MATERIAL_DATA);
  _boundary_material_data = _problem->getMaterialData(Moose::BOUNDARY_MATERIAL_DATA);

  // A complete list of all MaterialBase objects
  const auto & material_ptrs = _problem->getMaterialWarehouse().getObjects();

  // Handle setting of material property output in [Outputs] sub-blocks
  // Output objects can enable material property output, the following code examines the parameters
  // for each Output object and sets a flag if any Output object has output set and also builds a
  // list if the
  // properties are limited via the 'show_material_properties' parameters
  bool outputs_has_properties = false;
  std::set<std::string> output_object_properties;

  const auto & output_actions = _app.actionWarehouse().getActionListByName("add_output");
  for (const auto & act : output_actions)
  {
    // Extract the Output action
    AddOutputAction * action = dynamic_cast<AddOutputAction *>(act);
    if (!action)
      continue;

    // Add the material property names from the output object parameters to the list of properties
    // to output
    InputParameters & params = action->getObjectParams();
    if (params.isParamValid("output_material_properties") &&
        params.get<bool>("output_material_properties"))
    {
      outputs_has_properties = true;
      std::vector<std::string> prop_names =
          params.get<std::vector<std::string>>("show_material_properties");
      output_object_properties.insert(prop_names.begin(), prop_names.end());
    }
  }

  // Loop through each material object
  std::set<std::string> material_names;
  std::set<std::string> unsupported_names;
  for (const auto & mat : material_ptrs)
  {
    // Extract the names of the output objects to which the material properties will be exported
    std::set<OutputName> outputs = mat->getOutputs();

    // Extract the property names that will actually be output
    std::vector<std::string> output_properties =
        mat->getParam<std::vector<std::string>>("output_properties");

    // Append the properties listed in the Outputs block
    if (outputs_has_properties)
      output_properties.insert(output_properties.end(),
                               output_object_properties.begin(),
                               output_object_properties.end());

    // Clear the list of variable names for the current material object, this list will be populated
    // with all the
    // variables names for the current material object and is needed for purposes of controlling the
    // which output objects
    // show the material property data
    _material_variable_names.clear();

    // Create necessary outputs for the properties if:
    //   (1) The Outputs block has material output enabled
    //   (2) If the MaterialBase object itself has set the 'outputs' parameter
    if (outputs_has_properties || outputs.find("none") == outputs.end())
    {
      // Add the material property for output if the name is contained in the 'output_properties'
      // list
      // or if the list is empty (all properties)
      const std::set<std::string> names = mat->getSuppliedItems();
      for (const auto & name : names)
      {
        // Add the material property for output
        if (output_properties.empty() ||
            std::find(output_properties.begin(), output_properties.end(), name) !=
                output_properties.end())
        {
          auto curr_material_names = materialOutput(name, *mat, get_names_only);
          if (curr_material_names.size() == 0)
            unsupported_names.insert(name);
          material_names.insert(curr_material_names.begin(), curr_material_names.end());
        }

        // If the material object has limited outputs, store the variables associated with the
        // output objects
        if (!outputs.empty())
          for (const auto & output_name : outputs)
            _material_variable_names_map[output_name].insert(_material_variable_names.begin(),
                                                             _material_variable_names.end());
      }
    }
  }
  if (unsupported_names.size() > 0 && get_names_only)
  {
    std::ostringstream oss;
    for (const auto & name : unsupported_names)
      oss << "\n  " << name;
    mooseWarning("The types for total ",
                 unsupported_names.size(),
                 " material properties:",
                 oss.str(),
                 "\nare not supported for automatic output by ",
                 type(),
                 ".");
  }

  if (_current_task == "add_output_aux_variables")
  {
    auto params = _factory.getValidParams("MooseVariableConstMonomial");
    // currently only elemental variables are supported for material property output
    params.set<MooseEnum>("order") = "CONSTANT";
    params.set<MooseEnum>("family") = "MONOMIAL";

    // Create the AuxVariables
    std::ostringstream oss;
    for (const auto & var_name : material_names)
    {
      oss << "\n  " << var_name;
      if (_problem->hasVariable(var_name))
        mooseError("The material property output " + var_name +
                   " has the same name as an existing variable, either use the material"
                   " declare_suffix parameter to disambiguate or the output_properties parameter"
                   " to restrict the material properties to output");
      _problem->addAuxVariable("MooseVariableConstMonomial", var_name, params);
    }
    if (material_names.size() > 0)
      _console << COLOR_CYAN << "The following total " << material_names.size()
               << " aux variables:" << oss.str() << "\nare added for automatic output by " << type()
               << "." << COLOR_DEFAULT << std::endl;
  }
  else
  {
    // When a MaterialBase object has 'output_properties' defined all other properties not listed
    // must be added to the hide list for the output objects so that properties that are not desired
    // do not appear.
    for (const auto & it : _material_variable_names_map)
    {
      std::set<std::string> hide;
      std::set_difference(material_names.begin(),
                          material_names.end(),
                          it.second.begin(),
                          it.second.end(),
                          std::inserter(hide, hide.begin()));

      _output_warehouse.addInterfaceHideVariables(it.first, hide);
    }
  }
}

std::vector<std::string>
MaterialOutputAction::materialOutput(const std::string & property_name,
                                     const MaterialBase & material,
                                     bool get_names_only)
{
  std::vector<std::string> names;
  if (hasProperty<Real>(property_name))
    names = materialOutputHelper<Real>(property_name, material, get_names_only);

  else if (hasADProperty<Real>(property_name))
    names = materialOutputHelper<ADReal>(property_name, material, get_names_only);

  else if (hasProperty<RealVectorValue>(property_name))
    names = materialOutputHelper<RealVectorValue>(property_name, material, get_names_only);

  else if (hasADProperty<RealVectorValue>(property_name))
    names = materialOutputHelper<ADRealVectorValue>(property_name, material, get_names_only);

  else if (hasProperty<RealTensorValue>(property_name))
    names = materialOutputHelper<RealTensorValue>(property_name, material, get_names_only);

  else if (hasProperty<RankTwoTensor>(property_name))
    names = materialOutputHelper<RankTwoTensor>(property_name, material, get_names_only);

  else if (hasADProperty<RankTwoTensor>(property_name))
    names = materialOutputHelper<ADRankTwoTensor>(property_name, material, get_names_only);

  else if (hasProperty<RankFourTensor>(property_name))
    names = materialOutputHelper<RankFourTensor>(property_name, material, get_names_only);

  return names;
}

InputParameters
MaterialOutputAction::getParams(const std::string & type,
                                const std::string & property_name,
                                const std::string & variable_name,
                                const MaterialBase & material)
{
  // Append the list of output variables for the current material
  _material_variable_names.insert(variable_name);

  // Set the action parameters
  InputParameters params = _factory.getValidParams(type);

  params.set<MaterialPropertyName>("property") = property_name;
  params.set<AuxVariableName>("variable") = variable_name;
  if (_output_only_on_timestep_end)
    params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  else
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};

  if (material.boundaryRestricted())
    params.set<std::vector<BoundaryName>>("boundary") = material.boundaryNames();
  else
    params.set<std::vector<SubdomainName>>("block") = material.blocks();

  return params;
}

template <>
std::vector<std::string>
MaterialOutputAction::materialOutputHelper<Real>(const std::string & property_name,
                                                 const MaterialBase & material,
                                                 bool get_names_only)
{
  std::vector<std::string> names = {property_name};
  if (!get_names_only)
  {
    auto params = getParams("MaterialRealAux", property_name, property_name, material);
    _problem->addAuxKernel("MaterialRealAux", material.name() + property_name, params);
  }

  return names;
}

template <>
std::vector<std::string>
MaterialOutputAction::materialOutputHelper<ADReal>(const std::string & property_name,
                                                   const MaterialBase & material,
                                                   bool get_names_only)
{
  std::vector<std::string> names = {property_name};
  if (!get_names_only)
  {
    auto params = getParams("ADMaterialRealAux", property_name, property_name, material);
    _problem->addAuxKernel("ADMaterialRealAux", material.name() + property_name, params);
  }

  return names;
}

template <>
std::vector<std::string>
MaterialOutputAction::materialOutputHelper<RealVectorValue>(const std::string & property_name,
                                                            const MaterialBase & material,
                                                            bool get_names_only)
{
  std::array<char, 3> suffix = {{'x', 'y', 'z'}};
  std::vector<std::string> names(3);
  for (const auto i : make_range(Moose::dim))
  {
    std::ostringstream oss;
    oss << property_name << "_" << suffix[i];
    names[i] = oss.str();

    if (!get_names_only)
    {
      auto params = getParams("MaterialRealVectorValueAux", property_name, oss.str(), material);
      params.set<unsigned int>("component") = i;
      _problem->addAuxKernel("MaterialRealVectorValueAux", material.name() + oss.str(), params);
    }
  }

  return names;
}

template <>
std::vector<std::string>
MaterialOutputAction::materialOutputHelper<ADRealVectorValue>(const std::string & property_name,
                                                              const MaterialBase & material,
                                                              bool get_names_only)
{
  std::array<char, 3> suffix = {{'x', 'y', 'z'}};
  std::vector<std::string> names(3);
  for (const auto i : make_range(Moose::dim))
  {
    std::ostringstream oss;
    oss << property_name << "_" << suffix[i];
    names[i] = oss.str();

    if (!get_names_only)
    {
      auto params = getParams("ADMaterialRealVectorValueAux", property_name, oss.str(), material);
      params.set<unsigned int>("component") = i;
      _problem->addAuxKernel("ADMaterialRealVectorValueAux", material.name() + oss.str(), params);
    }
  }

  return names;
}

template <>
std::vector<std::string>
MaterialOutputAction::materialOutputHelper<RealTensorValue>(const std::string & property_name,
                                                            const MaterialBase & material,
                                                            bool get_names_only)
{
  std::vector<std::string> names(Moose::dim * Moose::dim);

  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
    {
      std::ostringstream oss;
      oss << property_name << "_" << i << j;
      names[i * Moose::dim + j] = oss.str();

      if (!get_names_only)
      {
        auto params = getParams("MaterialRealTensorValueAux", property_name, oss.str(), material);
        params.set<unsigned int>("row") = i;
        params.set<unsigned int>("column") = j;
        _problem->addAuxKernel("MaterialRealTensorValueAux", material.name() + oss.str(), params);
      }
    }

  return names;
}

template <>
std::vector<std::string>
MaterialOutputAction::materialOutputHelper<RankTwoTensor>(const std::string & property_name,
                                                          const MaterialBase & material,
                                                          bool get_names_only)
{
  std::vector<std::string> names(Moose::dim * Moose::dim);

  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
    {
      std::ostringstream oss;
      oss << property_name << "_" << i << j;
      names[i * Moose::dim + j] = oss.str();

      if (!get_names_only)
      {
        auto params = getParams("MaterialRankTwoTensorAux", property_name, oss.str(), material);
        params.set<unsigned int>("i") = i;
        params.set<unsigned int>("j") = j;
        _problem->addAuxKernel("MaterialRankTwoTensorAux", material.name() + oss.str(), params);
      }
    }

  return names;
}

template <>
std::vector<std::string>
MaterialOutputAction::materialOutputHelper<ADRankTwoTensor>(const std::string & property_name,
                                                            const MaterialBase & material,
                                                            bool get_names_only)
{
  std::vector<std::string> names(Moose::dim * Moose::dim);

  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
    {
      std::ostringstream oss;
      oss << property_name << "_" << i << j;
      names[i * Moose::dim + j] = oss.str();

      if (!get_names_only)
      {
        auto params = getParams("ADMaterialRankTwoTensorAux", property_name, oss.str(), material);
        params.set<unsigned int>("i") = i;
        params.set<unsigned int>("j") = j;
        _problem->addAuxKernel("ADMaterialRankTwoTensorAux", material.name() + oss.str(), params);
      }
    }

  return names;
}

template <>
std::vector<std::string>
MaterialOutputAction::materialOutputHelper<RankFourTensor>(const std::string & property_name,
                                                           const MaterialBase & material,
                                                           bool get_names_only)
{
  std::vector<std::string> names(Utility::pow<4>(Moose::dim));

  unsigned int counter = 0;
  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto k : make_range(Moose::dim))
        for (const auto l : make_range(Moose::dim))
        {
          std::ostringstream oss;
          oss << property_name << "_" << i << j << k << l;
          names[counter++] = oss.str();

          if (!get_names_only)
          {
            auto params =
                getParams("MaterialRankFourTensorAux", property_name, oss.str(), material);
            params.set<unsigned int>("i") = i;
            params.set<unsigned int>("j") = j;
            params.set<unsigned int>("k") = k;
            params.set<unsigned int>("l") = l;
            _problem->addAuxKernel(
                "MaterialRankFourTensorAux", material.name() + oss.str(), params);
          }
        }

  return names;
}
