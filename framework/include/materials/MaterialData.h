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
#include "elem.h"

class Material;
class MaterialPropertyStorage;

/// Proxy for accessing MaterialPropertyStorage
class MaterialData
{
public:
  MaterialData(MaterialPropertyStorage & storage);

  virtual ~MaterialData();

//  virtual ~MaterialData()
//  {
//    {
//      MaterialProperties::iterator it;
//      for (it = _props.begin(); it != _props.end(); ++it)
//      {
//        if (it->second != NULL)
//        {
//          delete it->second;
//          it->second = NULL;
//        }
//      }
//
//      for (it = _props_old.begin(); it != _props_old.end(); ++it)
//      {
//        if (it->second != NULL)
//        {
//          delete it->second;
//          it->second = NULL;
//        }
//      }
//      for (it = _props_older.begin(); it != _props_older.end(); ++it)
//      {
//        if (it->second != NULL)
//        {
//          delete it->second;
//          it->second = NULL;
//        }
//      }
//    }
//  }

  /**
   * Size the properties
   */
  void size(unsigned int n_qpoints);

  bool hasStatefulProperties();

  void initStatefulProps(std::vector<Material *> & mats, unsigned int n_qpoints, const Elem & elem, unsigned int side = 0);

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

  // Reinit material properties for given element (and possible side)
  void reinit(std::vector<Material *> & mats, unsigned int n_qpoints, const Elem & elem, unsigned int side = 0);

  MaterialProperties & props() { return _props; }
  MaterialProperties & propsOld() { return _props_old; }
  MaterialProperties & propsOlder() { return _props_older; }

protected:
  MaterialPropertyStorage & _storage;

  bool _sized;

  /**
   * Whether or not this material has stateful properties.  This will get automatically
   * set to true if a stateful property is declared.
   */
  bool _has_stateful_props;

  /**
   * Data structure to map names with values.
   */
  std::set<std::string> _stateful_props;

  // holds material properties for currently selected element (and possibly a side), they are being copied from _strorage
  MaterialProperties _props;
  MaterialProperties _props_old;
  MaterialProperties _props_older;

  template <typename T>
  bool have_property(const std::string & prop_name) const;

  template <typename T>
  bool have_property_old(const std::string & prop_name) const;

  template <typename T>
  bool have_property_older(const std::string & prop_name) const;
};


template <typename T>
inline bool
MaterialData::have_property (const std::string & prop_name) const
{
  MaterialProperties::const_iterator it = _props.find(prop_name);

  if (it != _props.end())
    if (dynamic_cast<const MaterialProperty<T>*>(it->second) != NULL)
      return true;

  return false;
}

template <typename T>
inline bool
MaterialData::have_property_old (const std::string & prop_name) const
{
  MaterialProperties::const_iterator it = _props_old.find(prop_name);

  if (it != _props_old.end())
    if (dynamic_cast<const MaterialProperty<T>*>(it->second) != NULL)
      return true;

  return false;
}

template <typename T>
inline bool
MaterialData::have_property_older (const std::string & prop_name) const
{
  MaterialProperties::const_iterator it = _props_older.find(prop_name);

  if (it != _props_older.end())
    if (dynamic_cast<const MaterialProperty<T>*>(it->second) != NULL)
      return true;

  return false;
}

template<typename T>
MaterialProperty<T> &
MaterialData::declareProperty(const std::string & prop_name)
{
  if (!this->have_property<T>(prop_name))
    _props[prop_name] = new MaterialProperty<T>;

  MaterialProperty<T> *prop = dynamic_cast<MaterialProperty<T>*>(_props[prop_name]);
  mooseAssert(prop != NULL, "Internal error in declaring material property: " + prop_name);

  return *prop;
}

template<typename T>
MaterialProperty<T> &
MaterialData::declarePropertyOld(const std::string & prop_name)
{
  _has_stateful_props = true;
  _stateful_props.insert(prop_name);

  if (!this->have_property_old<T>(prop_name))
    _props_old[prop_name] = new MaterialProperty<T>;

  MaterialProperty<T> *prop = dynamic_cast<MaterialProperty<T>*>(_props_old[prop_name]);
  mooseAssert(prop != NULL, "Internal error in declaring material property: " + prop_name);

  return *prop;
}

template<typename T>
MaterialProperty<T> &
MaterialData::declarePropertyOlder(const std::string & prop_name)
{
  _has_stateful_props = true;
  _stateful_props.insert(prop_name);

  if (!this->have_property_older<T>(prop_name))
    _props_older[prop_name] = new MaterialProperty<T>;

  MaterialProperty<T> *prop = dynamic_cast<MaterialProperty<T>*>(_props_older[prop_name]);
  mooseAssert(prop != NULL, "Internal error in declaring material property: " + prop_name);

  return *prop;
}

#endif /* MATERIALDATA_H */
