//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NeuralNetBase.h"

#ifdef TORCH_ENABLED
#include <torch/torch.h>
#include "MooseError.h"

namespace StochasticTools
{

// This base class is meant to gather the functions and members common in
// every libtorch-based class. Note, it already inherits from the torch::nn::Module
// so this will manage the layers (submodules) of the derived classes as well.
class LibtorchNeuralNetBase : public NeuralNetBase, public torch::nn::Module
{
public:
  // Generic constructor
  LibtorchNeuralNetBase() {}

  // Virtual destructor
  virtual ~LibtorchNeuralNetBase() {}

  // Overriding the function from NeuralNetBase
  virtual void addLayer(std::string layer_name,
                        std::unordered_map<std::string, unsigned int> parameters) override
  {
    ::mooseError("You are calling the addLayer function of an unfunctional base class!");
  }
};

}

#endif
