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
class PorousFlowFluidStateSingleComponent : public PorousFlowVariableBase
{
public:
  static InputParameters validParams();

  PorousFlowFluidStateSingleComponent(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Size material property vectors and initialise with zeros
  void setMaterialVectorSize() const;

  /**
   * Calculates all required thermophysical properties and derivatives for each phase
   * and fluid component
   */
  virtual void thermophysicalProperties();

  /// Porepressure
  const VariableValue & _liquid_porepressure;
  /// Gradient of porepressure (only defined at the qps)
  const VariableGradient & _liquid_gradp_qp;
  /// Moose variable number of the porepressure
  const unsigned int _liquid_porepressure_varnum;
  /// PorousFlow variable number of the porepressure
  const unsigned int _pvar;
  /// Enthalpy
  const VariableValue & _enthalpy;
  /// Gradient of enthalpy (only defined at the qps)
  const VariableGradient & _gradh_qp;
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
  MaterialProperty<Real> & _temperature;
  /// Gradient of temperature (only defined at the qps)
  MaterialProperty<RealGradient> * _grad_temperature_qp;
  /// Derivative of temperature wrt PorousFlow variables
  MaterialProperty<std::vector<Real>> & _dtemperature_dvar;
  /// d(grad temperature)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<Real>> * const _dgrad_temperature_dgradv;
  /// d(grad temperature)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<RealGradient>> * const _dgrad_temperature_dv;
  /// Mass fraction matrix
  MaterialProperty<std::vector<std::vector<Real>>> & _mass_frac;
  /// Gradient of the mass fraction matrix (only defined at the qps)
  MaterialProperty<std::vector<std::vector<RealGradient>>> * _grad_mass_frac_qp;
  /// Derivative of the mass fraction matrix with respect to the Porous Flow variables
  MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmass_frac_dvar;
  /// Fluid density of each phase
  MaterialProperty<std::vector<Real>> & _fluid_density;
  /// Derivative of the fluid density for each phase wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_density_dvar;
  /// Viscosity of each phase
  MaterialProperty<std::vector<Real>> & _fluid_viscosity;
  /// Derivative of the fluid viscosity for each phase wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_viscosity_dvar;
  /// Enthalpy of each phase
  MaterialProperty<std::vector<Real>> & _fluid_enthalpy;
  /// Derivative of the fluid enthalpy for each phase wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_enthalpy_dvar;
  /// Internal energy of each phase
  MaterialProperty<std::vector<Real>> & _fluid_internal_energy;
  /// Derivative of the fluid internal energy for each phase wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_internal_energy_dvar;
  /// Conversion from degrees Celsius to degrees Kelvin
  const Real _T_c2k;
  /// Flag to indicate whether to calculate stateful properties
  bool _is_initqp;
  /// FluidStateProperties data structure
  std::vector<FluidStateProperties> _fsp;
  /// FluidStatePhaseEnum
  FluidStatePhaseEnum _phase_state;
  /// Capillary pressure UserObject
  const PorousFlowCapillaryPressure & _pc;
  /// Index of derivative wrt pressure
  const unsigned int _pidx;
  /// Index of derivative wrt enthalpy
  const unsigned int _hidx;
};
