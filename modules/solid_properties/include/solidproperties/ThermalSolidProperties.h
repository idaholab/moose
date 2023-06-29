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
 * Adds AD versions of each solid property. These functions use the Real versions of these methods
 * to compute the AD variables complete with derivatives. Typically, these do not need to be
 * overriden in derived classes.
 */
#define propfuncAD(want)                                                                           \
  virtual DualReal want##_from_T(const DualReal & T) const                                         \
  {                                                                                                \
    Real x = 0;                                                                                    \
    Real raw = T.value();                                                                          \
    Real dxdT = 0;                                                                                 \
    want##_from_T(raw, x, dxdT);                                                                   \
                                                                                                   \
    DualReal result = x;                                                                           \
    result.derivatives() = T.derivatives() * dxdT;                                                 \
    return result;                                                                                 \
  }                                                                                                \
                                                                                                   \
/**                                                                                                \
 * Adds function definitions with not implemented error. These functions should be overriden in    \
 * derived classes where required. AD versions are constructed automatically using propfuncAD.     \
 */
#define propfunc(want)                                                                             \
  virtual Real want##_from_T(const Real &) const                                                   \
  {                                                                                                \
    mooseError(__PRETTY_FUNCTION__, " not implemented.");                                          \
  }                                                                                                \
                                                                                                   \
  virtual void want##_from_T(const Real & T, Real & val, Real & d##want##dT) const                 \
  {                                                                                                \
    solidPropError(__PRETTY_FUNCTION__, " derivatives not implemented.");                          \
    d##want##dT = 0;                                                                               \
    val = want##_from_T(T);                                                                        \
  }                                                                                                \
                                                                                                   \
  propfuncAD(want)

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
};

#pragma GCC diagnostic pop
