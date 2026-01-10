//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"

#include "KokkosElementIntegralVariablePostprocessor.h"
#include "KokkosSideIntegralVariablePostprocessor.h"

template <typename Base>
class KokkosAverageValue : public Base
{
public:
  static InputParameters validParams();

  KokkosAverageValue(const InputParameters & parameters);
};

typedef KokkosAverageValue<KokkosElementIntegralVariablePostprocessor> KokkosElementAverageValue;
typedef KokkosAverageValue<KokkosSideIntegralVariablePostprocessor> KokkosSideAverageValue;
