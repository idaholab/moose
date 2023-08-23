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
#include "SubProblem.h"

// Forward declarations
class MooseObject;
class FEProblemBase;

/**
 * Helper class for deferred getting of material properties after the construction
 * phase for materials. This enables "optional material properties" in materials.
 * It works by returning a reference to a pointer to a material property (rather
 * than a reference to the property value). The pointer will be set to point to
 * either an existing material property or to nullptr if the requested property
 * does not exist.
 */
template <class M>
class OptionalMaterialPropertyProxyBase
{
public:
  OptionalMaterialPropertyProxyBase(const std::string & name, const unsigned int state)
    : _name(name), _state(state)
  {
  }
  virtual ~OptionalMaterialPropertyProxyBase() {}
  virtual void resolve(M & material) = 0;

protected:
  const std::string _name;
  const unsigned int _state;
};

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

  /// The material property ID for a default (parsed from input) property
  static constexpr PropertyValue::id_type default_property_id =
      PropertyValue::invalid_property_id - 1;
  /// The material property ID for a zero property
  static constexpr PropertyValue::id_type zero_property_id = PropertyValue::invalid_property_id - 2;

  ///@{
  /**
   * Retrieve reference to material property or one of it's old or older values.
   * The name required by this method is the name that is hard-coded into
   * your source code as the input parameter key. If no input parameter is found
   * this behaves like the getMaterialPropertyByName family as a fall back.
   * @param name The name of the parameter key of the material property to retrieve
   * @param state The state (current = 0, old = 1, older = 2)
   * @return Reference to the desired material property
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> & getGenericMaterialProperty(const std::string & name,
                                                                       const unsigned int state = 0)
  {
    return getGenericMaterialProperty<T, is_ad>(name, _material_data, state);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name,
                                                  const unsigned int state = 0)
  {
    return getGenericMaterialProperty<T, false>(name, state);
  }
  template <typename T>
  const ADMaterialProperty<T> & getADMaterialProperty(const std::string & name)
  {
    return getGenericMaterialProperty<T, true>(name, 0);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name)
  {
    return getMaterialProperty<T>(name, 1);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name)
  {
    return getMaterialProperty<T>(name, 2);
  }
  ///@}

  ///@{
  /**
   * Retrieve reference to material property or its old or older value
   * The name required by this method is the name defined in the input file.
   * @param name The name of the material property to retrieve
   * @param state The state (current = 0, old = 1, older = 2)
   * @return Reference to the material property with the name 'name'
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> &
  getGenericMaterialPropertyByName(const MaterialPropertyName & name, const unsigned int state = 0)
  {
    return getGenericMaterialPropertyByName<T, is_ad>(name, _material_data, state);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyByName(const MaterialPropertyName & name,
                                                        const unsigned int state = 0)
  {
    return getGenericMaterialPropertyByName<T, false>(name, state);
  }
  template <typename T>
  const ADMaterialProperty<T> & getADMaterialPropertyByName(const MaterialPropertyName & name)
  {
    return getGenericMaterialPropertyByName<T, true>(name, 0);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOldByName(const MaterialPropertyName & name)
  {
    return getMaterialPropertyByName<T>(name, 1);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderByName(const MaterialPropertyName & name)
  {
    return getMaterialPropertyByName<T>(name, 2);
  }
  ///@}

  ///@{ Optional material property getters
  /// \p state is the property state; 0 = current, 1 = old, 2 = older, etc.
  template <typename T, bool is_ad>
  const GenericOptionalMaterialProperty<T, is_ad> &
  getGenericOptionalMaterialProperty(const std::string & name, const unsigned int state = 0);

  template <typename T>
  const OptionalMaterialProperty<T> & getOptionalMaterialProperty(const std::string & name,
                                                                  const unsigned int state = 0)
  {
    return getGenericOptionalMaterialProperty<T, false>(name, state);
  }
  template <typename T>
  const OptionalADMaterialProperty<T> & getOptionalADMaterialProperty(const std::string & name)
  {
    return getGenericOptionalMaterialProperty<T, true>(name);
  }

  template <typename T>
  const OptionalMaterialProperty<T> & getOptionalMaterialPropertyOld(const std::string & name)
  {
    return getOptionalMaterialProperty<T>(name, 1);
  }
  template <typename T>
  const OptionalMaterialProperty<T> & getOptionalMaterialPropertyOlder(const std::string & name)
  {
    return getOptionalMaterialProperty<T>(name, 2);
  }
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
  template <typename T, bool is_ad>
  bool hasGenericMaterialProperty(const std::string & name)
  {
    if constexpr (is_ad)
      return hasADMaterialProperty<T>(name);
    else
      return hasMaterialProperty<T>(name);
  }
  template <typename T, bool is_ad>
  bool hasGenericMaterialPropertyByName(const std::string & name)
  {
    if constexpr (is_ad)
      return hasADMaterialPropertyByName<T>(name);
    else
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

  /// resolve all optional properties
  virtual void resolveOptionalProperties();

  /**
   * Retrieve the generic property named "name" for the specified \p material_data at state \p state
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> & getGenericMaterialProperty(
      const std::string & name, MaterialData & material_data, const unsigned int state = 0);

  /**
   * Retrieve the property named "name" for the specified \p material_data
   *
   * \p state is the property state; 0 = current, 1 = old, 2 = older, etc.
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name,
                                                  MaterialData & material_data,
                                                  const unsigned int state = 0)
  {
    return getGenericMaterialProperty<T, false>(name, material_data, state);
  }

  /**
   * Retrieve the AD property named "name" for the specified \p material_data
   *
   * \p state is the property state; 0 = current, 1 = old, 2 = older, etc.
   */
  template <typename T>
  const ADMaterialProperty<T> & getADMaterialProperty(const std::string & name,
                                                      MaterialData & material_data)
  {
    return getGenericMaterialProperty<T, true>(name, material_data, 0);
  }

  /**
   * Retrieve the generic property named "name" without any deduction for the specified \p
   * material_data for state \p state
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> & getGenericMaterialPropertyByName(
      const MaterialPropertyName & name, MaterialData & material_data, const unsigned int state);

  /**
   * Retrieve the property named "name" without any deduction for the specified \p material_data
   *
   * \p state is the property state; 0 = current, 1 = old, 2 = older, etc.
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyByName(const MaterialPropertyName & name,
                                                        MaterialData & material_data,
                                                        const unsigned int state = 0)
  {
    return getGenericMaterialPropertyByName<T, false>(name, material_data, state);
  }

  /**
   * Retrieve the AD property named "name" without any deduction for the specified \q
   * material_data
   */
  template <typename T>
  const ADMaterialProperty<T> & getADMaterialPropertyByName(const MaterialPropertyName & name,
                                                            MaterialData & material_data)
  {
    return getGenericMaterialPropertyByName<T, true>(name, material_data, 0);
  }

  /**
   * Retrieve the old property deduced from the name \p name for the specified \p material_data
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name,
                                                     MaterialData & material_data)
  {
    return getMaterialProperty<T>(name, material_data, 1);
  }

  /**
   * Retrieve the older property deduced from the name \p name for the specified \p
   * material_data
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name,
                                                       MaterialData & material_data)
  {
    return getMaterialProperty<T>(name, material_data, 2);
  }

  /**
   * Retrieve the old property named \p name without any deduction for the specified \p
   * material_data
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOldByName(const MaterialPropertyName & name,
                                                           MaterialData & material_data)
  {
    return getMaterialPropertyByName<T>(name, material_data, 1);
  }

  /**
   * Retrieve the older property named \p name without any deduction for the specified \p
   * material_data
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderByName(const MaterialPropertyName & name,
                                                             MaterialData & material_data)
  {
    return getMaterialPropertyByName<T>(name, material_data, 2);
  }

private:
  /// The MooseObject creating the MaterialPropertyInterface
  const MooseObject & _mi_moose_object;

protected:
  /// Parameters of the object with this interface
  const InputParameters & _mi_params;

  /// The name of the object that this interface belongs to
  const std::string _mi_name;

  /// The "complete" name of the object that this interface belongs for material property output
  const MooseObjectName _mi_moose_object_name;

  /// Reference to the FEProblemBase class
  FEProblemBase & _mi_feproblem;

  /// Reference to the subproblem
  SubProblem & _mi_subproblem;

  /// Current threaded it
  const THREAD_ID _mi_tid;

  /// The type of data
  const Moose::MaterialDataType _material_data_type;

  /// The material data class that stores properties
  MaterialData & _material_data;

  /**
   * A helper method for checking material properties
   * This method was required to avoid a compiler problem with the template
   * getMaterialProperty method
   */
  void checkMaterialProperty(const std::string & name, const unsigned int state);

  /**
   * A proxy method for _mi_feproblem.markMatPropRequested(name)
   */
  void markMatPropRequested(const std::string &);

  /**
   * @return The name of the material property associated with name \p name.
   *
   * If \p name is the name of a material property parameter and the parameter is
   * valid, this will return the value of said parameter. Otherwise, it will just
   * return the name.
   */
  MaterialPropertyName getMaterialPropertyName(const std::string & name) const;

  /**
   * @return The default material property with the name \p name, if any.
   *
   * "Default" properties are properties whose default values are set from within
   * the name. That is, if we can cast \p name to a Real, _and_ the prop type is
   * a Real or RealVectorValue, we'll return said value.
   */
  ///@{
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> *
  defaultGenericMaterialProperty(const std::string & name);
  template <typename T>
  const MaterialProperty<T> * defaultMaterialProperty(const std::string & name)
  {
    return defaultGenericMaterialProperty<T, false>(name);
  }
  template <typename T>
  const ADMaterialProperty<T> * defaultADMaterialProperty(const std::string & name)
  {
    return defaultGenericMaterialProperty<T, true>(name);
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

  /// Storage vector for default properties
  std::vector<std::unique_ptr<PropertyValue>> _default_properties;

  /// The set of material properties (as given by their IDs) that _this_ object depends on
  std::set<unsigned int> _material_property_dependencies;

  const MaterialPropertyName _get_suffix;

private:
  /**
   * @returns The MaterialDataType given the interface's parameters
   */
  Moose::MaterialDataType getMaterialDataType(const std::set<BoundaryID> & boundary_ids) const;

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

  /// optional material properties
  std::vector<std::unique_ptr<OptionalMaterialPropertyProxyBase<MaterialPropertyInterface>>>
      _optional_property_proxies;
};

