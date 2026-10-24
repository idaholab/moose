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
#include "GasTransport.h"

class SinglePhaseFluidProperties;

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
   * @param[in] p   Pressure
   * @param[in] T   Temperature
   * @param[in] x   All mass fractions
   */
  template <typename CppType>
  CppType mixtureSpecificHeatRatio_templ(const CppType & p,
                                         const CppType & T,
                                         const std::vector<CppType> & x) const;
  ADReal
  mixtureSpecificHeatRatio(const ADReal & p, const ADReal & T, const std::vector<ADReal> & x) const;
  Real mixtureSpecificHeatRatio(Real p, Real T, const std::vector<Real> & x) const;

  /**
   * Computes the specific heat ratio of a mixture whose components all have constant specific
   * heats. Deprecated: use the version with pressure and temperature arguments.
   *
   * @param[in] x   All mass fractions
   */
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
  override_property(rho, p, T);
  override_property(e, p, T);
  override_property(s, p, T);
  override_property(c, p, T);
  override_property(cp, p, T);
  override_property(cv, p, T);
  override_property(mu, p, T);
  override_property(k, p, T);
  override_property(e, p, rho);

#undef override_property

  /**
   * Computes the Chapman-Enskog binary diffusion coefficient of a pair of components, which must
   * both have Lennard-Jones parameters (see IdealGasMixtureInterface) [m^2/s]
   */
  virtual Real
  binaryDiffusionCoefficient(Real p, Real T, unsigned int i, unsigned int j) const override;
  virtual ADReal binaryDiffusionCoefficient(const ADReal & p,
                                            const ADReal & T,
                                            unsigned int i,
                                            unsigned int j) const override;

protected:
  /// Names of component fluid properties
  const std::vector<UserObjectName> _component_fp_names;
  /// Number of components
  const unsigned int _n_components;
  /// Number of secondary components
  const unsigned int _n_secondary_components;

  /// Component fluid properties objects
  std::vector<const SinglePhaseFluidProperties *> _component_fps;
  /// Lennard-Jones parameters of the components, or nullptr for components without them
  std::vector<const GasTransport::LennardJonesParameters *> _lennard_jones;

  /// Whether all components have constant specific heats, so that the internal energy is linear
  /// in temperature and the temperature follows explicitly from it
  bool _constant_specific_heats;
  /// Temperature at which Newton's method starts to compute the temperature from the internal
  /// energy when some components have temperature-dependent specific heats [K]
  Real _T_newton_start;

  /// Rules that combine the viscosities and thermal conductivities of the components
  enum class TransportMixingRule
  {
    MOLE_FRACTION_AVERAGE,
    WILKE
  };
  /// Rule that combines the viscosities and thermal conductivities of the components
  const TransportMixingRule _transport_mixing_rule;
  ///@{ Factors (M_j / M_i)^(1/4) and [8 (1 + M_i / M_j)]^(-1/2) of the interaction factors of
  /// Wilke, which depend on the molar masses M_i and M_j of components i and j only
  std::vector<std::vector<Real>> _wilke_molar_mass_factor;
  std::vector<std::vector<Real>> _wilke_normalization;
  ///@}

private:
  /**
   * Combines a transport property of the components with the transport mixing rule. The option
   * wilke weights the component values with the interaction factors of Wilke, computed from the
   * component viscosities and molar masses, which gives the rule of Wilke for the viscosity and
   * that of Mason and Saxena for the thermal conductivity.
   *
   * @param[in] psi   Molar fractions of the components
   * @param[in] y     Transport property of the components
   * @param[in] mu    Viscosities of the components, used by the rule of Wilke only
   */
  template <typename CppType>
  CppType mixTransportProperty(const std::vector<CppType> & psi,
                               const std::vector<CppType> & y,
                               const std::vector<CppType> & mu) const;

  /// Implements binaryDiffusionCoefficient for Real and AD arguments
  template <typename CppType>
  CppType binaryDiffusionCoefficient_templ(const CppType & p,
                                           const CppType & T,
                                           unsigned int i,
                                           unsigned int j) const;

  /// Implements the deprecated mixtureSpecificHeatRatio(x)
  template <typename CppType>
  CppType constantSpecificHeatRatio(const std::vector<CppType> & x) const;
};
