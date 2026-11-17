//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// NEML2_ENABLED implies MOOSE_LIBTORCH_ENABLED (neml2 is built against libtorch), so the
// TorchKernel base is available here.
#ifdef NEML2_ENABLED

// MOOSE includes
#include "TorchKernel.h"
#include "NEML2ModelExecutor.h"

/// Base class for Torch FEM kernels that operate _after_ the constitutive update, reading a NEML2
/// model's batched output.
class TorchPostKernel : public TorchKernel
{
public:
  static InputParameters validParams();

  TorchPostKernel(const InputParameters & parameters);

protected:
  /// the constitutive model
  const NEML2ModelExecutor & _constitutive;
};

#endif // NEML2_ENABLED
