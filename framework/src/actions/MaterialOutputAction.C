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
#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

#include "libmesh/utility.h"

// Declare the output helper specializations
template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<Real>(
    const std::string & material_name, const Material & material, bool get_names_only);

template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<RealVectorValue>(
    const std::string & material_name, const Material & material, bool get_names_only);

template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<RealTensorValue>(
    const std::string & material_name, const Material & material, bool get_names_only);

template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<RankTwoTensor>(
    const std::string & material_name, const Material & material, bool get_names_only);

template <>
std::vector<std::string> MaterialOutputAction::materialOutputHelper<RankFourTensor>(
    const std::string & material_name, const Material & material, bool get_names_only);

registerMooseAction("MooseApp", MaterialOutputAction, "add_output_aux_variables");

registerMooseAction("MooseApp", MaterialOutputAction, "add_aux_kernel");

template <>
InputParameters
validParams<MaterialOutputAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

MaterialOutputAction::MaterialOutputAction(InputParameters params)
  : Action(params), _output_warehouse(_app.getOutputWarehouse())
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

  // A complete list of all Material objects
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
    //   (2) If the Material object itself has set the 'outputs' parameter
    if (outputs_has_properties || outputs.find("none") == outputs.end())
    {
      // Add the material property for output if the name is contained in the 'output_properties'
      // list
      // or if the list is empty (all properties)
      const std::set<std::string> names = mat->getSuppliedItems();
      std::vector<std::string> curr_material_names;
      for (const auto & name : names)
      {
        // Add the material property for output
        if (output_properties.empty() ||
            std::find(output_properties.begin(), output_properties.end(), name) !=
                output_properties.end())
        {
          if (hasProperty<Real>(name))
          {
            curr_material_names = materialOutputHelper<Real>(name, *mat, get_names_only);
            material_names.insert(curr_material_names.begin(), curr_material_names.end());
          }

          else if (hasProperty<RealVectorValue>(name))
          {
            curr_material_names = materialOutputHelper<RealVectorValue>(name, *mat, get_names_only);
            material_names.insert(curr_material_names.begin(), curr_material_names.end());
          }

          else if (hasProperty<RealTensorValue>(name))
          {
            curr_material_names = materialOutputHelper<RealTensorValue>(name, *mat, get_names_only);
            material_names.insert(curr_material_names.begin(), curr_material_names.end());
          }

          else if (hasProperty<RankTwoTensor>(name))
          {
            curr_material_names = materialOutputHelper<RankTwoTensor>(name, *mat, get_names_only);
            material_names.insert(curr_material_names.begin(), curr_material_names.end());
          }

          else if (hasProperty<RankFourTensor>(name))
          {
            curr_material_names = materialOutputHelper<RankFourTensor>(name, *mat, get_names_only);
            material_names.insert(curr_material_names.begin(), curr_material_names.end());
          }

          else
            mooseWarning("The type for material property '",
                         name,
                         "' is not supported for automatic output.");
        }

        // If the material object as limited outputs, store the variables associated with the output
        // objects
        if (!outputs.empty())
          for (const auto & output_name : outputs)
            _material_variable_names_map[output_name].insert(_material_variable_names.begin(),
                                                             _material_variable_names.end());
      }
    }
  }

  if (_current_task == "add_output_aux_variables")
  {

    // Create the AuxVariables
    FEType fe_type(
        CONSTANT,
        MONOMIAL); // currently only elemental variables are support for material property output
    for (const auto & var_name : material_names)
      _problem->addAuxVariable(var_name, fe_type);
  }
  else
  {
    // When a Material object has 'output_properties' defined all other properties not listed must
    // be added to the hide list for the output objects so that properties that are not desired do
    // not appear.
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

InputParameters
MaterialOutputAction::getParams(const std::string & type,
                                const std::string & property_name,
                                const std::string & variable_name,
                                const Material & material)
{
  // Append the list of output variables for the current material
  _material_variable_names.insert(variable_name);

  // Set the action parameters
  InputParameters params = _factory.getValidParams(type);

  params.set<MaterialPropertyName>("property") = property_name;
  params.set<AuxVariableName>("variable") = variable_name;
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

  if (material.boundaryRestricted())
    params.set<std::vector<BoundaryName>>("boundary") = material.boundaryNames();
  else
    params.set<std::vector<SubdomainName>>("block") = material.blocks();

  return params;
}

template <>
std::vector<std::string>
MaterialOutputAction::materialOutputHelper<Real>(const std::string & property_name,
                                                 const Material & material,
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
MaterialOutputAction::materialOutputHelper<RealVectorValue>(const std::string & property_name,
                                                            const Material & material,
                                                            bool get_names_only)
{
  std::array<char, 3> suffix = {{'x', 'y', 'z'}};
  std::vector<std::string> names(3);
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
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
MaterialOutputAction::materialOutputHelper<RealTensorValue>(const std::string & property_name,
                                                            const Material & material,
                                                            bool get_names_only)
{
  std::vector<std::string> names(LIBMESH_DIM * LIBMESH_DIM);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    {
      std::ostringstream oss;
      oss << property_name << "_" << i << j;
      names[i * LIBMESH_DIM + j] = oss.str();

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
                                                          const Material & material,
                                                          bool get_names_only)
{
  std::vector<std::string> names(LIBMESH_DIM * LIBMESH_DIM);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    {
      std::ostringstream oss;
      oss << property_name << "_" << i << j;
      names[i * LIBMESH_DIM + j] = oss.str();

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
MaterialOutputAction::materialOutputHelper<RankFourTensor>(const std::string & property_name,
                                                           const Material & material,
                                                           bool get_names_only)
{
  std::vector<std::string> names(Utility::pow<4>(LIBMESH_DIM));

  unsigned int counter = 0;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
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
