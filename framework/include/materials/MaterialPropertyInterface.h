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
#include "MaterialProperty.h"
#include "MooseTypes.h"
#include "MaterialData.h"
#include "MathUtils.h"
#include "MooseObjectName.h"
#include "InputParameters.h"

// Forward declarations
class MaterialPropertyInterface;
class MooseObject;
class FEProblemBase;

template <>
InputParameters validParams<MaterialPropertyInterface>();

/**
 * \class MaterialPropertyInterface
 * \brief An interface for accessing Materials
 *
 * Any object that needs material properties should inherit this interface.
 * If your object is also restricted to blocks and/or boundaries via the
 * BlockRestrictable and/or BoundaryRestrictable class, then MaterialPropertyInterface
 * must be inherited following these two classes for the material property checks
 * to operate correctly.
 */
class MaterialPropertyInterface
{
public:
  MaterialPropertyInterface(const MooseObject * moose_object,
                            const std::set<SubdomainID> & block_ids,
                            const std::set<BoundaryID> & boundary_ids);

  static InputParameters validParams();

  ///@{
  /**
   * Retrieve reference to material property or one of it's old or older values.
   * The name required by this method is the name that is hard-coded into
   * your source code as the input parameter key. If no input parameter is found
   * this behaves like the getMaterialPropertyByName family as a fall back.
   * @param name The name of the parameter key of the material property to retrieve
   * @return Reference to the desired material property
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name);
  template <typename T>
  const ADMaterialProperty<T> & getADMaterialProperty(const std::string & name);
  template <typename T, bool is_ad, typename std::enable_if<is_ad, int>::type = 0>
  const ADMaterialProperty<T> & getGenericMaterialProperty(const std::string & name)
  {
    return getADMaterialProperty<T>(name);
  }
  template <typename T, bool is_ad, typename std::enable_if<!is_ad, int>::type = 0>
  const MaterialProperty<T> & getGenericMaterialProperty(const std::string & name)
  {
    return getMaterialProperty<T>(name);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);
  ///@}

  ///@{
  /**
   * Retrieve reference to material property or its old or older value
   * The name required by this method is the name defined in the input file.
   * @param name The name of the material property to retrieve
   * @return Reference to the material property with the name 'name'
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyByName(const MaterialPropertyName & name);
  template <typename T>
  const ADMaterialProperty<T> & getADMaterialPropertyByName(const MaterialPropertyName & name);
  template <typename T, bool is_ad, typename std::enable_if<is_ad, int>::type = 0>
  const ADMaterialProperty<T> & getGenericMaterialPropertyByName(const std::string & name)
  {
    return getADMaterialPropertyByName<T>(name);
  }
  template <typename T, bool is_ad, typename std::enable_if<!is_ad, int>::type = 0>
  const MaterialProperty<T> & getGenericMaterialPropertyByName(const std::string & name)
  {
    return getMaterialPropertyByName<T>(name);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOldByName(const MaterialPropertyName & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderByName(const MaterialPropertyName & name);
  ///@}

  /**
   * Retrieve pointer to a material property with the mesh blocks where it is defined
   * The name required by this method is the name defined in the input file.
   * This function can be thought as the combination of getMaterialPropertyByName and
   * getMaterialPropertyBlocks.
   * It can be called after the action of all actions.
   * @param name The name of the material property to retrieve
   * @return Pointer to the material property with the name 'name' and the set of blocks where the
   * property is valid
   */
  template <typename T>
  std::pair<const MaterialProperty<T> *, std::set<SubdomainID>>
  getBlockMaterialProperty(const MaterialPropertyName & name);

