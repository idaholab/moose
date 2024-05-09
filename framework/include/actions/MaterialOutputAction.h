//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "MaterialData.h"
#include "FEProblemBase.h"

class MooseObjectAction;
class MaterialBase;

/**
 * Creates AuxVariables and AuxKernels for automatic output of material properties
 */
class MaterialOutputAction : public Action
{
public:
  static InputParameters validParams();

  MaterialOutputAction(const InputParameters & params);

  virtual void act() override;

  /// Meta data describing the setup of an output object
  struct OutputMetaData
  {
    std::string _aux_kernel_name;
    std::string _index_symbols;
    std::vector<std::string> _param_names;
  };

protected:
  /**
   * A function to be overriden by derived actions to handle a set of material property types
   */
  virtual std::vector<std::string> materialOutput(const std::string & property_name,
                                                  const MaterialBase & material,
                                                  bool get_names_only);

  /// Universal output object setup function
  std::vector<std::string> outputHelper(const OutputMetaData & metadata,
                                        const std::string & property_name,
                                        const std::string & var_name_base,
                                        const MaterialBase & material,
                                        bool get_names_only);

  /**
   * Helper method for testing if the material exists as a block or boundary material
   * @tparam T The property type (e.g., REAL)
   * @param property_name The name of the property to test
   */
  template <typename T>
  bool hasProperty(const std::string & property_name);

  /**
   * Helper method for testing if the material exists as a block or boundary material
   * @tparam T The AD property type (e.g., Real)
   * @param property_name The name of the AD property to test
   */
  template <typename T>
  bool hasADProperty(const std::string & property_name);

  /**
   * Helper method for testing if the functor material property exists
   * @tparam T The functor property type (e.g., Real)
   * @param property_name The name of the property to test
   */
  template <typename T>
  bool hasFunctorProperty(const std::string & property_name);

  /**
   * A method for retrieving and partially filling the InputParameters object for an AuxVariable
   * @param type The type of AuxVariable
   * @param property_name The property name to associated with that action
   * @param variable_name The AuxVariable name to create
   * @param material A MaterialBase object containing the property of interest
   *
   * @return An InputParameter object with common properties added.
   */
  InputParameters getParams(const std::string & type,
                            const std::string & property_name,
                            const std::string & variable_name,
                            const MaterialBase & material);

private:
  /// Pointer the MaterialData object storing the block restricted materials
  const MaterialData * _block_material_data;

  /// Pointer the MaterialData object storing the boundary restricted materials
  const MaterialData * _boundary_material_data;

  /// Map of variable name that contains the blocks to which the variable should be restricted
  std::map<std::string, std::set<SubdomainID>> _block_variable_map;

  /// variables for the current MaterialBase object
  std::set<std::string> _material_variable_names;

  /// Map of output names and list of variables associated with the output
  std::map<OutputName, std::set<std::string>> _material_variable_names_map;

  /// Reference to the OutputWarehouse
  OutputWarehouse & _output_warehouse;

  /// Output only on TIMESTEP_END, not on INITIAL?
  const bool _output_only_on_timestep_end;
};

template <typename T>
bool
MaterialOutputAction::hasProperty(const std::string & property_name)
{
  if (_block_material_data->haveProperty<T>(property_name) ||
      _boundary_material_data->haveProperty<T>(property_name))
    return true;
  else
    return false;
}

template <typename T>
bool
MaterialOutputAction::hasADProperty(const std::string & property_name)
{
  if (_block_material_data->haveADProperty<T>(property_name) ||
      _boundary_material_data->haveADProperty<T>(property_name))
    return true;
  else
    return false;
}

template <typename T>
bool
MaterialOutputAction::hasFunctorProperty(const std::string & property_name)
{
  return _problem->hasFunctorWithType<T>(property_name, 0);
}
