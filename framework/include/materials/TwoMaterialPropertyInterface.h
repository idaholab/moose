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
   * Retrieve the neighbor property deduced from the name \p name
   *
   * \p state is the property state; 0 = current, 1 = old, 2 = older, etc.
   */
  ///@{
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> &
  getGenericNeighborMaterialProperty(const std::string & name, const unsigned int state = 0)
  {
    return getGenericMaterialProperty<T, is_ad>(name, _neighbor_material_data, state);
  }
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialProperty(const std::string & name,
                                                          const unsigned int state = 0)
  {
    return getGenericNeighborMaterialProperty<T, false>(name, state);
  }
  template <typename T>
  const ADMaterialProperty<T> & getNeighborADMaterialProperty(const std::string & name)
  {
    return getGenericNeighborMaterialProperty<T, true>(name, 0);
  }
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyOld(const std::string & name)
  {
    return getGenericNeighborMaterialProperty<T, false>(name, 1);
  }
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyOlder(const std::string & name)
  {
    return getGenericNeighborMaterialProperty<T, false>(name, 2);
  }
  ///@}

  /**
   * Retrieve the neighbor property named "name" without any deduction
   *
   * \p state is the property state; 0 = current, 1 = old, 2 = older, etc.
   */
  ///@{
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> &
  getGenericNeighborMaterialPropertyByName(const std::string & name, const unsigned int state = 0)
  {
    return getGenericMaterialPropertyByName<T, is_ad>(name, _neighbor_material_data, state);
  }
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyByName(const std::string & name,
                                                                const unsigned int state = 0)
  {
    return getGenericNeighborMaterialPropertyByName<T, false>(name, state);
  }
  template <typename T>
  const ADMaterialProperty<T> & getNeighborADMaterialPropertyByName(const std::string & name)
  {
    return getGenericNeighborMaterialPropertyByName<T, true>(name, 0);
  }
  ///@}

protected:
  MaterialData & _neighbor_material_data;
};
