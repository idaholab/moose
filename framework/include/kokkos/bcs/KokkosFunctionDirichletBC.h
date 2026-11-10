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

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const
  {
    return _func.value(_t, datum.q_point(qp));
  }

protected:
  /// The function describing the Dirichlet condition
  const Moose::Kokkos::Function _func;

  using Moose::Kokkos::DirichletBCBaseTempl<is_ad>::_t;
};

typedef KokkosFunctionDirichletBCTempl<false> KokkosFunctionDirichletBC;
typedef KokkosFunctionDirichletBCTempl<true> KokkosADFunctionDirichletBC;