template <class M, typename T, bool is_ad>
class OptionalMaterialPropertyProxy : public OptionalMaterialPropertyProxyBase<M>
{
public:
  OptionalMaterialPropertyProxy(const std::string & name, const unsigned int state)
    : OptionalMaterialPropertyProxyBase<M>(name, state)
  {
  }
  const GenericOptionalMaterialProperty<T, is_ad> & value() const { return _value; }
  void resolve(M & mpi) override
  {
    if (mpi.template hasGenericMaterialProperty<T, is_ad>(this->_name))
    {
      if constexpr (is_ad)
        if (this->_state > 0)
          mooseError("Non-current (state > 0) material properties are not available as AD");

      _value.set(&mpi.template getGenericMaterialProperty<T, is_ad>(this->_name, this->_state));
    }
  }

private:
  GenericOptionalMaterialProperty<T, is_ad> _value;
};

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> *
MaterialPropertyInterface::defaultGenericMaterialProperty(const std::string & name)
{
  if constexpr (std::is_same_v<T, Real> || std::is_same_v<T, RealVectorValue>)
  {
    std::istringstream ss(name);
    Real real_value;

    // check if the string parsed cleanly into a Real number
    if (ss >> real_value && ss.eof())
    {
      using prop_type = GenericMaterialProperty<T, is_ad>;

      const auto nqp = Moose::constMaxQpsPerElem;
      auto & property =
          _default_properties.emplace_back(std::make_unique<prop_type>(default_property_id));
      auto & T_property = static_cast<prop_type &>(*property);

      T_property.resize(nqp);
      for (const auto qp : make_range(nqp))
        T_property[qp] = real_value;

      return &T_property;
    }
  }

  return nullptr;
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

  using pair_type = std::pair<const MaterialProperty<T> *, std::set<SubdomainID>>;

  if (!hasMaterialPropertyByName<T>(name))
    return pair_type(nullptr, {});

  // Call first so that the ID gets registered
  const auto & prop = _material_data.getProperty<T, false>(name, 0, _mi_moose_object);
  auto blocks = getMaterialPropertyBlocks(name);
  auto prop_blocks_pair = pair_type(&prop, std::move(blocks));

  _material_property_dependencies.insert(_material_data.getPropertyId(name));

  // Update consumed properties in MaterialPropertyDebugOutput
  addConsumedPropertyName(_mi_moose_object_name, name);

  return prop_blocks_pair;
}

