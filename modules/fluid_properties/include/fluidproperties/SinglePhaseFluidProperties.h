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
#include "NewtonInversion.h"
#include "metaphysicl/dualnumberarray.h"

/**
 * Adds AD versions of each fluid property. These functions use the Real versions of these methods
 * to compute the AD variables complete with derivatives. Typically, these do not need to be
 * overriden in derived classes.
 */
#define propfuncAD(want, prop1, prop2)                                                             \
  virtual ADReal want##_from_##prop1##_##prop2(const ADReal & p1, const ADReal & p2) const         \
  {                                                                                                \
    Real x = 0;                                                                                    \
    Real raw1 = p1.value();                                                                        \
    Real raw2 = p2.value();                                                                        \
    Real dxd1 = 0;                                                                                 \
    Real dxd2 = 0;                                                                                 \
    want##_from_##prop1##_##prop2(raw1, raw2, x, dxd1, dxd2);                                      \
                                                                                                   \
    ADReal result = x;                                                                             \
    result.derivatives() = p1.derivatives() * dxd1 + p2.derivatives() * dxd2;                      \
    return result;                                                                                 \
  }                                                                                                \
                                                                                                   \
  virtual void want##_from_##prop1##_##prop2(const ADReal & prop1,                                 \
                                             const ADReal & prop2,                                 \
                                             ADReal & val,                                         \
                                             ADReal & d##want##d1,                                 \
                                             ADReal & d##want##d2) const                           \
  {                                                                                                \
    unimplementedDerivativeMethod(__PRETTY_FUNCTION__);                                            \
    Real dummy, tmp1, tmp2;                                                                        \
    val = want##_from_##prop1##_##prop2(prop1, prop2);                                             \
    want##_from_##prop1##_##prop2(prop1.value(), prop2.value(), dummy, tmp1, tmp2);                \
    d##want##d1 = tmp1;                                                                            \
    d##want##d2 = tmp2;                                                                            \
  }

/**
 * Adds function definitions with not implemented error. These functions should be overriden in
 * derived classes where required. AD versions are constructed automatically using propfuncAD.
 */
#define propfunc(want, prop1, prop2)                                                               \
  virtual Real want##_from_##prop1##_##prop2(Real, Real) const                                     \
  {                                                                                                \
    mooseError(                                                                                    \
        "The fluid properties class '",                                                            \
        type(),                                                                                    \
        "' has not implemented the method below. If your application requires this method, you "   \
        "must either implement it or use a different fluid properties class.\n\n",                 \
        __PRETTY_FUNCTION__);                                                                      \
  }                                                                                                \
                                                                                                   \
  virtual void want##_from_##prop1##_##prop2(                                                      \
      Real prop1, Real prop2, Real & val, Real & d##want##d1, Real & d##want##d2) const            \
  {                                                                                                \
    unimplementedDerivativeMethod(__PRETTY_FUNCTION__);                                            \
    d##want##d1 = 0;                                                                               \
    d##want##d2 = 0;                                                                               \
    val = want##_from_##prop1##_##prop2(prop1, prop2);                                             \
  }                                                                                                \
                                                                                                   \
  propfuncAD(want, prop1, prop2)

/**
 * Adds Real declarations of functions that have a default implementation.
 * Important: properties declared using this macro must be defined in SinglePhaseFluidProperties.C.
 * AD versions are constructed automatically using propfuncAD.
 */
#define propfuncWithDefault(want, prop1, prop2)                                                    \
  virtual Real want##_from_##prop1##_##prop2(Real, Real) const;                                    \
  virtual void want##_from_##prop1##_##prop2(                                                      \
      Real prop1, Real prop2, Real & val, Real & d##want##d1, Real & d##want##d2) const;           \
                                                                                                   \
  propfuncAD(want, prop1, prop2)

/**
 * Adds Real and ADReal declarations of functions that have an implementation.
 */
