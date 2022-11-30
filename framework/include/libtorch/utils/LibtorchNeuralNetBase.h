//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include <torch/torch.h>
#include "MooseError.h"

namespace Moose
{

/**
 * This base class is meant to gather the functions and members common in
 * every neural network based on Libtorch.
 */
class LibtorchNeuralNetBase
{
public:
  // Virtual destructor
  virtual ~LibtorchNeuralNetBase() {}

  // Evaluation function of the libtorch modules. Since there are considerable
  // differences between self-built modules and modules read using a torch-script
  // format, this serves as a common denominator between the two.
  virtual torch::Tensor forward(torch::Tensor & x) = 0;
};

}

#endif