  /**
   * Return a material property that is initialized to zero by default and does
   * not need to (but can) be declared by another material.
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> &
  getGenericZeroMaterialProperty(const std::string & name);
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> &
  getGenericZeroMaterialPropertyByName(const std::string & prop_name);

  /**
   * Return a constant zero anonymous material property
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> & getGenericZeroMaterialProperty();

  /// for backwards compatibility
  template <typename T, typename... Ts>
  const MaterialProperty<T> & getZeroMaterialProperty(Ts... args)
  {
    return getGenericZeroMaterialProperty<T, false>(args...);
  }

  /**
   * Retrieve the block ids that the material property is defined
   * @param name The name of the material property
   * @return A vector the the block ids for the property
   */
  std::set<SubdomainID> getMaterialPropertyBlocks(const std::string & name);

  /**
   * Retrieve the block names that the material property is defined
   * @param name The name of the material property
   * @return A vector the the block names for the property
   */
  std::vector<SubdomainName> getMaterialPropertyBlockNames(const std::string & name);

  /**
   * Retrieve the boundary ids that the material property is defined
   * @param name The name of the material property
   * @return A vector the the boundary ids for the property
   */
  std::set<BoundaryID> getMaterialPropertyBoundaryIDs(const std::string & name);

  /**
   * Retrieve the boundary namess that the material property is defined
   * @param name The name of the material property
   * @return A vector the the boundary names for the property
   */
  std::vector<BoundaryName> getMaterialPropertyBoundaryNames(const std::string & name);

  /**
   * Check if block and boundary restrictions of a given material are compatible with the current
   * material. Error out otherwise.
   */
  void checkBlockAndBoundaryCompatibility(std::shared_ptr<MaterialBase> discrete);

  ///@{
  /**
   * Return a MaterialBase reference - usable for computing directly.
   *
   * @param name The name of the input parameter or explicit material name.
   * @param no_warn If true, suppress warning about retrieving the material
   * potentially during its calculation. If you don't know what this is/means,
   * then you don't need it.
   */
  MaterialBase & getMaterial(const std::string & name);
  MaterialBase & getMaterialByName(const std::string & name, bool no_warn = false);
  ///@}

  ///@{
  /**
   * Check if the material property exists
   * @param name the name of the property to query
   * @return true if the property exists, otherwise false
   */
  template <typename T>
  bool hasMaterialProperty(const std::string & name);
  template <typename T>
  bool hasMaterialPropertyByName(const std::string & name);
  template <typename T>
  bool hasADMaterialProperty(const std::string & name);
  template <typename T>
  bool hasADMaterialPropertyByName(const std::string & name);
  ///@}

  ///@{ generic hasMaterialProperty helper
  template <typename T, bool is_ad, typename std::enable_if<is_ad, int>::type = 0>
  bool hasGenericMaterialProperty(const std::string & name)
  {
    return hasADMaterialProperty<T>(name);
  }
  template <typename T, bool is_ad, typename std::enable_if<!is_ad, int>::type = 0>
  bool hasGenericMaterialProperty(const std::string & name)
  {
    return hasMaterialProperty<T>(name);
  }
  template <typename T, bool is_ad, typename std::enable_if<is_ad, int>::type = 0>
  bool hasGenericMaterialPropertyByName(const std::string & name)
  {
    return hasADMaterialPropertyByName<T>(name);
  }
  template <typename T, bool is_ad, typename std::enable_if<!is_ad, int>::type = 0>
  bool hasGenericMaterialPropertyByName(const std::string & name)
  {
    return hasMaterialPropertyByName<T>(name);
  }
  ///@}

  /**
   * Derived classes can declare whether or not they work with
   * stateful material properties.  See, for example, DiracKernel.  By
   * default, they are allowed.
   */
  void statefulPropertiesAllowed(bool);

  /**
   * Returns true if getMaterialProperty() has been called, false otherwise.
   */
  bool getMaterialPropertyCalled() const { return _get_material_property_called; }

  /**
   * Retrieve the set of material properties that _this_ object depends on.
   *
   * @return The IDs corresponding to the material properties that
   * MUST be reinited before evaluating this object
   */
  const std::set<unsigned int> & getMatPropDependencies() const
  {
    return _material_property_dependencies;
  }

protected:
  /// Parameters of the object with this interface
  const InputParameters & _mi_params;

