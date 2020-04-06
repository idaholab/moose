//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FluidProperties.h"
#include "SinglePhaseFluidProperties.h"

#define propfunc(want, prop1, prop2, prop3)                                                        \
  virtual Real want##_from_##prop1##_##prop2##_##prop3(Real, Real, Real) const                     \
  {                                                                                                \
    mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");                            \
  }                                                                                                \
                                                                                                   \
  virtual void want##_from_##prop1##_##prop2##_##prop3(Real prop1,                                 \
                                                       Real prop2,                                 \
                                                       Real prop3,                                 \
                                                       Real & val,                                 \
                                                       Real & d##want##d1,                         \
                                                       Real & d##want##d2,                         \
                                                       Real & d##want##d3) const                   \
  {                                                                                                \
    if (_allow_imperfect_jacobians)                                                                \
      mooseWarning(name(), ": ", __PRETTY_FUNCTION__, " derivatives not implemented.");            \
    else                                                                                           \
      mooseError(name(), ": ", __PRETTY_FUNCTION__, " derivatives not implemented.");              \
                                                                                                   \
    d##want##d1 = 0.0;                                                                             \
    d##want##d2 = 0.0;                                                                             \
    d##want##d3 = 0.0;                                                                             \
    val = want##_from_##prop1##_##prop2##_##prop3(prop1, prop2, prop3);                            \
  }                                                                                                \
                                                                                                   \
  DualReal want##_from_##prop1##_##prop2##_##prop3(                                                \
      const DualReal & p1, const DualReal & p2, const DualReal & p3) const                         \
  {                                                                                                \
    const Real raw1 = p1.value();                                                                  \
    const Real raw2 = p2.value();                                                                  \
    const Real raw3 = p3.value();                                                                  \
    Real x = 0.0;                                                                                  \
    Real dxd1 = 0.0;                                                                               \
    Real dxd2 = 0.0;                                                                               \
    Real dxd3 = 0.0;                                                                               \
    want##_from_##prop1##_##prop2##_##prop3(raw1, raw2, raw3, x, dxd1, dxd2, dxd3);                \
                                                                                                   \
    DualReal result = x;                                                                           \
    result.derivatives() =                                                                         \
        p1.derivatives() * dxd1 + p2.derivatives() * dxd2 + p3.derivatives() * dxd3;               \
                                                                                                   \
    return result;                                                                                 \
  }

/**
 * Common class for multiple component fluid
 * properties using a pressure and
 * temperature formulation
 */
class MultiComponentFluidProperties : public FluidProperties
{
public:
  static InputParameters validParams();

  MultiComponentFluidProperties(const InputParameters & parameters);
  virtual ~MultiComponentFluidProperties();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
  // clang-format off

    /**
     * @brief Compute a fluid property given for the state defined by three given properties.
     *
     * For all functions, the first three arguments are the given properties that define the fluid
     * state.  For the three-argument variants, the desired property is the return value.
     * The seven-argument variants also provide partial derivatives x/da, dx/db and dx/dc where x
     * is the desired property being computed, a is the first given property,  b is the second given property etc.
     *  The desired property, dx/da, dx/db and dx/dc are stored into the 4rd, 5th, 6th and 7th arguments respectively.
     *
     * Properties/parameters used in these function are listed below with their units:
     *
     * @begincode
     * p      pressure [Pa]
     * T      temperature [K]
     * X      solute mass fraction [-]
     * e      specific internal energy [J/kg]
     * rho    density [kg/m^3]
     * h      specific enthalpy [J/kg]
     * mu     viscosity [Pa*s]
     * k      thermal conductivity [W/(m*K)]
     * c      speed of sound [m/s]
     * cp     constant-pressure specific heat [J/K]
     * cv     constant-volume specific heat [J/K]
     * @endcode
     *
     * As an example:
     *
     * @begincode
     * // calculate desnity given pressure, temperature and solute mass fraction:
     * auto density = your_fluid_properties_object.rho_from_p_T_X(p, T, X);
     *
     * // or use the derivative variant:
     * Real rho = 0; // density will be stored into here
     * Real drho_dp = 0; // derivative will be stored into here
     * Real drho_dT = 0; // derivative will be stored into here
     * Real drho_dX = 0; // derivative will be stored into here
     * your_fluid_properties_object.rho_from_p_T_X(p, T, X, rho, drho_dp, drho_dT, drho_dX);
     * @endcode
     *
     * Automatic differentiation (AD) support is provided through prop_from_p_T_X(DualReal p, DualReal T, DualReal X) versions
     * of the functions where p, T and X must be ADReal/DualNumber's calculated using all AD-supporting values.
     */
    ///@{
    propfunc(rho, p, T, X)
    propfunc(mu, p, T, X)
    propfunc(h, p, T, X)
    propfunc(cp, p, T, X)
    propfunc(e, p, T, X)
    propfunc(k, p, T, X)
    ///@}

  // clang-format on

#undef propfunc
#pragma GCC diagnostic pop

      /**
       * Fluid name
       * @return string representing fluid name
       */
      virtual std::string fluidName() const;

  /**
   * Density and viscosity
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @param[out] rho density (kg/m^3)
   */
  virtual void
  rho_mu_from_p_T_X(Real pressure, Real temperature, Real xmass, Real & rho, Real & mu) const;

  virtual void rho_mu_from_p_T_X(
      DualReal pressure, DualReal temperature, DualReal xmass, DualReal & rho, DualReal & mu) const;

  /**
   * Density and viscosity and their derivatives wrt pressure, temperature
   * and mass fraction
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xmass mass fraction (-)
   * @param[out] rho density (kg/m^3)
   * @param[out] drho_dp derivative of density wrt pressure
   * @param[out] drho_dT derivative of density wrt temperature
   * @param[out] drho_dx derivative of density wrt mass fraction
   * @param[out] mu viscosity (Pa.s)
   * @param[out] dmu_dp derivative of viscosity wrt pressure
   * @param[out] dmu_dT derivative of viscosity wrt temperature
   * @param[out] dmu_dx derivative of viscosity wrt mass fraction
   */
  virtual void rho_mu_from_p_T_X(Real pressure,
                                 Real temperature,
                                 Real xmass,
                                 Real & rho,
                                 Real & drho_dp,
                                 Real & drho_dT,
                                 Real & drho_dx,
                                 Real & mu,
                                 Real & dmu_dp,
                                 Real & dmu_dT,
                                 Real & dmu_dx) const;

  /**
   * Get UserObject for specified component
   * @param component fluid component
   * @return reference to
   * SinglePhaseFluidPropertiesPT UserObject
   * for component
   */
  virtual const SinglePhaseFluidProperties & getComponent(unsigned int component) const;
};
