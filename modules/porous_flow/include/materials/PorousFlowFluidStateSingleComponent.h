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
#include "PorousFlowFluidStateSingleComponentBase.h"

class PorousFlowCapillaryPressure;

/**
 * Fluid state class using a persistent set of primary variables for
 * the mutliphase, single component case.
 *
 * Primary variables are: liquid pressure and enthalpy.
 *
 * The PorousFlow kernels expect saturation and mass fractions (as well as pressure
 * and temperature), so these must be calculated from the primary variables once the
 * state of the system is determined.
 */
template <bool is_ad>
class PorousFlowFluidStateSingleComponentTempl : public PorousFlowFluidStateBaseMaterialTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowFluidStateSingleComponentTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;
  virtual void thermophysicalProperties() override;
  virtual void setMaterialVectorSize() const override;

  /// Porepressure
  const GenericVariableValue<is_ad> & _liquid_porepressure;
  /// Gradient of porepressure (only defined at the qps)
  const GenericVariableGradient<is_ad> & _liquid_gradp_qp;
  /// Moose variable number of the porepressure
  const unsigned int _liquid_porepressure_varnum;
  /// PorousFlow variable number of the porepressure
  const unsigned int _pvar;
  /// Enthalpy
  const GenericVariableValue<is_ad> & _enthalpy;
  /// Gradient of enthalpy (only defined at the qps)
  const GenericVariableGradient<is_ad> & _gradh_qp;
  /// Moose variable number of the enthalpy
  const unsigned int _enthalpy_varnum;
  /// PorousFlow variable number of the enthalpy
  const unsigned int _hvar;
  /// FluidState UserObject
  const PorousFlowFluidStateSingleComponentBase & _fs;
  /// Phase number of the aqueous phase
  const unsigned int _aqueous_phase_number;
  /// Phase number of the gas phase
  const unsigned int _gas_phase_number;
  /// Temperature
  GenericMaterialProperty<Real, is_ad> & _temperature;
  /// Gradient of temperature (only defined at the qps)
  GenericMaterialProperty<RealGradient, is_ad> * const _grad_temperature_qp;
  /// Derivative of temperature wrt PorousFlow variables
  MaterialProperty<std::vector<Real>> * const _dtemperature_dvar;
  /// d(grad temperature)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<Real>> * const _dgrad_temperature_dgradv;
  /// d(grad temperature)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<RealGradient>> * const _dgrad_temperature_dv;
  /// Index of derivative wrt pressure
  const unsigned int _pidx;
  /// Index of derivative wrt enthalpy
  const unsigned int _hidx;

#if (is_ad)
  usingPorousFlowFluidStateBaseMaterialMembers;
#else
  usingPorousFlowFluidStateBaseMaterialDerivativeMembers;
#endif
};

typedef PorousFlowFluidStateSingleComponentTempl<false> PorousFlowFluidStateSingleComponent;
typedef PorousFlowFluidStateSingleComponentTempl<true> ADPorousFlowFluidStateSingleComponent;