  /// The name of the object that this interface belongs to
  const std::string _mi_name;

  /// The "complete" name of the object that this interface belongs for material property output
  const MooseObjectName _mi_moose_object_name;

  /// The type of data
  Moose::MaterialDataType _material_data_type;

  /// Pointer to the material data class that stores properties
  std::shared_ptr<MaterialData> _material_data;

  /// Reference to the FEProblemBase class
  FEProblemBase & _mi_feproblem;

  /// Current threaded it
  const THREAD_ID _mi_tid;

  /**
   * A helper method for checking material properties
   * This method was required to avoid a compiler problem with the template
   * getMaterialProperty method
   */
  void checkMaterialProperty(const std::string & name);

  /**
   * A proxy method for _mi_feproblem.markMatPropRequested(name)
   */
  void markMatPropRequested(const std::string &);

  /**
   * Small helper to look up a material property name through the input parameter keys
   */
  std::string deducePropertyName(const std::string & name);

  /**
   * Helper function to parse default material property values. This is implemented
   * as a specialization for supported types and returns NULL in all other cases.
   */
  template <typename T>
  const MaterialProperty<T> * defaultMaterialProperty(const std::string & name);

  /**
   * Helper function to parse default material property values. This is implemented
   * as a specialization for supported types and returns NULL in all other cases.
   */
  template <typename T>
  const ADMaterialProperty<T> * defaultADMaterialProperty(const std::string & name);

  ///@{ generic default material property helper
  template <typename T, bool is_ad, typename std::enable_if<is_ad, int>::type = 0>
  const ADMaterialProperty<T> * defaultGenericMaterialProperty(const std::string & name)
  {
    return defaultADMaterialProperty<T>(name);
  }
  template <typename T, bool is_ad, typename std::enable_if<!is_ad, int>::type = 0>
  const MaterialProperty<T> * defaultGenericMaterialProperty(const std::string & name)
  {
    return defaultMaterialProperty<T>(name);
  }
  ///@}

  /**
   * Check and throw an error if the execution has progressed past the construction stage
   */
  void checkExecutionStage();

  /**
   * True by default. If false, this class throws an error if any of
   * the stateful material properties interfaces are used.
   */
  bool _stateful_allowed;

  /**
   * Initialized to false.  Gets set to true when getMaterialProperty()
   * is called.  Clients of this class can inquire whether getMaterialProperty()
   * has been called by calling getMaterialPropertyCalled().
   */
  bool _get_material_property_called;

  /// Storage vector for MaterialProperty<Real> default objects
  std::vector<std::unique_ptr<MaterialProperty<Real>>> _default_real_properties;
  /// Storage vector for ADMaterialProperty<Real> default objects
  std::vector<std::unique_ptr<ADMaterialProperty<Real>>> _default_ad_real_properties;
  /// Storage vector for MaterialProperty<RealVectorValue> default objects
  std::vector<std::unique_ptr<MaterialProperty<RealVectorValue>>> _default_real_vector_properties;
  /// Storage vector for ADMaterialProperty<RealVectorValue> default objects
  std::vector<std::unique_ptr<ADMaterialProperty<RealVectorValue>>>
      _default_ad_real_vector_properties;

  /// The set of material properties (as given by their IDs) that _this_ object depends on
  std::set<unsigned int> _material_property_dependencies;

  const MaterialPropertyName _get_suffix;

private:
  /*
   * A proxy method for _mi_feproblem.getMaxQps()
   */
  unsigned int getMaxQps() const;

  /*
   * A proxy method for _mi_feproblem.addConsumedPropertyName()
   */
  void addConsumedPropertyName(const MooseObjectName & obj_name, const std::string & prop_name);

  /// BoundaryRestricted flag
  const bool _mi_boundary_restricted;

  /// Storage for the block ids created by BlockRestrictable
  const std::set<SubdomainID> & _mi_block_ids;

  /// Storage for the boundary ids created by BoundaryRestrictable
  const std::set<BoundaryID> & _mi_boundary_ids;
};

