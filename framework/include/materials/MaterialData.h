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

#include <vector>
#include "MaterialProperty.h"
#include "MaterialPropertyStorage.h"
#include "elem.h"

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
  void reinit(std::vector<Material *> & mats, unsigned int n_qpoints, const Elem & elem, unsigned int side = 0);

  MaterialProperties & props() { return _props; }
  MaterialProperties & propsOld() { return _props_old; }
  MaterialProperties & propsOlder() { return _props_older; }

  unsigned int getPropertyId (const std::string & prop_name) const;

  template <typename T>
  bool haveProperty(const std::string & prop_name) const;

  template <typename T>
  bool havePropertyOld(const std::string & prop_name) const;

  template <typename T>
  bool havePropertyOlder(const std::string & prop_name) const;

protected:
  MaterialPropertyStorage & _storage;

  bool _sized;

  // holds material properties for currently selected element (and possibly a side), they are being copied from _storage
  MaterialProperties _props;
  MaterialProperties _props_old;
  MaterialProperties _props_older;

  /// mapping from property name to property ID
  std::map<std::string, unsigned int> _prop_ids;

  unsigned int addProperty (const std::string & prop_name);
};


inline unsigned int
MaterialData::getPropertyId (const std::string & prop_name) const
{
  std::map<std::string, unsigned int>::const_iterator it = _prop_ids.find(prop_name);
  if (it == _prop_ids.end())
    mooseError("No property name mapping for '" + prop_name + "'");

  return it->second;
}

inline unsigned int
MaterialData::addProperty (const std::string & prop_name)
{
  std::map<std::string, unsigned int>::iterator it = _prop_ids.find(prop_name);
  if (it == _prop_ids.end())
  {
    unsigned int id = _prop_ids.size();
    _prop_ids[prop_name] = id;
    // reserve the space for props
    _props.resize(id + 1);
    _props_old.resize(id + 1);
    _props_older.resize(id + 1);
    return id;
  }
  else
    return it->second;
}

template <typename T>
inline bool
MaterialData::haveProperty (const std::string & prop_name) const
{
  std::map<std::string, unsigned int>::const_iterator it = _prop_ids.find(prop_name);
  if (it == _prop_ids.end())
    return false;

  if (dynamic_cast<const MaterialProperty<T>*>(_props[it->second]) != NULL)
    return true;

  return false;
}

template <typename T>
inline bool
MaterialData::havePropertyOld (const std::string & prop_name) const
{
  std::map<std::string, unsigned int>::const_iterator it = _prop_ids.find(prop_name);
  if (it == _prop_ids.end())
    return false;

  if (dynamic_cast<const MaterialProperty<T>*>(_props_old[it->second]) != NULL)
    return true;

  return false;
}

template <typename T>
inline bool
MaterialData::havePropertyOlder (const std::string & prop_name) const
{
  std::map<std::string, unsigned int>::const_iterator it = _prop_ids.find(prop_name);
  if (it == _prop_ids.end())
    return false;

  if (dynamic_cast<const MaterialProperty<T>*>(_props_older[it->second]) != NULL)
    return true;

  return false;
}

template<typename T>
MaterialProperty<T> &
MaterialData::declareProperty(const std::string & prop_name)
{
  unsigned int prop_id;
  if (this->haveProperty<T>(prop_name))
    prop_id = _prop_ids[prop_name];
  else
  {
    prop_id = addProperty(prop_name);
    _props[prop_id] = new MaterialProperty<T>;
  }
  _storage.addProperty(prop_id, prop_name);

  MaterialProperty<T> *prop = dynamic_cast<MaterialProperty<T>*>(_props[prop_id]);
  mooseAssert(prop != NULL, "Internal error in declaring material property: " + prop_name);

  return *prop;
}

template<typename T>
MaterialProperty<T> &
MaterialData::declarePropertyOld(const std::string & prop_name)
{
  unsigned int prop_id;
  if (this->havePropertyOld<T>(prop_name))
    prop_id = _prop_ids[prop_name];
  else
  {
    prop_id = addProperty(prop_name);
    _props_old[prop_id] = new MaterialProperty<T>;
  }
  _storage.addPropertyOld(prop_id, prop_name);

  MaterialProperty<T> *prop = dynamic_cast<MaterialProperty<T>*>(_props_old[prop_id]);
  mooseAssert(prop != NULL, "Internal error in declaring material property: " + prop_name);

  return *prop;
}

template<typename T>
MaterialProperty<T> &
MaterialData::declarePropertyOlder(const std::string & prop_name)
{
  unsigned int prop_id;
  if (this->havePropertyOlder<T>(prop_name))
    prop_id = _prop_ids[prop_name];
  else
  {
    prop_id = addProperty(prop_name);
    _props_older[prop_id] = new MaterialProperty<T>;
  }
  _storage.addPropertyOlder(prop_id, prop_name);

  MaterialProperty<T> *prop = dynamic_cast<MaterialProperty<T>*>(_props_older[prop_id]);
  mooseAssert(prop != NULL, "Internal error in declaring material property: " + prop_name);

  return *prop;
}

#endif /* MATERIALDATA_H */
