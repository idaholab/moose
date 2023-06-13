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
#include "MaterialPropertyStorage.h"
#include "MooseUtils.h"

// libMesh
#include "libmesh/elem.h"

#include <vector>

class Material;

/**
 * Proxy for accessing MaterialPropertyStorage.
 * MaterialData stores the values associated with a particular material object
 */
class MaterialData
{
public:
  MaterialData(MaterialPropertyStorage & storage);
  virtual ~MaterialData();

  /**
   * Calls the destroy() methods for the properties currently stored
   */
  void release();

  /**
   * Resize the data to hold properties for n_qpoints quadrature points.
   */
  void resize(unsigned int n_qpoints);

  /**
   * Returns the number of quadrature points the material properties
   * support/hold.
   */
  unsigned int nQPoints() const;

  /**
   * Declare the Real valued property named "name".
   * Calling any of the declareProperty
   * functions multiple times with the same property name is okay and
   * will result in a single identical reference returned every time.
   */
  template <typename T>
  MaterialProperty<T> & declareProperty(const std::string & prop_name)
  {
    return declareHelper<T, false>(prop_name, 0);
    ;
  }

  /**
   * Declare the AD property named "name".
   * Calling any of the declareProperty
   * functions multiple times with the same property name is okay and
   * will result in a single identical reference returned every time.
   */
  template <typename T>
  ADMaterialProperty<T> & declareADProperty(const std::string & prop_name)
  {
    return declareHelper<T, true>(prop_name, 0);
  }

  /// copy material properties from one element to another
  void copy(const Elem & elem_to, const Elem & elem_from, unsigned int side);

  /// copy material properties from one element to another
  void copy(const Elem * elem_to, const Elem * elem_from, unsigned int side);

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

  ///@{
  /**
   * Deprecated Methods for retrieving older MaterialProperties objects
   */
  MaterialProperties & propsOld() { return props(1); }
  MaterialProperties & propsOlder() { return props(2); }
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
   * @{ Methods for retieving a MaterialProperty object
   * @tparam T The type of the property
   * @param prop_name The name of the property
   * @param state The time state (0 = current, 1 = old, etc; defaults to 0)
   * @return The property for the supplied type and name
   */
  template <typename T, bool is_ad>
  GenericMaterialProperty<T, is_ad> & getGenericProperty(const std::string & prop_name,
                                                         const unsigned int state = 0)
  {
    return declareHelper<T, is_ad>(prop_name, state);
  }
  template <typename T>
  MaterialProperty<T> & getProperty(const std::string & prop_name, const unsigned int state = 0)
  {
    return getGenericProperty<T, false>(prop_name, state);
  }
  template <typename T>
  ADMaterialProperty<T> & getADProperty(const std::string & prop_name)
  {
    return getGenericProperty<T, true>(prop_name, 0);
  }
  template <typename T>
  MaterialProperty<T> & getPropertyOld(const std::string & prop_name)
  {
    return declareHelper<T, false>(prop_name, 1);
  }
  template <typename T>
  MaterialProperty<T> & getPropertyOlder(const std::string & prop_name)
  {
    return declareHelper<T, false>(prop_name, 2);
  }
  ///@}

  /**
   * Returns true if the stateful material is in a swapped state.
   */
  bool isSwapped() const;

  /**
   * Provide read-only access to the underlying MaterialPropertyStorage object.
   */
  const MaterialPropertyStorage & getMaterialPropertyStorage() const { return _storage; }

  /**
   * Wrapper for MaterialStorage::getPropertyId. Allows classes with a MaterialData object
   * (i.e. MaterialPropertyInterface) to access material property IDs.
   * @param prop_name The name of the material property
   *
   * @return An unsigned int corresponding to the property ID of the passed in prop_name
   */
  unsigned int getPropertyId(const std::string & prop_name) const
  {
    return _storage.getPropertyId(prop_name);
  }

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
  void eraseProperty(const Elem * elem) { _storage.eraseProperty(elem); };

private:
  /// Reference to the MaterialStorage class
  MaterialPropertyStorage & _storage;

  /// Number of quadrature points
  unsigned int _n_qpoints;

  /// The underlying property data
  std::array<MaterialProperties, MaterialPropertyStorage::max_state + 1> _props;

  /**
   * Calls resizeProps helper function for regular material properties
   */
  template <typename T, bool is_ad>
  void resizeProps(unsigned int id);

  /// Status of storage swapping (calling swap sets this to true; swapBack sets it to false)
  bool _swapped;

  /// Use non-destructive resize of material data (calling resize() will not reduce size).
  /// Default is false (normal resize behaviour)
  bool _resize_only_if_smaller;

  template <typename T, bool is_ad>
  GenericMaterialProperty<T, is_ad> & declareHelper(const std::string & prop_name,
                                                    const unsigned int state);
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
  if (!_storage.hasProperty(prop_name))
    return false;

  const auto prop_id = getPropertyId(prop_name);
  // the property id exists, but the property was not created in this instance of the material type
  if (prop_id >= props(0).size())
    return false;

  return dynamic_cast<const GenericMaterialProperty<T, is_ad> *>(props(0)[prop_id]) != nullptr;
}

template <typename T, bool is_ad>
void
MaterialData::resizeProps(unsigned int id)
{
  const auto size = id + 1;
  for (const auto state : index_range(_props))
  {
    auto & entry = props(state);
    if (entry.size() < size)
      entry.resize(size, nullptr);
    if (entry[id] == nullptr)
    {
      if (is_ad && state == 0)
        entry[id] = new ADMaterialProperty<T>;
      else
        entry[id] = new MaterialProperty<T>;
    }
  }
}

template <typename T, bool is_ad>
GenericMaterialProperty<T, is_ad> &
MaterialData::declareHelper(const std::string & prop_name, const unsigned int state)
{
  if constexpr (is_ad)
    if (state != 0)
      mooseError("Cannot request/declare AD properties for states other than zero");

  const auto prop_id = _storage.addProperty(prop_name, state);
  resizeProps<T, is_ad>(prop_id);

  auto prop = dynamic_cast<GenericMaterialProperty<T, is_ad> *>(props(state)[prop_id]);
  if (!prop)
  {
    const std::string type = is_ad ? "AD" : "non-AD";
    const std::string other_type = is_ad ? "non-AD" : "AD";
    const auto T_type = MooseUtils::prettyCppType<T>();

    // See if a property of the other type exists first
    if (state == 0 && dynamic_cast<GenericMaterialProperty<T, !is_ad> *>(props(0)[prop_id]))
      mooseError("The requested/declared ",
                 " material property '" + prop_name + "' of type '",
                 T_type,
                 "',\nbut it is already retrieved/declared as a ",
                 other_type,
                 " property.");

    mooseError("Material has no ", type, " property '", prop_name, "' of type '", T_type, "'");
  }

  prop->setName(prop_name);

  return *prop;
}

template <typename MatContainer>
void
MaterialData::reinit(const MatContainer & mats)
{
  for (const auto & mat : mats)
    mat->computeProperties();
}
