//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Postprocessor.h"

namespace Moose
{
namespace Kokkos
{

class Postprocessor : public ::Postprocessor
{
public:
  static InputParameters validParams();

  Postprocessor(const MooseObject * moose_object);

  /**
   * Copy constructor for parallel dispatch
   */
  Postprocessor(const Postprocessor & object);
};

} // namespace Kokkos
} // namespace Moose
