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
/**
 * Adds AD versions of each fluid property. These functions use the Real versions of these
 methods
 * to compute the AD variables complete with derivatives. Typically, these do not need to be
 * overriden in derived classes.
 */

#define propfuncAD(want, prop1, prop2)                                                             \
  virtual ADReal want##_from_##prop1##_##prop2(                                                    \
      const ADReal & p1, const ADReal & p2, const std::vector<ADReal> & x) const                   \
  {                                                                                                \
    Real p1_raw = p1.value();                                                                      \
    Real p2_raw = p2.value();                                                                      \
    std::vector<Real> x_raw(x.size(), 0);                                                          \
    for (unsigned int i = 0; i < x.size(); ++i)                                                    \
      x_raw[i] = x[i].value();                                                                     \
                                                                                                   \
    Real y_raw = 0;                                                                                \
    Real dy_dp1 = 0;                                                                               \
    Real dy_dp2 = 0;                                                                               \
    std::vector<Real> dy_dx(x.size(), 0);                                                          \
    want##_from_##prop1##_##prop2(p1_raw, p2_raw, x_raw, y_raw, dy_dp1, dy_dp2, dy_dx);            \
                                                                                                   \
    ADReal result = y_raw;                                                                         \
    result.derivatives() = p1.derivatives() * dy_dp1 + p2.derivatives() * dy_dp2;                  \
    for (unsigned int i = 0; i < x.size(); ++i)                                                    \
      result.derivatives() += x[i].derivatives() * dy_dx[i];                                       \
    return result;                                                                                 \
  }                                                                                                \
                                                                                                   \
  virtual void want##_from_##prop1##_##prop2(const ADReal & prop1,                                 \
                                             const ADReal & prop2,                                 \
                                             std::vector<ADReal> & x,                              \
                                             ADReal & val,                                         \
                                             ADReal & d##want##d1,                                 \
                                             ADReal & d##want##d2,                                 \
                                             std::vector<ADReal> & d##want##dx) const              \
  {                                                                                                \
    fluidPropError(                                                                                \
        name(), ": ", __PRETTY_FUNCTION__, " derivative derivatives not  implemented.");           \
    Real dummy, tmp1, tmp2;                                                                        \
    std::vector<Real> x_raw(x.size(), 0);                                                          \
    std::vector<Real> tmp3(x.size(), 0);                                                           \
    for (unsigned int i = 0; i < x.size(); ++i)                                                    \
      x_raw[i] = x[i].value();                                                                     \
    val = want##_from_##prop1##_##prop2(prop1, prop2, x);                                          \
    want##_from_##prop1##_##prop2(prop1.value(), prop2.value(), x_raw, dummy, tmp1, tmp2, tmp3);   \
    d##want##d1 = tmp1;                                                                            \
    d##want##d2 = tmp2;                                                                            \
    for (unsigned int i = 0; i < x.size(); ++i)                                                    \
      d##want##dx[i] = tmp3[i];                                                                    \
  }

/**
 * Adds function definitions with not implemented error. These functions should be overriden in
 * derived classes where required. AD versions are constructed automatically using propfuncAD.
 */
#define propfunc(want, prop1, prop2)                                                               \
  virtual Real want##_from_##prop1##_##prop2(Real, Real, const std::vector<Real> &) const          \
  {                                                                                                \
    mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");                            \
  }                                                                                                \
                                                                                                   \
  virtual void want##_from_##prop1##_##prop2(Real prop1,                                           \
                                             Real prop2,                                           \
                                             const std::vector<Real> & x,                          \
                                             Real & val,                                           \
                                             Real & d##want##d1,                                   \
                                             Real & d##want##d2,                                   \
                                             std::vector<Real> & d##want##dx) const                \
  {                                                                                                \
    fluidPropError(name(), ": ", __PRETTY_FUNCTION__, " derivatives not implemented.");            \
    d##want##d1 = 0;                                                                               \
    d##want##d2 = 0;                                                                               \
    std::fill(d##want##dx.begin(), d##want##dx.end(), 0.);                                         \
    val = want##_from_##prop1##_##prop2(prop1, prop2, x);                                          \
  }                                                                                                \
                                                                                                   \
  propfuncAD(want, prop1, prop2)

/**
 * Base class for fluid properties of vapor mixtures
 *
 * Each interface, in addition to requiring 2 intensive thermodynamic properties,
 * requires the mass fractions of N-1 vapors in the mixture, where N is the
 * number of vapors in the mixture. The mass fraction of the remaining vapor
 * is inferred from the fact that the mass fractions sum to unity.
 */
class VaporMixtureFluidProperties : public FluidProperties
{
public:
  static InputParameters validParams();

  VaporMixtureFluidProperties(const InputParameters & parameters);
  virtual ~VaporMixtureFluidProperties();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
  // clang-format off

  ///@{
  propfunc(p, v, e)
  propfunc(T, v, e)
  propfunc(c, v, e)
  propfunc(rho, p, T)
  propfunc(e, p, T)
  propfunc(c, p, T)
  propfunc(cp, p, T)
  propfunc(cv, p, T)
  propfunc(mu, p, T)
  propfunc(k, p, T)
  propfunc(e, p, rho)
  ///@}

  // clang-format on

#undef propfunc
#undef propfuncAD

      /**
       * Returns the number of secondary vapors
       */
      virtual unsigned int getNumberOfSecondaryVapors() const = 0;

  /**
   * Computes the mass fraction of the primary vapor given mass fractions of the
   * secondary vapors.
   *
   * This uses the relation
   * \f[
   *   \sum\limits_i^N x_i = 1 ,
   * \f]
   * where the mass fractions \f$x_i, i=2\ldots N\f$ correspond to the secondary
   * vapors.
   */
  Real primaryMassFraction(const std::vector<Real> & x) const;

  /**
   * Computes the mixture molar mass for given molar fractions and molar masses
   *
   * @param[in] molar_fractions  Molar fractions for all vapors
   * @param[in] molar_masses  Molar masses for all vapors
   */
  Real mixtureMolarMass(const std::vector<Real> & molar_fractions,
                        const std::vector<Real> & molar_masses) const;

  /**
   * Computes the mass fractions for given molar fractions and molar masses
   *
   * @param[in] molar_fractions  Molar fractions for all vapors
   * @param[in] molar_masses  Molar masses for all vapors
   */
  std::vector<Real> massFractionsFromMolarFractions(const std::vector<Real> & molar_fractions,
                                                    const std::vector<Real> & molar_masses) const;

private:
  template <typename... Args>
  void fluidPropError(Args... args) const
  {
    if (_allow_imperfect_jacobians)
      mooseDoOnce(mooseWarning(std::forward<Args>(args)...));
    else
      mooseError(std::forward<Args>(args)...);
  }
};

#pragma GCC diagnostic pop