#define propfuncWithDefinitionOverride(want, prop1, prop2)                                         \
  Real want##_from_##prop1##_##prop2(Real, Real) const override;                                   \
  void want##_from_##prop1##_##prop2(                                                              \
      Real prop1, Real prop2, Real & val, Real & d##want##d1, Real & d##want##d2) const override;  \
  ADReal want##_from_##prop1##_##prop2(const ADReal &, const ADReal &) const override;             \
  void want##_from_##prop1##_##prop2(const ADReal & prop1,                                         \
                                     const ADReal & prop2,                                         \
                                     ADReal & val,                                                 \
                                     ADReal & d##want##d1,                                         \
                                     ADReal & d##want##d2) const override;                         \
  template <typename CppType>                                                                      \
  CppType want##_from_##prop1##_##prop2##_template(const CppType & prop1, const CppType & prop2)   \
      const;                                                                                       \
  template <typename CppType>                                                                      \
  void want##_from_##prop1##_##prop2##_template(const CppType & prop1,                             \
                                                const CppType & prop2,                             \
                                                CppType & val,                                     \
                                                CppType & d##want##d1,                             \
                                                CppType & d##want##d2) const

/**
 * Common class for single phase fluid properties
 */
class SinglePhaseFluidProperties : public FluidProperties
{
public:
  static InputParameters validParams();

  SinglePhaseFluidProperties(const InputParameters & parameters);
  virtual ~SinglePhaseFluidProperties();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
  // clang-format off

  /**
   * @brief Compute a fluid property given for the state defined by two given properties.
   *
   * For all functions, the first two arguments are the given properties that define the fluid
   * state.  For the two-argument variants, the desired property is the return value.
   * The five-argument variants also provide partial derivatives dx/da and dx/db where x is the
   * desired property being computed, a is the first given property, and b is the second given
   * property.  The desired property, dx/da, and dx/db are stored into the 3rd, 4th, and 5th
   * arguments respectively.
   *
   * Properties/parameters used in these function are listed below with their units:
   *
   * @begincode
   * p      pressure [Pa]
   * T      temperature [K]
   * e      specific internal energy [J/kg]
   * v      specific volume [m^3/kg]
   * rho    density [kg/m^3]
   * h      specific enthalpy [J/kg]
   * s      specific entropy [J/(kg*K)]
   * mu     viscosity [Pa*s]
   * k      thermal conductivity [W/(m*K)]
   * c      speed of sound [m/s]
   * cp     constant-pressure specific heat [J/K]
   * cv     constant-volume specific heat [J/K]
   * beta   volumetric thermal expansion coefficient [1/K]
   * g      Gibbs free energy [J]
   * pp_sat partial pressure at saturation [Pa]
   * gamma  Adiabatic ratio (cp/cv) [-]
   * @endcode
   *
   * As an example:
   *
   * @begincode
   * // calculate pressure given specific vol and energy:
   * auto pressure = your_fluid_properties_object.p_from_v_e(specific_vol, specific_energy);
   *
   * // or use the derivative variant:
   * Real dp_dv = 0; // derivative will be stored into here
   * Real dp_de = 0; // derivative will be stored into here
   * your_fluid_properties_object.p_from_v_e(specific_vol, specific_energy, pressure, dp_dv, dp_de);
   * @endcode
   *
   * Automatic differentiation (AD) support is provided through x_from_a_b(ADReal a, ADReal b) and
   * x_from_a_b(ADReal a, ADReal b, ADReal x, ADReal dx_da, ADReal dx_db) versions of the
   * functions where a and b must be ADReal/DualNumber's calculated using all AD-supporting values:
   *
   * @begincode
   * auto v = 1/rho; // rho must be an AD non-linear variable.
   * auto e = rhoE/rho - vel_energy; // rhoE and vel_energy must be AD variables/numbers also.
   * auto pressure = your_fluid_properties_object.p_from_v_e(v, e);
   * // pressure now contains partial derivatives w.r.t. all degrees of freedom
   * @endcode
   */
  ///@{
  propfunc(p, v, e)
  propfunc(T, v, e)
  propfunc(c, v, e)
  propfunc(cp, v, e)
  propfunc(cv, v, e)
  propfunc(mu, v, e)
  propfunc(k, v, e)
  propfuncWithDefault(s, v, e)
  propfunc(s, h, p)
  propfunc(rho, p, s)
  propfunc(e, v, h)
  propfuncWithDefault(s, p, T)
  propfunc(pp_sat, p, T)
  propfunc(mu, rho, T)
  propfunc(k, rho, T)
  propfuncWithDefault(c, p, T)
  propfuncWithDefault(cp, p, T)
  propfuncWithDefault(cv, p, T)
  propfuncWithDefault(mu, p, T)
  propfuncWithDefault(k, p, T)
  propfunc(rho, p, T)
  propfunc(e, p, rho)
  propfunc(e, T, v)
  propfunc(p, T, v)
  propfunc(h, T, v)
  propfunc(s, T, v)
  propfunc(cv, T, v)
  propfunc(h, p, T)
  propfuncWithDefault(h, v, e)
  propfunc(g, v, e)
  propfuncWithDefault(p, h, s)
  propfunc(T, h, p)  // temporary, until uniformization
  propfuncWithDefault(T, p, h)
  propfuncWithDefault(beta, p, T)
  propfuncWithDefault(v, p, T)
  propfuncWithDefault(e, p, T)
  propfuncWithDefault(gamma, v, e)
  propfuncWithDefault(gamma, p, T)
  ///@}

