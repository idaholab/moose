//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SinglePhaseFluidProperties.h"
#include "LinearInterpolation.h"

/**
 * Fluid properties from a table of pressure, formation volume factor and
 * viscosity (PVDO/PVDG style)
 */
class TabulatedPVTFluidProperties : public SinglePhaseFluidProperties
{
public:
  static InputParameters validParams();

  TabulatedPVTFluidProperties(const InputParameters & parameters);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  virtual std::string fluidName() const override;
  virtual Real molarMass() const override;

  using SinglePhaseFluidProperties::rho_from_p_T;
  virtual Real rho_from_p_T(Real p, Real T) const override;
  virtual void
  rho_from_p_T(Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  using SinglePhaseFluidProperties::mu_from_p_T;
  virtual Real mu_from_p_T(Real p, Real T) const override;
  virtual void mu_from_p_T(Real p, Real T, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  using SinglePhaseFluidProperties::e_from_p_T;
  virtual Real e_from_p_T(Real p, Real T) const override;
  virtual void e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const override;

  using SinglePhaseFluidProperties::h_from_p_T;
  virtual Real h_from_p_T(Real p, Real T) const override;
  virtual void h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const override;

#pragma GCC diagnostic pop

protected:
  /**
   * Interpolate a tabulated quantity in pressure, applying the out-of-bounds behavior
   * @param p pressure (Pa)
   * @param table interpolation of the tabulated quantity
   * @param[out] value interpolated value at p
   * @param[out] dvalue_dp derivative of the interpolated value with respect to p
   */
  void interpolate(Real p, const LinearInterpolation & table, Real & value, Real & dvalue_dp) const;

  /// Density at standard (surface) conditions (kg/m^3)
  const Real _density_sc;

  /// Tabulated pressures (Pa), strictly increasing
  const std::vector<Real> _p;

  /// Interpolation of the reciprocal of the formation volume factor, 1/B, in pressure
  LinearInterpolation _inv_B_table;

  /// Interpolation of the reciprocal of the product of formation volume factor and viscosity,
  /// 1/(B mu), in pressure
  LinearInterpolation _inv_B_mu_table;

  /// Constant specific heat capacity at constant volume (J/kg/K)
  const Real _cv;

  /// Property evaluation behavior outside the tabulated pressure range (as in
  /// TabulatedFluidProperties). Must remain in the same order as the MooseEnum in validParams
  enum class OOBBehavior
  {
    Ignore,
    Throw,
    DeclareInvalid,
    WarnInvalid,
    SetToClosestBound
  };
  const OOBBehavior _OOBBehavior;
};
