//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MaterialProperty.h"
#include "Moose.h"
#include "MooseUtils.h"

// libMesh
#include "libmesh/elem.h"

#include <vector>
#include <memory>

class MaterialPropertyStorage;
class MooseObject;
class Material;
class XFEM;

/**
 * Proxy for accessing MaterialPropertyStorage.
 * MaterialData stores the values associated with a particular material object
 */
class MaterialData
{
public:
  MaterialData(MaterialPropertyStorage & storage, const THREAD_ID tid);

  /// The max time state supported (2 = older)
  static constexpr unsigned int max_state = 2;

  /**
   * Resize the data to hold properties for n_qpoints quadrature points.
   */
  void resize(unsigned int n_qpoints);

  /**
   * Returns the number of quadrature points the material properties
   * support/hold.
   */
  unsigned int nQPoints() const { return _n_qpoints; }

  /// copy material properties from one element to another
  void copy(const Elem & elem_to, const Elem & elem_from, unsigned int side);

  /// material properties for given element (and possible side)
  void swap(const Elem & elem, unsigned int side = 0);

  /**
   * Reinit material properties for given element (and possible side)
   * @param mats The material objects for which to compute properties
   * @param execute_stateful Whether to execute material objects that have stateful properties. This
   * should be \p false when for example executing material objects for mortar contexts in which
   * stateful properties don't make sense
   */
  template <typename MatContainer>
  void reinit(const MatContainer & mats);

  /// Calls the reset method of Materials to ensure that they are in a proper state.
  void reset(const std::vector<std::shared_ptr<MaterialBase>> & mats);

  /// material properties for given element (and possible side)
  void swapBack(const Elem & elem, unsigned int side = 0);

  /**
   * @returns The properties for the state \p state (defaults to zero).
   *
   * This should NEVER be used to modify the size of these objects.
   */
  ///{
  const MaterialProperties & props(const unsigned int state = 0) const;
  MaterialProperties & props(const unsigned int state = 0);
  ///@}

  template <typename T, bool is_ad>
  bool haveGenericProperty(const std::string & prop_name) const;

  /// Returns true if the regular material property exists - defined by any material.
  template <typename T>
  bool haveProperty(const std::string & prop_name) const
  {
    return haveGenericProperty<T, false>(prop_name);
  }

  /// Returns true if the AD material property exists - defined by any material.
  template <typename T>
  bool haveADProperty(const std::string & prop_name) const
  {
    return haveGenericProperty<T, true>(prop_name);
  }

  /**
   * Retrieves a material property
   * @tparam T The type of the property
   * @tparam is_ad Whether or not the property is AD
   * @param prop_name The name of the property
   * @param state The time state (0 = current, 1 = old, etc; defaults to 0)
   * @param requestor The MooseObject requesting the property
   * @return The property for the supplied type and name
   */
  template <typename T, bool is_ad = false>
  GenericMaterialProperty<T, is_ad> & getProperty(const std::string & prop_name,
                                                  const unsigned int state,
                                                  const MooseObject & requestor)
  {
    return getPropertyHelper<T, is_ad, false>(prop_name, state, requestor);
  }
  /**
   * Declares a material property
   * @tparam T The type of the property
   * @tparam is_ad Whether or not the property is AD
   * @param prop_name The name of the property
   * @param requestor The MooseObject declaring the property
   * @return The property for the supplied type and name
   */
  template <typename T, bool is_ad>
  GenericMaterialProperty<T, is_ad> & declareProperty(const std::string & prop_name,
                                                      const MooseObject & requestor)
  {
    return getPropertyHelper<T, is_ad, true>(prop_name, 0, requestor);
  }

  /**
   * Returns true if the stateful material is in a swapped state.
   */
  bool isSwapped() const { return _swapped; }

  /**
   * Provide read-only access to the underlying MaterialPropertyStorage object.
   */
  const MaterialPropertyStorage & getMaterialPropertyStorage() const { return _storage; }

  /**
   * Key that provides access to only the XFEM class.
   */
  class XFEMKey
  {
    friend class XFEM;
    XFEMKey() {}
    XFEMKey(const XFEM &) {}
  };

  /**
   * Provide write-only access to the underlying MaterialPropertyStorage object JUST FOR XFEM.
   *
   * This should be removed. To be clear - you should not ever expect to have write access
   * to this data. It just turned out that XFEM got away with it when we were storing things
   * as pointers instead of smart pointers...
   *
   * These dirty reasons are why this method is named so egregiously.
   */
  MaterialPropertyStorage & getMaterialPropertyStorageForXFEM(const XFEMKey) { return _storage; }

  /**
   * @return Whether or not a property exists with the name \p name
   */
  bool hasProperty(const std::string & prop_name) const;

  /**
   * Wrapper for MaterialStorage::getPropertyId. Allows classes with a MaterialData object
   * (i.e. MaterialPropertyInterface) to access material property IDs.
   * @param prop_name The name of the material property
   *
   * @return An unsigned int corresponding to the property ID of the passed in prop_name
   */
  unsigned int getPropertyId(const std::string & prop_name) const;

