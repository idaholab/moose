//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VaporMixtureFluidProperties.h"
#include "NaNInterface.h"
#include "IdealGasFluidProperties.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

class SinglePhaseFluidProperties;
class IdealGasFluidProperties;

/**
 * Overrides the value methods for a property from some arguments
 */
#define override_property(want, prop1, prop2)                                                      \
  template <typename CppType>                                                                      \
  CppType want##_from_##prop1##_##prop2##_templ(                                                   \
      const CppType & prop1, const CppType & prop2, const std::vector<CppType> & x) const;         \
  virtual Real want##_from_##prop1##_##prop2(Real prop1, Real prop2, const std::vector<Real> & x)  \
      const override;                                                                              \
  virtual ADReal want##_from_##prop1##_##prop2(                                                    \
      const ADReal & prop1, const ADReal & prop2, const std::vector<ADReal> & x) const override

/**
 * Class for fluid properties of an ideal gas mixture
 */
class IdealGasMixtureFluidProperties : public VaporMixtureFluidProperties, public NaNInterface
{
public:
  static InputParameters validParams();

  IdealGasMixtureFluidProperties(const InputParameters & parameters);

  usingVaporMixtureFluidPropertiesMembers;

  virtual unsigned int numberOfComponents() const override { return _n_components; }

  virtual const SinglePhaseFluidProperties & getPrimaryFluidProperties() const override;
  virtual const SinglePhaseFluidProperties &
  getSecondaryFluidProperties(unsigned int i = 0) const override;

  /**
   * Computes all mass fractions
   *
   * @param[in] x_secondary   Secondary mass fractions
   */
  template <typename CppType>
  std::vector<CppType>
  secondaryToAllMassFractions_templ(const std::vector<CppType> & x_secondary) const;
  std::vector<ADReal> secondaryToAllMassFractions(const std::vector<ADReal> & x_secondary) const;
  std::vector<Real> secondaryToAllMassFractions(const std::vector<Real> & x_secondary) const;

  /**
   * Computes the mixture molar mass
   *
   * @param[in] x   All mass fractions
   */
  template <typename CppType>
  CppType mixtureMolarMass_templ(const std::vector<CppType> & x) const;
  ADReal mixtureMolarMass(const std::vector<ADReal> & x) const;
  Real mixtureMolarMass(const std::vector<Real> & x) const;

  /**
   * Computes molar fractions
   *
   * @param[in] x   All mass fractions
   */
  template <typename CppType>
  std::vector<CppType> molarFractionsFromMassFractions_templ(const std::vector<CppType> & x) const;
  std::vector<ADReal> molarFractionsFromMassFractions(const std::vector<ADReal> & x) const;
  std::vector<Real> molarFractionsFromMassFractions(const std::vector<Real> & x) const;

  override_property(p, v, e);
  override_property(T, v, e);
  override_property(v, p, T);
  override_property(e, p, T);
  override_property(s, p, T);
  override_property(c, p, T);
  override_property(cp, p, T);
  override_property(cv, p, T);
  override_property(mu, p, T);
  override_property(k, p, T);

#undef override_property

protected:
  /// Names of component fluid properties
  const std::vector<UserObjectName> _component_fp_names;
  /// Number of components
  const unsigned int _n_components;
  /// Number of secondary components
  const unsigned int _n_secondary_components;

  /// Component fluid properties objects
  std::vector<const IdealGasFluidProperties *> _component_fps;
};

#pragma GCC diagnostic pop

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

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::T_from_v_e_templ(const CppType & /*v*/,
                                                 const CppType & e,
                                                 const std::vector<CppType> & x_secondary) const
{
  const auto x = secondaryToAllMassFractions(x_secondary);
  mooseAssert(x.size() == _n_components, "Size mismatch");

  CppType e_ref_sum = 0;
  CppType cv_sum = 0;
  for (const auto i : make_range(_n_components))
  {
    e_ref_sum += x[i] * _component_fps[i]->referenceSpecificInternalEnergy();
    cv_sum += x[i] * _component_fps[i]->cv_from_p_T(0, 0);
  }

  return (e - e_ref_sum) / cv_sum;
}

template <typename CppType>
CppType
IdealGasMixtureFluidProperties::c_from_p_T_templ(const CppType & /*p*/,
                                                 const CppType & T,
                                                 const std::vector<CppType> & x_secondary) const
{
  const auto x = secondaryToAllMassFractions(x_secondary);
  const auto M = mixtureMolarMass(x);

  return std::sqrt(_R * T / M);
}
