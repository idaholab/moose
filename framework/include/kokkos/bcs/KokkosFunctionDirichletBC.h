//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDirichletBCBase.h"
#include "KokkosFunction.h"

/**
 * Imposes the essential boundary condition u = g(t, x), where g is a Kokkos function.
 */
template <bool is_ad>
class KokkosFunctionDirichletBCTempl : public Moose::Kokkos::DirichletBCBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  KokkosFunctionDirichletBCTempl(const InputParameters & parameters);

  /// The AD BC depends on the nodal value of its own variable only
  static constexpr unsigned int max_ad_slots = is_ad ? 1 : 0;

  template <typename Derived, typename DatumType>
  KOKKOS_FUNCTION auto computeValue(const unsigned int qp, DatumType & datum) const
  {
    const Real value = _func.value(_t, datum.q_point(qp));

    if constexpr (is_ad)
      return Moose::Kokkos::DenseADReal<max_ad_slots>(value);
    else
      return value;
  }

  /// Get the AD accessor of the nodal value of the BC variable
  const auto & nodalValue() const { return _nodal_value; }

protected:
  /// Placeholder for the AD nodal value accessor, which the non-AD BC replaces with _u
  struct NoNodalValue
  {
  };

  /// Create the nodal value accessor of the BC variable
  auto makeNodalValue()
  {
    if constexpr (is_ad)
      return this->adNodalValue();
    else
      return NoNodalValue();
  }

  /// The function describing the Dirichlet condition
  const Moose::Kokkos::Function _func;

  /// Nodal value of the BC variable, with its derivative, read by the AD BC
  const std::conditional_t<is_ad, Moose::Kokkos::ADVariableValue, NoNodalValue> _nodal_value;

  using Moose::Kokkos::DirichletBCBaseTempl<is_ad>::_t;
};

typedef KokkosFunctionDirichletBCTempl<false> KokkosFunctionDirichletBC;
typedef KokkosFunctionDirichletBCTempl<true> KokkosADFunctionDirichletBC;