  // clang-format on

#undef propfunc
#undef propfuncWithDefault
#undef propfuncAD

      /**
       * Fluid name
       * @return string representing fluid name
       */
      virtual std::string fluidName() const;

  /**
   * Molar mass [kg/mol]
   * @return molar mass
   */
  virtual Real molarMass() const;

  /**
   * Critical pressure
   * @return critical pressure (Pa)
   */
  virtual Real criticalPressure() const;

  /**
   * Critical temperature
   * @return critical temperature (K)
   */
  virtual Real criticalTemperature() const;

  /**
   * Critical density
   * @return critical density (kg/m^3)
   */
  virtual Real criticalDensity() const;

  /**
   * Critical specific internal energy
   * @return specific internal energy (J/kg)
   */
  virtual Real criticalInternalEnergy() const;

  /**
   * Triple point pressure
   * @return triple point pressure (Pa)
   */
  virtual Real triplePointPressure() const;

  /**
   * Triple point temperature
   * @return triple point temperature (K)
   */
  virtual Real triplePointTemperature() const;

  /**
   * Specific internal energy from temperature and specific volume
   *
   * @param[in] T     temperature
   * @param[in] v     specific volume
   */
  virtual Real e_spndl_from_v(Real v) const;

  /**
   * Specific internal energy from temperature and specific volume
   *
   * @param[in] T     temperature
   * @param[in] v     specific volume
   */
  virtual void v_e_spndl_from_T(Real T, Real & v, Real & e) const;

  /**
   * Vapor pressure. Used to delineate liquid and gas phases.
   * Valid for temperatures between the triple point temperature
   * and the critical temperature
   *
   * @param T fluid temperature (K)
   * @param[out] saturation pressure (Pa)
   * @param[out] derivative of saturation pressure wrt temperature (Pa/K)
   */
  virtual Real vaporPressure(Real T) const;
  virtual void vaporPressure(Real T, Real & psat, Real & dpsat_dT) const;
  virtual ADReal vaporPressure(const ADReal & T) const;

  /**
   * Vapor temperature. Used to delineate liquid and gas phases.
   * Valid for pressures between the triple point pressure
   * and the critical pressure
   *
   * @param p fluid pressure (Pa)
   * @param[out] saturation temperature (K)
   * @param[out] derivative of saturation temperature wrt pressure
   */
  virtual Real vaporTemperature(Real p) const;
  virtual void vaporTemperature(Real p, Real & Tsat, Real & dTsat_dp) const;
  virtual ADReal vaporTemperature(const ADReal & p) const;

