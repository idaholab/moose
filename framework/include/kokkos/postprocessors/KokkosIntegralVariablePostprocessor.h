//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosElementIntegralPostprocessor.h"
#include "KokkosSideIntegralPostprocessor.h"

template <typename Base>
class KokkosIntegralVariablePostprocessor : public Base, public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  KokkosIntegralVariablePostprocessor(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpIntegral(const unsigned int qp, Datum & datum) const;

protected:
  /// Holds the solution at current quadrature points
  Moose::Kokkos::VariableValue _u;
  /// Holds the solution gradient at the current quadrature points
  Moose::Kokkos::VariableGradient _grad_u;
  /// Option to use absolute variable value
  const bool _use_abs_value;
};

template <typename Base>
KOKKOS_FUNCTION inline Real
KokkosIntegralVariablePostprocessor<Base>::computeQpIntegral(const unsigned int qp,
                                                             Datum & datum) const
{
  if (_use_abs_value)
    return Kokkos::abs(_u(datum, qp));
  else
    return _u(datum, qp);
}

typedef KokkosIntegralVariablePostprocessor<KokkosElementIntegralPostprocessor>
    KokkosElementIntegralVariablePostprocessor;
typedef KokkosIntegralVariablePostprocessor<KokkosSideIntegralPostprocessor>
    KokkosSideIntegralVariablePostprocessor;
