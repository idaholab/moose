//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalVectorPostprocessor.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
NodalVectorPostprocessor::validParams()
{
  InputParameters params = NodalReducer::validParams();
  params += VectorPostprocessor::validParams();
  return params;
}

NodalVectorPostprocessor::NodalVectorPostprocessor(const InputParameters & parameters)
  : NodalReducer(parameters), VectorPostprocessor(this)
{
}

} // namespace Kokkos
} // namespace Moose
