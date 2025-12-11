//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosIntegralPostprocessor.h"

template <typename Base>
InputParameters
KokkosIntegralPostprocessor<Base>::validParams()
{
  InputParameters params = Base::validParams();
  params.addParam<bool>("average", false, "Whether to compute volume average");
  return params;
}

template <typename Base>
KokkosIntegralPostprocessor<Base>::KokkosIntegralPostprocessor(const InputParameters & parameters)
  : Base(parameters), _average(Base::template getParam<bool>("average"))
{
}

template <typename Base>
void
KokkosIntegralPostprocessor<Base>::initialize()
{
  Base::allocateReductionBuffer(_average ? 2 : 1);
}

template <typename Base>
Real
KokkosIntegralPostprocessor<Base>::getValue() const
{
  return _average ? Base::_reduction_buffer[0] / Base::_reduction_buffer[1]
                  : Base::_reduction_buffer[0];
}

template <typename Base>
void
KokkosIntegralPostprocessor<Base>::finalize()
{
  Base::gatherSum(Base::_reduction_buffer[0]);

  if (_average)
    Base::gatherSum(Base::_reduction_buffer[1]);
}

template class KokkosIntegralPostprocessor<Moose::Kokkos::ElementPostprocessor>;
template class KokkosIntegralPostprocessor<Moose::Kokkos::SidePostprocessor>;
