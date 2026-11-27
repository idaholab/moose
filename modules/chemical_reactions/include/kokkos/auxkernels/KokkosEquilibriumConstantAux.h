//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAuxKernel.h"

/**
 * Kokkos equilibrium constant log10(Keq) of an equilibrium species as a function of temperature.
 * The fit coefficients are generated on the host and evaluated on the device.
 */
class KokkosEquilibriumConstantAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  KokkosEquilibriumConstantAux(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Form of the log10(Keq) fit
  enum class FitType
  {
    CONSTANT,
    LINEAR,
    MAIER_KELLEY
  };

  /// Temperature
  const Moose::Kokkos::VariableValue _temperature;
  /// Form of the fit
  FitType _fit_type;
  /// Fit coefficients
  Moose::Kokkos::Array<Real> _coeffs;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosEquilibriumConstantAux::computeValue(const unsigned int qp, AssemblyDatum & datum) const
{
  const Real T = _temperature(datum, qp);

  switch (_fit_type)
  {
    case FitType::CONSTANT:
      return -_coeffs[0];
    case FitType::LINEAR:
      return -(_coeffs[0] + _coeffs[1] * T);
    default:
      // Maier-Kelley form of EquilibriumConstantFit
      return -(_coeffs[0] * ::Kokkos::log(T) + _coeffs[1] + _coeffs[2] * T + _coeffs[3] / T +
               _coeffs[4] / T / T);
  }
}
