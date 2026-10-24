//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasMixtureFluidProperties.h"
#include "IdealGasFluidProperties.h"
#include "NASA7IdealGasFluidProperties.h"
#include "NewtonInversion.h"

#include <limits>

registerMooseObject("FluidPropertiesApp", IdealGasMixtureFluidProperties);

/**
 * Defines a mass-specific property z from (p,T)
 */
#define define_mass_specific_prop_from_p_T(prop)                                                   \
  Real IdealGasMixtureFluidProperties::prop##_from_p_T(                                            \
      Real p, Real T, const std::vector<Real> & x_secondary) const                                 \
  {                                                                                                \
    const auto x = secondaryToAllMassFractions(x_secondary);                                       \
    mooseAssert(x.size() == _n_components, "Size mismatch");                                       \
                                                                                                   \
    Real z = 0;                                                                                    \
    for (const auto i : make_range(_n_components))                                                 \
      z += x[i] * _component_fps[i]->prop##_from_p_T(p, T);                                        \
                                                                                                   \
    return z;                                                                                      \
  }                                                                                                \
  ADReal IdealGasMixtureFluidProperties::prop##_from_p_T(                                          \
      const ADReal & p, const ADReal & T, const std::vector<ADReal> & x_secondary) const           \
  {                                                                                                \
    const auto x = secondaryToAllMassFractions(x_secondary);                                       \
    mooseAssert(x.size() == _n_components, "Size mismatch");                                       \
                                                                                                   \
    ADReal z = 0;                                                                                  \
    for (const auto i : make_range(_n_components))                                                 \
      z += x[i] * _component_fps[i]->prop##_from_p_T(p, T);                                        \
                                                                                                   \
    return z;                                                                                      \
  }

/**
 * Defines a transport property y from (p,T)
 */
#define define_transport_prop_from_p_T(prop)                                                       \
  Real IdealGasMixtureFluidProperties::prop##_from_p_T(                                            \
      Real p, Real T, const std::vector<Real> & x_secondary) const                                 \
  {                                                                                                \
    const auto x = secondaryToAllMassFractions(x_secondary);                                       \
    const auto psi = molarFractionsFromMassFractions(x);                                           \
    mooseAssert(psi.size() == _n_components, "Size mismatch");                                     \
                                                                                                   \
    Real y = 0;                                                                                    \
    for (const auto i : make_range(_n_components))                                                 \
      y += psi[i] * _component_fps[i]->prop##_from_p_T(p, T);                                      \
                                                                                                   \
    return y;                                                                                      \
  }                                                                                                \
  ADReal IdealGasMixtureFluidProperties::prop##_from_p_T(                                          \
      const ADReal & p, const ADReal & T, const std::vector<ADReal> & x_secondary) const           \
  {                                                                                                \
    const auto x = secondaryToAllMassFractions(x_secondary);                                       \
    const auto psi = molarFractionsFromMassFractions(x);                                           \
    mooseAssert(psi.size() == _n_components, "Size mismatch");                                     \
                                                                                                   \
    ADReal y = 0;                                                                                  \
    for (const auto i : make_range(_n_components))                                                 \
      y += psi[i] * _component_fps[i]->prop##_from_p_T(p, T);                                      \
                                                                                                   \
    return y;                                                                                      \
  }

// clang-format off
define_mass_specific_prop_from_p_T(v)
define_mass_specific_prop_from_p_T(e)
define_mass_specific_prop_from_p_T(s)
define_mass_specific_prop_from_p_T(cp)
define_mass_specific_prop_from_p_T(cv)

define_transport_prop_from_p_T(mu)
define_transport_prop_from_p_T(k)

#undef define_mass_specific_prop_from_p_T
#undef define_transport_prop_from_p_T

InputParameters IdealGasMixtureFluidProperties::validParams()
// clang-format on
{
  InputParameters params = VaporMixtureFluidProperties::validParams();
  params += NaNInterface::validParams();

  params.addClassDescription("Class for fluid properties of an ideal gas mixture");

  params.addRequiredParam<std::vector<UserObjectName>>(
      "component_fluid_properties",
      "Name of component fluid properties user objects. The first entry should be the primary "
      "component.");

  return params;
}

