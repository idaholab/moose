//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolidProperties.h"

/**
 * Adds default error implementation for non-AD value, which should be overridden in child classes
 */
#define propfuncNonADValueOnly(want)                                                               \
  virtual Real want##_from_T(const Real &) const                                                   \
  {                                                                                                \
    mooseError(__PRETTY_FUNCTION__, " not implemented.");                                          \
  }

/**
 * Adds default error implementation for non-AD value and derivatives, which should be overridden
 * in child classes
 */
#define propfuncNonADDerivatives(want)                                                             \
  virtual void want##_from_T(const Real & T, Real & val, Real & d##want##dT) const                 \
  {                                                                                                \
    solidPropError(__PRETTY_FUNCTION__, " derivatives not implemented.");                          \
    d##want##dT = 0;                                                                               \
    val = want##_from_T(T);                                                                        \
  }

/**
 * Adds AD versions of each solid property. These functions use the Real versions of these methods
 * to compute the AD variables complete with derivatives. Typically, these do not need to be
 * overridden in derived classes.
 */
#define propfuncAD(want)                                                                           \
  virtual ADReal want##_from_T(const ADReal & T) const                                             \
  {                                                                                                \
    Real x = 0;                                                                                    \
    Real raw = T.value();                                                                          \
    Real dxdT = 0;                                                                                 \
    want##_from_T(raw, x, dxdT);                                                                   \
                                                                                                   \
    ADReal result = x;                                                                             \
    result.derivatives() = T.derivatives() * dxdT;                                                 \
    return result;                                                                                 \
  }

/**
 * Adds function definitions with not implemented error. These functions should be overridden in
 * derived classes where required. AD versions are constructed automatically using propfuncAD.
 */
#define propfunc(want) propfuncNonADValueOnly(want) propfuncNonADDerivatives(want) propfuncAD(want)

/**
 * Common class for solid properties that are a function of temperature
 */
class ThermalSolidProperties : public SolidProperties
{
public:
  static InputParameters validParams();

  ThermalSolidProperties(const InputParameters & parameters);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
  // clang-format off

  /**
   * @brief Compute a solid property as a function of temperature (Kelvin)
   *
   * For all functions, the first argument is the given property (temperature)
   * that defines the solid properties. For the one-argument variants, the desired
   * property is the return value. The three-argument variants also provide partial
   * derivatives dx/dT where x is the desired property being computed, and T is the
   * temperature. The desired property and dx/dT are stored into the 2nd and 3rd
   * arguments, respectively.
   *
   * Properties/parameters used in these function are listed below with their units:
   *
   * @begincode
   * k      thermal conductivity [W/(m*K)]
   * cp     isobaric specific heat capacity [J/(kg*K)]
   * rho    density [kg/m^3]
   * @endcode
   */
  ///@{
  propfunc(k)
  propfunc(cp)
  propfunc(rho)

  /**
   * Computes the integral of isobaric specific heat capacity w.r.t. temperature,
   * from the zero-e reference temperature, provided by a user parameter.
   *
   * Note that the constant of integration is not included. This function is
   * called in the \c e_from_T(T) method.
   */
  virtual Real cp_integral(const Real & /*T*/) const
  {
    mooseError(__PRETTY_FUNCTION__, " not implemented.");
  }

  // Specific internal energy methods; these designed to not be virtual; only
  // cp_integral(T) needs to be implemented.
  Real e_from_T(const Real & T) const;
  void e_from_T(const Real & T, Real & val, Real & dedT) const;
  propfuncAD(e)
  ///@}

  // clang-format on

#undef propfunc
#undef propfuncAD

      protected :
    /// Function to determine how to elevate errors/warnings about missing AD derivatives
    template <typename... Args>
    void solidPropError(Args... args) const
  {
    if (_allow_imperfect_jacobians)
      mooseDoOnce(mooseWarning(std::forward<Args>(args)...));
    else
      mooseError(std::forward<Args>(args)...);
  }

protected:
  /// Temperature at which the specific internal energy is assumed to be zero
  const Real _T_zero_e;
};

#pragma GCC diagnostic pop
