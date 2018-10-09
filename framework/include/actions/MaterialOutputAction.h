//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALOUTPUTACTION_H
#define MATERIALOUTPUTACTION_H

// MOOSE includes
#include "Action.h"
#include "MaterialData.h"

// Forward declarations
class MaterialOutputAction;
class MooseObjectAction;
class Material;

template <>
InputParameters validParams<MaterialOutputAction>();

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
  MaterialOutputAction(InputParameters params);
  virtual void act() override;

private:
  /**
   * Helper method for testing if the material exists as a block or boundary material
   * @tparam T The property type (e.g., REAL)
   * @param property_name The name of the property to test
   */
  template <typename T>
  bool hasProperty(const std::string & property_name);

  /**
   * Template method for creating the necessary objects for the various material property types
   * @tparam T The type of material property that automatic output is being performed
   * @param property_name The name of the material property to output
   * @param material A pointer to the Material object containing the property of interest
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
                                                const Material & material,
                                                bool get_names_only);

  /**
   * A method for retrieving and partially filling the InputParameters object for an AuxVariable
   * @param type The type of AuxVariable
   * @param property_name The property name to associated with that action
   * @param variable_name The AuxVariable name to create
   * @param material A Material object containing the property of interest
   *
   * @return An InputParameter object with common properties added.
   */
  InputParameters getParams(const std::string & type,
                            const std::string & property_name,
                            const std::string & variable_name,
                            const Material & material);

  /// Pointer the MaterialData object storing the block restricted materials
  std::shared_ptr<MaterialData> _block_material_data;

  /// Pointer the MaterialData object storing the boundary restricted materials
  std::shared_ptr<MaterialData> _boundary_material_data;

  /// Map of variable name that contains the blocks to which the variable should be restricted
  std::map<std::string, std::set<SubdomainID>> _block_variable_map;

  /// List of variables for the current Material object
  std::set<std::string> _material_variable_names;

  /// Map of output names and list of variables associated with the output
  std::map<OutputName, std::set<std::string>> _material_variable_names_map;

  /// Reference to the OutputWarehouse
  OutputWarehouse & _output_warehouse;
};

template <typename T>
std::vector<std::string>
MaterialOutputAction::materialOutputHelper(const std::string & /*property_name*/,
                                           const Material & /*material*/,
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

#endif // MATERIALOUTPUTACTION_H