IdealGasMixtureFluidProperties::IdealGasMixtureFluidProperties(const InputParameters & parameters)
  : VaporMixtureFluidProperties(parameters),
    NaNInterface(this),
    _component_fp_names(getParam<std::vector<UserObjectName>>("component_fluid_properties")),
    _n_components(_component_fp_names.size()),
    _n_secondary_components(_n_components - 1)
{
  if (_n_components == 0)
    mooseError("There must be at least one entry in 'component_fluid_properties'.");

  // Newton's method starts above the solution, at the lowest upper end of the temperature ranges
  // of the NASA 7-coefficient components, because their internal energies are convex in temperature
  // over nearly all of these ranges (see NASA7IdealGasFluidProperties). The internal energies of
  // IdealGasFluidProperties components are linear in temperature, so without NASA 7-coefficient
  // components Newton's method converges in one iteration from any start temperature.
  _T_newton_start = std::numeric_limits<Real>::max();

  _component_fps.resize(_n_components);
  for (const auto i : make_range(_n_components))
  {
    const auto & single_phase_fp =
        getUserObjectByName<SinglePhaseFluidProperties>(_component_fp_names[i]);
    const auto nasa7_fp = dynamic_cast<const NASA7IdealGasFluidProperties *>(&single_phase_fp);
    if (nasa7_fp)
      _T_newton_start = std::min(_T_newton_start, nasa7_fp->maximumTemperature());
    else if (!dynamic_cast<const IdealGasFluidProperties *>(&single_phase_fp))
      mooseError("Each entry in 'component_fluid_properties' must have type "
                 "'IdealGasFluidProperties' or 'NASA7IdealGasFluidProperties'.");
    _component_fps[i] = &single_phase_fp;
  }

  // Any start temperature works without NASA 7-coefficient components
  if (_T_newton_start == std::numeric_limits<Real>::max())
    _T_newton_start = 300;
}

const SinglePhaseFluidProperties &
IdealGasMixtureFluidProperties::getPrimaryFluidProperties() const
{
  return *_component_fps[0];
}

const SinglePhaseFluidProperties &
IdealGasMixtureFluidProperties::getSecondaryFluidProperties(unsigned int i) const
{
  mooseAssert(i < getNumberOfSecondaryVapors(), "Requested secondary index too high.");
  return *_component_fps[i + 1];
}

template <typename CppType>
std::vector<CppType>
IdealGasMixtureFluidProperties::secondaryToAllMassFractions_templ(
    const std::vector<CppType> & x_secondary) const
{
  mooseAssert(x_secondary.size() == _n_secondary_components, "Size mismatch");

  CppType sum = 0;
  for (const auto i : make_range(_n_secondary_components))
    sum += x_secondary[i];

  const CppType x_primary = 1.0 - sum;

  std::vector<CppType> x;
  x.push_back(x_primary);
  x.insert(x.end(), x_secondary.begin(), x_secondary.end());

  return x;
}

std::vector<ADReal>
IdealGasMixtureFluidProperties::secondaryToAllMassFractions(
    const std::vector<ADReal> & x_secondary) const
{
  return secondaryToAllMassFractions_templ(x_secondary);
}

std::vector<Real>
IdealGasMixtureFluidProperties::secondaryToAllMassFractions(
    const std::vector<Real> & x_secondary) const
{
  return secondaryToAllMassFractions_templ(x_secondary);
}

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::mixtureMolarMass_templ(const std::vector<CppType> & x) const
{
  mooseAssert(x.size() == _n_components, "Size mismatch");

  CppType sum = 0;
  for (const auto i : make_range(_n_components))
    sum += x[i] / _component_fps[i]->molarMass();

  return 1.0 / sum;
}

ADReal
IdealGasMixtureFluidProperties::mixtureMolarMass(const std::vector<ADReal> & x) const
{
  return mixtureMolarMass_templ(x);
}

