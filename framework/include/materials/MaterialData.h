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
  unsigned int nQPoints();

  /**
   * Declare the Real valued property named "name".
   * Calling any of the declareProperty
   * functions multiple times with the same property name is okay and
   * will result in a single identical reference returned every time.
   */
  template <typename T>
  MaterialProperty<T> & declareProperty(const std::string & prop_name);

  /**
   * Declare the Real valued property prop_name.
   */
  template <typename T>
  MaterialProperty<T> & declarePropertyOld(const std::string & prop_name);

  /**
   * Declare the Real valued property named prop_name.
   */
  template <typename T>
  MaterialProperty<T> & declarePropertyOlder(const std::string & prop_name);

  /**
   * Declare the AD property named "name".
   * Calling any of the declareProperty
   * functions multiple times with the same property name is okay and
   * will result in a single identical reference returned every time.
   */
  template <typename T>
  ADMaterialProperty<T> & declareADProperty(const std::string & prop_name);

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

  ///@{
  /**
   *  Methods for retrieving MaterialProperties object.  These functions
   *  should NEVER be used to modify the sizes of the MaterialProperties
   *  objects.
   */
  MaterialProperties & props() { return _props; }
  MaterialProperties & propsOld() { return _props_old; }
  MaterialProperties & propsOlder() { return _props_older; }
  ///@}

  /// Returns true if the regular material property exists - defined by any material.
  template <typename T>
  bool haveProperty(const std::string & prop_name) const;

  /// Returns true if the AD material property exists - defined by any material.
  template <typename T>
  bool haveADProperty(const std::string & prop_name) const;

  template <typename T, bool is_ad>
  bool haveGenericProperty(const std::string & prop_name) const
  {
    if constexpr (is_ad)
      return haveADProperty<T>(prop_name);
    else
      return haveProperty<T>(prop_name);
  }

  /**
   * @{ Methods for retieving a MaterialProperty object
   * @tparam T The type of the property
   * @param prop_name The name of the property
   * @return The property for the supplied type and name
   */
  template <typename T, bool is_ad>
  auto & getGenericProperty(const std::string & prop_name)
  {
    if constexpr (is_ad)
      return getADProperty<T>(prop_name);
    else
      return getProperty<T>(prop_name);
  }
  template <typename T>
  MaterialProperty<T> & getProperty(const std::string & prop_name);
  template <typename T>
  ADMaterialProperty<T> & getADProperty(const std::string & prop_name);
  template <typename T>
  MaterialProperty<T> & getPropertyOld(const std::string & prop_name);
  template <typename T>
  MaterialProperty<T> & getPropertyOlder(const std::string & prop_name);
  ///@}

  /**
   * Returns true if the stateful material is in a swapped state.
   */
  bool isSwapped();

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
   * flag to true means that resize(n) will not decrease the size of _material_data
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

protected:
  /// Reference to the MaterialStorage class
  MaterialPropertyStorage & _storage;

  /// Number of quadrature points
  unsigned int _n_qpoints;

  ///@{
  /// Holds material properties for currently selected element (and possibly a side), they are being copied from _storage
  MaterialProperties _props;
  MaterialProperties _props_old;
  MaterialProperties _props_older;
  ///@}

  /**
   * Calls resizeProps helper function for regular material properties
   */
  template <typename T>
  void resizeProps(unsigned int size);

  /**
   * Calls resizeProps helper function for AD material properties
   */
  template <typename T>
  void resizePropsAD(unsigned int size);

  /// Status of storage swapping (calling swap sets this to true; swapBack sets it to false)
  bool _swapped;

  /// Use non-destructive resize of material data (calling resize() will not reduce size).
  /// Default is false (normal resize behaviour)
  bool _resize_only_if_smaller;

private:
  template <typename T>
  MaterialProperty<T> &
  declareHelper(MaterialProperties & props, const std::string & prop_name, unsigned int prop_id);

  template <typename T>
  ADMaterialProperty<T> &
  declareADHelper(MaterialProperties & props, const std::string & prop_name, unsigned int prop_id);
};

template <typename T>
inline bool
MaterialData::haveProperty(const std::string & prop_name) const
{
  if (!_storage.hasProperty(prop_name))
    return false;

  unsigned int prop_id = getPropertyId(prop_name);
  if (prop_id >= _props.size())
    return false; // the property id exists, but the property was not created in this instance of
                  // the material type

  return dynamic_cast<const MaterialProperty<T> *>(_props[prop_id]) != nullptr;
}

template <typename T>
inline bool
MaterialData::haveADProperty(const std::string & prop_name) const
{
  if (!_storage.hasProperty(prop_name))
    return false;

  unsigned int prop_id = getPropertyId(prop_name);
  if (prop_id >= _props.size())
    return false; // the property id exists, but the property was not created in this instance of
                  // the material type

  return dynamic_cast<const ADMaterialProperty<T> *>(_props[prop_id]) != nullptr;
}

template <typename T>
void
MaterialData::resizeProps(unsigned int size)
{
  auto n = size + 1;
  if (_props.size() < n)
    _props.resize(n, nullptr);
  if (_props_old.size() < n)
    _props_old.resize(n, nullptr);
  if (_props_older.size() < n)
    _props_older.resize(n, nullptr);

  if (_props[size] == nullptr)
    _props[size] = new MaterialProperty<T>;
  if (_props_old[size] == nullptr)
    _props_old[size] = new MaterialProperty<T>;
  if (_props_older[size] == nullptr)
    _props_older[size] = new MaterialProperty<T>;
}

