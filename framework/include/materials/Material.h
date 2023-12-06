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
#include "FEProblemBase.h"

#include <string>

#define usingMaterialMembers                                                                       \
  usingMaterialBaseMembers;                                                                        \
  usingCoupleableMembers;                                                                          \
  using Material::_q_point;                                                                        \
  using Material::_qrule;                                                                          \
  using Material::_JxW;                                                                            \
  using Material::_current_elem;                                                                   \
  using Material::_current_subdomain_id;                                                           \
  using Material::_current_side

/**
 * Materials compute MaterialProperties.
 */
class Material : public MaterialBase, public Coupleable, public MaterialPropertyInterface
{
public:
  static InputParameters validParams();

  Material(const InputParameters & parameters);

  /**
   * Gets an element integer for the proper current element with a parameter of
   * the object derived from this interface
   * Note: This overrides the function in ElementIDInterface to assure derived materials
   *       call the functions in ElementIDInterface properly.
   */
  virtual const dof_id_type & getElementID(const std::string & id_parameter_name,
                                           unsigned int comp = 0) const override
  {
    return _neighbor ? ElementIDInterface::getElementIDNeighbor(id_parameter_name, comp)
                     : ElementIDInterface::getElementID(id_parameter_name, comp);
  }
  /**
   * Directly calling this function is not needed for materials because the same material has
   * three copies for element interior, element side and neighbor side.
   */
  virtual const dof_id_type & getElementIDNeighbor(const std::string & id_parameter_name,
                                                   unsigned int comp = 0) const override
  {
    mooseError("Directly calling 'getElementIDNeighbor' is not allowed for materials. Please call "
               "'getElementID' instead");
    return ElementIDInterface::getElementIDNeighbor(id_parameter_name, comp);
  }

  /**
   * Gets an element integer for the proper current element with the element integer name
   * Note: This overrides the function in ElementIDInterface to assure derived materials
   *       call the functions in ElementIDInterface properly.
   */
  virtual const dof_id_type &
  getElementIDByName(const std::string & id_parameter_name) const override
  {
    return _neighbor ? ElementIDInterface::getElementIDNeighborByName(id_parameter_name)
                     : ElementIDInterface::getElementIDByName(id_parameter_name);
  }
  /**
   * Directly calling this function is not needed for materials because the same material has
   * three copies for element interior, element side and neighbor side.
   */
  virtual const dof_id_type &
  getElementIDNeighborByName(const std::string & id_parameter_name) const override
  {
    mooseError("Directly calling 'getElementIDNeighborByName' is not allowed for materials. Please "
               "call 'getElementIDByName' instead");
    return ElementIDInterface::getElementIDNeighborByName(id_parameter_name);
  }

  virtual void computeProperties() override;

  ///@{
  /**
   * Retrieve the property through a given input parameter key with a fallback
   * to getting it by name
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

  /**
   * Retrieve the discrete material with a given parameter key named "name"
   */
  MaterialBase & getMaterial(const std::string & name)
  {
    return getMaterialByName(parameters().get<MaterialName>(name));
  }

  /**
   * Retrieve the discrete material named "name".
   *
   * @param no_warn If true, suppress warning about retrieving the material potentially during its
   * calculation. If you don't know what this is/means, then you don't need it.
   * @param no_dep Use no_dep = true if no dependency resolution for the material is required. Using
   * no_dep = false is useful for discrete materials.
   */
  MaterialBase &
  getMaterialByName(const std::string & name, bool no_warn = false, bool no_dep = false);

  ///@{ Optional material property getters
  template <typename T, bool is_ad>
  const GenericOptionalMaterialProperty<T, is_ad> &
  getGenericOptionalMaterialProperty(const std::string & name, const unsigned int state = 0);
  template <typename T>
  const OptionalMaterialProperty<T> & getOptionalMaterialProperty(const std::string & name,
                                                                  const unsigned int state = 0)
  {
    return getGenericOptionalMaterialProperty<T, false>(name, state);
  }
  template <typename T>
  const OptionalADMaterialProperty<T> & getOptionalADMaterialProperty(const std::string & name)
  {
    return getGenericOptionalMaterialProperty<T, true>(name, 0);
  }
  template <typename T>
  const OptionalMaterialProperty<T> & getOptionalMaterialPropertyOld(const std::string & name)
  {
    return getGenericOptionalMaterialProperty<T, false>(name, 1);
  }
  template <typename T>
  const OptionalMaterialProperty<T> & getOptionalMaterialPropertyOlder(const std::string & name)
  {
    return getGenericOptionalMaterialProperty<T, false>(name, 2);
  }
  ///@}

  using MaterialBase::getGenericZeroMaterialProperty;
  using MaterialBase::getGenericZeroMaterialPropertyByName;
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

  bool ghostable() const override final { return _ghostable; }

  /// resolve all optional properties
  virtual void resolveOptionalProperties() override;

protected:
  virtual const MaterialData & materialData() const override { return _material_data; }
  virtual MaterialData & materialData() override { return _material_data; }

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

  /// Whether this material can be computed in a ghosted context. If properties are constant or
  /// depend only on finite volume variables, then this material can be computed in a ghosted
  /// context. If properties depend on finite element variables, then this material cannot be
  /// computed in a ghosted context
  bool _ghostable;

  /// optional material properties
  std::vector<std::unique_ptr<OptionalMaterialPropertyProxyBase<Material>>>
      _optional_property_proxies;
};

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
Material::getGenericMaterialProperty(const std::string & name, const unsigned int state)
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
Material::getGenericMaterialPropertyByName(const std::string & prop_name_in,
                                           const unsigned int state)
{
  MaterialBase::checkExecutionStage();

  // The property may not exist yet, so declare it (declare/getMaterialProperty are referencing the
  // same memory)
  const auto prop_name =
      _get_suffix.empty()
          ? prop_name_in
          : MooseUtils::join(std::vector<std::string>({prop_name_in, _get_suffix}), "_");

  if (state == 0)
    _requested_props.insert(prop_name);

  // Do this before so that we register the ID first
  auto & prop =
      MaterialPropertyInterface::getGenericMaterialPropertyByName<T, is_ad>(prop_name_in, state);

  registerPropName(prop_name, true, state);

  return prop;
}

template <typename T, bool is_ad>
const GenericOptionalMaterialProperty<T, is_ad> &
Material::getGenericOptionalMaterialProperty(const std::string & name, const unsigned int state)
{
  auto proxy = std::make_unique<OptionalMaterialPropertyProxy<Material, T, is_ad>>(name, state);
  auto & optional_property = proxy->value();
  _optional_property_proxies.push_back(std::move(proxy));
  return optional_property;
}