Real
IdealGasMixtureFluidProperties::mixtureMolarMass(const std::vector<Real> & x) const
{
  return mixtureMolarMass_templ(x);
}

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::mixtureSpecificHeatRatio_templ(const CppType & p,
                                                               const CppType & T,
                                                               const std::vector<CppType> & x) const
{
  mooseAssert(x.size() == _n_components, "Size mismatch");

  CppType cp_mix = 0, cv_mix = 0;
  for (const auto i : make_range(_n_components))
  {
    cp_mix += x[i] * _component_fps[i]->cp_from_p_T(p, T);
    cv_mix += x[i] * _component_fps[i]->cv_from_p_T(p, T);
  }

  return cp_mix / cv_mix;
}

ADReal
IdealGasMixtureFluidProperties::mixtureSpecificHeatRatio(const ADReal & p,
                                                         const ADReal & T,
                                                         const std::vector<ADReal> & x) const
{
  return mixtureSpecificHeatRatio_templ(p, T, x);
}

Real
IdealGasMixtureFluidProperties::mixtureSpecificHeatRatio(Real p,
                                                         Real T,
                                                         const std::vector<Real> & x) const
{
  return mixtureSpecificHeatRatio_templ(p, T, x);
}

template <typename CppType>
std::vector<CppType>
IdealGasMixtureFluidProperties::molarFractionsFromMassFractions_templ(
    const std::vector<CppType> & x) const
{
  mooseAssert(x.size() == _n_components, "Size mismatch");

  const auto M = mixtureMolarMass(x);
  std::vector<CppType> psi(_n_components);
  for (const auto i : make_range(_n_components))
    psi[i] = x[i] * M / _component_fps[i]->molarMass();

  return psi;
}

std::vector<ADReal>
IdealGasMixtureFluidProperties::molarFractionsFromMassFractions(const std::vector<ADReal> & x) const
{
  return molarFractionsFromMassFractions_templ(x);
}

std::vector<Real>
IdealGasMixtureFluidProperties::molarFractionsFromMassFractions(const std::vector<Real> & x) const
{
  return molarFractionsFromMassFractions_templ(x);
}

ADReal
IdealGasMixtureFluidProperties::rho_from_p_T(const ADReal & p,
                                             const ADReal & T,
                                             const std::vector<ADReal> & x_secondary) const
{
  return 1.0 / v_from_p_T(p, T, x_secondary);
}

Real
IdealGasMixtureFluidProperties::rho_from_p_T(Real p,
                                             Real T,
                                             const std::vector<Real> & x_secondary) const
{
  return 1.0 / v_from_p_T(p, T, x_secondary);
}

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::e_from_p_rho_templ(const CppType & p,
                                                   const CppType & rho,
                                                   const std::vector<CppType> & x_secondary) const
{
  const auto x = secondaryToAllMassFractions(x_secondary);
  const auto T = p * mixtureMolarMass(x) / (_R * rho);
  return e_from_p_T(p, T, x_secondary);
}

ADReal
IdealGasMixtureFluidProperties::e_from_p_rho(const ADReal & p,
                                             const ADReal & rho,
                                             const std::vector<ADReal> & x_secondary) const
{
  return e_from_p_rho_templ(p, rho, x_secondary);
}

Real
IdealGasMixtureFluidProperties::e_from_p_rho(Real p,
                                             Real rho,
                                             const std::vector<Real> & x_secondary) const
{
  return e_from_p_rho_templ(p, rho, x_secondary);
}

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::p_from_v_e_templ(const CppType & v,
                                                 const CppType & e,
                                                 const std::vector<CppType> & x_secondary) const
{
  const auto x = secondaryToAllMassFractions(x_secondary);
  const auto M = mixtureMolarMass(x);

  return _R * T_from_v_e(v, e, x_secondary) / (M * v);
}

ADReal
IdealGasMixtureFluidProperties::p_from_v_e(const ADReal & v,
                                           const ADReal & e,
                                           const std::vector<ADReal> & x_secondary) const
{
  return p_from_v_e_templ(v, e, x_secondary);
}

