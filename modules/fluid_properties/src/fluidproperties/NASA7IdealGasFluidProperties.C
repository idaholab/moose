//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NASA7IdealGasFluidProperties.h"

registerMooseObject("FluidPropertiesApp", NASA7IdealGasFluidProperties);

InputParameters
NASA7IdealGasFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();

  MooseEnum species(NASA7::speciesNames());
  params.addRequiredParam<MooseEnum>("species", species, "Gas species");

  // The temperature inversions start at the upper end of the temperature range of the data, and
  // none of them needs a pressure guess
  params.suppressParameter<Real>("T_initial_guess");
  params.suppressParameter<Real>("p_initial_guess");

  params.addClassDescription("Ideal gas with temperature-dependent heat capacity, enthalpy, and "
                             "entropy from NASA 7-coefficient polynomials");

  return params;
}

NASA7IdealGasFluidProperties::NASA7IdealGasFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _data(NASA7::speciesData(getParam<MooseEnum>("species"))),
    _R_specific(_R / _data.molar_mass)
{
}

std::string
NASA7IdealGasFluidProperties::fluidName() const
{
  return _data.name;
}

Real
NASA7IdealGasFluidProperties::molarMass() const
{
  return _data.molar_mass;
}

Real
NASA7IdealGasFluidProperties::newtonStartTemperature() const
{
  // The internal energy is convex in temperature over nearly all of the range of the data, so
  // Newton's method started above the solution approaches it from above (see T_from_energy)
  return _data.T_max;
}

