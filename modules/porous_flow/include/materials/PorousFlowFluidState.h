//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowFluidStateBaseMaterial.h"
#include "PorousFlowFluidStateMultiComponentBase.h"

class PorousFlowCapillaryPressure;

/**
 * Fluid state class using a persistent set of primary variables for
 * the mutliphase, multicomponent case.
 *
 * Primary variables are: gas pressure, total mass fraction
 * of a component summed over all phases (and optionally temperature in a
 * non-isothermal case).
 *
 * The total mass fraction of component i summed over all phases, Z_i,
 * is defined as (for two phases)
 *
 * Z_i = (S_g rho_g Y_i + S_l rho_l X_i) / (S_g rho_g + S_l rho_l)
 *
 * where S is saturation, rho is density, and the subscripts correspond to gas
 * and liquid phases, respectively, and Y_i and X_i are the mass fractions of
 * the ith component in the gas and liquid phase, respectively.
 *
 * Depending on the phase conditions, the primary variable Z_i can represent either
 * a mass fraction (when only a single phase is present), or a saturation when
 * two phases are present, and hence it is a persistent variable.
 *
 * The PorousFlow kernels expect saturation and mass fractions (as well as pressure
 * and temperature), so these must be calculated from Z_i once the state of the
 * system is determined.
 *
 * A compositional flash calculation using the Rachford-Rice equation is solved
 * to determine vapor fraction (gas saturation), and subsequently the composition
 * of each phase.
 */
template <bool is_ad>
class PorousFlowFluidStateTempl : public PorousFlowFluidStateBaseMaterialTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowFluidStateTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;
  virtual void thermophysicalProperties() override;

  /// Porepressure
  const GenericVariableValue<is_ad> & _gas_porepressure;
  /// Gradient of porepressure (only defined at the qps)
  const GenericVariableGradient<is_ad> & _gas_gradp_qp;
  /// Moose variable number of the gas porepressure
  const unsigned int _gas_porepressure_varnum;
  /// PorousFlow variable number of the gas porepressure
  const unsigned int _pvar;
  /// Total mass fraction(s) of the gas component(s) summed over all phases
  std::vector<const GenericVariableValue<is_ad> *> _Z;
  /// Gradient(s) of total mass fraction(s) of the gas component(s) (only defined at the qps)
  std::vector<const GenericVariableGradient<is_ad> *> _gradZ_qp;
  /// Moose variable number of Z
  std::vector<unsigned int> _Z_varnum;
  /// PorousFlow variable number of Z
  std::vector<unsigned int> _Zvar;
  /// Number of coupled total mass fractions. Should be _num_phases - 1
  const unsigned int _num_Z_vars;
  /// Flag for nodal NaCl mass fraction
  const bool _is_Xnacl_nodal;
  /// Salt mass fraction (kg/kg)
  const GenericVariableValue<is_ad> & _Xnacl;
  /// Gradient of salt mass fraction (only defined at the qps)
  const GenericVariableGradient<is_ad> & _grad_Xnacl_qp;
  /// Salt mass fraction variable number
  const unsigned int _Xnacl_varnum;
  /// Salt mass fraction PorousFlow variable number
  const unsigned int _Xvar;
  /// FluidState UserObject
  const PorousFlowFluidStateMultiComponentBase & _fs;
  /// Phase number of the aqueous phase
  const unsigned int _aqueous_phase_number;
  /// Phase number of the gas phase
  const unsigned int _gas_phase_number;
  /// Fluid component number of the aqueous component
  const unsigned int _aqueous_fluid_component;
  /// Fluid component number of the gas phase
  const unsigned int _gas_fluid_component;
  /// Salt component index
  const unsigned int _salt_component;
  /// Temperature
  const GenericMaterialProperty<Real, is_ad> & _temperature;
  /// Gradient of temperature (only defined at the qps)
  const GenericMaterialProperty<RealGradient, is_ad> * const _gradT_qp;
  /// Derivative of temperature wrt PorousFlow variables
  const MaterialProperty<std::vector<Real>> * const _dtemperature_dvar;
  /// Moose variable number of the temperature
  const unsigned int _temperature_varnum;
  /// PorousFlow variable number of the temperature
  const unsigned int _Tvar;
  /// Index of derivative wrt pressure
  const unsigned int _pidx;
  /// Index of derivative wrt temperature
  const unsigned int _Tidx;
  /// Index of derivative wrt total mass fraction Z
  const unsigned int _Zidx;
  /// Index of derivative wrt salt mass fraction X
  const unsigned int _Xidx;

#if (is_ad)
  usingPorousFlowFluidStateBaseMaterialMembers;
#else
  usingPorousFlowFluidStateBaseMaterialDerivativeMembers;
#endif
};

typedef PorousFlowFluidStateTempl<false> PorousFlowFluidState;
typedef PorousFlowFluidStateTempl<true> ADPorousFlowFluidState;
