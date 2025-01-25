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
#include "FEProblemBase.h"
#include "MooseApp.h"
#include "AddOutputAction.h"
#include "MaterialBase.h"
#include "RankTwoTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankFourTensor.h"
#include "MooseEnum.h"
#include "MooseVariableConstMonomial.h"
#include "FunctorMaterial.h"

#include "libmesh/utility.h"

registerMooseAction("MooseApp", MaterialOutputAction, "add_output_aux_variables");
registerMooseAction("MooseApp", MaterialOutputAction, "add_aux_kernel");

InputParameters
MaterialOutputAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Outputs material properties to various Outputs objects, based on the "
                             "parameters set in each Material");
  /// A flag to tell this action whether or not to print the unsupported properties
  /// Note: A derived class can set this to false, override materialOutput and output
  ///       a particular property that is not supported by this class.
  params.addPrivateParam("print_unsupported_prop_names", true);
  params.addParam<bool>("print_automatic_aux_variable_creation",
                        true,
                        "Flag to print list of aux variables created for automatic output by "
                        "MaterialOutputAction.");
  return params;
}

MaterialOutputAction::MaterialOutputAction(const InputParameters & params)
  : Action(params),
    _block_material_data(nullptr),
    _boundary_material_data(nullptr),
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
  _block_material_data = &_problem->getMaterialData(Moose::BLOCK_MATERIAL_DATA);
  _boundary_material_data = &_problem->getMaterialData(Moose::BOUNDARY_MATERIAL_DATA);

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
      // Get all material properties supplied by this material as a starting point
      std::set<std::string> names = mat->getSuppliedItems();
      if (const auto fmat_ptr = dynamic_cast<const FunctorMaterial *>(mat.get()))
        names.insert(fmat_ptr->getSuppliedFunctors().begin(),
                     fmat_ptr->getSuppliedFunctors().end());

      for (const auto & name : names)
      {
        // Output the property only if the name is contained in the 'output_properties'
        // list or if the list is empty (all properties)
        if (output_properties.empty() ||
            std::find(output_properties.begin(), output_properties.end(), name) !=
                output_properties.end())
        {
          // Add the material property for output
          auto curr_material_names = materialOutput(name, *mat, get_names_only);
          if (curr_material_names.size() == 0)
            unsupported_names.insert(name);
          material_names.insert(curr_material_names.begin(), curr_material_names.end());
        }
      }
      // If the material object has limited outputs, store the variables associated with the
      // output objects
      if (!outputs.empty())
        for (const auto & output_name : outputs)
          _material_variable_names_map[output_name].insert(_material_variable_names.begin(),
                                                           _material_variable_names.end());
    }
  }
  if (unsupported_names.size() > 0 && get_names_only &&
      getParam<bool>("print_unsupported_prop_names"))
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

    if (material_names.size() > 0 && getParam<bool>("print_automatic_aux_variable_creation"))
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

  // Material Properties
  if (hasProperty<Real>(property_name))
    names = outputHelper(
        {"MaterialRealAux", "", {}}, property_name, property_name, material, get_names_only);

  else if (hasADProperty<Real>(property_name))
    names = outputHelper(
        {"ADMaterialRealAux", "", {}}, property_name, property_name, material, get_names_only);

  else if (hasProperty<RealVectorValue>(property_name))
    names = outputHelper({"MaterialRealVectorValueAux", "xyz", {"component"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  else if (hasADProperty<RealVectorValue>(property_name))
    names = outputHelper({"ADMaterialRealVectorValueAux", "xyz", {"component"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  else if (hasProperty<RealTensorValue>(property_name))
    names = outputHelper({"MaterialRealTensorValueAux", "012", {"row", "column"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  else if (hasADProperty<RealTensorValue>(property_name))
    names = outputHelper({"ADMaterialRealTensorValueAux", "012", {"row", "column"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  else if (hasProperty<RankTwoTensor>(property_name))
    names = outputHelper({"MaterialRankTwoTensorAux", "012", {"i", "j"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  else if (hasADProperty<RankTwoTensor>(property_name))
    names = outputHelper({"ADMaterialRankTwoTensorAux", "012", {"i", "j"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  else if (hasProperty<RankFourTensor>(property_name))
    names = outputHelper({"MaterialRankFourTensorAux", "012", {"i", "j", "k", "l"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  else if (hasADProperty<RankFourTensor>(property_name))
    names = outputHelper({"ADMaterialRankFourTensorAux", "012", {"i", "j", "k", "l"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  else if (hasProperty<SymmetricRankTwoTensor>(property_name))
    names = outputHelper({"MaterialSymmetricRankTwoTensorAux", "012345", {"component"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  else if (hasADProperty<SymmetricRankTwoTensor>(property_name))
    names = outputHelper({"ADMaterialSymmetricRankTwoTensorAux", "012345", {"component"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  else if (hasProperty<SymmetricRankFourTensor>(property_name))
    names = outputHelper({"MaterialSymmetricRankFourTensorAux", "012345", {"i", "j"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  else if (hasADProperty<SymmetricRankFourTensor>(property_name))
    names = outputHelper({"ADMaterialSymmetricRankFourTensorAux", "012345", {"i", "j"}},
                         property_name,
                         property_name + "_",
                         material,
                         get_names_only);

  // Functors
  else if (hasFunctorProperty<Real>(property_name))
    names = outputHelper({"FunctorMaterialRealAux", "", {}},
                         property_name,
                         property_name + "_out",
                         material,
                         get_names_only);

  else if (hasFunctorProperty<ADReal>(property_name))
    names = outputHelper({"ADFunctorMaterialRealAux", "", {}},
                         property_name,
                         property_name + "_out",
                         material,
                         get_names_only);

  else if (hasFunctorProperty<RealVectorValue>(property_name))
    names = outputHelper({"FunctorMaterialRealVectorValueAux", "xyz", {"component"}},
                         property_name,
                         property_name + "_out_",
                         material,
                         get_names_only);

  else if (hasFunctorProperty<ADRealVectorValue>(property_name))
    names = outputHelper({"ADFunctorMaterialRealVectorValueAux", "xyz", {"component"}},
                         property_name,
                         property_name + "_out_",
                         material,
                         get_names_only);

  return names;
}

std::vector<std::string>
MaterialOutputAction::outputHelper(const MaterialOutputAction::OutputMetaData & metadata,
                                   const std::string & property_name,
                                   const std::string & var_name_base,
                                   const MaterialBase & material,
                                   bool get_names_only)
{
  const auto & [kernel_name, index_symbols, param_names] = metadata;
  const auto dim = param_names.size();
  const auto size = index_symbols.size();

  std::vector<std::string> names;
  // general 0 to 4 dimensional loop
  std::array<std::size_t, 4> i;
  for (i[3] = 0; i[3] < (dim < 4 ? 1 : size); ++i[3])
    for (i[2] = 0; i[2] < (dim < 3 ? 1 : size); ++i[2])
      for (i[1] = 0; i[1] < (dim < 2 ? 1 : size); ++i[1])
        for (i[0] = 0; i[0] < (dim < 1 ? 1 : size); ++i[0])
        {
          std::string var_name = var_name_base;
          for (const auto j : make_range(dim))
            var_name += Moose::stringify(index_symbols[i[j]]);
          names.push_back(var_name);

          if (!get_names_only)
          {
            auto params = getParams(kernel_name, property_name, var_name, material);
            for (const auto j : make_range(dim))
              params.template set<unsigned int>(param_names[j]) = i[j];
            _problem->addAuxKernel(kernel_name, material.name() + var_name, params);
          }
        }
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
  if (params.have_parameter<MaterialPropertyName>("property"))
    params.set<MaterialPropertyName>("property") = property_name;
  else if (params.have_parameter<MooseFunctorName>("functor"))
    params.set<MooseFunctorName>("functor") = property_name;
  else
    mooseError("Internal error. AuxKernel has neither a `functor` nor a `property` parameter.");

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
