//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"

namespace Moose::Kokkos
{

/**
 * Interface class for controlling local DOF parallelization of kernels and integrated BCs
 */
class LocalParallelInterface
{
public:
  static InputParameters validParams();
  LocalParallelInterface(const MooseObject * moose_object);

protected:
  /**
   * Number of threads for local DOF parallelization
   */
  ///@{
  const unsigned int _num_local_residual_threads;
  const unsigned int _num_local_jacobian_threads;
  ///@}
};

} // namespace Moose::Kokkos
