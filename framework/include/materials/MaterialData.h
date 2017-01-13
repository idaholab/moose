/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIALDATA_H
#define MATERIALDATA_H

#include "Moose.h"
#include "MaterialProperty.h"
#include "MaterialPropertyStorage.h"

//libMesh
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
  void size(unsigned int n_qpoints);

  /**
   * Returns the number of quadrature points the material properties
   * support/hold.
   */
  unsigned int nQPoints();

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using any of the getProperty function variants.
   * Calling any of the declareProperty
   * functions multiple times with the same property name is okay and
   * will result in a single identical reference returned every time.
   */
  template<typename T>
  MaterialProperty<T> & declareProperty(const std::string & prop_name);

  /**
   * Declare the Real valued property prop_name.
   * This must be done _before_ attempts to
   * retrieved the property using getPropertyOld().
   */
  template<typename T>
  MaterialProperty<T> & declarePropertyOld(const std::string & prop_name);

  /**
   * Declare the Real valued property named prop_name.
   * This must be done _before_ attempts to
   * retrieved the property using getPropertyOlder().
   */
  template<typename T>
  MaterialProperty<T> & declarePropertyOlder(const std::string & prop_name);

  /// copy material properties from one element to another
  void copy(const Elem & elem_to, const Elem & elem_from, unsigned int side);

  /// material properties for given element (and possible side)
  void swap(const Elem & elem, unsigned int side = 0);

  /// Reinit material properties for given element (and possible side)
  void reinit(const std::vector<MooseSharedPointer<Material> > & mats);

  /// Calls the reset method of Materials to ensure that they are in a proper state.
  void reset(const std::vector<MooseSharedPointer<Material> > & mats);

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

  /// Returns true if the property exists - defined by any material (i.e. not
  /// necessarily just this one).
  template <typename T>
  bool haveProperty(const std::string & prop_name) const;
  template <typename T>
  bool havePropertyOld(const std::string & prop_name) const;
  template <typename T>
  bool havePropertyOlder(const std::string & prop_name) const;
  ///@}

  ///@{
  /**
   * Methods for retieving a MaterialProperty object
   * @tparam T The type of the property
   * @param prop_name The name of the property
   * @return The property for the supplied type and name
   */
  template <typename T>
  MaterialProperty<T> & getProperty(const std::string & prop_name);
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
   * Resizes the number of properties to the specified size (including
   * stateful old and older properties.  Newly added elements are set to
   * default-constructed material properties.
   */
  template<typename T>
  void resizeProps(unsigned int size);

  /// Status of storage swapping (calling swap sets this to true; swapBack sets it to false)
  bool _swapped;

};

template <typename T>
inline bool
MaterialData::haveProperty (const std::string & prop_name) const
{
  if (!_storage.hasProperty(prop_name))
    return false;

  unsigned int prop_id = _storage.getPropertyId(prop_name);
  if (prop_id >= _props.size())
    return false;           // the property id exists, but the property was not created in this instance of the material type

  if (dynamic_cast<const MaterialProperty<T>*>(_props[prop_id]) != NULL)
    return true;

  return false;
}

template <typename T>
inline bool
MaterialData::havePropertyOld (const std::string & prop_name) const
{

  if (!_storage.hasProperty(prop_name))
    return false;

  unsigned int prop_id = _storage.getPropertyId(prop_name);
  if (dynamic_cast<const MaterialProperty<T>*>(_props_old[prop_id]) != NULL)
    return true;

  return false;
}

template <typename T>
inline bool
MaterialData::havePropertyOlder (const std::string & prop_name) const
{
  if (!_storage.hasProperty(prop_name))
    return false;

  unsigned int prop_id = _storage.getPropertyId(prop_name);
  if (dynamic_cast<const MaterialProperty<T>*>(_props_older[prop_id]) != NULL)
    return true;

  return false;
}

template<typename T>
void
MaterialData::resizeProps(unsigned int size)
{
  if (_props.size() <= size)
    _props.resize(size + 1, NULL);
  if (_props_old.size() <= size)
    _props_old.resize(size + 1, NULL);
  if (_props_older.size() <= size)
    _props_older.resize(size + 1, NULL);

  if (_props[size] == NULL)
    _props[size] = new MaterialProperty<T>;
  if (_props_old[size] == NULL)
    _props_old[size] = new MaterialProperty<T>;
  if (_props_older[size] == NULL)
    _props_older[size] = new MaterialProperty<T>;
}

template<typename T>
MaterialProperty<T> &
MaterialData::declareProperty(const std::string & prop_name)
{
  unsigned int prop_id = _storage.addProperty(prop_name);
  resizeProps<T>(prop_id);

  MaterialProperty<T> *prop = dynamic_cast<MaterialProperty<T>*>(_props[prop_id]);
  mooseAssert(prop != NULL, "Internal error in declaring material property: " + prop_name);

  return *prop;
}

template<typename T>
MaterialProperty<T> &
MaterialData::declarePropertyOld(const std::string & prop_name)
{
  unsigned int prop_id = _storage.addPropertyOld(prop_name);
  resizeProps<T>(prop_id);

  MaterialProperty<T> *prop = dynamic_cast<MaterialProperty<T>*>(_props_old[prop_id]);
  mooseAssert(prop != NULL, "Internal error in declaring material property: " + prop_name);

  return *prop;
}

template<typename T>
MaterialProperty<T> &
MaterialData::declarePropertyOlder(const std::string & prop_name)
{
  unsigned int prop_id = _storage.addPropertyOlder(prop_name);
  resizeProps<T>(prop_id);

  MaterialProperty<T> *prop = dynamic_cast<MaterialProperty<T>*>(_props_older[prop_id]);
  mooseAssert(prop != NULL, "Internal error in declaring material property: " + prop_name);

  return *prop;
}


template<typename T>
MaterialProperty<T> &
MaterialData::getProperty(const std::string & name)
{
  unsigned int prop_id = _storage.getPropertyId(name);
  resizeProps<T>(prop_id);

  MaterialProperty<T> * prop = dynamic_cast<MaterialProperty<T> *>(_props[prop_id]);
  if (prop != NULL)
    return *prop;
  else
    mooseError("Material has no property named: " + name);
}

template<typename T>
MaterialProperty<T> &
MaterialData::getPropertyOld(const std::string & name)
{
  unsigned int prop_id = _storage.getPropertyId(name);
  resizeProps<T>(prop_id);

  MaterialProperty<T> * prop = dynamic_cast<MaterialProperty<T> *>(_props_old[prop_id]);
  if (prop != NULL)
    return *prop;
  else
    mooseError("Material has no property named: " + name);
}

template<typename T>
MaterialProperty<T> &
MaterialData::getPropertyOlder(const std::string & name)
{
  unsigned int prop_id = _storage.getPropertyId(name);
  resizeProps<T>(prop_id);

  MaterialProperty<T> * prop = dynamic_cast<MaterialProperty<T> *>(_props_older[prop_id]);
  if (prop != NULL)
    return *prop;
  else
    mooseError("Material has no property named: " + name);
}

#endif /* MATERIALDATA_H */
