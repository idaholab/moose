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

#include "MaterialProperty.h"
#include "InputParameters.h"
#include "MaterialData.h"

class SubProblem;

/**
 * Any object that needs material properties has to inherit this interface
 */
class MaterialPropertyInterface
{
public:
  MaterialPropertyInterface(InputParameters & parameters) :
      _material_data(*parameters.get<MaterialData *>("_material_data"))
    {
    }

  /**
   * Retrieve the property named "name"
   */
  template<typename T>
  MaterialProperty<T> & getMaterialProperty(const std::string & name);

  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);

  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);

  /**
   * Check if the material property exists
   * @param name - the name of the property to query
   * @return true if the property exists, otherwise false
   */
  template<typename T>
  bool hasMaterialProperty(const std::string & name);

protected:
  MaterialData & _material_data;
};


template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialProperty(const std::string & name)
{
  return _material_data.getProperty<T>(name);
}

template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOld(const std::string & name)
{
  return _material_data.getPropertyOld<T>(name);
}

template<typename T>
MaterialProperty<T> &
MaterialPropertyInterface::getMaterialPropertyOlder(const std::string & name)
{
  return _material_data.getPropertyOlder<T>(name);
}

template<typename T>
bool
MaterialPropertyInterface::hasMaterialProperty(const std::string & name)
{
  return _material_data.haveProperty<T>(name);
}


#endif //MATERIALPROPERTYINTERFACE_H
