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
   *
   * \p state is the property state; 0 = current, 1 = old, 2 = older, etc.
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> &
  getGenericMaterialProperty(const std::string & name, const unsigned int state = 0);
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name,
                                                  const unsigned int state = 0)
  {
    return getGenericMaterialProperty<T, false>(name, state);
  }
  template <typename T>
  const ADMaterialProperty<T> & getADMaterialProperty(const std::string & name)
  {
    return getGenericMaterialProperty<T, true>(name, 0);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name)
  {
    return getGenericMaterialProperty<T, false>(name, 1);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name)
  {
    return getGenericMaterialProperty<T, false>(name, 2);
  }
  ///@}

  ///@{
  /**
   * Retrieve the property named "name"
   *
   * \p state is the property state; 0 = current, 1 = old, 2 = older, etc.
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> &
  getGenericMaterialPropertyByName(const std::string & name, const unsigned int state = 0);
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyByName(const std::string & prop_name,
                                                        const unsigned int state = 0)
  {

    return getGenericMaterialPropertyByName<T, false>(prop_name, state);
  }
  template <typename T>
  const ADMaterialProperty<T> & getADMaterialPropertyByName(const std::string & prop_name)
  {

    return getGenericMaterialPropertyByName<T, true>(prop_name, 0);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOldByName(const std::string & prop_name)
  {

    return getGenericMaterialPropertyByName<T, false>(prop_name, 1);
  }
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlderByName(const std::string & prop_name)
  {

    return getGenericMaterialPropertyByName<T, false>(prop_name, 2);
  }
  ///@}

  ///@{
  /**
   * Retrieve the neighbor property through a given input parameter key with a fallback
   * to getting it by name
   *
   * \p state is the property state; 0 = current, 1 = old, 2 = older, etc.
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> &
  getGenericNeighborMaterialProperty(const std::string & name, const unsigned int state = 0);
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

  ///@{
  /**
   * Retrieve the neighbor property named "name"
   *
   * \p state is the property state; 0 = current, 1 = old, 2 = older, etc.
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> &
  getGenericNeighborMaterialPropertyByName(const std::string & name, const unsigned int state = 0);
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyByName(const std::string & prop_name,
                                                                const unsigned int state = 0)
  {
    return getGenericNeighborMaterialPropertyByName<T, false>(prop_name, state);
  }
  template <typename T>
  const ADMaterialProperty<T> & getNeighborADMaterialPropertyByName(const std::string & prop_name)
  {
    return getGenericNeighborMaterialPropertyByName<T, true>(prop_name, 0);
  }
  ///@}

  using MaterialBase::getZeroMaterialProperty;

  virtual bool isBoundaryMaterial() const override { return true; }

  virtual const std::set<unsigned int> & getMatPropDependencies() const override
  {
    return TwoMaterialPropertyInterface::getMatPropDependencies();
  }

protected:
  virtual const MaterialData & materialData() const override { return _material_data; }
  virtual MaterialData & materialData() override { return _material_data; }

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

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
InterfaceMaterial::getGenericMaterialProperty(const std::string & name, const unsigned int state)
{
  // Check if the supplied parameter is a valid input parameter key
  const auto prop_name = getMaterialPropertyName(name);

  // Check if it's just a constant.
  if (const auto * default_property = defaultGenericMaterialProperty<T, is_ad>(prop_name))
    return *default_property;

  return getGenericMaterialPropertyByName<T, is_ad>(prop_name, state);
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
InterfaceMaterial::getGenericMaterialPropertyByName(const std::string & prop_name,
                                                    const unsigned int state)
{
  MaterialBase::checkExecutionStage();

  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  if (state == 0)
    _requested_props.insert(prop_name);

  // Do this before hand so that the propery id is defined
  auto & prop =
      TwoMaterialPropertyInterface::getGenericMaterialPropertyByName<T, is_ad>(prop_name, state);

  registerPropName(prop_name, true, state);

  return prop;
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
InterfaceMaterial::getGenericNeighborMaterialProperty(const std::string & name,
                                                      const unsigned int state)
{
  // Check if the supplied parameter is a valid input parameter key
  const auto prop_name = getMaterialPropertyName(name);

  // Check if it's just a constant.
  if (const auto * default_property = defaultGenericMaterialProperty<T, is_ad>(prop_name))
    return *default_property;

  return getGenericNeighborMaterialPropertyByName<T, is_ad>(prop_name, state);
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
InterfaceMaterial::getGenericNeighborMaterialPropertyByName(const std::string & name,
                                                            const unsigned int state)
{
  MaterialBase::checkExecutionStage();

  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  if (state == 0)
    _requested_props.insert(name);

  // Do this before hand so that the propery id is defined
  auto & prop =
      TwoMaterialPropertyInterface::getGenericNeighborMaterialPropertyByName<T, is_ad>(name, state);

  registerPropName(name, true, state);

  return prop;
}
