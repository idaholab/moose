//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SinglePhaseFluidProperties.h"
#include "NASA7Polynomials.h"
#include "IdealGasMixtureInterface.h"
#include "GasTransport.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Ideal gas whose heat capacity, enthalpy, and entropy depend on temperature through NASA
 * 7-coefficient polynomials. The enthalpy includes the enthalpy of formation, and the entropy is
 * the absolute entropy.
 */
class NASA7IdealGasFluidProperties : public SinglePhaseFluidProperties,
                                     public IdealGasMixtureInterface
{
public:
  static InputParameters validParams();

  NASA7IdealGasFluidProperties(const InputParameters & parameters);

  propfuncWithDefinitionOverride(rho, p, T);
  propfuncWithDefinitionOverride(v, p, T);
  propfuncWithDefinitionOverride(e, p, T);
  propfuncWithDefinitionOverride(h, p, T);
  propfuncWithDefinitionOverride(s, p, T);
  propfuncWithDefinitionOverride(cp, p, T);
  propfuncWithDefinitionOverride(cv, p, T);
  propfuncWithDefinitionOverride(c, p, T);
  propfuncWithDefinitionOverride(gamma, p, T);
  propfuncWithDefinitionOverride(mu, p, T);
  propfuncWithDefinitionOverride(k, p, T);
  propfuncWithDefinitionOverride(p, T, v);
  propfuncWithDefinitionOverride(e, T, v);
  propfuncWithDefinitionOverride(h, T, v);
  propfuncWithDefinitionOverride(s, T, v);
  propfuncWithDefinitionOverride(cv, T, v);
  propfuncWithDefinitionOverride(e, p, rho);
  propfuncWithDefinitionOverride(T, v, e);
  propfuncWithDefinitionOverride(p, v, e);
  propfuncWithDefinitionOverride(c, v, e);
  propfuncWithDefinitionOverride(cp, v, e);
  propfuncWithDefinitionOverride(cv, v, e);
  propfuncWithDefinitionOverride(gamma, v, e);
  propfuncWithDefinitionOverride(s, v, e);
  propfuncWithDefinitionOverride(g, v, e);
  propfuncWithDefinitionOverride(mu, v, e);
  propfuncWithDefinitionOverride(k, v, e);
  propfuncWithDefinitionOverride(T, p, h);
  propfuncWithDefinitionOverride(s, h, p);
  propfuncWithDefinitionOverride(p, h, s);
  propfuncWithDefinitionOverride(e, v, h);

  std::string fluidName() const override;
  Real molarMass() const override;

  virtual bool hasConstantSpecificHeats() const override { return false; }
  /// The upper end of the temperature range of the NASA 7-coefficient data [K]
  virtual Real newtonStartTemperature() const override;

  virtual const GasTransport::LennardJonesParameters * lennardJonesParameters() const override;

private:
  /// NASA 7-coefficient data of the species
  const NASA7::SpeciesData _data;
  /// Specific gas constant [J/(kg K)]
  const Real _R_specific;
  /// Lennard-Jones parameters of the species
  const GasTransport::LennardJonesParameters _lennard_jones;
  /// Whether the species is steam, whose viscosity and thermal conductivity come from IAPWS
  const bool _is_steam;

  /**
   * Flags a temperature outside the range of the data as a solution warning, because the
   * polynomials are then extrapolated, and a non-positive or not-a-number temperature as an
   * invalid solution
   */
  template <typename CppType>
  void checkTemperature(const CppType & T) const;

  /**
   * Checks the temperature as checkTemperature does and, for steam, also flags a temperature
   * outside the range of validity of the IAPWS transport formulations as a solution warning
   */
  template <typename CppType>
  void checkTransportTemperature(const CppType & T) const;

  ///@{ Properties that depend on temperature only
  template <typename CppType>
  CppType e_from_T(const CppType & T) const;
  template <typename CppType>
  CppType h_from_T(const CppType & T) const;
  template <typename CppType>
  CppType cp_from_T(const CppType & T) const;
  template <typename CppType>
  CppType cv_from_T(const CppType & T) const;
  template <typename CppType>
  CppType gamma_from_T(const CppType & T) const;
  template <typename CppType>
  CppType mu_from_T(const CppType & T) const;
  template <typename CppType>
  CppType k_from_T(const CppType & T) const;
  ///@}

  /**
   * Temperature at which R_s T (h/(RT) - k) equals y, by Newton's method: k = 1 for the specific
   * internal energy and k = 0 for the specific enthalpy. The temperature range is not checked.
   */
  template <typename CppType>
  CppType T_from_energy(const CppType & y, const Real k) const;
};

#pragma GCC diagnostic pop