  /**
   * Henry's law coefficients for dissolution in water
   * @return Henry's constant coefficients
   */
  virtual std::vector<Real> henryCoefficients() const;

  template <typename CppType>
  void v_e_from_p_T(const CppType & p, const CppType & T, CppType & v, CppType & e) const;
  template <typename CppType>
  void v_e_from_p_T(const CppType & p,
                    const CppType & T,
                    CppType & v,
                    CppType & dv_dp,
                    CppType & dv_dT,
                    CppType & e,
                    CppType & de_dp,
                    CppType & de_dT) const;

  /**
   * Combined methods. These methods are particularly useful for the PorousFlow
   * module, where density and viscosity are typically both computed everywhere.
   * The combined methods allow the most efficient means of calculating both
   * properties, especially where rho(p, T) and mu(rho, T). In this case, an
   * extra density calculation would be required to calculate mu(p, T). All
   * property names are described above.
   */
  virtual void rho_mu_from_p_T(Real p, Real T, Real & rho, Real & mu) const;
  virtual void rho_mu_from_p_T(Real p,
                               Real T,
                               Real & rho,
                               Real & drho_dp,
                               Real & drho_dT,
                               Real & mu,
                               Real & dmu_dp,
                               Real & dmu_dT) const;
  virtual void rho_mu_from_p_T(const ADReal & p, const ADReal & T, ADReal & rho, ADReal & mu) const;

  virtual void rho_e_from_p_T(Real p,
                              Real T,
                              Real & rho,
                              Real & drho_dp,
                              Real & drho_dT,
                              Real & e,
                              Real & de_dp,
                              Real & de_dT) const;

  /**
   * Determines (p,T) from (v,e) using Newton Solve in 2D
   * Useful for conversion between different sets of state variables
   *
   * @param[in] v specific volume (m^3 / kg)
   * @param[in] e specific internal energy (J / kg)
   * @param[in] p0 initial guess for pressure (Pa / kg)
   * @param[in] T0 initial guess for temperature (K)
   * @param[out] fluid pressure (Pa / kg)
   * @param[out] Temperature (K)
   */
  template <typename CppType>
  void p_T_from_v_e(const CppType & v,
                    const CppType & e,
                    Real p0,
                    Real T0,
                    CppType & p,
                    CppType & T,
                    bool & conversion_succeeded) const;

  /**
   * Determines (p,T) from (v,h) using Newton Solve in 2D
   * Useful for conversion between different sets of state variables
   *
   * @param[in] v specific volume (m^3 / kg)
   * @param[in] h specific enthalpy (J / kg)
   * @param[in] p0 initial guess for pressure (Pa / kg)
   * @param[in] T0 initial guess for temperature (K)
   * @param[out] fluid pressure (Pa / kg)
   * @param[out] Temperature (K)
   */
  template <typename T>
  void p_T_from_v_h(const T & v,
                    const T & h,
                    Real p0,
                    Real T0,
                    T & pressure,
                    T & temperature,
                    bool & conversion_succeeded) const;
  /**
   * Determines (p,T) from (h,s) using Newton Solve in 2D
   * Useful for conversion between different sets of state variables
   *
   * @param[in] h specific enthalpy (J / kg)
   * @param[in] s specific entropy (J/K*kg)
   * @param[in] p0 initial guess for pressure (Pa / kg)
   * @param[in] T0 initial guess for temperature (K)
   * @param[out] fluid pressure (Pa / kg)
   * @param[out] Temperature (K)
   */
  template <typename T>
  void p_T_from_h_s(const T & h,
                    const T & s,
                    Real p0,
                    Real T0,
                    T & pressure,
                    T & temperature,
                    bool & conversion_succeeded) const;

protected:
  /**
   * Computes the dependent variable z and its derivatives with respect to the independent
   * variables x and y using the simple two parameter \p z_from_x_y functor. The derivatives are
   * computed using a compound automatic differentiation type
   */
  template <typename T, typename Functor>
  static void
  xyDerivatives(const T x, const T & y, T & z, T & dz_dx, T & dz_dy, const Functor & z_from_x_y);

