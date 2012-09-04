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
};


template<typename T>
MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialProperty(const std::string & name)
{
  return _neighbor_material_data.getProperty<T>(name);
}

template<typename T>
MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOld(const std::string & name)
{
  return _neighbor_material_data.getPropertyOld<T>(name);
}

template<typename T>
MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOlder(const std::string & name)
{
  return _neighbor_material_data.getPropertyOlder<T>(name);
}

#endif //TWOMATERIALPROPERTYINTERFACE_H
