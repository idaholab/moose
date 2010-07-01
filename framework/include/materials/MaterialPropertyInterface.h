#ifndef MATERIALPROPERTYINTERFACE_H
#define MATERIALPROPERTYINTERFACE_H

#include <map>
#include <string>

#include "Moose.h"
#include "MaterialData.h"

// Forward Declarations

class MaterialPropertyInterface
{
public:
  MaterialPropertyInterface(MaterialData & material_data);

  /**
   * Retrieve the property named "name"
   */
  template<typename T>
  MaterialProperty<T> & getMaterialProperty(const std::string & name);

  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);

  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);

private:
  MaterialData & _material_data;
};


template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialProperty(const std::string & name)
{
  Material::const_iterator it = _material_data._props.find(name);

  if (it != _material_data._props.end())
  {
    mooseAssert (dynamic_cast<const MaterialProperty<T>*>(it->second) != NULL, "");
    return *dynamic_cast<MaterialProperty<T>*>(it->second);
  }

  mooseError("Material has no property named: " + name);
}

template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOld(const std::string & name)
{
  std::map<std::string, T>::iterator it = _material_data._props_old.find(name);

  if(it != _material_data._props_old.end())
  {
    mooseAssert (dynamic_cast<const MaterialProperty<T>*>(it->second) != NULL, "");
    return *dynamic_cast<MaterialProperty<T>*>(it->second);
  }

  mooseError("Material has no property named: " + name);
}

template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOlder(const std::string & name)
{
  std::map<std::string, T>::iterator it = _material_data._props_older.find(name);

  if(it != _material_data._props_older.end())
  {
    mooseAssert (dynamic_cast<const MaterialProperty<T>*>(it->second) != NULL, "");
    return *dynamic_cast<MaterialProperty<T>*>(it->second);
  }

  mooseError("Material has no property named: " + name);
}

#endif //MATERIALPROPERTYINTERFACE_H
