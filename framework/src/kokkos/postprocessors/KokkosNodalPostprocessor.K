//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalPostprocessor.h"

namespace Moose::Kokkos
{

InputParameters
NodalPostprocessor::validParams()
{
  InputParameters params = NodalReducer::validParams();
  params += Postprocessor::validParams();
  return params;
}

NodalPostprocessor::NodalPostprocessor(const InputParameters & parameters)
  : NodalReducer(parameters), Postprocessor(this)
{
}

} // namespace Moose::Kokkos