template <typename T>
void
MaterialData::resizePropsAD(unsigned int size)
{
  auto n = size + 1;
  if (_props.size() < n)
    _props.resize(n, nullptr);
  if (_props_old.size() < n)
    _props_old.resize(n, nullptr);
  if (_props_older.size() < n)
    _props_older.resize(n, nullptr);

  if (_props[size] == nullptr)
    _props[size] = new ADMaterialProperty<T>;
  if (_props_old[size] == nullptr)
    _props_old[size] = new MaterialProperty<T>;
  if (_props_older[size] == nullptr)
    _props_older[size] = new MaterialProperty<T>;
}

template <typename T>
MaterialProperty<T> &
MaterialData::declareProperty(const std::string & prop_name)
{
  return declareHelper<T>(_props, prop_name, _storage.addProperty(prop_name));
}

template <typename T>
ADMaterialProperty<T> &
MaterialData::declareADProperty(const std::string & prop_name)
{
  return declareADHelper<T>(_props, prop_name, _storage.addProperty(prop_name));
}

template <typename T>
MaterialProperty<T> &
MaterialData::declarePropertyOld(const std::string & prop_name)
{
  // TODO: add mooseDeprecated("'declarePropertyOld' is deprecated an no longer necessary");
  return getPropertyOld<T>(prop_name);
}

template <typename T>
MaterialProperty<T> &
MaterialData::declarePropertyOlder(const std::string & prop_name)
{
  // TODO: add mooseDeprecated("'declarePropertyOlder' is deprecated an no longer necessary");
  return getPropertyOlder<T>(prop_name);
}

template <typename T>
MaterialProperty<T> &
MaterialData::declareHelper(MaterialProperties & props,
                            const std::string & prop_name,
                            unsigned int prop_id)
{
  resizeProps<T>(prop_id);
  auto prop = dynamic_cast<MaterialProperty<T> *>(props[prop_id]);
  if (!prop)
  {
    // We didn't find a regular material property so we're going to error out. But we can check to
    // see whether there is an AD property of the same name in the hope that we can give the user a
    // more meaningful error message
    auto ad_prop = dynamic_cast<ADMaterialProperty<T> *>(_props[prop_id]);
    if (ad_prop)
      mooseError("Attempting to declare regular material property " + prop_name +
                 ", but it is already retrieved/declared as an AD property.");
    else
      mooseError("Material has no property named: " + prop_name);
  }
  prop->setName(prop_name);
  return *prop;
}

template <typename T>
ADMaterialProperty<T> &
MaterialData::declareADHelper(MaterialProperties & props,
                              const std::string & prop_name,
                              unsigned int prop_id)
{
  resizePropsAD<T>(prop_id);
  auto prop = dynamic_cast<ADMaterialProperty<T> *>(props[prop_id]);
  if (!prop)
  {
    // We didn't find an AD material property so we're going to error out. But we can check to
    // see whether there is a regular property of the same name in the hope that we can give the
    // user a more meaningful error message
    auto regular_prop = dynamic_cast<MaterialProperty<T> *>(_props[prop_id]);
    if (regular_prop)
      mooseError("Attempting to declare AD material property " + prop_name +
                 ", but it is already retrieved/declared as a regular material property.");
    else
      mooseError("Material has no property named: " + prop_name);
  }
  prop->setName(prop_name);
  return *prop;
}

template <typename T>
MaterialProperty<T> &
MaterialData::getProperty(const std::string & name)
{
  auto prop_id = getPropertyId(name);
  resizeProps<T>(prop_id);
  auto prop = dynamic_cast<MaterialProperty<T> *>(_props[prop_id]);
  if (!prop)
  {
    // We didn't find a regular material property so we're going to error out. But we can check to
    // see whether there is an AD property of the same name in the hope that we can give the user a
    // more meaningful error message
    auto ad_prop = dynamic_cast<ADMaterialProperty<T> *>(_props[prop_id]);
    if (ad_prop)
      mooseError("The requested regular material property " + name +
                 " is declared as an AD property. Either retrieve it as an AD property with "
                 "getADMaterialProperty or declare it as a regular property with declareProperty");
    else
      mooseError("Material has no property named: " + name);
  }
  return *prop;
}

template <typename T>
ADMaterialProperty<T> &
MaterialData::getADProperty(const std::string & name)
{
  auto prop_id = getPropertyId(name);
  resizePropsAD<T>(prop_id);
  auto prop = dynamic_cast<ADMaterialProperty<T> *>(_props[prop_id]);
  if (!prop)
  {
    // We didn't find an AD material property so we're going to error out. But we can check to
    // see whether there is a regular property of the same name in the hope that we can give the
    // user a more meaningful error message
    auto regular_prop = dynamic_cast<MaterialProperty<T> *>(_props[prop_id]);
    if (regular_prop)
      mooseError("The requested AD material property " + name +
                 " is declared as a regular material property. Either retrieve it as a regular "
                 "material property with getMaterialProperty or declare it as an AD property with "
                 "declareADProperty");
    else
      mooseError("Material has no property named: " + name);
  }
  return *prop;
}

template <typename T>
MaterialProperty<T> &
MaterialData::getPropertyOld(const std::string & name)
{
  return declareHelper<T>(_props_old, name, _storage.addPropertyOld(name));
}

template <typename T>
MaterialProperty<T> &
MaterialData::getPropertyOlder(const std::string & name)
{
  return declareHelper<T>(_props_older, name, _storage.addPropertyOlder(name));
}

template <typename MatContainer>
void
MaterialData::reinit(const MatContainer & mats)
{
  for (const auto & mat : mats)
    mat->computeProperties();
}
