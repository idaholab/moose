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
  static InputParameters validParams();

  Material(const InputParameters & parameters);

  virtual void computeProperties() override;

  ///@{
  /**
   * Retrieve the property through a given input parameter key with a fallback
   * to getting it by name
   */
  template <typename T, bool is_ad, typename std::enable_if<is_ad, int>::type = 0>
  const ADMaterialProperty<T> & getGenericMaterialProperty(const std::string & name)
  {
    return getADMaterialProperty<T>(name);
  }
  template <typename T, bool is_ad, typename std::enable_if<!is_ad, int>::type = 0>
  const MaterialProperty<T> & getGenericMaterialProperty(const std::string & name)
  {
    return getMaterialProperty<T>(name);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name);
  template <typename T>
  const ADMaterialProperty<T> & getADMaterialProperty(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);
  ///@}

  ///@{
  /**
   * Retrieve the property named "name"
   */
  template <typename T, bool is_ad, typename std::enable_if<is_ad, int>::type = 0>
  const ADMaterialProperty<T> & getGenericMaterialPropertyByName(const std::string & name)
  {
    return getADMaterialPropertyByName<T>(name);
  }
  template <typename T, bool is_ad, typename std::enable_if<!is_ad, int>::type = 0>
  const MaterialProperty<T> & getGenericMaterialPropertyByName(const std::string & name)
  {
    return getMaterialPropertyByName<T>(name);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyByName(const std::string & prop_name);
  template <typename T>
  const ADMaterialProperty<T> & getADMaterialPropertyByName(const std::string & prop_name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOldByName(const std::string & prop_name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderByName(const std::string & prop_name);
  ///@}

  using MaterialBase::getGenericZeroMaterialProperty;
  using MaterialBase::getZeroMaterialProperty;

  virtual bool isBoundaryMaterial() const override { return _bnd; }

  virtual const std::set<unsigned int> & getMatPropDependencies() const override
  {
    return MaterialPropertyInterface::getMatPropDependencies();
  }
  virtual void subdomainSetup() override;

  enum class ConstantTypeEnum
  {
    NONE,
    ELEMENT,
    SUBDOMAIN
  };

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

  /// Options of the constantness level of the material
  const ConstantTypeEnum _constant_option;

private:
  ConstantTypeEnum computeConstantOption();
};

template <typename T>
const MaterialProperty<T> &
Material::getMaterialProperty(const std::string & name)
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
const ADMaterialProperty<T> &
Material::getADMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const ADMaterialProperty<T> * default_property = defaultADMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getADMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOld(const std::string & name)
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
Material::getMaterialPropertyOlder(const std::string & name)
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
Material::getMaterialPropertyByName(const std::string & prop_name)
{
  MaterialBase::checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, MaterialBase::CURRENT);
  return MaterialPropertyInterface::getMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const ADMaterialProperty<T> &
Material::getADMaterialPropertyByName(const std::string & prop_name)
{
  MaterialBase::checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getADMaterialProperty are referencing
  // the same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, Material::CURRENT);
  return MaterialPropertyInterface::getADMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOldByName(const std::string & prop_name)
{
  registerPropName(prop_name, true, MaterialBase::OLD);
  return MaterialPropertyInterface::getMaterialPropertyOldByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
Material::getMaterialPropertyOlderByName(const std::string & prop_name)
{
  registerPropName(prop_name, true, Material::OLDER);
  return MaterialPropertyInterface::getMaterialPropertyOlderByName<T>(prop_name);
}