template <typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const ADMaterialProperty<T> &
MaterialPropertyInterface::getADMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const ADMaterialProperty<T> * default_property = defaultADMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getADMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOld(const std::string & name)
{
  if (!_stateful_allowed)
    mooseError("Stateful material properties not allowed for this object."
               " Old property for \"",
               name,
               "\" was requested.");

  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyOldByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOlder(const std::string & name)
{
  if (!_stateful_allowed)
    mooseError("Stateful material properties not allowed for this object."
               " Older property for \"",
               name,
               "\" was requested.");

  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyOlderByName<T>(prop_name);
}

// General version for types that do not accept default values
template <typename T>
const MaterialProperty<T> *
MaterialPropertyInterface::defaultMaterialProperty(const std::string & /*name*/)
{
  return NULL;
}

// General version for types that do not accept default values
template <typename T>
const ADMaterialProperty<T> *
MaterialPropertyInterface::defaultADMaterialProperty(const std::string & /*name*/)
{
  return NULL;
}

// Forward declare explicit specializations
template <>
const MaterialProperty<Real> *
MaterialPropertyInterface::defaultMaterialProperty<Real>(const std::string & name);

template <>
const ADMaterialProperty<Real> *
MaterialPropertyInterface::defaultADMaterialProperty<Real>(const std::string & name);

template <>
const MaterialProperty<RealVectorValue> *
MaterialPropertyInterface::defaultMaterialProperty<RealVectorValue>(const std::string & name);

template <>
const ADMaterialProperty<RealVectorValue> *
MaterialPropertyInterface::defaultADMaterialProperty<RealVectorValue>(const std::string & name);

template <typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyByName(const MaterialPropertyName & name_in)
{
  const auto name = _get_suffix.empty()
                        ? static_cast<const std::string &>(name_in)
                        : MooseUtils::join(std::vector<std::string>({name_in, _get_suffix}), "_");
  checkExecutionStage();
  checkMaterialProperty(name);

  // mark property as requested
  markMatPropRequested(name);

  // Update the boolean flag.
  _get_material_property_called = true;

  _material_property_dependencies.insert(_material_data->getPropertyId(name));

  // Update consumed properties in MaterialPropertyDebugOutput
  addConsumedPropertyName(_mi_moose_object_name, name);

  return _material_data->getProperty<T>(name);
}

template <typename T>
const ADMaterialProperty<T> &
MaterialPropertyInterface::getADMaterialPropertyByName(const MaterialPropertyName & name_in)
{
  const auto name = _get_suffix.empty()
                        ? static_cast<const std::string &>(name_in)
                        : MooseUtils::join(std::vector<std::string>({name_in, _get_suffix}), "_");
  checkExecutionStage();
  checkMaterialProperty(name);

  // mark property as requested
  markMatPropRequested(name);

  // Update the boolean flag.
  _get_material_property_called = true;

  _material_property_dependencies.insert(_material_data->getPropertyId(name));

  // Update consumed properties in MaterialPropertyDebugOutput
  addConsumedPropertyName(_mi_moose_object_name, name);

  return _material_data->getADProperty<T>(name);
}

template <typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOldByName(const MaterialPropertyName & name_in)
{
  const auto name = _get_suffix.empty()
                        ? static_cast<const std::string &>(name_in)
                        : MooseUtils::join(std::vector<std::string>({name_in, _get_suffix}), "_");

  if (!_stateful_allowed)
    mooseError("Stateful material properties not allowed for this object."
               " Old property for \"",
               name,
               "\" was requested.");

  // mark property as requested
  markMatPropRequested(name);

  _material_property_dependencies.insert(_material_data->getPropertyId(name));

  return _material_data->getPropertyOld<T>(name);
}

template <typename T>
const MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOlderByName(const MaterialPropertyName & name_in)
{
  const auto name = _get_suffix.empty()
                        ? static_cast<const std::string &>(name_in)
                        : MooseUtils::join(std::vector<std::string>({name_in, _get_suffix}), "_");

  if (!_stateful_allowed)
    mooseError("Stateful material properties not allowed for this object."
               " Older property for \"",
               name,
               "\" was requested.");

  // mark property as requested
  markMatPropRequested(name);

  _material_property_dependencies.insert(_material_data->getPropertyId(name));

  return _material_data->getPropertyOlder<T>(name);
}

template <typename T>
std::pair<const MaterialProperty<T> *, std::set<SubdomainID>>
MaterialPropertyInterface::getBlockMaterialProperty(const MaterialPropertyName & name_in)
{
  const auto name = _get_suffix.empty()
                        ? static_cast<const std::string &>(name_in)
                        : MooseUtils::join(std::vector<std::string>({name_in, _get_suffix}), "_");

  if (_mi_block_ids.empty())
    mooseError("getBlockMaterialProperty must be called by a block restrictable object");

  if (!hasMaterialPropertyByName<T>(name))
    return std::pair<const MaterialProperty<T> *, std::set<SubdomainID>>(NULL,
                                                                         std::set<SubdomainID>());

  _material_property_dependencies.insert(_material_data->getPropertyId(name));

  // Update consumed properties in MaterialPropertyDebugOutput
  addConsumedPropertyName(_mi_moose_object_name, name);

  return std::pair<const MaterialProperty<T> *, std::set<SubdomainID>>(
      &_material_data->getProperty<T>(name), getMaterialPropertyBlocks(name));
}

template <typename T>
bool
MaterialPropertyInterface::hasMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);
  return hasMaterialPropertyByName<T>(prop_name);
}

