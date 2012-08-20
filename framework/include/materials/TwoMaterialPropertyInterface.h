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
  unsigned int prop_id = _neighbor_material_data.getPropertyId(name);
  MaterialProperty<T> * prop = dynamic_cast<MaterialProperty<T> *>(_neighbor_material_props[prop_id]);
  if (prop != NULL)
    return *prop;
  else
    mooseError("Material has no property named: " + name);
}

template<typename T>
MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOld(const std::string & name)
{
  unsigned int prop_id = _neighbor_material_data.getPropertyId(name);
  MaterialProperty<T> * prop = dynamic_cast<MaterialProperty<T> *>(_neighbor_material_props_old[prop_id]);
  if (prop != NULL)
    return *prop;
  else
    mooseError("Material has no property named: " + name);
}

template<typename T>
MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOlder(const std::string & name)
{
  unsigned int prop_id = _neighbor_material_data.getPropertyId(name);
  MaterialProperty<T> * prop = dynamic_cast<MaterialProperty<T> *>(_neighbor_material_props_older[prop_id]);
  if (prop != NULL)
    return *prop;
  else
    mooseError("Material has no property named: " + name);
}

#endif //TWOMATERIALPROPERTYINTERFACE_H
