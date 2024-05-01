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
#include "FEProblemBase.h"
#include "SerialAccess.h"

class MooseObjectAction;
class MaterialBase;

struct MaterialOutputActionDescriptor
{
};

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

  /// output function called from SetupOutput::apply (through Moose::typeLoop)
  template <typename T, int I>
  void setupOutput(const MaterialPropertyName & prop_name, const MaterialBase & material);

protected:
  template <typename T, int I, bool is_ad, bool is_functor>
  void setupOutputHelper(const MaterialPropertyName & prop_name, const MaterialBase & material);

  /// List of supported raw types (equivalent AD types are also supported)
  typedef Moose::TypeList<Real,
                          RealVectorValue,
                          RealTensorValue,
                          RankTwoTensor,
                          RankFourTensor,
                          SymmetricRankTwoTensor,
                          SymmetricRankFourTensor>
      SupportedTypes;

  /// List of AuxKernels used for the respective property type output ('AD' prefix is added automatically for is_ad)
  static const std::vector<std::string> _aux_kernel_names;

  /// List of index symbols
  static const std::vector<std::string> _index_symbols;

  /// List of coefficient parameter names
  static const std::vector<std::vector<std::string>> _param_names;

  template <typename T, int I>
  struct SetupOutput
  {
    static void apply(MaterialOutputAction * self,
                      const MaterialPropertyName & prop_name,
                      const MaterialBase & material)
    {
      self->setupOutput<T, I>(prop_name, material);
    }
  };

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
  template <bool is_functor>
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

  /// all variables added by this action
  std::set<std::string> _all_variable_names;

  /// property names we succeeded to set output up for
  std::set<std::string> _supported_properties;

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
  // functors support a limited set of types
  if constexpr (std::is_same_v<T, Real> || std::is_same_v<T, RealVectorValue>)
    return _problem->hasFunctorWithType<T>(property_name, 0);
  else
    return false;
}
