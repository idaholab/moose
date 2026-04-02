//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSideReporter.h"

namespace Moose::Kokkos
{

InputParameters
SideReporter::validParams()
{
  InputParameters params = SideUserObject::validParams();
  params += Reporter::validParams();
  return params;
}

SideReporter::SideReporter(const InputParameters & parameters)
  : SideUserObject(parameters), Reporter(this)
{
}

} // namespace Moose::Kokkos
