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
#include "NeighborCoupleable.h"
#include "TwoMaterialPropertyInterface.h"

// forward declarations
class InterfaceMaterial;

template <>
InputParameters validParams<InterfaceMaterial>();

/**
 * Interface materials compute MaterialProperties.
 */
class InterfaceMaterial : public MaterialBase,
                          public NeighborCoupleable,
                          public TwoMaterialPropertyInterface
{
public:
  InterfaceMaterial(const InputParameters & parameters);

  virtual bool isInterfaceMaterial() override { return true; };
  void computeProperties() override;

  ///@{
  /**
   * Retrieve the property through a given input parameter key with a fallback
   * to getting it by name
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyTempl(const std::string & name);
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
  const MaterialProperty<T> & getMaterialPropertyOldByNameTempl(const std::string & prop_name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderByNameTempl(const std::string & prop_name);
  ///@}

  ///@{
  /**
   * Retrieve the neighbor property through a given input parameter key with a fallback
   * to getting it by name
   */
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyTempl(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyOld(const std::string & name);
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyOlder(const std::string & name);
  ///@}

  ///@{
  /**
   * Retrieve the neighbor property named "name"
   */
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyByNameTempl(const std::string & prop_name);
  ///@}

  using MaterialBase::getZeroMaterialProperty;

  virtual bool isBoundaryMaterial() const override { return true; }

  virtual const std::set<unsigned int> & getMatPropDependencies() const override
  {
    return TwoMaterialPropertyInterface::getMatPropDependencies();
  }

protected:
  virtual const MaterialData & materialData() const override { return *_material_data; }
  virtual MaterialData & materialData() override { return *_material_data; }

  virtual const QBase & qRule() const override { return *_qrule; }

  bool _bnd;
  const MooseArray<Point> & _q_point;
  const QBase * const & _qrule;
  const MooseArray<Real> & _JxW;

  /// Current element
  const Elem * const & _current_elem;
  /// Current neighbor element
  const Elem * const & _neighbor_elem;
  /// current side of the current element
  const unsigned int & _current_side;
  /// current side of the neighbor element
  const unsigned int & _neighbor_side;
};

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getMaterialPropertyTempl(const std::string & name)
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
const MaterialProperty<T> &
InterfaceMaterial::getMaterialPropertyOldTempl(const std::string & name)
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
InterfaceMaterial::getMaterialPropertyOlderTempl(const std::string & name)
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
InterfaceMaterial::getMaterialPropertyByNameTempl(const std::string & prop_name)
{
  MaterialBase::checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, MaterialBase::CURRENT);
  return TwoMaterialPropertyInterface::getMaterialPropertyByNameTempl<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getMaterialPropertyOldByNameTempl(const std::string & prop_name)
{
  registerPropName(prop_name, true, MaterialBase::OLD);
  return TwoMaterialPropertyInterface::getMaterialPropertyOldByNameTempl<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getMaterialPropertyOlderByNameTempl(const std::string & prop_name)
{
  registerPropName(prop_name, true, MaterialBase::OLDER);
  return TwoMaterialPropertyInterface::getMaterialPropertyOlderByNameTempl<T>(prop_name);
}

// Neighbor material properties

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getNeighborMaterialPropertyTempl(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getNeighborMaterialPropertyByNameTempl<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getNeighborMaterialPropertyByNameTempl(const std::string & prop_name)
{
  MaterialBase::checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, MaterialBase::CURRENT);
  return TwoMaterialPropertyInterface::getNeighborMaterialPropertyByNameTempl<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getNeighborMaterialPropertyOld(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return TwoMaterialPropertyInterface::getNeighborMaterialPropertyOld<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getNeighborMaterialPropertyOlder(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return TwoMaterialPropertyInterface::getNeighborMaterialPropertyOlder<T>(prop_name);
}