  /**
   * Set _resize_only_if_smaller to perform a non-destructive resize. Setting this
   * flag to true means that resize(n) will not decrease the size of _props
   * if n is smaller than the size of the material data object.
   */
  void onlyResizeIfSmaller(bool flag) { _resize_only_if_smaller = flag; };

  /**
   * Check value of _resize_only_if_smaller
   */
  bool isOnlyResizeIfSmaller() const { return _resize_only_if_smaller; };

  /**
   * Remove the property storage and element pointer from MaterialPropertyStorage data structures
   * Use this when elements are deleted so we don't end up with invalid elem pointers (for e.g.
   * stateful properties) hanging around in our data structures
   */
  void eraseProperty(const Elem * elem);

private:
  /// Reference to the MaterialStorage class
  MaterialPropertyStorage & _storage;

  /// The thread id
  const THREAD_ID _tid;

  /// Number of quadrature points
  unsigned int _n_qpoints;

  /// The underlying property data
  std::array<MaterialProperties, max_state + 1> _props;

  unsigned int addPropertyHelper(const std::string & prop_name, const unsigned int state);

  template <typename T, bool is_ad, bool declare>
  GenericMaterialProperty<T, is_ad> & getPropertyHelper(const std::string & prop_name,
                                                        const unsigned int state,
                                                        const MooseObject & requestor);

  static void mooseErrorHelper(const MooseObject & object, const std::string_view & error);

  /// Status of storage swapping (calling swap sets this to true; swapBack sets it to false)
  bool _swapped;

  /// Use non-destructive resize of material data (calling resize() will not reduce size).
  /// Default is false (normal resize behaviour)
  bool _resize_only_if_smaller;
};

inline const MaterialProperties &
MaterialData::props(const unsigned int state) const
{
  mooseAssert(_props.size() > state, "Invalid state");
  return _props[state];
}

inline MaterialProperties &
MaterialData::props(const unsigned int state)
{
  mooseAssert(_props.size() > state, "Invalid state");
  return _props[state];
}

template <typename T, bool is_ad>
inline bool
MaterialData::haveGenericProperty(const std::string & prop_name) const
{
  if (!hasProperty(prop_name))
    return false;

  const auto prop_id = getPropertyId(prop_name);
  // the property id exists, but the property was not created in this instance of the material type
  if (prop_id >= props(0).size())
    return false;

  const PropertyValue * const base_prop = props(0).queryValue(prop_id);
  return dynamic_cast<const GenericMaterialProperty<T, is_ad> *>(base_prop) != nullptr;
}

template <typename T, bool is_ad, bool declare>
GenericMaterialProperty<T, is_ad> &
MaterialData::getPropertyHelper(const std::string & prop_name,
                                const unsigned int state,
                                const MooseObject & requestor)
{
  if constexpr (is_ad)
    mooseAssert(state == 0, "Cannot request/declare AD properties for states other than zero");
  if constexpr (declare)
    mooseAssert(state == 0, "Cannot declare properties for states other than zero");

  // Register/get the ID of the property
  const auto prop_id = addPropertyHelper(prop_name, state);

  // Initialize the states that we need
  const auto size = prop_id + 1;
  for (const auto state_i : make_range(state + 1))
  {
    auto & entry = props(state_i);
    if (entry.size() < size)
      entry.resize(size, {});
    if (!entry.hasValue(prop_id))
    {
      std::unique_ptr<PropertyValue> value =
          state_i == 0 ? std::make_unique<GenericMaterialProperty<T, is_ad>>(prop_id)
                       : _props[0][prop_id].clone(0);
      entry.setPointer(prop_id, std::move(value), {});
    }
  }

  // Should be available now
  auto & base_prop = props(state)[prop_id];

  // In the event that this property was already declared/requested, make sure
  // that the types are consistent
  auto prop = dynamic_cast<GenericMaterialProperty<T, is_ad> *>(&base_prop);
  if (!prop)
  {
    constexpr std::string_view action = declare ? "declared" : "requested";
    constexpr auto is_ad_to_str = [](const bool is_ad_bool)
    { return std::string_view(is_ad_bool ? "AD" : "non-AD"); };
    constexpr std::string_view ad_type = is_ad_to_str(is_ad);

    std::stringstream error;
    error << "The " << action << " " << ad_type << " "
          << "material property '" + prop_name + "' of type '" << MooseUtils::prettyCppType<T>()
          << "'\nis already retrieved or declared as a " << is_ad_to_str(base_prop.isAD())
          << " property of type '" << base_prop.type() << "'.";
    mooseErrorHelper(requestor, error.str());
  }

  return *prop;
}

template <typename MatContainer>
void
MaterialData::reinit(const MatContainer & mats)
{
  for (const auto & mat : mats)
    mat->computeProperties();
}
