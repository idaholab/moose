//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED

// MOOSE includes
#include "GeneralUserObject.h"
#include "NEML2Assembly.h"
#include "NEML2FEInterpolation.h"

/**
 * @brief NEML2Kernel is a conceptual extension of MOOSE kernel that operates on NEML2 tensors
 *
 * While this is a general user object, it is conceptually a kernel in the sense that it defines the
 * same set of operations on a batch of input. Typical usage of this object includes
 *
 * 1. Calculating a batch of quantities given variable values/gradients interpolated onto the finite
 *    element space. In this case, the NEML2FEInterpolation should be used to get the interpolated
 *    values. Note that this case could be roughly interpreted as being equivalent to a MOOSE
 *    Material.
 * 2. Calculating a batch of quantities given some NEML2 tensors. If the output represents the
 *    elemental residuals/Jacobians, this could be roughly interpreted as being equivalent to a
 *    MOOSE Kernel.
 */
class NEML2Kernel : public GeneralUserObject
{
public:
  static InputParameters validParams();

  NEML2Kernel(const InputParameters & parameters);

  void initialize() override {}
  void execute() override;
  void finalize() override {}

protected:
  /// The forward operator of this kernel
  virtual void forward() = 0;

  /// The assembly object with cached assembly information
  NEML2Assembly & _neml2_assembly;

  /// The FEM interface for getting variable values/gradients interpolated onto the finite element space
  NEML2FEInterpolation & _fe;

  /// The output of the forward operator
  neml2::Tensor _output;
};

#endif
