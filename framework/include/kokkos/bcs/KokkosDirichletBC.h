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

template <bool is_ad>
class KokkosDirichletBCTempl : public Moose::Kokkos::DirichletBCBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  KokkosDirichletBCTempl(const InputParameters & parameters);

  /// The AD BC depends on the nodal value of its own variable only
  static constexpr unsigned int max_ad_slots = is_ad ? 1 : 0;

  template <typename Derived>
  KOKKOS_FUNCTION auto computeValue(const unsigned int /* qp */, AssemblyDatum & /* datum */) const
  {
    if constexpr (is_ad)
      return Moose::Kokkos::DenseADReal<max_ad_slots>(_value);
    else
      return Real(_value);
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
      return this->template adNodalValue<max_ad_slots>();
    else
      return NoNodalValue();
  }

  const Moose::Kokkos::Scalar<const Real> _value;

  /// Nodal value of the BC variable, with its derivative, read by the AD BC
  const std::conditional_t<is_ad, Moose::Kokkos::ADVariableValue<max_ad_slots>, NoNodalValue>
      _nodal_value;
};

typedef KokkosDirichletBCTempl<false> KokkosDirichletBC;
typedef KokkosDirichletBCTempl<true> KokkosADDirichletBC;
