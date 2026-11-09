//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  /// Composition passed to the fluid state: the total mass fraction(s) summed over all phases,
  /// followed by the salt mass fraction if the fluid state has salt
  std::vector<const GenericVariableValue<is_ad> *> _composition;
  /// Gradient(s) of the composition variables (only defined at the qps)
  std::vector<const GenericVariableGradient<is_ad> *> _grad_composition_qp;
  /// Moose variable numbers of the composition variables
  std::vector<unsigned int> _composition_varnum;
  /// PorousFlow variable numbers of the composition variables
  std::vector<unsigned int> _composition_var;
  /// Number of coupled total mass fractions: one fewer than the number of fluid components, not
  /// counting salt
  const unsigned int _num_Z_vars;
  /// FluidState UserObject
  const PorousFlowFluidStateMultiComponentBase & _fs;
  /// Phase number of the aqueous phase
  const unsigned int _aqueous_phase_number;
  /// Phase number of the gas phase
  const unsigned int _gas_phase_number;
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

#if (is_ad)
  usingPorousFlowFluidStateBaseMaterialMembers;
#else
  usingPorousFlowFluidStateBaseMaterialDerivativeMembers;
#endif
};

typedef PorousFlowFluidStateTempl<false> PorousFlowFluidState;
typedef PorousFlowFluidStateTempl<true> ADPorousFlowFluidState;
