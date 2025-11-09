//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosAverageValue.h"

template <typename Base>
InputParameters
KokkosAverageValue<Base>::validParams()
{
  InputParameters params = Base::validParams();
  params.set<bool>("average") = true;
  params.suppressParameter<bool>("average");
  return params;
}

template <typename Base>
KokkosAverageValue<Base>::KokkosAverageValue(const InputParameters & parameters) : Base(parameters)
{
}

template class KokkosAverageValue<KokkosElementIntegralVariablePostprocessor>;
template class KokkosAverageValue<KokkosSideIntegralVariablePostprocessor>;
