//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSideVectorPostprocessor.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
SideVectorPostprocessor::validParams()
{
  InputParameters params = SideReducer::validParams();
  params += VectorPostprocessor::validParams();
  return params;
}

SideVectorPostprocessor::SideVectorPostprocessor(const InputParameters & parameters)
  : SideReducer(parameters), VectorPostprocessor(this)
{
}

} // namespace Kokkos
} // namespace Moose