  /**
   * Given a type example, this method returns zero and unity representations of that type (first
   * and second members of returned pair respectively)
   */
  template <typename T>
  static std::pair<T, T> makeZeroAndOne(const T &);

  /**
   * Newton's method may be used to convert between variable sets
   */
  /// Relative tolerance of the solves
  const Real _tolerance;
  /// Initial guess for temperature (or temperature used to compute the initial guess)
  const Real _T_initial_guess;
  /// Initial guess for pressure (or pressure used to compute the initial guess)
  const Real _p_initial_guess;
  /// Maximum number of iterations for the variable conversion newton solves
  const unsigned int _max_newton_its;

private:
  void unimplementedDerivativeMethod(const std::string & property_function_name) const
  {
    const std::string message =
        "The fluid properties class '" + type() +
        "' has not implemented the method below, which computes derivatives of fluid properties "
        "with regards to the flow variables. If your application requires this "
        "method, you must either implement it or use a different fluid properties "
        " class.\n\n" +
        property_function_name;

    if (_allow_imperfect_jacobians)
      mooseDoOnce(mooseWarning(message + "\nThe unimplemented derivatives for this fluid property "
                                         "are currently neglected, set to 0."));
    else
      mooseError(message + "\n\nYou can avoid this error by neglecting the "
                           "unimplemented derivatives of fluid properties by setting the "
                           "'allow_imperfect_jacobians' parameter");
  }
};

#pragma GCC diagnostic pop

template <typename T>
std::pair<T, T>
SinglePhaseFluidProperties::makeZeroAndOne(const T & /*ex*/)
{
  return {T{0, 0}, T{1, 0}};
}

template <>
inline std::pair<Real, Real>
SinglePhaseFluidProperties::makeZeroAndOne(const Real & /*ex*/)
{
  return {Real{0}, Real{1}};
}

template <typename T, typename Functor>
void
SinglePhaseFluidProperties::xyDerivatives(
    const T x, const T & y, T & z, T & dz_dx, T & dz_dy, const Functor & z_from_x_y)
{
  typedef MetaPhysicL::DualNumber<T, MetaPhysicL::NumberArray<2, T>> CompoundType;
  const auto [zero, one] = makeZeroAndOne(x);

  CompoundType x_c(x, zero);
  auto & x_cd = x_c.derivatives();
  x_cd[0] = one;
  CompoundType y_c(y, zero);
  auto & y_cd = y_c.derivatives();
  y_cd[1] = one;

  const auto z_c = z_from_x_y(x_c, y_c);
  z = z_c.value();
  dz_dx = z_c.derivatives()[0];
  dz_dy = z_c.derivatives()[1];
}

template <typename CppType>
void
SinglePhaseFluidProperties::p_T_from_v_e(const CppType & v, // v value
                                         const CppType & e, // e value
                                         const Real p0,     // initial guess
                                         const Real T0,     // initial guess
                                         CppType & p,       // returned pressure
                                         CppType & T,       // returned temperature
                                         bool & conversion_succeeded) const
{
  auto v_lambda = [&](const CppType & pressure,
                      const CppType & temperature,
                      CppType & new_v,
                      CppType & dv_dp,
                      CppType & dv_dT) { v_from_p_T(pressure, temperature, new_v, dv_dp, dv_dT); };
  auto e_lambda = [&](const CppType & pressure,
                      const CppType & temperature,
                      CppType & new_e,
                      CppType & de_dp,
                      CppType & de_dT) { e_from_p_T(pressure, temperature, new_e, de_dp, de_dT); };
  try
  {
    FluidPropertiesUtils::NewtonSolve2D(
        v, e, p0, T0, p, T, _tolerance, _tolerance, v_lambda, e_lambda);
    conversion_succeeded = true;
  }
  catch (MooseException &)
  {
    conversion_succeeded = false;
  }

  if (!conversion_succeeded)
    mooseDoOnce(mooseWarning("Conversion from (v, e)=(", v, ", ", e, ") to (p, T) failed"));
}

