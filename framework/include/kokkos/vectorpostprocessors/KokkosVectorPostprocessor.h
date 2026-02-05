//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorPostprocessor.h"

namespace Moose
{
namespace Kokkos
{

class VectorPostprocessor : public ::VectorPostprocessor
{
public:
  static InputParameters validParams();

  VectorPostprocessor(const MooseObject * moose_object);

  /**
   * Copy constructor for parallel dispatch
   */
  VectorPostprocessor(const VectorPostprocessor & object);
};

} // namespace Kokkos
} // namespace Moose
