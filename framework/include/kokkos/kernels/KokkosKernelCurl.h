//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorKernel.h"

namespace Moose::Kokkos
{

/**
 * A base class that defines the curl operators for Kokkos vector kernels.
 */
class KernelCurl : public VectorKernel
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  KernelCurl(const InputParameters & parameters);

protected:
  /**
   * Curl of the current vector test function
   */
  const VectorVariableTestCurl _curl_test;
  /**
   * Curl of the current vector shape function
   */
  const VectorVariablePhiCurl _curl_phi;
  /**
   * Curl of the current vector solution at quadrature points
   */
  const VectorVariableCurl _curl_u;
};

} // namespace Moose::Kokkos
