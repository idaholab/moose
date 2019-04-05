//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEMATERIAL_H
#define INTERFACEMATERIAL_H

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
  const MaterialProperty<T> & getMaterialProperty(const std::string & name);
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
  const MaterialProperty<T> & getNeighborMaterialPropertyOldByName(const std::string & prop_name);
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyOlderByName(const std::string & prop_name);
  ///@}

  using MaterialBase::getZeroMaterialProperty;

  virtual bool isBoundaryMaterial() const override { return _bnd; }

  virtual const std::set<unsigned int> & getMatPropDependencies() const override
  {
    return TwoMaterialPropertyInterface::getMatPropDependencies();
  }

protected:
  virtual const MaterialData & materialData() const override { return *_material_data; }
  virtual MaterialData & materialData() override { return *_material_data; }

  virtual const QBase & qRule() const override { return *_qrule; }

  bool _bnd;
  bool _neighbor;
  const MooseArray<Point> & _q_point;
  QBase *& _qrule;
  const MooseArray<Real> & _JxW;

  const Elem *& _current_elem;

  const SubdomainID & _current_subdomain_id;

  /// current side of the current element
  unsigned int & _current_side;

  // virtual bool hasActiveInterfaceMaterials(BoundaryID bnd_id, THREAD_ID tid) override
  // {
  //   MaterialWarehouse<THREAD_ID>::checkThreadID(tid);
  //   return true;
  // }
};

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getMaterialProperty(const std::string & name)
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
InterfaceMaterial::getMaterialPropertyOld(const std::string & name)
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
InterfaceMaterial::getMaterialPropertyOlder(const std::string & name)
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
InterfaceMaterial::getMaterialPropertyByName(const std::string & prop_name)
{
  MaterialBase::checkExecutionStage();
  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, MaterialBase::CURRENT);
  return TwoMaterialPropertyInterface::getMaterialPropertyByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getMaterialPropertyOldByName(const std::string & prop_name)
{
  registerPropName(prop_name, true, MaterialBase::OLD);
  return TwoMaterialPropertyInterface::getMaterialPropertyOldByName<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getMaterialPropertyOlderByName(const std::string & prop_name)
{
  registerPropName(prop_name, true, MaterialBase::OLDER);
  return TwoMaterialPropertyInterface::getMaterialPropertyOlderByName<T>(prop_name);
}

// Neighbor material properties

template <typename T>
const MaterialProperty<T> &
InterfaceMaterial::getNeighborMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid imput parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant.
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;

  return TwoMaterialPropertyInterface::getNeighborMaterialProperty<T>(prop_name);
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

#endif // INTERFACEMATERIAL_H
