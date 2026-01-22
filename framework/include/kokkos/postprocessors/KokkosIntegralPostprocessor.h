//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosElementPostprocessor.h"
#include "KokkosSidePostprocessor.h"

template <typename Base>
class KokkosIntegralPostprocessor : public Base
{
public:
  static InputParameters validParams();

  KokkosIntegralPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual Real getValue() const override;
  virtual void finalize() override;

  template <typename Derived>
  KOKKOS_FUNCTION void
  executeShim(const Derived & postprocessor, Datum & datum, Real * result) const;

  KOKKOS_FUNCTION void join(typename Base::DefaultLoop, Real * result, const Real * source) const;
  KOKKOS_FUNCTION void init(typename Base::DefaultLoop, Real * result) const;

protected:
  const bool _average;
};

template <typename Base>
template <typename Derived>
KOKKOS_FUNCTION void
KokkosIntegralPostprocessor<Base>::executeShim(const Derived & postprocessor,
                                               Datum & datum,
                                               Real * result) const
{
  Real sum = 0;
  Real vol = 0;

  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
  {
    datum.reinit();

    sum += datum.JxW(qp) * postprocessor.computeQpIntegral(qp, datum);
    vol += datum.JxW(qp);
  }

  result[0] += sum;

  if (_average)
    result[1] += vol;
}

template <typename Base>
KOKKOS_FUNCTION void
KokkosIntegralPostprocessor<Base>::join(typename Base::DefaultLoop,
                                        Real * result,
                                        const Real * source) const
{
  result[0] += source[0];

  if (_average)
    result[1] += source[1];
}

template <typename Base>
KOKKOS_FUNCTION void
KokkosIntegralPostprocessor<Base>::init(typename Base::DefaultLoop, Real * result) const
{
  result[0] = 0;

  if (_average)
    result[1] = 0;
}

typedef KokkosIntegralPostprocessor<Moose::Kokkos::ElementPostprocessor>
    KokkosElementIntegralPostprocessor;
typedef KokkosIntegralPostprocessor<Moose::Kokkos::SidePostprocessor>
    KokkosSideIntegralPostprocessor;
