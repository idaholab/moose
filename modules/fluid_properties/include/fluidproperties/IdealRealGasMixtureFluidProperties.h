//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VaporMixtureFluidProperties.h"
#include "NaNInterface.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

class SinglePhaseFluidProperties;

/**
 * Overrides the value and derivative methods for a property from some arguments
 */
#define override_property(want, prop1, prop2)                                                      \
  virtual Real want##_from_##prop1##_##prop2(Real prop1, Real prop2, const std::vector<Real> & x)  \
      const override;                                                                              \
  virtual void want##_from_##prop1##_##prop2(Real prop1,                                           \
                                             Real prop2,                                           \
                                             const std::vector<Real> & x,                          \
                                             Real & want,                                          \
                                             Real & d##want##_d##prop1,                            \
                                             Real & d##want##_d##prop2,                            \
                                             std::vector<Real> & dp_dx) const override

/**
 * Declares the value and derivative methods for a property from some arguments
 */
#define declare_property(want, prop1, prop2)                                                       \
  virtual Real want##_from_##prop1##_##prop2(Real prop1, Real prop2, const std::vector<Real> & x)  \
      const;                                                                                       \
  virtual void want##_from_##prop1##_##prop2(Real prop1,                                           \
                                             Real prop2,                                           \
                                             const std::vector<Real> & x,                          \
                                             Real & want,                                          \
                                             Real & d##want##_d##prop1,                            \
                                             Real & d##want##_d##prop2,                            \
                                             std::vector<Real> & dp_dx) const

/**
 * Class for fluid properties of an arbitrary vapor mixture
 *
 *
 */
class IdealRealGasMixtureFluidProperties : public VaporMixtureFluidProperties, public NaNInterface
{
public:
  static InputParameters validParams();

  IdealRealGasMixtureFluidProperties(const InputParameters & parameters);

  usingVaporMixtureFluidPropertiesMembers;

  virtual const SinglePhaseFluidProperties & getPrimaryFluidProperties() const override;
  virtual const SinglePhaseFluidProperties &
  getSecondaryFluidProperties(unsigned int i = 0) const override;

  virtual unsigned int numberOfComponents() const override { return _n_secondary_vapors + 1; }

  override_property(p, v, e);
  override_property(T, v, e);
  override_property(c, v, e);
  override_property(v, p, T);
  override_property(rho, p, T);
  override_property(e, p, T);
  override_property(s, p, T);
  override_property(c, p, T);
  override_property(cp, p, T);
  override_property(cv, p, T);
  override_property(mu, p, T);
  override_property(k, p, T);
  override_property(e, p, rho);

  declare_property(T, p, v);
  declare_property(p, T, v);
  declare_property(e, T, v);
  declare_property(s, T, v);
  declare_property(c, T, v);
  declare_property(cp, T, v);
  declare_property(cv, T, v);
  declare_property(mu, T, v);
  declare_property(k, T, v);

  /**
   * Mass fraction of primary (condensable) component at saturation from pressure and temperature
   *
   * @param[in] T   temperature
   * @param[in] p   pressure
   * @return        mass fraction of primary (condensable) component at saturation
   */
  Real xs_prim_from_p_T(Real p, Real T, const std::vector<Real> & x) const;

protected:
  /// Primary vapor fluid properties
  const SinglePhaseFluidProperties * const _fp_primary;
  /// Secondary vapor fluid properties
  std::vector<const SinglePhaseFluidProperties *> _fp_secondary;
  /// Names of secondary vapor fluid properties
  const std::vector<UserObjectName> _fp_secondary_names;
  /// Number of secondary vapors
  const unsigned int _n_secondary_vapors;
  /// molar (or universal) gas constant
  constexpr static const Real R_molar = 8.3144598;
  /// maximum temperature of all components
  const Real _T_mix_max;
};

#pragma GCC diagnostic pop
