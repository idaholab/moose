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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Ideal gas whose heat capacity, enthalpy, and entropy depend on temperature through NASA
 * 7-coefficient polynomials. The enthalpy includes the enthalpy of formation, and the entropy is
 * the absolute entropy.
 */
class NASA7IdealGasFluidProperties : public SinglePhaseFluidProperties
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
  propfuncWithDefinitionOverride(p, T, v);
  propfuncWithDefinitionOverride(e, T, v);
  propfuncWithDefinitionOverride(h, T, v);
  propfuncWithDefinitionOverride(s, T, v);
  propfuncWithDefinitionOverride(cv, T, v);
  propfuncWithDefinitionOverride(e, p, rho);

  std::string fluidName() const override;
  Real molarMass() const override;

private:
  /// NASA 7-coefficient data of the species
  const NASA7::SpeciesData _data;
  /// Specific gas constant [J/(kg K)]
  const Real _R_specific;

  /**
   * Flags a temperature outside the range of the data as a solution warning, because the
   * polynomials are then extrapolated, and a non-positive temperature as an invalid solution
   */
  template <typename CppType>
  void checkTemperature(const CppType & T) const;

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
  ///@}
};

#pragma GCC diagnostic pop