template <typename T>
void
SinglePhaseFluidProperties::p_T_from_v_h(const T & v,     // v value
                                         const T & h,     // e value
                                         const Real p0,   // initial guess
                                         const Real T0,   // initial guess
                                         T & pressure,    // returned pressure
                                         T & temperature, // returned temperature
                                         bool & conversion_succeeded) const
{
  auto v_lambda = [&](const T & pressure, const T & temperature, T & new_v, T & dv_dp, T & dv_dT)
  { v_from_p_T(pressure, temperature, new_v, dv_dp, dv_dT); };
  auto h_lambda = [&](const T & pressure, const T & temperature, T & new_h, T & dh_dp, T & dh_dT)
  { h_from_p_T(pressure, temperature, new_h, dh_dp, dh_dT); };
  try
  {
    FluidPropertiesUtils::NewtonSolve2D(
        v, h, p0, T0, pressure, temperature, _tolerance, _tolerance, v_lambda, h_lambda);
    conversion_succeeded = true;
  }
  catch (MooseException &)
  {
    conversion_succeeded = false;
  }

  if (!conversion_succeeded)
    mooseDoOnce(mooseWarning("Conversion from (v, h)=(", v, ", ", h, ") to (p, T) failed"));
}

template <typename T>
void
SinglePhaseFluidProperties::p_T_from_h_s(const T & h,     // h value
                                         const T & s,     // s value
                                         const Real p0,   // initial guess
                                         const Real T0,   // initial guess
                                         T & pressure,    // returned pressure
                                         T & temperature, // returned temperature
                                         bool & conversion_succeeded) const
{
  auto h_lambda = [&](const T & pressure, const T & temperature, T & new_h, T & dh_dp, T & dh_dT)
  { h_from_p_T(pressure, temperature, new_h, dh_dp, dh_dT); };
  auto s_lambda = [&](const T & pressure, const T & temperature, T & new_s, T & ds_dp, T & ds_dT)
  { s_from_p_T(pressure, temperature, new_s, ds_dp, ds_dT); };
  try
  {
    FluidPropertiesUtils::NewtonSolve2D(
        h, s, p0, T0, pressure, temperature, _tolerance, _tolerance, h_lambda, s_lambda);
    conversion_succeeded = true;
  }
  catch (MooseException &)
  {
    conversion_succeeded = false;
  }

  if (!conversion_succeeded)
    mooseDoOnce(mooseWarning("Conversion from (h, s)=(", h, ", ", s, ") to (p, T) failed"));
}

template <typename CppType>
void
SinglePhaseFluidProperties::v_e_from_p_T(const CppType & p,
                                         const CppType & T,
                                         CppType & v,
                                         CppType & e) const
{
  const CppType rho = rho_from_p_T(p, T);
  v = 1.0 / rho;
  try
  {
    // more likely to not involve a Newton search
    e = e_from_p_T(p, T);
  }
  catch (...)
  {
    e = e_from_p_rho(p, rho);
  }
}

template <typename CppType>
void
SinglePhaseFluidProperties::v_e_from_p_T(const CppType & p,
                                         const CppType & T,
                                         CppType & v,
                                         CppType & dv_dp,
                                         CppType & dv_dT,
                                         CppType & e,
                                         CppType & de_dp,
                                         CppType & de_dT) const
{
  CppType rho, drho_dp, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);

  v = 1.0 / rho;
  const CppType dv_drho = -1.0 / (rho * rho);
  dv_dp = dv_drho * drho_dp;
  dv_dT = dv_drho * drho_dT;

  CppType de_dp_partial, de_drho;
  e_from_p_rho(p, rho, e, de_dp_partial, de_drho);
  de_dp = de_dp_partial + de_drho * drho_dp;
  de_dT = de_drho * drho_dT;
}
