/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWFLUIDSTATEFLASHBASE_H
#define POROUSFLOWFLUIDSTATEFLASHBASE_H

#include "PorousFlowVariableBase.h"

class PorousFlowFluidStateFlashBase;

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
  virtual void thermophysicalProperties() const = 0;

  /**
   * Rachford-Rice equation for vapor fraction. Can be solved analytically for two
   * components in two phases, but must be solved iteratively using a root finding
   * algorithm for more components. This equation has the nice property
   * that it is monotonic in the interval [0,1], so that only a small number of
   * iterations are typically required to find the root.
   *
   * The Rachford-Rice equation can also be used to check whether the phase state
   * is two phase, single phase gas, or single phase liquid.
   * Evaluate f(v), the Rachford-Rice equation evaluated at the vapor mass fraction.
   *
   * If f(0) < 0, then the mixture is below the bubble point, and only a single phase
   * liquid can exist
   *
   * If f(1) > 0, then the mixture is above the dew point, and only a single phase gas exists.
   *
   * If f(0) >= 0 and f(1) <= 0, the mixture is between the bubble and dew points, and both
   * gas and liquid phases exist.
   *
   * @param x vapor fraction
   * @param Ki equilibrium constants
   * @return f(x)
   */
  Real rachfordRice(Real x, std::vector<Real> & Ki) const;

  /**
   * Derivative of Rachford-Rice equation wrt vapor fraction.
   * Has the nice property that it is strictly negative in the interval [0,1]
   *
   * @param x vapor fraction
   * @param Ki equilibrium constants
   * @return f'(x)
   */
  Real rachfordRiceDeriv(Real x, std::vector<Real> & Ki) const;

  /**
   * Solves Rachford-Rice equation to provide vapor mass fraction. For two components,
   * the analytical solution is used, while for cases with more than two components,
   * a Newton-Raphson iterative solution is calculated.
   *
   * @param Ki equilibrium constants
   * @return vapor mass fraction
   */
  Real vaporMassFraction(std::vector<Real> & Ki) const;

  /**
   * Effective saturation of liquid phase
   * @param saturation true saturation
   * @return effective saturation
   */
  virtual Real effectiveSaturation(Real saturation) const;

  /**
   * Capillary pressure as a function of saturation.
   * Default is constant capillary pressure = 0.0.
   * Override in derived classes to implement other capillary pressure forulations
   *
   * @param saturation saturation
   * @return capillary pressure
   */
  virtual Real capillaryPressure(Real saturation) const;

  /**
   * Derivative of capillary pressure wrt to saturation.
   * Default = 0 for constant capillary pressure.
   * Override in derived classes to implement other capillary pressure forulations
   *
   * @param saturation saturation (-)
   * @return derivative of capillary pressure wrt saturation
   */
  virtual Real dCapillaryPressure_dS(Real pressure) const;

  /**
   * Second derivative of capillary pressure wrt to saturation.
   * Default = 0 for constant capillary pressure.
   * Override in derived classes to implement other capillary pressure forulations
   *
   * @param saturation saturation (-)
   * @return second derivative of capillary pressure wrt saturation
   */
  virtual Real d2CapillaryPressure_dS2(Real pressure) const;

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
  /// Universal gas constant (J/mol/K)
  const Real _R;
  /// Constant capillary pressure (Pa)
  const Real _pc;
  /// Liquid residual saturation
  const Real _sat_lr;
  /// Derivative of effective saturation wrt saturation
  const Real _dseff_ds;
  /// Maximum number of iterations for the Newton-Raphson iterations
  const Real _nr_max_its;
  /// Tolerance for Newton-Raphson iterations
  const Real _nr_tol;
  /// Flag to indicate whether to calculate stateful properties
  bool _is_initqp;
};

#endif // POROUSFLOWFLUIDSTATEFLASHBASE_H
