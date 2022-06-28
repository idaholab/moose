//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TwoMaterialPropertyInterface.h"

// Forward Declarations
class MaterialData;
class ThreeMaterialPropertyInterface : public TwoMaterialPropertyInterface
{
public:
  ThreeMaterialPropertyInterface(const MooseObject * moose_object,
                                 const std::set<SubdomainID> & blocks_ids,
                                 const std::set<BoundaryID> & boundary_ids);

  static InputParameters validParams();

  /**
   * Retrieve the property deduced from the name \p name
   */
  template <typename T>
  const MaterialProperty<T> & getFaceMaterialProperty(const std::string & name);

  /**
   * Retrieve the property named "name" without any deduction
   */
  template <typename T>
  const MaterialProperty<T> & getFaceMaterialPropertyByName(const std::string & name);

  /**
   * Retrieve the ADMaterialProperty named "name"
   */
  template <typename T>
  const ADMaterialProperty<T> & getFaceADMaterialProperty(const std::string & name);

  template <typename T>
  const MaterialProperty<T> & getFaceMaterialPropertyOld(const std::string & name);

  template <typename T>
  const MaterialProperty<T> & getFaceMaterialPropertyOlder(const std::string & name);

  /**
   * Retrieve the face material property whether AD or not
   */
  template <typename T, bool is_ad, typename std::enable_if<is_ad, int>::type = 0>
  const ADMaterialProperty<T> & getGenericFaceMaterialProperty(const std::string & name)
  {
    return getFaceADMaterialProperty<T>(name);
  }
  template <typename T, bool is_ad, typename std::enable_if<!is_ad, int>::type = 0>
  const MaterialProperty<T> & getGenericFaceMaterialProperty(const std::string & name)
  {
    return getFaceMaterialProperty<T>(name);
  }

protected:
  std::shared_ptr<MaterialData> _face_material_data;
};

template <typename T>
const MaterialProperty<T> &
ThreeMaterialPropertyInterface::getFaceMaterialProperty(const std::string & name)
{
  return getMaterialProperty<T>(name, *_face_material_data);
}

template <typename T>
const MaterialProperty<T> &
ThreeMaterialPropertyInterface::getFaceMaterialPropertyByName(const std::string & name)
{
  return getMaterialPropertyByName<T>(name, *_face_material_data);
}

template <typename T>
const ADMaterialProperty<T> &
ThreeMaterialPropertyInterface::getFaceADMaterialProperty(const std::string & name)
{
  return getADMaterialProperty<T>(name, *_face_material_data);
}

template <typename T>
const MaterialProperty<T> &
ThreeMaterialPropertyInterface::getFaceMaterialPropertyOld(const std::string & name)
{
  return getMaterialPropertyOld<T>(name, *_face_material_data);
}

template <typename T>
const MaterialProperty<T> &
ThreeMaterialPropertyInterface::getFaceMaterialPropertyOlder(const std::string & name)
{
  return getMaterialPropertyOlder<T>(name, *_face_material_data);
}