template <typename T>
bool
MaterialPropertyInterface::hasMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  const auto prop_name = getMaterialPropertyName(name);
  return hasMaterialPropertyByName<T>(prop_name);
}

template <typename T>
bool
MaterialPropertyInterface::hasMaterialPropertyByName(const std::string & name_in)
{
  const auto name = _get_suffix.empty()
                        ? name_in
                        : MooseUtils::join(std::vector<std::string>({name_in, _get_suffix}), "_");
  return _material_data.haveProperty<T>(name);
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
MaterialPropertyInterface::getGenericZeroMaterialProperty(const std::string & name)
{
  const auto prop_name = getMaterialPropertyName(name);
  return getGenericZeroMaterialPropertyByName<T, is_ad>(prop_name);
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
MaterialPropertyInterface::getGenericZeroMaterialPropertyByName(const std::string & prop_name)
{
  // if found return the requested property
  if (hasGenericMaterialPropertyByName<T, is_ad>(prop_name))
    return getGenericMaterialPropertyByName<T, is_ad>(prop_name);

  return getGenericZeroMaterialProperty<T, is_ad>();
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
MaterialPropertyInterface::getGenericZeroMaterialProperty()
{
  // static zero property storage
  static GenericMaterialProperty<T, is_ad> zero(zero_property_id);

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
  const auto prop_name = getMaterialPropertyName(name);
  return hasADMaterialPropertyByName<T>(prop_name);
}

template <typename T>
bool
MaterialPropertyInterface::hasADMaterialPropertyByName(const std::string & name_in)
{
  const auto name = _get_suffix.empty()
                        ? name_in
                        : MooseUtils::join(std::vector<std::string>({name_in, _get_suffix}), "_");
  return _material_data.haveADProperty<T>(name);
}

template <typename T, bool is_ad>
const GenericOptionalMaterialProperty<T, is_ad> &
MaterialPropertyInterface::getGenericOptionalMaterialProperty(const std::string & name,
                                                              const unsigned int state)
{
  auto proxy = std::make_unique<OptionalMaterialPropertyProxy<MaterialPropertyInterface, T, is_ad>>(
      name, state);
  auto & optional_property = proxy->value();
  _optional_property_proxies.push_back(std::move(proxy));
  return optional_property;
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
MaterialPropertyInterface::getGenericMaterialProperty(const std::string & name,
                                                      MaterialData & material_data,
                                                      const unsigned int state)
{
  // Check if the supplied parameter is a valid input parameter key
  const auto prop_name = getMaterialPropertyName(name);

  // Check if it's just a constant
  if (const auto * default_property = defaultGenericMaterialProperty<T, is_ad>(prop_name))
    return *default_property;

  if (state > 0 && !_stateful_allowed)
    mooseError("Stateful material properties not allowed for this object."
               " State ",
               state,
               " property for \"",
               name,
               "\" was requested.");

  return this->getGenericMaterialPropertyByName<T, is_ad>(prop_name, material_data, state);
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
MaterialPropertyInterface::getGenericMaterialPropertyByName(const MaterialPropertyName & name_in,
                                                            MaterialData & material_data,
                                                            const unsigned int state)
{
  const auto name = _get_suffix.empty()
                        ? static_cast<const std::string &>(name_in)
                        : MooseUtils::join(std::vector<std::string>({name_in, _get_suffix}), "_");

  checkExecutionStage();
  checkMaterialProperty(name, state);

  // mark property as requested
  markMatPropRequested(name);

  // Update the boolean flag.
  _get_material_property_called = true;

  // Call first so that the ID gets registered
  auto & prop = material_data.getProperty<T, is_ad>(name, state, _mi_moose_object);

  // Does the material data used here matter?
  _material_property_dependencies.insert(material_data.getPropertyId(name));

  if (state == 0)
    addConsumedPropertyName(_mi_moose_object_name, name);

  return prop;
}
