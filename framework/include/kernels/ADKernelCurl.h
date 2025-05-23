//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVectorKernel.h"

/**
 *  A base class that defines the AD curl operators.
 */
class ADKernelCurl : public ADVectorKernel
{
public:
  static InputParameters validParams();

  ADKernelCurl(const InputParameters & parameters);

protected:
  /// curl of the test function
  const VectorVariableTestCurl & _curl_test;

  /**
   *  curl of the shape function
   *  Note: This still needs to be defined for the uses of GenericKernelCurl
   */
  const VectorVariablePhiCurl & _curl_phi;

  /// Holds the solution curl at the current quadrature points
  const ADVectorVariableCurl & _curl_u;
};
