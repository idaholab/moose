//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MaterialPropertyInterface.h"

// Forward Declarations
class MaterialData;

/**
 * This interface is designed for DGKernel, InternalSideUserObject, InterfaceUserObject,
 * where material properties on a side of both its primary side (face) and its secondary side
 * (neighbor) all required.
 */
class TwoMaterialPropertyInterface : public MaterialPropertyInterface
{
public:
  TwoMaterialPropertyInterface(const MooseObject * moose_object,
                               const std::set<SubdomainID> & blocks_ids,
                               const std::set<BoundaryID> & boundary_ids);

  static InputParameters validParams();

  /**
   * Retrieve the property deduced from the name \p name
   */
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialProperty(const std::string & name);

  /**
   * Retrieve the property named "name" without any deduction
   */
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyByName(const std::string & name);

  /**
   * Retrieve the ADMaterialProperty named "name"
   */
  template <typename T>
  const ADMaterialProperty<T> & getNeighborADMaterialProperty(const std::string & name);

  template <typename T>
  const ADMaterialProperty<T> & getNeighborADMaterialPropertyByName(const std::string & name);

  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyOld(const std::string & name);

  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyOlder(const std::string & name);

  /**
   * Retrieve the neighbor material property whether AD or not
   */
  template <typename T, bool is_ad>
  const auto & getGenericNeighborMaterialProperty(const std::string & name)
  {
    if constexpr (is_ad)
      return getADMaterialProperty<T>(name, *_neighbor_material_data);
    else
      return getMaterialProperty<T>(name, *_neighbor_material_data);
  }

protected:
  std::shared_ptr<MaterialData> _neighbor_material_data;
};

template <typename T>
const MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialProperty(const std::string & name)
{
  return getMaterialProperty<T>(name, *_neighbor_material_data);
}

template <typename T>
const MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyByName(const std::string & name)
{
  return getMaterialPropertyByName<T>(name, *_neighbor_material_data);
}

template <typename T>
const ADMaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborADMaterialPropertyByName(const std::string & name)
{
  return getADMaterialPropertyByName<T>(name, *_neighbor_material_data);
}

template <typename T>
const ADMaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborADMaterialProperty(const std::string & name)
{
  return getADMaterialProperty<T>(name, *_neighbor_material_data);
}

template <typename T>
const MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOld(const std::string & name)
{
  return getMaterialPropertyOld<T>(name, *_neighbor_material_data);
}

template <typename T>
const MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOlder(const std::string & name)
{
  return getMaterialPropertyOlder<T>(name, *_neighbor_material_data);
}
