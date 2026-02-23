//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorPostprocessor.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
VectorPostprocessor::validParams()
{
  InputParameters params = ::VectorPostprocessor::validParams();

  return params;
}

VectorPostprocessor::VectorPostprocessor(const MooseObject * moose_object)
  : ::VectorPostprocessor(moose_object)
{
}

VectorPostprocessor::VectorPostprocessor(const VectorPostprocessor & object)
  : ::VectorPostprocessor(object, {})
{
}

} // namespace Kokkos
} // namespace Moose
