#ifndef MATERIALPROPERTYINTERFACE_H_
#define MATERIALPROPERTYINTERFACE_H_

#include <map>
#include <string>

#include "Moose.h"
#include "MaterialProperty.h"

namespace Moose {
class Problem;
}

/**
 * Any object that needs material properties has to inherit this interface
 */
class MaterialPropertyInterface
{
public:
  MaterialPropertyInterface(Moose::Problem &problem);

  /**
   * Retrieve the property named "name"
   */
  template<typename T>
  MaterialProperty<T> & getMaterialProperty(const std::string & name);

  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);

  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);


protected:
  MaterialProperties & _material_props;
  MaterialProperties & _material_props_old;
  MaterialProperties & _material_props_older;
};


template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialProperty(const std::string & name)
{
  MaterialProperties::const_iterator it = _material_props.find(name);

  if (it != _material_props.end())
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
  MaterialProperties::const_iterator it = _material_props_old.find(name);

  if (it != _material_props_old.end())
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
  MaterialProperties::const_iterator it = _material_props_older.find(name);

  if (it != _material_props_older.end())
  {
    mooseAssert (dynamic_cast<const MaterialProperty<T>*>(it->second) != NULL, "");
    return *dynamic_cast<MaterialProperty<T>*>(it->second);
  }

  mooseError("Material has no property named: " + name);
}

#endif //MATERIALPROPERTYINTERFACE_H_
