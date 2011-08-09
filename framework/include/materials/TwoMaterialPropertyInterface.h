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

#ifndef TWOMATERIALPROPERTYINTERFACE_H
#define TWOMATERIALPROPERTYINTERFACE_H

#include <map>
#include <string>

#include "Moose.h"
#include "MaterialData.h"
#include "MaterialPropertyInterface.h"

// Forward Declarations

class TwoMaterialPropertyInterface : public MaterialPropertyInterface
{
public:
  TwoMaterialPropertyInterface(InputParameters & parameters);

  /**
   * Retrieve the property named "name"
   */
  template<typename T>
  MaterialProperty<T> & getNeighborMaterialProperty(const std::string & name);

  template<typename T>
  MaterialProperty<T> & getNeighborMaterialPropertyOld(const std::string & name);

  template<typename T>
  MaterialProperty<T> & getNeighborMaterialPropertyOlder(const std::string & name);

protected:
  MaterialData & _neighbor_material_data;
  MaterialProperties & _neighbor_material_props;
  MaterialProperties & _neighbor_material_props_old;
  MaterialProperties & _neighbor_material_props_older;
};


template<typename T>
MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialProperty(const std::string & name)
{
  MaterialProperties::const_iterator it = _neighbor_material_props.find(name);

  if (it != _neighbor_material_props.end())
  {
    mooseAssert (dynamic_cast<const MaterialProperty<T>*>(it->second) != NULL, "");
    return *dynamic_cast<MaterialProperty<T>*>(it->second);
  }

  mooseError("Material has no property named: " + name);
}

template<typename T>
MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOld(const std::string & name)
{
  MaterialProperties::const_iterator it = _neighbor_material_props_old.find(name);

  if (it != _neighbor_material_props_old.end())
  {
    mooseAssert (dynamic_cast<const MaterialProperty<T>*>(it->second) != NULL, "");
    return *dynamic_cast<MaterialProperty<T>*>(it->second);
  }

  mooseError("Material has no property named: " + name);
}

template<typename T>
MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOlder(const std::string & name)
{
  MaterialProperties::const_iterator it = _neighbor_material_props_older.find(name);

  if (it != _neighbor_material_props_older.end())
  {
    mooseAssert (dynamic_cast<const MaterialProperty<T>*>(it->second) != NULL, "");
    return *dynamic_cast<MaterialProperty<T>*>(it->second);
  }

  mooseError("Material has no property named: " + name);
}

#endif //TWOMATERIALPROPERTYINTERFACE_H
