//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosSideVariablePostprocessor.h"
#include "KokkosExtremeValueBase.h"

class KokkosSideExtremeValue : public KokkosExtremeValueBase<KokkosSideVariablePostprocessor>
{
public:
  static InputParameters validParams();

  KokkosSideExtremeValue(const InputParameters & parameters);

  KOKKOS_FUNCTION Kokkos::pair<Real, Real> getProxyValuePair(const unsigned int qp,
                                                             Datum & datum) const;

  template <typename Derived>
  KOKKOS_FUNCTION void
  executeShim(const Derived & postprocessor, Datum & datum, Real * result) const;

protected:
  const Moose::Kokkos::VariableValue _proxy_variable;
};

KOKKOS_FUNCTION inline Kokkos::pair<Real, Real>
KokkosSideExtremeValue::getProxyValuePair(const unsigned int qp, Datum & datum) const
{
  return Kokkos::make_pair(_proxy_variable(datum, qp), _u(datum, qp));
}

template <typename Derived>
KOKKOS_FUNCTION void
KokkosSideExtremeValue::executeShim(const Derived & postprocessor,
                                    Datum & datum,
                                    Real * result) const
{
  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    postprocessor.computeExtremeValue(postprocessor, qp, datum, result);
}
