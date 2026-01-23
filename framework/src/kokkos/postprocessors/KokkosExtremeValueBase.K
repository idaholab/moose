//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosExtremeValueBase.h"

#include "KokkosElementPostprocessor.h"
#include "KokkosElementVariablePostprocessor.h"
#include "KokkosNodalVariablePostprocessor.h"
#include "KokkosSideVariablePostprocessor.h"

template <typename Base>
InputParameters
KokkosExtremeValueBase<Base>::validParams()
{
  InputParameters params = Base::validParams();
  params.addParam<MooseEnum>(
      "value_type",
      MooseEnum("max=0 min=1 max_abs=2", "max"),
      "Type of extreme value to return. 'max' "
      "returns the maximum value. 'min' returns "
      "the minimum value. 'max_abs' returns the maximum of the absolute value.");
  return params;
}

template <typename Base>
KokkosExtremeValueBase<Base>::KokkosExtremeValueBase(const InputParameters & parameters)
  : Base(parameters), _type(parameters.get<MooseEnum>("value_type").getEnum<ExtremeType>())
{
}

template <typename Base>
void
KokkosExtremeValueBase<Base>::initialize()
{
  this->allocateReductionBuffer(2);
}

template <typename Base>
void
KokkosExtremeValueBase<Base>::finalize()
{
  if (_type == ExtremeType::MAX || _type == ExtremeType::MAX_ABS)
    this->gatherProxyValueMax(this->_reduction_buffer(0), this->_reduction_buffer(1));
  else if (_type == ExtremeType::MIN)
    this->gatherProxyValueMin(this->_reduction_buffer(0), this->_reduction_buffer(1));
}

template <typename Base>
Real
KokkosExtremeValueBase<Base>::getValue() const
{
  return this->_reduction_buffer(1);
}

template class KokkosExtremeValueBase<Moose::Kokkos::ElementPostprocessor>;
template class KokkosExtremeValueBase<KokkosElementVariablePostprocessor>;
template class KokkosExtremeValueBase<KokkosNodalVariablePostprocessor>;
template class KokkosExtremeValueBase<KokkosSideVariablePostprocessor>;
