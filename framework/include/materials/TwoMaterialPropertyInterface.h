//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TWOMATERIALPROPERTYINTERFACE_H
#define TWOMATERIALPROPERTYINTERFACE_H

#include "MaterialPropertyInterface.h"

// Forward Declarations
class MaterialData;
class TwoMaterialPropertyInterface;

template <>
InputParameters validParams<TwoMaterialPropertyInterface>();

class TwoMaterialPropertyInterface : public MaterialPropertyInterface
{
public:
  TwoMaterialPropertyInterface(const MooseObject * moose_object);

  TwoMaterialPropertyInterface(const MooseObject * moose_object,
                               const std::set<SubdomainID> & blocks_ids);

  TwoMaterialPropertyInterface(const MooseObject * moose_object,
                               const std::set<BoundaryID> & boundary_ids);

  TwoMaterialPropertyInterface(const MooseObject * moose_object,
                               const std::set<SubdomainID> & blocks_ids,
                               const std::set<BoundaryID> & boundary_ids);

  /**
   * Retrieve the property named "name"
   */
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialProperty(const std::string & name);

  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyOld(const std::string & name);

  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyOlder(const std::string & name);

protected:
  std::shared_ptr<MaterialData> _neighbor_material_data;
};

template <typename T>
const MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;
  else
    return _neighbor_material_data->getProperty<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOld(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;
  else
    return _neighbor_material_data->getPropertyOld<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOlder(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;
  else
    return _neighbor_material_data->getPropertyOlder<T>(prop_name);
}

#endif // TWOMATERIALPROPERTYINTERFACE_H
