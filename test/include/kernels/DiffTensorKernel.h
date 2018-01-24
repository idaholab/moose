//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DIFFTENSORKERNEL_H
#define DIFFTENSORKERNEL_H

#include "Kernel.h"
#include "MooseParsedVectorFunction.h"
#include "MaterialProperty.h"

// Forward Declaration
class DiffTensorKernel;

template <>
InputParameters validParams<DiffTensorKernel>();

/**
 * A Kernel for Testing ParsedVectorFunction
 */
class DiffTensorKernel : public Kernel
{
public:
  /** Class constructor */
  DiffTensorKernel(const InputParameters & parameters);

  /** Class destructor */
  virtual ~DiffTensorKernel() {}

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  /// A vector function containing the components of k for the tensor
  Function & _k_comp;

private:
  /** Compute the k Tensor from the vector function input */
  RealTensorValue computeConductivity(Real t, const Point & pt);
};

#endif // DIFFTENSORKERNEL_H
