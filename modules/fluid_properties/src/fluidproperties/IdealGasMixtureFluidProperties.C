//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasMixtureFluidProperties.h"
#include "IdealGasMixtureInterface.h"
#include "SinglePhaseFluidProperties.h"
#include "NewtonInversion.h"

#include "libmesh/utility.h"

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

// clang-format off
define_mass_specific_prop_from_p_T(v)
define_mass_specific_prop_from_p_T(e)
define_mass_specific_prop_from_p_T(cp)
define_mass_specific_prop_from_p_T(cv)

#undef define_mass_specific_prop_from_p_T

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

  MooseEnum transport_mixing_rule("mole_fraction_average wilke", "mole_fraction_average");
  params.addParam<MooseEnum>(
      "transport_mixing_rule",
      transport_mixing_rule,
      "Rule that combines the viscosities and thermal conductivities of the components: their "
      "average weighted by molar fraction, or the rule of Wilke for the viscosity and that of "
      "Mason and Saxena for the thermal conductivity");

  return params;
}

IdealGasMixtureFluidProperties::IdealGasMixtureFluidProperties(const InputParameters & parameters)
  : VaporMixtureFluidProperties(parameters),
    NaNInterface(this),
    _component_fp_names(getParam<std::vector<UserObjectName>>("component_fluid_properties")),
    _n_components(_component_fp_names.size()),
    _n_secondary_components(_n_components - 1),
    _constant_specific_heats(true),
    _T_newton_start(std::numeric_limits<Real>::max()),
    _transport_mixing_rule(
        getParam<MooseEnum>("transport_mixing_rule").getEnum<TransportMixingRule>())
{
  if (_n_components == 0)
    mooseError("There must be at least one entry in 'component_fluid_properties'.");

  // With components whose specific heats depend on temperature, Newton's method starts at the
  // lowest of their start temperatures (see IdealGasMixtureInterface)
  _component_fps.resize(_n_components);
  for (const auto i : make_range(_n_components))
  {
    const auto & single_phase_fp =
        getUserObjectByName<SinglePhaseFluidProperties>(_component_fp_names[i]);
    const auto ideal_gas_fp = dynamic_cast<const IdealGasMixtureInterface *>(&single_phase_fp);
    if (!ideal_gas_fp)
      mooseError("Each entry in 'component_fluid_properties' must be an ideal gas that implements "
                 "IdealGasMixtureInterface, such as 'IdealGasFluidProperties' or "
                 "'NASA7IdealGasFluidProperties'.");
    if (!ideal_gas_fp->hasConstantSpecificHeats())
    {
      _constant_specific_heats = false;
      _T_newton_start = std::min(_T_newton_start, ideal_gas_fp->newtonStartTemperature());
    }
    _component_fps[i] = &single_phase_fp;
  }

  if (_transport_mixing_rule == TransportMixingRule::WILKE)
  {
    _wilke_molar_mass_factor.assign(_n_components, std::vector<Real>(_n_components));
    _wilke_normalization.assign(_n_components, std::vector<Real>(_n_components));
    for (const auto i : make_range(_n_components))
      for (const auto j : make_range(_n_components))
      {
        const Real M_i = _component_fps[i]->molarMass();
        const Real M_j = _component_fps[j]->molarMass();
        _wilke_molar_mass_factor[i][j] = std::pow(M_j / M_i, 0.25);
        _wilke_normalization[i][j] = 1 / std::sqrt(8 * (1 + M_i / M_j));
      }
  }
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
CppType
IdealGasMixtureFluidProperties::constantSpecificHeatRatio(const std::vector<CppType> & x) const
{
  mooseDoOnce(mooseDeprecated("mixtureSpecificHeatRatio(x) is deprecated; use "
                              "mixtureSpecificHeatRatio(p, T, x) instead."));
  if (!_constant_specific_heats)
    mooseError("mixtureSpecificHeatRatio(x) requires components with constant specific heats; use "
               "mixtureSpecificHeatRatio(p, T, x) instead.");

  // The specific heats are constant, so any pressure and temperature give the same ratio
  return mixtureSpecificHeatRatio_templ(CppType(0), CppType(0), x);
}

ADReal
IdealGasMixtureFluidProperties::mixtureSpecificHeatRatio(const std::vector<ADReal> & x) const
{
  return constantSpecificHeatRatio(x);
}

Real
IdealGasMixtureFluidProperties::mixtureSpecificHeatRatio(const std::vector<Real> & x) const
{
  return constantSpecificHeatRatio(x);
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

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::s_from_p_T_templ(const CppType & p,
                                                 const CppType & T,
                                                 const std::vector<CppType> & x_secondary) const
{
  const auto x = secondaryToAllMassFractions(x_secondary);
  const auto psi = molarFractionsFromMassFractions(x);
  mooseAssert(x.size() == _n_components, "Size mismatch");

  // Gibbs-Dalton: each component contributes its entropy at its partial pressure
  CppType s = 0;
  for (const auto i : make_range(_n_components))
    s += x[i] * _component_fps[i]->s_from_p_T(psi[i] * p, T);

  return s;
}

ADReal
IdealGasMixtureFluidProperties::s_from_p_T(const ADReal & p,
                                           const ADReal & T,
                                           const std::vector<ADReal> & x_secondary) const
{
  return s_from_p_T_templ(p, T, x_secondary);
}

Real
IdealGasMixtureFluidProperties::s_from_p_T(Real p,
                                           Real T,
                                           const std::vector<Real> & x_secondary) const
{
  return s_from_p_T_templ(p, T, x_secondary);
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
IdealGasMixtureFluidProperties::T_from_v_e_templ(const CppType & /*v*/,
                                                 const CppType & e,
                                                 const std::vector<CppType> & x_secondary) const
{
  const auto x = secondaryToAllMassFractions(x_secondary);
  mooseAssert(x.size() == _n_components, "Size mismatch");

  // The internal energy and specific heats of an ideal gas do not depend on pressure, so the
  // components are evaluated at zero pressure
  if (_constant_specific_heats)
  {
    // The internal energy of each component is linear in temperature, e_k(T) = e_k(0) + cv_k T, so
    // the temperature follows explicitly
    CppType e0_sum = 0;
    CppType cv_sum = 0;
    for (const auto i : make_range(_n_components))
    {
      e0_sum += x[i] * _component_fps[i]->e_from_p_T(0.0, 0.0);
      cv_sum += x[i] * _component_fps[i]->cv_from_p_T(0.0, 0.0);
    }
    return (e - e0_sum) / cv_sum;
  }

  // Mixture internal energy and isochoric specific heat at a temperature
  const auto e_cv_from_T = [this, &x](const CppType & T, CppType & e_T, CppType & cv_T)
  {
    const CppType p = 0;
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

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::mixTransportProperty(const std::vector<CppType> & psi,
                                                     const std::vector<CppType> & y,
                                                     const std::vector<CppType> & mu) const
{
  mooseAssert(psi.size() == _n_components && y.size() == _n_components, "Size mismatch");

  CppType y_mix = 0;
  if (_transport_mixing_rule == TransportMixingRule::MOLE_FRACTION_AVERAGE)
  {
    for (const auto i : make_range(_n_components))
      y_mix += psi[i] * y[i];
    return y_mix;
  }

  // y_mix = sum_i psi_i y_i / (sum_j psi_j phi_ij), with the interaction factor of Wilke
  // phi_ij = [1 + (mu_i / mu_j)^(1/2) (M_j / M_i)^(1/4)]^2 / [8 (1 + M_i / M_j)]^(1/2)
  // (Poling et al., 2001). The rule of Mason and Saxena for the thermal conductivity has the same
  // factors when its ratio of translational thermal conductivities is written with viscosities,
  // mu_i M_j / (mu_j M_i), and its empirical constant is one.
  using std::sqrt;
  mooseAssert(mu.size() == _n_components, "Size mismatch");
  for (const auto i : make_range(_n_components))
  {
    CppType denominator = 0;
    for (const auto j : make_range(_n_components))
    {
      const CppType phi_ij =
          Utility::pow<2>(1 + sqrt(mu[i] / mu[j]) * _wilke_molar_mass_factor[i][j]) *
          _wilke_normalization[i][j];
      denominator += psi[j] * phi_ij;
    }
    y_mix += psi[i] * y[i] / denominator;
  }
  return y_mix;
}

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::mu_from_p_T_templ(const CppType & p,
                                                  const CppType & T,
                                                  const std::vector<CppType> & x_secondary) const
{
  const auto psi = molarFractionsFromMassFractions(secondaryToAllMassFractions(x_secondary));
  std::vector<CppType> mu(_n_components);
  for (const auto i : make_range(_n_components))
    mu[i] = _component_fps[i]->mu_from_p_T(p, T);
  return mixTransportProperty(psi, mu, mu);
}

ADReal
IdealGasMixtureFluidProperties::mu_from_p_T(const ADReal & p,
                                            const ADReal & T,
                                            const std::vector<ADReal> & x_secondary) const
{
  return mu_from_p_T_templ(p, T, x_secondary);
}

Real
IdealGasMixtureFluidProperties::mu_from_p_T(Real p,
                                            Real T,
                                            const std::vector<Real> & x_secondary) const
{
  return mu_from_p_T_templ(p, T, x_secondary);
}

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::k_from_p_T_templ(const CppType & p,
                                                 const CppType & T,
                                                 const std::vector<CppType> & x_secondary) const
{
  const auto psi = molarFractionsFromMassFractions(secondaryToAllMassFractions(x_secondary));
  std::vector<CppType> k(_n_components);
  for (const auto i : make_range(_n_components))
    k[i] = _component_fps[i]->k_from_p_T(p, T);

  // The rule of Mason and Saxena needs the viscosities of the components
  std::vector<CppType> mu;
  if (_transport_mixing_rule == TransportMixingRule::WILKE)
  {
    mu.resize(_n_components);
    for (const auto i : make_range(_n_components))
      mu[i] = _component_fps[i]->mu_from_p_T(p, T);
  }
  return mixTransportProperty(psi, k, mu);
}

ADReal
IdealGasMixtureFluidProperties::k_from_p_T(const ADReal & p,
                                           const ADReal & T,
                                           const std::vector<ADReal> & x_secondary) const
{
  return k_from_p_T_templ(p, T, x_secondary);
}

Real
IdealGasMixtureFluidProperties::k_from_p_T(Real p,
                                           Real T,
                                           const std::vector<Real> & x_secondary) const
{
  return k_from_p_T_templ(p, T, x_secondary);
}
