//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosIntegratedBCValue.h"

namespace Moose::Kokkos
{

InputParameters
IntegratedBCValue::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  return params;
}

IntegratedBCValue::IntegratedBCValue(const InputParameters & parameters) : IntegratedBC(parameters)
{
}

} // namespace Moose::Kokkos
