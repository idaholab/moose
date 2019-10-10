//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOOSE includes
#include "MaterialBase.h"
#include "Coupleable.h"
#include "MaterialPropertyInterface.h"

// forward declarations
class Material;

template <>
InputParameters validParams<Material>();

/**
 * Materials compute MaterialProperties.
 */
class Material : public MaterialBase, public Coupleable, public MaterialPropertyInterface
{
public:
  Material(const InputParameters & parameters);

  virtual void computeProperties() override;

  ///@{
  /**
   * Retrieve the property through a given input parameter key with a fallback
   * to getting it by name
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyTempl(const std::string & name);
  template <typename T>
  const ADMaterialPropertyObject<T> & getADMaterialPropertyTempl(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOldTempl(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderTempl(const std::string & name);
  ///@}

  ///@{
  /**
   * Retrieve the property named "name"
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyByNameTempl(const std::string & prop_name);
  template <typename T>
  const ADMaterialPropertyObject<T> &
  getADMaterialPropertyByNameTempl(const std::string & prop_name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOldByNameTempl(const std::string & prop_name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderByNameTempl(const std::string & prop_name);
  ///@}

  using MaterialBase::getZeroMaterialProperty;

  virtual bool isBoundaryMaterial() const override { return _bnd; }

  virtual const std::set<unsigned int> & getMatPropDependencies() const override
  {
    return MaterialPropertyInterface::getMatPropDependencies();
  }
  virtual void subdomainSetup() override;

protected:
  virtual const MaterialData & materialData() const override { return *_material_data; }
  virtual MaterialData & materialData() override { return *_material_data; }

  virtual const QBase & qRule() const override { return *_qrule; }

  bool _bnd;
  bool _neighbor;

  const MooseArray<Point> & _q_point;
  const QBase * const & _qrule;

  const MooseArray<Real> & _JxW;

  const Elem * const & _current_elem;

  const SubdomainID & _current_subdomain_id;

  /// current side of the current element
  const unsigned int & _current_side;

  enum ConstantTypeEnum
  {
    NONE,
    ELEMENT,
    SUBDOMAIN
  };

  /// Options of the constantness level of the material
  const ConstantTypeEnum _constant_option;
};

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyTempl(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const ADMaterialPropertyObject<T> &
Material::getADMaterialPropertyTempl(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const ADMaterialPropertyObject<T> * default_property = defaultADMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getADMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOldTempl(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyOldByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOlderTempl(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyOlderByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyByNameTempl(const std::string & prop_name)
{
  MaterialBase::checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, MaterialBase::CURRENT);
  return MaterialPropertyInterface::getMaterialPropertyByNameTempl<T>(prop_name);
}

template <typename T>
const ADMaterialPropertyObject<T> &
Material::getADMaterialPropertyByNameTempl(const std::string & prop_name)
{
  MaterialBase::checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getADMaterialProperty are referencing
  // the same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, Material::CURRENT);
  return MaterialPropertyInterface::getADMaterialPropertyByNameTempl<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOldByNameTempl(const std::string & prop_name)
{
  registerPropName(prop_name, true, MaterialBase::OLD);
  return MaterialPropertyInterface::getMaterialPropertyOldByNameTempl<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOlderByNameTempl(const std::string & prop_name)
{
  registerPropName(prop_name, true, Material::OLDER);
  return MaterialPropertyInterface::getMaterialPropertyOlderByNameTempl<T>(prop_name);
}