Real
IdealGasMixtureFluidProperties::p_from_v_e(Real v,
                                           Real e,
                                           const std::vector<Real> & x_secondary) const
{
  return p_from_v_e_templ(v, e, x_secondary);
}

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::T_from_v_e_templ(const CppType & v,
                                                 const CppType & e,
                                                 const std::vector<CppType> & x_secondary) const
{
  const auto x = secondaryToAllMassFractions(x_secondary);
  mooseAssert(x.size() == _n_components, "Size mismatch");
  const auto M = mixtureMolarMass(x);

  // Mixture internal energy and isochoric specific heat at a temperature. The internal energy of
  // an ideal gas does not depend on pressure; the components are evaluated at the mixture pressure
  // for that temperature and specific volume.
  const auto e_cv_from_T = [this, &x, &v, &M](const CppType & T, CppType & e_T, CppType & cv_T)
  {
    const CppType p = _R * T / (M * v);
    e_T = 0;
    cv_T = 0;
    for (const auto i : make_range(_n_components))
    {
      e_T += x[i] * _component_fps[i]->e_from_p_T(p, T);
      cv_T += x[i] * _component_fps[i]->cv_from_p_T(p, T);
    }
  };

  const Real T0 = _T_newton_start;

  // The internal energy can be close to zero, where the convergence test of NewtonSolve, which is
  // relative to the target value, would demand accuracy below roundoff. Solve e(T) + shift =
  // e + shift instead, with the shift cv T - e(T) at T0. Both sides are then close to cv T, and the
  // convergence test bounds the relative error in temperature.
  CppType e0, cv0;
  e_cv_from_T(CppType(T0), e0, cv0);
  const Real shift = MetaPhysicL::raw_value(cv0) * T0 - MetaPhysicL::raw_value(e0);

  const auto shifted_e = [&e_cv_from_T, shift](const CppType & /*e*/,
                                               const CppType & T,
                                               CppType & e_T,
                                               CppType & de_T_de,
                                               CppType & de_T_dT)
  {
    e_cv_from_T(T, e_T, de_T_dT);
    e_T += shift;
    de_T_de = 0;
  };

  // Relative tolerance of the Newton iteration, the default of the corresponding parameter of
  // SinglePhaseFluidProperties. Newton's method applies one more update after it converges, so the
  // temperature is accurate to roundoff.
  const Real tolerance = 1e-8;
  const CppType target = e + shift;
  return FluidPropertiesUtils::NewtonSolve(
             e, target, T0, tolerance, shifted_e, name() + "::T_from_v_e")
      .first;
}

ADReal
IdealGasMixtureFluidProperties::T_from_v_e(const ADReal & v,
                                           const ADReal & e,
                                           const std::vector<ADReal> & x_secondary) const
{
  return T_from_v_e_templ(v, e, x_secondary);
}

Real
IdealGasMixtureFluidProperties::T_from_v_e(Real v,
                                           Real e,
                                           const std::vector<Real> & x_secondary) const
{
  return T_from_v_e_templ(v, e, x_secondary);
}

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::c_from_p_T_templ(const CppType & p,
                                                 const CppType & T,
                                                 const std::vector<CppType> & x_secondary) const
{
  using std::sqrt;

  const auto x = secondaryToAllMassFractions(x_secondary);
  const auto M = mixtureMolarMass(x);
  const auto gamma = mixtureSpecificHeatRatio(p, T, x);

  return sqrt(gamma * _R * T / M);
}

ADReal
IdealGasMixtureFluidProperties::c_from_p_T(const ADReal & p,
                                           const ADReal & T,
                                           const std::vector<ADReal> & x_secondary) const
{
  return c_from_p_T_templ(p, T, x_secondary);
}

Real
IdealGasMixtureFluidProperties::c_from_p_T(Real p,
                                           Real T,
                                           const std::vector<Real> & x_secondary) const
{
  return c_from_p_T_templ(p, T, x_secondary);
}
