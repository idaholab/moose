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
 *
 */
class MaterialData
{
public:
  MaterialData(MaterialPropertyStorage & storage);
  virtual ~MaterialData();

  /**
   * Size the properties
   */
  void size(unsigned int n_qpoints);

  /**
   * Get the number of quadrature points
   * @return The number of quadrature points
   */
  unsigned int nQPoints();

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  template<typename T>
  MaterialProperty<T> & declareProperty(const std::string & prop_name);

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOld().
   */
  template<typename T>
  MaterialProperty<T> & declarePropertyOld(const std::string & prop_name);

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOlder().
   */
  template<typename T>
  MaterialProperty<T> & declarePropertyOlder(const std::string & prop_name);

  /* Non-templated property routines */
  bool have_property_name(const std::string & prop_name) const;
  bool have_property_name_old(const std::string & prop_name) const;
  bool have_property_name_older(const std::string & prop_name) const;

  // Reinit material properties for given element (and possible side)
  void reinit(std::vector<Material *> & mats, const Elem & elem, unsigned int side = 0);

  MaterialProperties & props() { return _props; }
  MaterialProperties & propsOld() { return _props_old; }
  MaterialProperties & propsOlder() { return _props_older; }

  template <typename T>
  bool haveProperty(const std::string & prop_name) const;

  template <typename T>
  bool havePropertyOld(const std::string & prop_name) const;

  template <typename T>
  bool havePropertyOlder(const std::string & prop_name) const;


  template <typename T>
  MaterialProperty<T> & getProperty(const std::string & prop_name);

  template <typename T>
  MaterialProperty<T> & getPropertyOld(const std::string & prop_name);

  template <typename T>
  MaterialProperty<T> & getPropertyOlder(const std::string & prop_name);

protected:
  MaterialPropertyStorage & _storage;
  /// Number of quadrature points
  unsigned int _n_qpoints;

  // holds material properties for currently selected element (and possibly a side), they are being copied from _storage
  MaterialProperties _props;
  MaterialProperties _props_old;
  MaterialProperties _props_older;

  template<typename T>
  void resizeProps(unsigned int size);
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