template <typename T>
bool
MaterialPropertyInterface::hasMaterialPropertyByName(const std::string & name_in)
{
  const auto name = _get_suffix.empty()
                        ? name_in
                        : MooseUtils::join(std::vector<std::string>({name_in, _get_suffix}), "_");
  return _material_data->haveProperty<T>(name);
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
MaterialPropertyInterface::getGenericZeroMaterialProperty(const std::string & name)
{
  std::string prop_name = deducePropertyName(name);
  return getGenericZeroMaterialPropertyByName<T, is_ad>(prop_name);
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
MaterialPropertyInterface::getGenericZeroMaterialPropertyByName(const std::string & prop_name)
{
  // if found return the requested property
  if (hasGenericMaterialPropertyByName<T, is_ad>(prop_name))
    return getGenericMaterialPropertyByName<T, is_ad>(prop_name);

  // static zero property storage
  static GenericMaterialProperty<T, is_ad> zero;

  // resize to accomodate maximum number of qpoints
  // (in multiapp scenarios getMaxQps can return different values in each app; we need the max)
  unsigned int nqp = getMaxQps();
  if (nqp > zero.size())
    zero.resize(nqp);

  // set values for all qpoints to zero
  for (unsigned int qp = 0; qp < nqp; ++qp)
    MathUtils::mooseSetToZero(zero[qp]);

  return zero;
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
MaterialPropertyInterface::getGenericZeroMaterialProperty()
{
  // static zero property storage
  static GenericMaterialProperty<T, is_ad> zero;

  // resize to accomodate maximum number of qpoints
  // (in multiapp scenarios getMaxQps can return different values in each app; we need the max)
  unsigned int nqp = getMaxQps();
  if (nqp > zero.size())
    zero.resize(nqp);

  // set values for all qpoints to zero
  for (unsigned int qp = 0; qp < nqp; ++qp)
    MathUtils::mooseSetToZero(zero[qp]);

  return zero;
}

template <typename T>
bool
MaterialPropertyInterface::hasADMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);
  return hasADMaterialPropertyByName<T>(prop_name);
}

template <typename T>
bool
MaterialPropertyInterface::hasADMaterialPropertyByName(const std::string & name_in)
{
  const auto name = _get_suffix.empty()
                        ? name_in
                        : MooseUtils::join(std::vector<std::string>({name_in, _get_suffix}), "_");
  return _material_data->haveADProperty<T>(name);
}
