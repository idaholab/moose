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

#define usingInterfaceMaterialMembers                                                              \
  usingMaterialBaseMembers;                                                                        \
  usingNeighborCoupleableMembers;                                                                  \
  using InterfaceMaterial::_q_point;                                                               \
  using InterfaceMaterial::_qrule;                                                                 \
  using InterfaceMaterial::_JxW;                                                                   \
  using InterfaceMaterial::_current_elem;                                                          \
  using InterfaceMaterial::_neighbor_elem;                                                         \
  using InterfaceMaterial::_current_side;                                                          \
  using InterfaceMaterial::_neighbor_side

/**
 * Interface materials compute MaterialProperties.
 */
class InterfaceMaterial : public MaterialBase,
                          public NeighborCoupleable,
                          public TwoMaterialPropertyInterface
{
public:
  static InputParameters validParams();

  InterfaceMaterial(const InputParameters & parameters);

  virtual bool isInterfaceMaterial() override { return true; };
  void computeProperties() override;

  ///@{
  /**
   * Retrieve the property through a given input parameter key with a fallback
   * to getting it by name
   */
  template <typename T, bool is_ad>
  const auto & getGenericMaterialProperty(const std::string & name)
  {
    if constexpr (is_ad)
      return getADMaterialProperty<T>(name);
    else
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
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyByName(const std::string & prop_name);
  template <typename T>
  const ADMaterialProperty<T> & getADMaterialPropertyByName(const std::string & prop_name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOldByName(const std::string & prop_name);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderByName(const std::string & prop_name);
  ///@}

  ///@{
  /**
   * Retrieve the neighbor property through a given input parameter key with a fallback
   * to getting it by name
   */
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialProperty(const std::string & name);

  template <typename T>
  const ADMaterialProperty<T> & getNeighborADMaterialProperty(const std::string & name);

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
  const MaterialProperty<T> & getNeighborMaterialPropertyByName(const std::string & prop_name);

  template <typename T>
  const ADMaterialProperty<T> & getNeighborADMaterialPropertyByName(const std::string & name);

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
InterfaceMaterial::getMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const ADMaterialProperty<T> &
InterfaceMaterial::getADMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const ADMaterialProperty<T> * default_property = defaultADMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getADMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getMaterialPropertyOld(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyOldByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getMaterialPropertyOlder(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getMaterialPropertyOlderByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getMaterialPropertyByName(const std::string & prop_name)
{
  MaterialBase::checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, MaterialPropState::CURRENT);
  return TwoMaterialPropertyInterface::getMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const ADMaterialProperty<T> &
InterfaceMaterial::getADMaterialPropertyByName(const std::string & prop_name)
{
  MaterialBase::checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getADMaterialProperty are referencing
  // the same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, MaterialPropState::CURRENT);
  return TwoMaterialPropertyInterface::getADMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getMaterialPropertyOldByName(const std::string & prop_name)
{
  registerPropName(prop_name, true, MaterialPropState::OLD);
  return TwoMaterialPropertyInterface::getMaterialPropertyOldByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getMaterialPropertyOlderByName(const std::string & prop_name)
{
  registerPropName(prop_name, true, MaterialPropState::OLDER);
  return TwoMaterialPropertyInterface::getMaterialPropertyOlderByName<T>(prop_name);
}

// Neighbor material properties

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getNeighborMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getNeighborMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getNeighborMaterialPropertyByName(const std::string & prop_name)
{
  MaterialBase::checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, MaterialPropState::CURRENT);
  return TwoMaterialPropertyInterface::getNeighborMaterialPropertyByName<T>(prop_name);
}
template <typename T>
const ADMaterialProperty<T> &
InterfaceMaterial::getNeighborADMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const ADMaterialProperty<T> * default_property = defaultADMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return getNeighborADMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const ADMaterialProperty<T> &
InterfaceMaterial::getNeighborADMaterialPropertyByName(const std::string & prop_name)
{
  MaterialBase::checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, MaterialPropState::CURRENT);
  return TwoMaterialPropertyInterface::getNeighborADMaterialPropertyByName<T>(prop_name);
}
template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getNeighborMaterialPropertyOld(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
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
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return TwoMaterialPropertyInterface::getNeighborMaterialPropertyOlder<T>(prop_name);
}
