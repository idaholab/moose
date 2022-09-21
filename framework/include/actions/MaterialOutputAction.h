//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Action.h"
#include "MaterialData.h"

class MooseObjectAction;
class MaterialBase;

/**
 * Creates AuxVariables and AuxKernels for automatic output of material properties
 */
class MaterialOutputAction : public Action
{
public:
  /**
   * Class constructor
   * @param params Input parameters for this action object
   */
  static InputParameters validParams();

  MaterialOutputAction(const InputParameters & params);
  virtual void act() override;

protected:
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
   * A function to be overriden by derived actions to handle a set of material property types
   */
  virtual std::vector<std::string> materialOutput(const std::string & property_name,
                                                  const MaterialBase & material,
                                                  bool get_names_only);

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
  /**
   * Template method for creating the necessary objects for the various material property types
   * @tparam T The type of material property that automatic output is being performed
   * @param property_name The name of the material property to output
   * @param material A pointer to the MaterialBase object containing the property of interest
   * @param get_names_only A bool used to indicate that only the variable names should be returned
   *
   * @return A vector of names that can be used as AuxVariable names
   *
   * By default this function produces an mooseError, you must create a specialization for any type
   * that you wish to have the automatic output capability. Also, you need to add a test for this
   * type within the act() method.
   */
  template <typename T>
  std::vector<std::string> materialOutputHelper(const std::string & property_name,
                                                const MaterialBase & material,
                                                bool get_names_only);

  /// Pointer the MaterialData object storing the block restricted materials
  std::shared_ptr<MaterialData> _block_material_data;

  /// Pointer the MaterialData object storing the boundary restricted materials
  std::shared_ptr<MaterialData> _boundary_material_data;

  /// Map of variable name that contains the blocks to which the variable should be restricted
  std::map<std::string, std::set<SubdomainID>> _block_variable_map;

  /// List of variables for the current MaterialBase object
  std::set<std::string> _material_variable_names;

  /// Map of output names and list of variables associated with the output
  std::map<OutputName, std::set<std::string>> _material_variable_names_map;

  /// Reference to the OutputWarehouse
  OutputWarehouse & _output_warehouse;

  /// Output only on TIMESTEP_END, not on INITIAL?
  const bool _output_only_on_timestep_end;
};

template <typename T>
std::vector<std::string>
MaterialOutputAction::materialOutputHelper(const std::string & /*property_name*/,
                                           const MaterialBase & /*material*/,
                                           bool /*get_names_only*/)
{
  mooseError("Unknown type, you must create a specialization of materialOutputHelper");
}

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
