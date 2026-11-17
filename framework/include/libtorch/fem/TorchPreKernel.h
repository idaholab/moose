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
#include "MOOSEToNEML2.h"

/// Base class for Torch FEM kernels that operate _before_ the constitutive update, feeding their
/// gathered output into a NEML2 model as an input.
class TorchPreKernel : public TorchKernel, public MOOSEToNEML2
{
public:
  static InputParameters validParams();

  TorchPreKernel(const InputParameters & parameters);

  at::Tensor gatheredData() const override { return _output; }
};

#endif // NEML2_ENABLED
