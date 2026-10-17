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

template <typename CppType>
void
NASA7IdealGasFluidProperties::checkTemperature(const CppType & T) const
{
  const auto T_raw = MetaPhysicL::raw_value(T);
  if (T_raw <= 0)
    flagInvalidSolution("Non-positive temperature");
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
// clang-format on

#undef definePropertyFromTemplate
