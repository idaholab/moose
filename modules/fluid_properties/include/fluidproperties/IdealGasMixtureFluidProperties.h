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
   * Computes the mixture specific heat ratio
   *
   * @param[in] x   All mass fractions
   */
  template <typename CppType>
  CppType mixtureSpecificHeatRatio_templ(const std::vector<CppType> & x) const;
  ADReal mixtureSpecificHeatRatio(const std::vector<ADReal> & x) const;
  Real mixtureSpecificHeatRatio(const std::vector<Real> & x) const;

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
