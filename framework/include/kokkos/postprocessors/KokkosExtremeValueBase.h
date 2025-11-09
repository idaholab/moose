//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDatum.h"

class UserObject;
class InputParameters;

template <typename Base>
class KokkosExtremeValueBase : public Base
{
public:
  static InputParameters validParams();

  KokkosExtremeValueBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual Real getValue() const override;

  template <typename Derived>
  KOKKOS_FUNCTION void computeExtremeValue(const Derived & postprocessor,
                                           const unsigned int qp,
                                           Datum & datum,
                                           Real * result) const;
  KOKKOS_FUNCTION void join(typename Base::DefaultLoop, Real * result, const Real * source) const;
  KOKKOS_FUNCTION void init(typename Base::DefaultLoop, Real * result) const;

protected:
  /**
   * Type of extreme value we are going to compute
   */
  enum class ExtremeType
  {
    MAX,
    MIN,
    MAX_ABS
  } _type;
};

template <typename Base>
template <typename Derived>
KOKKOS_FUNCTION void
KokkosExtremeValueBase<Base>::computeExtremeValue(const Derived & postprocessor,
                                                  const unsigned int qp,
                                                  Datum & datum,
                                                  Real * result) const
{
  auto pv = postprocessor.getProxyValuePair(qp, datum);
  auto rpv = Kokkos::make_pair(result[0], result[1]);

  if ((_type == ExtremeType::MAX && pv > rpv) || (_type == ExtremeType::MIN && pv < rpv))
  {
    result[0] = pv.first;
    result[1] = pv.second;
  }
  else if (_type == ExtremeType::MAX_ABS && Kokkos::abs(pv.first) > rpv.first)
  {
    result[0] = Kokkos::abs(pv.first);
    result[1] = pv.second;
  }
}

template <typename Base>
KOKKOS_FUNCTION void
KokkosExtremeValueBase<Base>::join(typename Base::DefaultLoop,
                                   Real * result,
                                   const Real * source) const
{
  auto rpv = Kokkos::make_pair(result[0], result[1]);
  auto spv = Kokkos::make_pair(source[0], source[1]);

  if ((_type == ExtremeType::MAX && spv > rpv) || (_type == ExtremeType::MIN && spv < rpv))
  {
    result[0] = source[0];
    result[1] = source[1];
  }
  else if (_type == ExtremeType::MAX_ABS && Kokkos::abs(spv.first) > rpv.first)
  {
    result[0] = Kokkos::abs(source[0]);
    result[1] = source[1];
  }
}

template <typename Base>
KOKKOS_FUNCTION void
KokkosExtremeValueBase<Base>::init(typename Base::DefaultLoop, Real * result) const
{
  if (_type == ExtremeType::MAX || _type == ExtremeType::MAX_ABS)
  {
    result[0] = -std::numeric_limits<Real>::max();
    result[1] = -std::numeric_limits<Real>::max();
  }
  else if (_type == ExtremeType::MIN)
  {
    result[0] = std::numeric_limits<Real>::max();
    result[1] = std::numeric_limits<Real>::max();
  }
}