template <typename CppType>
void
NASA7IdealGasFluidProperties::checkTemperature(const CppType & T) const
{
  const auto T_raw = MetaPhysicL::raw_value(T);
  if (std::isnan(T_raw) || T_raw <= 0)
    flagInvalidSolution("Non-positive or not-a-number temperature");
  else if (T_raw < _data.T_min || T_raw > _data.T_max)
    flagSolutionWarning(
        "Temperature outside the range of the NASA-7 data; the polynomials are extrapolated");
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::e_from_T(const CppType & T) const
{
  checkTemperature(T);
  return _R_specific * T * (NASA7::hOverRT(_data, T) - 1);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::h_from_T(const CppType & T) const
{
  checkTemperature(T);
  return _R_specific * T * NASA7::hOverRT(_data, T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::cp_from_T(const CppType & T) const
{
  checkTemperature(T);
  return _R_specific * NASA7::cpOverR(_data, T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::cv_from_T(const CppType & T) const
{
  checkTemperature(T);
  return _R_specific * (NASA7::cpOverR(_data, T) - 1);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::gamma_from_T(const CppType & T) const
{
  checkTemperature(T);
  const auto cp_R = NASA7::cpOverR(_data, T);
  return cp_R / (cp_R - 1);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::rho_from_p_T_template(const CppType & p, const CppType & T) const
{
  return p / (_R_specific * T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::v_from_p_T_template(const CppType & p, const CppType & T) const
{
  return _R_specific * T / p;
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::e_from_p_T_template(const CppType & /*p*/, const CppType & T) const
{
  return e_from_T(T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::h_from_p_T_template(const CppType & /*p*/, const CppType & T) const
{
  return h_from_T(T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::s_from_p_T_template(const CppType & p, const CppType & T) const
{
  using std::log;
  checkTemperature(T);
  return _R_specific * (NASA7::sOverR(_data, T) - log(p / NASA7::reference_pressure));
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::cp_from_p_T_template(const CppType & /*p*/, const CppType & T) const
{
  return cp_from_T(T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::cv_from_p_T_template(const CppType & /*p*/, const CppType & T) const
{
  return cv_from_T(T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::c_from_p_T_template(const CppType & /*p*/, const CppType & T) const
{
  using std::sqrt;
  return sqrt(gamma_from_T(T) * _R_specific * T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::gamma_from_p_T_template(const CppType & /*p*/,
                                                      const CppType & T) const
{
  return gamma_from_T(T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::p_from_T_v_template(const CppType & T, const CppType & v) const
{
  return _R_specific * T / v;
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::e_from_T_v_template(const CppType & T, const CppType & /*v*/) const
{
  return e_from_T(T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::h_from_T_v_template(const CppType & T, const CppType & /*v*/) const
{
  return h_from_T(T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::s_from_T_v_template(const CppType & T, const CppType & v) const
{
  return s_from_p_T_template(p_from_T_v_template(T, v), T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::cv_from_T_v_template(const CppType & T, const CppType & /*v*/) const
{
  return cv_from_T(T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::e_from_p_rho_template(const CppType & p, const CppType & rho) const
{
  return e_from_T(p / (rho * _R_specific));
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::T_from_energy(const CppType & y, const Real k) const
{
  // Start at the upper end of the temperature range of the data. y(T) is convex over nearly all of
  // that range, because the heat capacity increases with temperature, so Newton's method started
  // above the solution approaches it from above. Started below it, the first step can overshoot
  // far beyond the range, where the extrapolated polynomials eventually give dy/dT <= 0 and a
  // second, spurious solution. For the built-in species, this start finds the solution up to at
  // least 1.7 T_max.
  const Real T0 = _data.T_max;

  // The energies include the enthalpy of formation, so they can be close to zero, where the
  // convergence test of NewtonSolve, which is relative to the target value, would demand accuracy
  // below roundoff. Solve y(T) + shift = y + shift instead, with the shift (dy/dT) T - y(T) at T0.
  // Both sides are then close to (dy/dT) T, and the convergence test bounds the relative error in
  // temperature.
  const Real shift = _R_specific * T0 * (NASA7::cpOverR(_data, T0) - NASA7::hOverRT(_data, T0));

  const auto shifted_y = [this, k, shift](const CppType & /*y*/,
                                          const CppType & T,
                                          CppType & y_T,
                                          CppType & dy_T_dy,
                                          CppType & dy_T_dT)
  {
    y_T = _R_specific * T * (NASA7::hOverRT(_data, T) - k) + shift;
    dy_T_dy = 0;
    dy_T_dT = _R_specific * (NASA7::cpOverR(_data, T) - k);
  };

  const CppType target = y + shift;
  return FluidPropertiesUtils::NewtonSolve(y,
                                           target,
                                           T0,
                                           _tolerance,
                                           shifted_y,
                                           name() + "::T_from_energy",
                                           _max_newton_its,
                                           _verbose_newton)
      .first;
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::T_from_v_e_template(const CppType & /*v*/, const CppType & e) const
{
  const auto T = T_from_energy(e, 1);
  checkTemperature(T);
  return T;
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::p_from_v_e_template(const CppType & v, const CppType & e) const
{
  const auto T = T_from_energy(e, 1);
  checkTemperature(T);
  return _R_specific * T / v;
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::c_from_v_e_template(const CppType & /*v*/, const CppType & e) const
{
  using std::sqrt;
  const auto T = T_from_energy(e, 1);
  return sqrt(gamma_from_T(T) * _R_specific * T);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::cp_from_v_e_template(const CppType & /*v*/, const CppType & e) const
{
  return cp_from_T(T_from_energy(e, 1));
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::cv_from_v_e_template(const CppType & /*v*/, const CppType & e) const
{
  return cv_from_T(T_from_energy(e, 1));
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::gamma_from_v_e_template(const CppType & /*v*/,
                                                      const CppType & e) const
{
  return gamma_from_T(T_from_energy(e, 1));
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::s_from_v_e_template(const CppType & v, const CppType & e) const
{
  return s_from_T_v_template(T_from_energy(e, 1), v);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::g_from_v_e_template(const CppType & v, const CppType & e) const
{
  using std::log;
  const auto T = T_from_energy(e, 1);
  checkTemperature(T);
  const auto p = _R_specific * T / v;
  // g = h - T s
  return _R_specific * T *
         (NASA7::hOverRT(_data, T) - NASA7::sOverR(_data, T) + log(p / NASA7::reference_pressure));
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::T_from_p_h_template(const CppType & /*p*/, const CppType & h) const
{
  const auto T = T_from_energy(h, 0);
  checkTemperature(T);
  return T;
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::s_from_h_p_template(const CppType & h, const CppType & p) const
{
  return s_from_p_T_template(p, T_from_energy(h, 0));
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::p_from_h_s_template(const CppType & h, const CppType & s) const
{
  using std::exp;
  const auto T = T_from_energy(h, 0);
  checkTemperature(T);
  // Inverse of s = R_s (s0/R - ln(p / p_ref))
  return NASA7::reference_pressure * exp(NASA7::sOverR(_data, T) - s / _R_specific);
}

template <typename CppType>
CppType
NASA7IdealGasFluidProperties::e_from_v_h_template(const CppType & /*v*/, const CppType & h) const
{
  const auto T = T_from_energy(h, 0);
  checkTemperature(T);
  return h - _R_specific * T;
}

/**
 * Defines the value and derivative methods of a property, for Real and AD arguments, from the
 * value template of the property. The derivatives come from forward-mode dual numbers.
 */
#define definePropertyFromTemplate(want, prop1, prop2)                                             \
  template <typename CppType>                                                                      \
  void NASA7IdealGasFluidProperties::want##_from_##prop1##_##prop2##_template(                     \
      const CppType & prop1,                                                                       \
      const CppType & prop2,                                                                       \
      CppType & val,                                                                               \
      CppType & d##want##d1,                                                                       \
      CppType & d##want##d2) const                                                                 \
  {                                                                                                \
    const auto value = [this](const auto & x, const auto & y)                                      \
    { return this->want##_from_##prop1##_##prop2##_template(x, y); };                              \
    xyDerivatives(prop1, prop2, val, d##want##d1, d##want##d2, value);                             \
  }                                                                                                \
                                                                                                   \
  Real NASA7IdealGasFluidProperties::want##_from_##prop1##_##prop2(Real prop1, Real prop2) const   \
  {                                                                                                \
    return want##_from_##prop1##_##prop2##_template(prop1, prop2);                                 \
  }                                                                                                \
                                                                                                   \
  ADReal NASA7IdealGasFluidProperties::want##_from_##prop1##_##prop2(const ADReal & prop1,         \
                                                                     const ADReal & prop2) const   \
  {                                                                                                \
    return want##_from_##prop1##_##prop2##_template(prop1, prop2);                                 \
  }                                                                                                \
                                                                                                   \
  void NASA7IdealGasFluidProperties::want##_from_##prop1##_##prop2(                                \
      Real prop1, Real prop2, Real & val, Real & d##want##d1, Real & d##want##d2) const            \
  {                                                                                                \
    want##_from_##prop1##_##prop2##_template(prop1, prop2, val, d##want##d1, d##want##d2);         \
  }                                                                                                \
                                                                                                   \
  void NASA7IdealGasFluidProperties::want##_from_##prop1##_##prop2(const ADReal & prop1,           \
                                                                   const ADReal & prop2,           \
                                                                   ADReal & val,                   \
                                                                   ADReal & d##want##d1,           \
                                                                   ADReal & d##want##d2) const     \
  {                                                                                                \
    want##_from_##prop1##_##prop2##_template(prop1, prop2, val, d##want##d1, d##want##d2);         \
  }

// clang-format off
definePropertyFromTemplate(rho, p, T)
definePropertyFromTemplate(v, p, T)
definePropertyFromTemplate(e, p, T)
definePropertyFromTemplate(h, p, T)
definePropertyFromTemplate(s, p, T)
definePropertyFromTemplate(cp, p, T)
definePropertyFromTemplate(cv, p, T)
definePropertyFromTemplate(c, p, T)
definePropertyFromTemplate(gamma, p, T)
definePropertyFromTemplate(p, T, v)
definePropertyFromTemplate(e, T, v)
definePropertyFromTemplate(h, T, v)
definePropertyFromTemplate(s, T, v)
definePropertyFromTemplate(cv, T, v)
definePropertyFromTemplate(e, p, rho)
definePropertyFromTemplate(T, v, e)
definePropertyFromTemplate(p, v, e)
definePropertyFromTemplate(c, v, e)
definePropertyFromTemplate(cp, v, e)
definePropertyFromTemplate(cv, v, e)
definePropertyFromTemplate(gamma, v, e)
definePropertyFromTemplate(s, v, e)
definePropertyFromTemplate(g, v, e)
definePropertyFromTemplate(T, p, h)
definePropertyFromTemplate(s, h, p)
definePropertyFromTemplate(p, h, s)
definePropertyFromTemplate(e, v, h)
// clang-format on

#undef definePropertyFromTemplate
