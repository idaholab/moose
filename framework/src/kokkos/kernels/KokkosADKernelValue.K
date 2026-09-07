//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADKernelValue.h"

namespace Moose::Kokkos
{

InputParameters
ADKernelValue::validParams()
{
  InputParameters params = KernelValue::validParams();
  return params;
}

ADKernelValue::ADKernelValue(const InputParameters & parameters) : KernelValue(parameters) {}

} // namespace Moose::Kokkos
