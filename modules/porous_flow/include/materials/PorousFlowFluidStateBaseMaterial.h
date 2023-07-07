//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowVariableBase.h"
#include "PorousFlowFluidStateBase.h"

class PorousFlowCapillaryPressure;

/**
 * Fluid state base class using a persistent set of primary variables for
 * multiphase, single and multicomponent cases.
 */
template <bool is_ad>
class PorousFlowFluidStateBaseMaterialTempl : public PorousFlowVariableBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowFluidStateBaseMaterialTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Size material property vectors and initialise with zeros
  virtual void setMaterialVectorSize() const;

  /**
   * Calculates all required thermophysical properties and derivatives for each phase
   * and fluid component
   */
  virtual void thermophysicalProperties() = 0;

  /**
   * Templated method that takes an ADReal value, and returns the
   * raw value when is_ad = false, and the AD value when is_ad = true
   * @param value AD value
   * @return raw or AD value depending on is_ad
   */
  GenericReal<is_ad> genericValue(const ADReal & value);

  /// Conversion from degrees Celsius to degrees Kelvin
  const Real _T_c2k;
  /// Flag to indicate whether stateful properties should be computed
  bool _is_initqp;
  /// FluidStateProperties data  for each fluid component in each fluid phase
  std::vector<FluidStateProperties> _fsp;
  /// Capillary pressure UserObject
  const PorousFlowCapillaryPressure & _pc;
  /// Suffix to append to material property names (either qp or nodal as required)
  const std::string _sfx;

  /// Mass fraction matrix (indexing is fluid component in fluid phase)
  GenericMaterialProperty<std::vector<std::vector<Real>>, is_ad> & _mass_frac;
  /// Gradient of the mass fraction matrix (only defined at the qps)
  GenericMaterialProperty<std::vector<std::vector<RealGradient>>, is_ad> * const _grad_mass_frac_qp;
  /// Derivative of the mass fraction matrix with respect to the Porous Flow variables
  MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const _dmass_frac_dvar;
  /// Fluid density of each phase
  GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_density;
  /// Derivative of the fluid density for each phase wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_dvar;
  /// Viscosity of each phase
  GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_viscosity;
  /// Derivative of the fluid viscosity for each phase wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_viscosity_dvar;
  /// Enthalpy of each phase
  GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_enthalpy;
  /// Derivative of the fluid enthalpy for each phase wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_enthalpy_dvar;
  /// Internal energy of each phase
  GenericMaterialProperty<std::vector<Real>, is_ad> & _fluid_internal_energy;
  /// Derivative of the fluid internal energy for each phase wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_internal_energy_dvar;

  usingPorousFlowVariableBaseMembers;
  using PorousFlowVariableBaseTempl<is_ad>::_num_components;
  using PorousFlowVariableBaseTempl<is_ad>::_num_pf_vars;
};

#define usingPorousFlowFluidStateBaseMaterialMembers                                               \
  usingPorousFlowVariableBaseMembers;                                                              \
  using Coupleable::coupledComponents;                                                             \
  using Coupleable::getFieldVar;                                                                   \
  using Coupleable::isCoupled;                                                                     \
  using PorousFlowVariableBaseTempl<is_ad>::name;                                                  \
  using PorousFlowVariableBaseTempl<is_ad>::_num_components;                                       \
  using PorousFlowVariableBaseTempl<is_ad>::_num_pf_vars;                                          \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_T_c2k;                                      \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_fsp;                                        \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_sfx;                                        \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_pc;                                         \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_is_initqp;                                  \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::genericValue;                                \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::setMaterialVectorSize;                       \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_mass_frac;                                  \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_grad_mass_frac_qp;                          \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_fluid_density;                              \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_fluid_viscosity;                            \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_fluid_enthalpy;                             \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_fluid_internal_energy

#define usingPorousFlowFluidStateBaseMaterialDerivativeMembers                                     \
  usingPorousFlowFluidStateBaseMaterialMembers;                                                    \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_dmass_frac_dvar;                            \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_dfluid_density_dvar;                        \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_dfluid_viscosity_dvar;                      \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_dfluid_enthalpy_dvar;                       \
  using PorousFlowFluidStateBaseMaterialTempl<is_ad>::_dfluid_internal_energy_dvar
