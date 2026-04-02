//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosReporter.h"
#include "KokkosSideUserObject.h"

namespace Moose::Kokkos
{

class SideReporter : public SideUserObject, public Reporter
{
public:
  static InputParameters validParams();

  SideReporter(const InputParameters & parameters);
};

} // namespace Moose::Kokkos
