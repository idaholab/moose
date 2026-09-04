//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_LIBTORCH_ENABLED

#include <torch/torch.h>

// MOOSE includes
#include "GeneralUserObject.h"
#include "TorchAssembly.h"
#include "TorchFEInterpolation.h"

/**
 * @brief TorchKernel is a conceptual extension of a MOOSE kernel that operates on batched libtorch
 * tensors.
 *
 * While this is a general user object, it is conceptually a kernel in the sense that it defines the
 * same set of operations on a batch of input. Typical usage of this object includes
 *
 * 1. Calculating a batch of quantities given variable values/gradients interpolated onto the finite
 *    element space. In this case, the TorchFEInterpolation should be used to get the interpolated
 *    values. Note that this case could be roughly interpreted as being equivalent to a MOOSE
 *    Material.
 * 2. Calculating a batch of quantities given some libtorch tensors. If the output represents the
 *    elemental residuals/Jacobians, this could be roughly interpreted as being equivalent to a
 *    MOOSE Kernel.
 */
class TorchKernel : public GeneralUserObject
{
public:
  static InputParameters validParams();

  TorchKernel(const InputParameters & parameters);

  void initialize() override {}
  void execute() override;
  void finalize() override {}

protected:
  /// The forward operator of this kernel
  virtual void forward() = 0;

  /// The assembly object with cached assembly information
  TorchAssembly & _assembly;

  /// The FEM interface for getting variable values/gradients interpolated onto the finite element
  /// space
  TorchFEInterpolation & _fe;

  /// The output of the forward operator
  at::Tensor _output;
};

#endif // MOOSE_LIBTORCH_ENABLED
