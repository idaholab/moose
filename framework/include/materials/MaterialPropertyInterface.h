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

#ifndef MATERIALPROPERTYINTERFACE_H
#define MATERIALPROPERTYINTERFACE_H

#include <map>
#include <string>

#include "Moose.h"
#include "MaterialProperty.h"
#include "InputParameters.h"

class SubProblem;
class MaterialData;

/**
 * Any object that needs material properties has to inherit this interface
 */
class MaterialPropertyInterface
{
public:
  MaterialPropertyInterface(InputParameters & parameters);

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
  MaterialData & _material_data;
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


#endif //MATERIALPROPERTYINTERFACE_H
