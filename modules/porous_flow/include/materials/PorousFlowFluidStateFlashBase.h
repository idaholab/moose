//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWFLUIDSTATEFLASHBASE_H
#define POROUSFLOWFLUIDSTATEFLASHBASE_H

#include "PorousFlowVariableBase.h"
#include "PorousFlowFluidStateBase.h"

class PorousFlowFluidStateFlashBase;
class PorousFlowCapillaryPressure;

template <>
InputParameters validParams<PorousFlowFluidStateFlashBase>();

/**
 * Base class for fluid states using a persistent set of primary variables for
 * the mutliphase, multicomponent case.
 *
 * Primary variables are: gas pressure, total mass fraction
 * of a component summed over all phases (and optionally temperature in a
 * non-isothermal case).
 *
 * The total mass fraction of component i summed over all phases, z_i,
 * is defined as (for two phases)
 *
 * z_i = (S_g rho_g y_i + S_l rho_l x_i) / (S_g rho_g + S_l rho_l)
 *
 * where S is saturation, rho is density, and the subscripts correspond to gas
 * and liquid phases, respectively, and y_i and x_i are the mass fractions of
 * the ith component in the gas and liquid phase, respectively.
 *
 * Depending on the phase conditions, the primary variable z_i can represent either
 * a mass fraction (when only a single phase is present), or a saturation when
 * two phases are present, and hence it is a persistent variable.
 *
 * The PorousFlow kernels expect saturation and mass fractions (as well as pressure
 * and temperature), so these must be calculated from z_i once the state of the
 * system is determined.
 *
 * A compositional flash calculation using the Rachford-Rice equation is solved
 * to determine vapor fraction (gas saturation), and subsequently the composition
 * of each phase.
 */
class PorousFlowFluidStateFlashBase : public PorousFlowVariableBase
{
public:
  PorousFlowFluidStateFlashBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Size material property vectors and initialise with zeros
  void setMaterialVectorSize() const;

  /**
   * Calculates all required thermophysical properties and derivatives for each phase
   * and fluid component. Must override in all derived classes.
   */
  virtual void thermophysicalProperties() = 0;

  /// Porepressure
  const VariableValue & _gas_porepressure;
  /// Gradient of porepressure (only defined at the qps)
  const VariableGradient & _gas_gradp_qp;
  /// Moose variable number of the gas porepressure
  const unsigned int _gas_porepressure_varnum;
  /// PorousFlow variable number of the gas porepressure
  const unsigned int _pvar;
  /// Total mass fraction(s) of the gas component(s) summed over all phases
  std::vector<const VariableValue *> _z;
  /// Gradient(s) of total mass fraction(s) of the gas component(s) (only defined at the qps)
  std::vector<const VariableGradient *> _gradz_qp;
  /// Moose variable number of z
  std::vector<unsigned int> _z_varnum;
  /// PorousFlow variable number of z
  std::vector<unsigned int> _zvar;
  /// Number of coupled total mass fractions. Should be _num_phases - 1
  const unsigned int _num_z_vars;
  /// FluidState UserObject
  const PorousFlowFluidStateBase & _fs_base;
  /// Phase number of the aqueous phase
  const unsigned int _aqueous_phase_number;
  /// Phase number of the gas phase
  const unsigned int _gas_phase_number;
  /// Fluid component number of the aqueous component
  const unsigned int _aqueous_fluid_component;
  /// Fluid component number of the gas phase
  const unsigned int _gas_fluid_component;
  /// Temperature
  const MaterialProperty<Real> & _temperature;
  /// Gradient of temperature (only defined at the qps)
  const MaterialProperty<RealGradient> & _gradT_qp;
  /// Derivative of temperature wrt PorousFlow variables
  const MaterialProperty<std::vector<Real>> & _dtemperature_dvar;
  /// Mass fraction matrix
  MaterialProperty<std::vector<std::vector<Real>>> & _mass_frac;
  /// Gradient of the mass fraction matrix (only defined at the qps)
  MaterialProperty<std::vector<std::vector<RealGradient>>> * _grad_mass_frac_qp;
  /// Derivative of the mass fraction matrix with respect to the Porous Flow variables
  MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmass_frac_dvar;
  /// Old value of saturation
  const MaterialProperty<std::vector<Real>> & _saturation_old;

  /// Fluid density of each phase
  MaterialProperty<std::vector<Real>> & _fluid_density;
  /// Derivative of the fluid density for each phase wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_density_dvar;
  /// Viscosity of each phase
  MaterialProperty<std::vector<Real>> & _fluid_viscosity;
  /// Derivative of the fluid viscosity for each phase wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_viscosity_dvar;

  /// Conversion from degrees Celsius to degrees Kelvin
  const Real _T_c2k;
  /// Flag to indicate whether to calculate stateful properties
  bool _is_initqp;
  /// FluidStateProperties data structure
  std::vector<FluidStateProperties> _fsp;
  /// Capillary pressure UserObject
  const PorousFlowCapillaryPressure & _pc_uo;
};

#endif // POROUSFLOWFLUIDSTATEFLASHBASE_H
