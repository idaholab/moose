//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosSideIntegralPostprocessor.h"

/**
 * Kokkos postprocessor computing the total convective heat transfer across a boundary
 */
class KokkosConvectiveHeatTransferSideIntegral : public KokkosSideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  KokkosConvectiveHeatTransferSideIntegral(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpIntegral(const unsigned int qp, Datum & datum) const;

private:
  /// Wall temperature
  const Moose::Kokkos::VariableValue _T_wall;
  /// Whether the fluid temperature is a variable instead of a material property
  const bool _T_fluid_is_var;
  /// Fluid temperature variable
  const Moose::Kokkos::VariableValue _T_fluid;
  /// Fluid temperature material property
  const Moose::Kokkos::MaterialProperty<Real> _T_fluid_mat;
  /// Whether the heat transfer coefficient is a variable instead of a material property
  const bool _hw_is_var;
  /// Heat transfer coefficient variable
  const Moose::Kokkos::VariableValue _hw;
  /// Heat transfer coefficient material property
  const Moose::Kokkos::MaterialProperty<Real> _hw_mat;
};

KOKKOS_FUNCTION inline Real
KokkosConvectiveHeatTransferSideIntegral::computeQpIntegral(const unsigned int qp,
                                                            Datum & datum) const
{
  const Real hw = _hw_is_var ? _hw(datum, qp) : _hw_mat(datum, qp);
  const Real Tf = _T_fluid_is_var ? _T_fluid(datum, qp) : _T_fluid_mat(datum, qp);

  return hw * (_T_wall(datum, qp) - Tf);
}
